#pragma once
#include "vks/Engine.hpp"

namespace vks {

    class EngineContext {
    public:
        static void set(Engine* engine) {
            s_engine = engine;
        }

        static Engine& get() {
            if (!s_engine)
                throw std::runtime_error("EngineContext not initialized");
            return *s_engine;
        }

    private:
        static inline thread_local Engine* s_engine = nullptr;
    };

}