#ifndef DE_CORE_STARTUP_ERROR_HPP
#define DE_CORE_STARTUP_ERROR_HPP

#include <string>

namespace de
{
/// Placed in the registry context by a setup callback that could not do its
/// job. GameLoop calls no further setup callbacks, reports it, and skips the
/// frame loop instead of running on with a null window.
///
/// Setup runs inside run(), not mount(), so failures cannot be returned from
/// the plugin that caused them.
struct StartupError
{
    std::string message;
};

} // namespace de

#endif // DE_CORE_STARTUP_ERROR_HPP
