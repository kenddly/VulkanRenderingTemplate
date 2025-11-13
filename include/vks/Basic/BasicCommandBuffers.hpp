#ifndef BASICCOMMANDBUFFERS_HPP
#define BASICCOMMANDBUFFERS_HPP

#include <vks/CommandBuffers.hpp>
#include <vks/Model.hpp>
#include <memory>

namespace vks {

class BasicCommandBuffers : public CommandBuffers {
public:
  BasicCommandBuffers(const Device &device, const RenderPass &renderpass,
                     const SwapChain &swapChain,
                     const GraphicsPipeline &graphicsPipeline,
                     const CommandPool &commandPool);
  void recreate();

private:
  void createCommandBuffers();
};
} // namespace vks

#endif // BASICCOMMANDBUFFERS_HPP
