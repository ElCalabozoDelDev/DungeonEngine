#ifndef GAME_SIM_TELEMETRY_WRITER_HPP
#define GAME_SIM_TELEMETRY_WRITER_HPP

#include <game/sim/telemetry_row.hpp>
#include <iosfwd>
#include <string>

/// Formats telemetry as strict CSV: a header row, then data, nothing else.
///
/// No `# key=value` metadata line, deliberately. It would make each file
/// self-describing at the cost of every consumer needing to be told to skip
/// comments; the run's metadata goes in the summary instead, and every
/// aggregate there can be recomputed from these rows anyway.
///
/// Takes a stream rather than a path so the tests can format into a
/// std::ostringstream with no temporary files.
class TelemetryWriter
{
public:
    explicit TelemetryWriter(std::ostream& out);

    void writeHeader();
    void write(const TelemetryRow& row);

    /// The header, for tests that check it against a formatted row.
    static std::string header();

    /// The human-readable end-of-run block. Free function in spirit: it
    /// touches no writer state.
    static std::string summary(const SimSummary& summary);

private:
    std::ostream& m_out;
};

#endif // GAME_SIM_TELEMETRY_WRITER_HPP
