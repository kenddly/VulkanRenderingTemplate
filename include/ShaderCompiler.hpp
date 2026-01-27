#pragma once
#include <filesystem>
#include <string>

#pragma once

#include <queue>
#include <unordered_set>
#include <mutex>
#include <functional>
#include <chrono>

class ShaderCompiler
{
public:
    using CompileCallback = std::function<void(const std::filesystem::path& spvPath)>;

    explicit ShaderCompiler(
        CompileCallback onSuccess,
        std::chrono::milliseconds debounce = std::chrono::milliseconds(200)
    );

    // Called by FileWatcher
    void requestCompile(const std::filesystem::path& shaderPath);

    // Call once per frame
    void update();

private:
    static bool compile(const std::filesystem::path& shader);

    std::queue<std::filesystem::path> m_queue;
    std::unordered_set<std::filesystem::path> m_pending;
    std::mutex m_mutex;

    CompileCallback m_onSuccess;

    std::chrono::milliseconds m_debounce;
    std::chrono::steady_clock::time_point m_lastCompile;
};
