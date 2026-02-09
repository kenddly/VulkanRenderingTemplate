#include "../../include/core/Log.hpp"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <filesystem>
#include <iostream>
#include <sstream>

#define MAX_LOGFILE_SIZE 1000000

namespace vks
{
    void Log::Init()
    {
        if (std::filesystem::exists("vks.log"))
            if (std::filesystem::file_size("vks.log") > MAX_LOGFILE_SIZE)
                std::filesystem::remove("vks.log");

        LOG_INFO("[Core] Initializing Log");
        spdlog::set_pattern("%^[%D] [%T] [%L]: %v%$");
        s_CoreLogger = spdlog::basic_logger_mt("VKS", "vks.log");
        s_CoreLogger->set_level(spdlog::level::trace);
        s_CoreLogger->flush_on(spdlog::level::trace);

        s_LoggingThread = std::thread(Log::LoggingThreadFunction);
    }

    void Log::Shutdown()
    {
        s_LoggingThreadRunning = false;
        s_ConditionVariable.notify_all();
        s_LoggingThread.join();
    }

    void Log::PushLogMessage(spdlog::level::level_enum level, const std::string& message)
    {
        s_LogBuffer.PushLogMessage(level, message);
        std::lock_guard<std::mutex> lock(s_Mutex);
        s_LogQueue.emplace(level, message.data());
        s_ConditionVariable.notify_one();
    }

    void Log::LoggingThreadFunction()
    {
        while (s_LoggingThreadRunning)
        {
            std::unique_lock<std::mutex> lock(s_Mutex);
            s_ConditionVariable.wait(lock);
            while (!s_LogQueue.empty())
            {
                auto [level, message] = s_LogQueue.front();
                s_LogQueue.pop();
                switch (level)
                {
                case spdlog::level::trace:
                    s_CoreLogger->trace(message);
                    std::cout << "[TRACE] ";
                    break;
                case spdlog::level::info:
                    s_CoreLogger->info(message);
                    std::cout << "[INFO] ";
                    break;
                case spdlog::level::warn:
                    s_CoreLogger->warn(message);
                    std::cout << "[WARN] ";
                    break;
                case spdlog::level::err:
                    s_CoreLogger->error(message);
                    std::cout << "[ERROR] ";
                    break;
                case spdlog::level::critical:
                    s_CoreLogger->critical(message);
                    std::cout << "[CRITICAL] ";
                    break;
                default:
                    break;
                }
                std::cout << message << std::endl;
            }
        }
    }

    void LogBuffer::PushLogMessage(const LogLevel& level, const std::string& message)
    {
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);

        std::ostringstream oss;
        oss << std::put_time(&tm, "%H:%M:%S");
        auto str = oss.str();

        auto str_level = "";
        switch (level)
        {
        case spdlog::level::trace:
            str_level = "TRACE";
            break;
        case spdlog::level::info:
            str_level = "INFO";
            break;
        case spdlog::level::warn:
            str_level = "WARN";
            break;
        case spdlog::level::err:
            str_level = "ERROR";
            break;
        case spdlog::level::critical:
            str_level = "CRITICAL";
            break;
        default:
            break;
        };

        auto formatted_message = "[" + str + "] " + "[" + str_level + "] " + message;
        m_Buffer.push_back({level, formatted_message});
    }

    void LogBuffer::Clear()
    {
        m_Buffer.clear();
    }
}
