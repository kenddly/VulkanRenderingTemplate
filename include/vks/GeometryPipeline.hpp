#pragma once

#include <NonCopyable.hpp>
#include <vulkan/vulkan.h>
#include <vector>
#include <map>
#include <string>
#include <memory>

#include <vks/Descriptors.hpp>

namespace vks {

// Forward-declarations
class Device;
class SwapChain;
class IRenderPass;

/**
 * @brief Manages the creation and storage of all VkPipeline objects.
 * This class acts as a factory and registry for:
 * - VkPipelines (the "recipes")
 * - VkPipelineLayouts
 * - Ref<DescriptorSetLayout> (the shader interface layouts)
 */
class GeometryPipeline : public NonCopyable {
public:
    GeometryPipeline(const Device &device, const SwapChain &swapChain, VkRenderPass renderPass);
    ~GeometryPipeline();

    /**
     * @brief Recreates all pipelines and layouts (e.g., on swapchain resize).
     */
    void recreate();

    /**
     * @brief Gets a compiled pipeline by its registered name.
     * @param name The name given during creation (e.g., "sphere").
     * @return A VkPipeline handle.
     */
    VkPipeline getPipeline(const std::string &name) const;

    /**
     * @brief Gets a pipeline layout by its registered name.
     * @param name The name given during creation (e.g., "sphere").
     * @return A VkPipelineLayout handle.
     */
    VkPipelineLayout getLayout(const std::string &name) const;

    /**
     * @brief Gets a descriptor set layout by its registered name.
     * This is used by the Material class to create matching descriptor sets.
     * @param name The name given during creation (e.g., "global", "material").
     * @return A shared_ptr to the DescriptorSetLayout.
     */
    Ref<DescriptorSetLayout> getDescriptorSetLayout(const std::string &name) const;


private:
    // --- Registries ---
    // These maps hold all the assets this manager creates.
    std::map<std::string, VkPipeline> m_pipelines;
    std::map<std::string, VkPipelineLayout> m_pipelineLayouts;
    std::map<std::string, Ref<DescriptorSetLayout>> m_descriptorSetLayouts;

    VkPipelineLayout m_oldLayout; // From your original file

    // --- Core Vulkan Objects ---
    const Device &m_device;
    const SwapChain &m_swapChain;

    // --- Private Helper Functions ---
    /**
     * @brief Main function to create all pipeline types.
     */
    void createPipelines();

    /**
     * @brief Creates the "base" pipeline (no vertex input).
     */
    void createBasePipeline();

    /**
     * @brief Creates the "sphere" pipeline (3D mesh vertex input).
     */
    void createSpherePipeline();

    /**
     * @brief Creates the "grid" pipeline (for grid rendering).
     */
    void createGridPipeline();

    /**
     * @brief Helper to create a shader module from byte code.
     */
    VkShaderModule createShaderModule(const std::vector<unsigned char> &code) const;
    VkShaderModule createShaderModuleFromFile(const std::string& path) const;

    // Stored render pass
    VkRenderPass renderPass;
};
} // namespace vks
