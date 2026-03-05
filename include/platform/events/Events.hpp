#pragma once
#include <cstdint>
#include <string>

namespace vks {
    struct PipelineReloadEvent {
        std::string pipelineName; // Optional: reload specific one or "all"
    };

    struct WindowResizeEvent {
        int newWidth;
        int newHeight;
    };

    struct ViewportResizeEvent {
        int newWidth;
        int newHeight;
    };
}
