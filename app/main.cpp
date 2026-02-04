#include <iostream>

#include <vks/Engine.hpp>
#include <SandboxApp.hpp>
#include <Log.hpp>

int main()
{
    try
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        // Logging
        vks::Log::Init();

        // Engine config
        vks::EngineConfig config;
        config.width = 1280;
        config.height = 720;
        config.appName = "Vulkan Sandbox";
        config.engineName = "VKS Engine";
        config.enableValidation = true;

        // Create engine + app
        vks::Engine engine(config);
        vks::SandboxApp app;

        // Run
        engine.run(app);

        vks::Log::Shutdown();
        glfwTerminate();
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }


    return EXIT_SUCCESS;
}
