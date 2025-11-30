// #include <vks/Material.hpp>
//
// namespace vks
// {
//     class GridMaterial : public Material
//     {
//     public:
//         GridMaterial(
//             const vks::Device& device,
//             const vks::GraphicsPipeline& pipelineManager,
//             Ref<vks::DescriptorPool> descriptorPool,
//             const std::string& pipelineName,
//             glm::vec4 color
//         );
//
//         void drawImguiEditor() override;
//
//     private:
//         struct MaterialUBO uboData;
//     };
//
//     inline void GridMaterial::drawImguiEditor()
//     {
//         // These widgets will return 'true' if they were changed
//         bool changed = false;
//
//         // Add a color picker for the baseColor
//         float color[4] = {
//             uboData.color.r,
//             uboData.color.g,
//             uboData.color.b,
//             uboData.color.a
//         };
//
//         changed |= ImGui::ColorEdit4("Base Color", color);
//
//         // If any widget was changed, update the material's UBO
//         if (changed)
//         {
//             MaterialUBO newUbo{};
//             newUbo.color = {color[0], color[1], color[2], color[3]};
//             updateUBO(newUbo);
//         }    }
// }
