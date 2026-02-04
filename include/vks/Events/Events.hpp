#pragma once
#include <string>

namespace vks {
    struct PipelineReloadEvent {
        std::string pipelineName; // Optional: reload specific one or "all"
    };
}
