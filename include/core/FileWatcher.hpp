#pragma once

#include <filesystem>
#include <unordered_map>
#include <vector>
#include <functional>
#include <chrono>

// Simple cross-platform file watcher using polling
class FileWatcher
{
public:
    using Callback = std::function<void(const std::filesystem::path&)>;

    explicit FileWatcher(std::chrono::milliseconds pollInterval = std::chrono::milliseconds(250));

    // Watch a single file
    void watchFile(const std::filesystem::path& path, Callback callback);

    // Watch all files in a directory (optionally recursive)
    void watchDirectory(const std::filesystem::path& directory,
                        const std::vector<std::string>& extensions = {},
                        bool recursive = true,
                        Callback callback = nullptr);

    // Must be called periodically
    void update();

    // Remove everything
    void clear();

private:
    struct WatchedFile
    {
        std::filesystem::file_time_type lastWriteTime;
        Callback callback;
    };

    void registerFile(const std::filesystem::path& path, Callback callback);

    std::unordered_map<std::filesystem::path, WatchedFile> m_files;

    std::chrono::milliseconds m_pollInterval;
    std::chrono::steady_clock::time_point m_lastPoll;
};
