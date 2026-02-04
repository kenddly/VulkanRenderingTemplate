#pragma once
#include <functional>
#include <vector>
#include <map>
#include <typeindex>

namespace vks {
    class EventManager {
    public:
        // A handle to allow unsubscribing later if needed
        using CallbackID = size_t;

        template<typename T>
        using EventCallback = std::function<void(const T&)>;

        // Subscribe to a specific event type
        template<typename T>
        static void subscribe(EventCallback<T> callback) {
            m_subscribers[typeid(T)].push_back([callback](const void* eventData) {
                callback(*static_cast<const T*>(eventData));
            });
        }

        // Fire an event to all subscribers
        template<typename T>
        static void emit(const T& event) {
            auto it = m_subscribers.find(typeid(T));
            if (it != m_subscribers.end()) {
                for (auto& callback : it->second) {
                    callback(&event);
                }
            }
        }

    private:
        using InternalCallback = std::function<void(const void*)>;
        inline static std::map<std::type_index, std::vector<InternalCallback>> m_subscribers{};
    };
}