#pragma once

// ============================================================
//  DebugRegistry.hpp
//
//  A lightweight singleton that lets you register any variable
//  from anywhere in the project. The DebugPanel reads from it
//  and renders an editable ImGui window.
//
//  Usage (anywhere you have a variable):
//
//    // Register once (e.g. constructor or init)
//    DebugRegistry::get().add("Physics/Gravity",   m_gravity);
//    DebugRegistry::get().add("Player/Speed",       m_speed);
//    DebugRegistry::get().add("Renderer/FOV",       m_fov);
//    DebugRegistry::get().add("Debug/ShowColliders",m_showColliders);
//    DebugRegistry::get().add("Player/Position",    m_position);  // glm::vec3
//
//  Supports: float, int, bool, glm::vec2, glm::vec3, glm::vec4
//
//  The name is used as the ImGui label. Use "/" to create
//  collapsible category groups (e.g. "Physics/Gravity").
// ============================================================

#include <string>
#include <vector>
#include <functional>
#include <variant>

#include <glm/glm.hpp>

namespace vks
{
    class DebugRegistry
    {
    public:
        // ---- Supported variable types ----------------------
        using VarValue = std::variant<
            float*,
            int*,
            bool*,
            glm::vec2*,
            glm::vec3*,
            glm::vec4*
        >;

        struct DebugVar
        {
            std::string category; // e.g. "Physics"
            std::string label;    // e.g. "Gravity"
            VarValue    ptr;      // non-owning pointer to the actual variable
        };

        // ---- Singleton access ------------------------------
        static DebugRegistry& get()
        {
            static DebugRegistry instance;
            return instance;
        }

        // ---- Registration ----------------------------------

        /**
         * @brief Register a variable by name.
         * @param path  Slash-separated path, e.g. "Physics/Gravity".
         *              A plain name with no slash goes into "General".
         * @param var   Reference to the variable to watch/edit.
         *
         * The registry stores a raw pointer — make sure the variable
         * outlives the registry (i.e. don't register stack locals).
         */
        template<typename T>
        void add(const std::string& path, T& var)
        {
            auto [category, label] = splitPath(path);
            m_vars.push_back({ category, label, &var });
        }

        /**
         * @brief Remove all variables registered under a given category.
         *        Call this in the destructor of the owning class.
         */
        void removeCategory(const std::string& category)
        {
            m_vars.erase(
                std::remove_if(m_vars.begin(), m_vars.end(),
                    [&](const DebugVar& v) { return v.category == category; }),
                m_vars.end());
        }

        /**
         * @brief Remove a single variable by full path.
         */
        void remove(const std::string& path)
        {
            auto [category, label] = splitPath(path);
            m_vars.erase(
                std::remove_if(m_vars.begin(), m_vars.end(),
                    [&](const DebugVar& v)
                    { return v.category == category && v.label == label; }),
                m_vars.end());
        }

        // ---- Access (used by DebugPanel) -------------------
        const std::vector<DebugVar>& vars() const { return m_vars; }

    private:
        DebugRegistry() = default;

        std::vector<DebugVar> m_vars;

        // Split "Category/Label" → {"Category", "Label"}
        // "Label" (no slash) → {"General", "Label"}
        static std::pair<std::string, std::string> splitPath(const std::string& path)
        {
            const auto slash = path.find('/');
            if (slash == std::string::npos)
                return { "General", path };
            return { path.substr(0, slash), path.substr(slash + 1) };
        }
    };

} // namespace vks