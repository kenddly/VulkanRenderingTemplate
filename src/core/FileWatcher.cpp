#include "../../include/core/FileWatcher.hpp"

FileWatcher::FileWatcher(std::chrono::milliseconds pollInterval)
    : m_pollInterval(pollInterval),
      m_lastPoll(std::chrono::steady_clock::now())
{
}

void FileWatcher::watchFile(const std::filesystem::path& path, Callback callback)
{
    if (!std::filesystem::exists(path))
        return;

    registerFile(path, callback);
}

void FileWatcher::watchDirectory(const std::filesystem::path& directory,
                                 const std::vector<std::string>& extensions,
                                 bool recursive,
                                 Callback callback)
{
    if (!std::filesystem::exists(directory))
        return;

    auto shouldAccept = [&](const std::filesystem::path& path)
    {
        if (extensions.empty())
            return true;

        const auto ext = path.extension().string();
        return std::find(extensions.begin(), extensions.end(), ext) != extensions.end();
    };

    if (recursive)
    {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(directory))
        {
            if (!entry.is_regular_file())
                continue;

            const auto& path = entry.path();
            if (!shouldAccept(path))
                continue;

            registerFile(path, callback);
        }
    }
    else
    {
        for (const auto& entry : std::filesystem::directory_iterator(directory))
        {
            if (!entry.is_regular_file())
                continue;

            const auto& path = entry.path();
            if (!shouldAccept(path))
                continue;

            registerFile(path, callback);
        }
    }
}
void FileWatcher::registerFile(const std::filesystem::path& path, Callback callback)
{
    WatchedFile watched{};
    watched.lastWriteTime = std::filesystem::last_write_time(path);
    watched.callback = callback;

    m_files[path] = watched;
}

void FileWatcher::update()
{
    const auto now = std::chrono::steady_clock::now();
    if (now - m_lastPoll < m_pollInterval)
        return;

    m_lastPoll = now;

    for (auto& [path, watched] : m_files)
    {
        if (!std::filesystem::exists(path))
            continue;

        auto currentWriteTime = std::filesystem::last_write_time(path);

        if (currentWriteTime != watched.lastWriteTime)
        {
            watched.lastWriteTime = currentWriteTime;

            if (watched.callback)
                watched.callback(path);
        }
    }
}

void FileWatcher::clear()
{
    m_files.clear();
}
