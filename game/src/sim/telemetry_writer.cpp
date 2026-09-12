#include <game/sim/telemetry_writer.hpp>
#include <iomanip>
#include <ostream>
#include <sstream>

namespace
{
/// Positions and distances to 3 decimals, times to 4. Fixed precision so two
/// runs of the same build produce byte-identical files -- the acceptance test
/// for the whole simulation mode is `diff` over two runs.
std::string fixed(float value, int decimals)
{
    std::ostringstream out;
    out << std::fixed << std::setprecision(decimals) << value;
    return out.str();
}

} // namespace

TelemetryWriter::TelemetryWriter(std::ostream& out) : m_out(out) {}

std::string TelemetryWriter::header()
{
    return "step,time,player_x,player_y,vel_x,vel_y,blocked,distance,health,"
           "invulnerable,damage_taken,items_collected,enemies_chasing,"
           "nearest_enemy_dist,target_x,target_y,act_x,act_y,game_over";
}

void TelemetryWriter::writeHeader() { m_out << header() << "\n"; }

void TelemetryWriter::write(const TelemetryRow& row)
{
    m_out << row.step << ',' << fixed(row.time, 4) << ','
          << fixed(row.playerX, 3) << ',' << fixed(row.playerY, 3) << ','
          << fixed(row.velocityX, 3) << ',' << fixed(row.velocityY, 3) << ','
          << (row.blocked ? 1 : 0) << ',' << fixed(row.distance, 3) << ','
          << row.health << ',' << fixed(row.invulnerable, 4) << ','
          << row.damageTaken << ',' << row.itemsCollected << ','
          << row.enemiesChasing << ',' << fixed(row.nearestEnemyDistance, 3)
          << ',' << fixed(row.targetX, 3) << ',' << fixed(row.targetY, 3) << ','
          << fixed(row.actionX, 1) << ',' << fixed(row.actionY, 1) << ','
          << (row.gameOver ? 1 : 0) << "\n";
}

std::string TelemetryWriter::summary(const SimSummary& summary)
{
    std::ostringstream out;
    out << "sim: outcome=" << summary.outcome << "\n";
    out << "sim: steps=" << summary.steps
        << " sim_seconds=" << fixed(summary.steps * summary.fixedDelta, 4)
        << " fixed_dt=" << fixed(summary.fixedDelta, 7) << "\n";

    out << "sim: items=" << summary.items << '/' << summary.itemsTotal
        << " unreachable=" << summary.unreachable << " pickups_at=";
    for (std::size_t i = 0; i < summary.pickupTimes.size(); ++i)
    {
        out << (i == 0 ? "" : ",") << fixed(summary.pickupTimes[i], 4);
    }
    if (summary.pickupTimes.empty())
    {
        out << "none";
    }
    out << "\n";

    out << "sim: damage_taken=" << summary.damageTaken
        << " hits=" << summary.hits << " final_health=" << summary.finalHealth
        << " first_hit_at="
        << (summary.firstHitAt < 0.0f ? "never" : fixed(summary.firstHitAt, 4))
        << "\n";

    const float seconds = summary.steps * summary.fixedDelta;
    out << "sim: distance_px=" << fixed(summary.distance, 3) << " mean_speed="
        << fixed(seconds > 0.0f ? summary.distance / seconds : 0.0f, 3)
        << " blocked_ratio="
        << fixed(summary.steps > 0 ? static_cast<float>(summary.blockedSteps) /
                                         static_cast<float>(summary.steps)
                                   : 0.0f,
                 4)
        << "\n";

    out << "sim: chase_ratio="
        << fixed(summary.steps > 0 ? static_cast<float>(summary.chasedSteps) /
                                         static_cast<float>(summary.steps)
                                   : 0.0f,
                 4)
        << " min_enemy_dist=" << fixed(summary.minEnemyDistance, 3) << "\n";

    out << "sim: seed=" << summary.seed << " policy=seek_items level=level1"
        << " csv=" << summary.csvPath << "\n";
    return out.str();
}
