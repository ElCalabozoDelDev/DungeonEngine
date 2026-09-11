#ifndef DE_CORE_HOOK_HPP
#define DE_CORE_HOOK_HPP
#include <functional>

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
            m_callbacks.insert(m_callbacks.begin(), callback);
        }
        else
        {
            m_callbacks.push_back(callback);
        }
    }

    void publish(Args... args)
    {
        for (auto& callback : m_callbacks)
        {
            callback(args...);
        }
    }
};

} // namespace de

#endif // DE_CORE_HOOK_HPP
