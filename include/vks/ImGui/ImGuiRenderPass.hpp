#ifndef IMGUIRENDERPASS_HPP
#define IMGUIRENDERPASS_HPP

#include <stdexcept>
#include <vks/RenderPass.hpp>
#include <vulkan/vulkan.h>

namespace vks {
class ImGuiRenderPass : public RenderPass {
public:
  ImGuiRenderPass(const Device &device, const SwapChain &swapChain);

private:
  void createRenderPass() override;
  void createFrameBuffers() override;
};
} // namespace vks

#endif // IMGUIRENDERPASS_HPP
