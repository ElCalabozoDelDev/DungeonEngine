#ifndef DE_CORE_HOOK_HPP
#define DE_CORE_HOOK_HPP
#include <functional>
#include <utility>
#include <vector>

namespace de
{
template <typename Type>
class Hook;

template <typename Ret, typename... Args>
class Hook<Ret(Args...)>
{
private:
    using CallbackType = std::function<Ret(Args...)>;
    std::vector<CallbackType> m_callbacks;

public:
    void connect(CallbackType callback, bool front = false)
    {
        if (front)
        {
            m_callbacks.insert(m_callbacks.begin(), std::move(callback));
        }
        else
        {
            m_callbacks.push_back(std::move(callback));
        }
    }

    void publish(Args... args)
    {
        for (auto& callback : m_callbacks)
        {
            callback(args...);
        }
    }

    /// Like publish(), but checks `stop` before each callback and returns as
    /// soon as it is true.
    template <typename Stop>
    void publishUntil(Stop stop, Args... args)
    {
        for (auto& callback : m_callbacks)
        {
            if (stop())
            {
                return;
            }
            callback(args...);
        }
    }
};

} // namespace de

#endif // DE_CORE_HOOK_HPP
