#ifndef HOOK_HPP
#define HOOK_HPP

#include <functional>

template <typename Type>
class Hook;

template <typename Ret, typename... Args>
class Hook<Ret(Args...)> {
private:
    using CallbackType = std::function<Ret(Args...)>;
    std::vector<CallbackType> m_callbacks;

public:
    void connect(CallbackType callback, bool front = false) {
        if (front) {
            m_callbacks.insert(m_callbacks.begin(), callback);
        } else {
            m_callbacks.push_back(callback);
        }
    }

    void publish(Args... args) {
        for (auto& callback : m_callbacks) {
            callback(args...);
        }
    }
};

#endif