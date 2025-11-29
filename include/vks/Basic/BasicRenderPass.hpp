#ifndef BASICRENDERPASS_HPP
#define BASICRENDERPASS_HPP

#include <stdexcept>
#include <vks/RenderPass.hpp>
#include <vulkan/vulkan.h>

namespace vks {
class BasicRenderPass : public RenderPass {
public:
  BasicRenderPass(const Device &device, const SwapChain &swapChain);

private:
  void createRenderPass() override;
  void createFrameBuffers() override;
};
} // namespace vks

#endif // BASICRENDERPASS_HPP
