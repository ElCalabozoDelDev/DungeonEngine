#ifndef DE_CORE_PAUSED_HPP
#define DE_CORE_PAUSED_HPP

namespace de
{
/// When true, GameLoop stops stepping fixed systems.
///
/// Pausing is "the simulation does not advance", so it belongs to the loop
/// rather than to each system remembering to check a flag. Frame systems keep
/// running, which is what lets the pause overlay draw.
struct Paused
{
    bool value = false;
};

} // namespace de

#endif // DE_CORE_PAUSED_HPP
