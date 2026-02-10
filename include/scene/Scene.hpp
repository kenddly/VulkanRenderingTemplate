#pragma once

#include <string>
#include <entt/entt.hpp>
#include <scene/Components.hpp>

namespace vks
{
    using Entity = entt::entity;

    class Scene
    {
    public:
        Entity createEntity(const std::string& name = {})
        {
            auto e = m_registry.create();

            if (!name.empty())
                m_registry.emplace<Name>(e, name);

            m_registry.emplace<Transform>(e);
            return e;
        }

        void destroyEntity(entt::entity e)
        {
            m_registry.destroy(e);
        }

        template <typename Component, typename... Args>
        Component& addComponent(entt::entity e, Args&&... args)
        {
            return m_registry.emplace<Component>(e, std::forward<Args>(args)...);
        }

        template <typename Component>
        Component& getComponent(entt::entity e)
        {
            return m_registry.get<Component>(e);
        }

        template <typename... Components>
        auto view()
        {
            return m_registry.view<Components...>();
        }

        template <typename... Components>
        bool hasComponent(entt::entity e)
        {
            return m_registry.all_of<Components...>(e);
        }

        entt::registry& getRegistry() { return m_registry; }
        const entt::registry& getRegistry() const { return m_registry; }

        void clear()
        {
            m_registry.clear();
        }

    private:
        entt::registry m_registry;
    };
}
