#pragma once

#include <unordered_map>
#include <string>
#include <vks/RenderObject.hpp>

namespace vks {

    class Scene {
    public:
        using ObjectMap = std::unordered_map<std::string, RenderObject>;

        RenderObject& create(const std::string& name) {
            return m_objects[name];
        }

        RenderObject* find(const std::string& name) {
            auto it = m_objects.find(name);
            return it != m_objects.end() ? &it->second : nullptr;
        }

        const ObjectMap& objects() const { return m_objects; }
        ObjectMap& objects() { return m_objects; }

        void clear() { m_objects.clear(); }

    private:
        ObjectMap m_objects;
    };

}
