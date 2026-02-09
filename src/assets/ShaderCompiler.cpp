#include "../../include/assets/ShaderCompiler.hpp"
#include "../../include/core/Process.hpp"

#include <iostream>

ShaderCompiler::ShaderCompiler(
    CompileCallback onSuccess,
    std::chrono::milliseconds debounce)
    : m_onSuccess(std::move(onSuccess)),
      m_debounce(debounce),
      m_lastCompile(std::chrono::steady_clock::now())
{
}

void ShaderCompiler::requestCompile(const std::filesystem::path& shaderPath)
{
    std::lock_guard lock(m_mutex);

    // Prevent duplicate spam
    if (m_pending.insert(shaderPath).second)
        m_queue.push(shaderPath);
}

void ShaderCompiler::update()
{
    const auto now = std::chrono::steady_clock::now();
    if (now - m_lastCompile < m_debounce)
        return;

    std::filesystem::path shader;

    {
        std::lock_guard lock(m_mutex);
        if (m_queue.empty())
            return;

        shader = m_queue.front();
        m_queue.pop();
        m_pending.erase(shader);
    }

    m_lastCompile = now;

    if (compile(shader))
    {
        m_onSuccess(shader.string() + ".spv");
    }
}

bool ShaderCompiler::compile(const std::filesystem::path& shader)
{
#if defined(_WIN32)
    const char* compiler = "glslc.exe";
#else
    const char* compiler = "glslc";
#endif

    const auto spv = shader.string() + ".spv";

    std::string cmd =
        std::string(compiler) +
        " -std=450 -g " +
        shader.string() +
        " -o " +
        spv +
        " 2>&1";

    auto result = runCommand(cmd);

    if (result.exitCode != 0)
    {
        std::cerr << "[ShaderCompiler] FAILED: " << shader << "\n"
                  << result.output << std::endl;
        return false;
    }

    std::cout << "[ShaderCompiler] OK: " << shader << std::endl;
    return true;
}
