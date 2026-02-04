#pragma once

#include "vks/Application.hpp"

namespace vks {

    class SandboxApp : public Application {
    public:
        void onInit(Engine& engine) override;
        void onUpdate(float dt) override;
        void onImGui() override;

    private:
        void loadAssets(Engine& engine);
        void buildScene(Engine& engine);
    };
}
