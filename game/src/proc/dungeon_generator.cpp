#include <algorithm>
#include <game/proc/dungeon_generator.hpp>
#include <random>
#include <utility>

namespace
{
struct Room
{
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;

    int centerX() const { return x + w / 2; }
    int centerY() const { return y + h / 2; }
};

void carveRoom(FloorBlueprint& floor, const Room& room)
{
    for (int row = room.y; row < room.y + room.h; ++row)
    {
        for (int col = room.x; col < room.x + room.w; ++col)
        {
            floor.set(col, row, FloorBlueprint::Cell::Floor);
        }
    }
}

void carveHCorridor(FloorBlueprint& floor, int x0, int x1, int y)
{
    const int a = std::min(x0, x1);
    const int b = std::max(x0, x1);
    for (int x = a; x <= b; ++x)
    {
        floor.set(x, y, FloorBlueprint::Cell::Floor);
    }
}

void carveVCorridor(FloorBlueprint& floor, int y0, int y1, int x)
{
    const int a = std::min(y0, y1);
    const int b = std::max(y0, y1);
    for (int y = a; y <= b; ++y)
    {
        floor.set(x, y, FloorBlueprint::Cell::Floor);
    }
}

void wallify(FloorBlueprint& floor)
{
    for (int row = 0; row < floor.rows; ++row)
    {
        for (int col = 0; col < floor.columns; ++col)
        {
            if (floor.at(col, row) != FloorBlueprint::Cell::Floor)
            {
                continue;
            }
            for (int dy = -1; dy <= 1; ++dy)
            {
                for (int dx = -1; dx <= 1; ++dx)
                {
                    if (dx == 0 && dy == 0)
                    {
                        continue;
                    }
                    const int nx = col + dx;
                    const int ny = row + dy;
                    if (!floor.inBounds(nx, ny) ||
                        floor.at(nx, ny) == FloorBlueprint::Cell::Void)
                    {
                        floor.set(nx, ny, FloorBlueprint::Cell::Wall);
                    }
                }
            }
        }
    }
}
} // namespace

FloorBlueprint DungeonGenerator::generate(const RunConfig& config,
                                          int floorIndex) const
{
    FloorBlueprint floor;
    floor.columns = std::max(16, config.mapColumns);
    floor.rows = std::max(12, config.mapRows);
    floor.tileSize = config.tileSize;
    floor.floorIndex = floorIndex;
    floor.cells.assign(static_cast<std::size_t>(floor.columns * floor.rows),
                       FloorBlueprint::Cell::Void);

    // Mix floor into the seed so each depth differs while staying
    // deterministic.
    std::mt19937 rng(config.seed +
                     static_cast<std::uint32_t>(floorIndex) * 9973u);

    std::uniform_int_distribution<int> roomW(5, 9);
    std::uniform_int_distribution<int> roomH(5, 8);
    std::uniform_int_distribution<int> roomX(1, floor.columns - 11);
    std::uniform_int_distribution<int> roomY(1, floor.rows - 10);

    std::vector<Room> rooms;
    const int targetRooms = 8;
    for (int attempt = 0;
         attempt < 80 && static_cast<int>(rooms.size()) < targetRooms;
         ++attempt)
    {
        Room candidate{roomX(rng), roomY(rng), roomW(rng), roomH(rng)};
        if (candidate.x + candidate.w >= floor.columns - 1 ||
            candidate.y + candidate.h >= floor.rows - 1)
        {
            continue;
        }

        bool overlaps = false;
        for (const Room& existing : rooms)
        {
            if (candidate.x <= existing.x + existing.w + 1 &&
                candidate.x + candidate.w + 1 >= existing.x &&
                candidate.y <= existing.y + existing.h + 1 &&
                candidate.y + candidate.h + 1 >= existing.y)
            {
                overlaps = true;
                break;
            }
        }
        if (overlaps)
        {
            continue;
        }

        carveRoom(floor, candidate);
        if (!rooms.empty())
        {
            const Room& prev = rooms.back();
            carveHCorridor(floor, prev.centerX(), candidate.centerX(),
                           prev.centerY());
            carveVCorridor(floor, prev.centerY(), candidate.centerY(),
                           candidate.centerX());
        }
        rooms.push_back(candidate);
    }

    if (rooms.empty())
    {
        Room fallback{2, 2, 8, 6};
        carveRoom(floor, fallback);
        rooms.push_back(fallback);
    }

    wallify(floor);

    const Room& first = rooms.front();
    const Room& last = rooms.back();

    floor.markers.push_back({FloorBlueprint::Marker::Kind::Entrance,
                             first.centerX(), first.centerY()});

    if (floorIndex < config.floorsPerRun)
    {
        floor.markers.push_back({FloorBlueprint::Marker::Kind::StairsDown,
                                 last.centerX(), last.centerY()});
    }
    if (floorIndex > 1)
    {
        // Stairs up near the entrance so return trips are discoverable.
        const int upCol = std::min(first.centerX() + 1, first.x + first.w - 2);
        floor.markers.push_back(
            {FloorBlueprint::Marker::Kind::StairsUp, upCol, first.centerY()});
    }

    const bool placeObjective = floorIndex >= config.objectiveFloor &&
                                floorIndex <= config.floorsPerRun;
    if (placeObjective)
    {
        const int objCol = std::max(last.x + 1, last.centerX() - 1);
        const int objRow = last.centerY();
        floor.markers.push_back(
            {FloorBlueprint::Marker::Kind::Objective, objCol, objRow});
    }

    std::uniform_int_distribution<std::size_t> roomPick(0, rooms.size() - 1);
    const int enemies = 3;
    for (int i = 0; i < enemies; ++i)
    {
        const Room& room = rooms[roomPick(rng)];
        std::uniform_int_distribution<int> ox(room.x + 1, room.x + room.w - 2);
        std::uniform_int_distribution<int> oy(room.y + 1, room.y + room.h - 2);
        floor.markers.push_back(
            {FloorBlueprint::Marker::Kind::Enemy, ox(rng), oy(rng)});
    }

    const int scoreItems = 2;
    for (int i = 0; i < scoreItems; ++i)
    {
        const Room& room = rooms[roomPick(rng)];
        std::uniform_int_distribution<int> ox(room.x + 1, room.x + room.w - 2);
        std::uniform_int_distribution<int> oy(room.y + 1, room.y + room.h - 2);
        floor.markers.push_back(
            {FloorBlueprint::Marker::Kind::ScoreItem, ox(rng), oy(rng)});
    }

    return floor;
}
