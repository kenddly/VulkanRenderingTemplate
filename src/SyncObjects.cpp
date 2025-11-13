#include <vks/SyncObjects.hpp>

#include <vks/Device.hpp>
#include <vks/Window.hpp>

#include <stdexcept>

using namespace vks;

static VkSemaphoreCreateInfo makeSemaphoreCreateInfo() {
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    return semaphoreInfo;
}

static VkFenceCreateInfo makeFenceCreateInfo() {
    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    // Initialize fence to already signaled so it doesn't hang on first frame
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    return fenceInfo;
}

SyncObjects::SyncObjects(const Device& device, uint32_t numImages,
    uint32_t maxFramesInFlight)
    : m_device(device),
    m_numImages(numImages),
    m_maxFramesInFlight(maxFramesInFlight),
    // per-frame imageAvailable semaphores and per-frame fences:
    m_imageAvailable(maxFramesInFlight),
    m_inFlightFences(maxFramesInFlight),
    // renderFinished will be allocated per-swapchain-image:
    m_renderFinished(),
    m_imagesInFlight() {
    // initialize per-image tracking
    m_imagesInFlight.resize(m_numImages);
    for (auto& f : m_imagesInFlight) {
        f = VK_NULL_HANDLE;
    }

    VkSemaphoreCreateInfo semaphoreInfo = makeSemaphoreCreateInfo();
    VkFenceCreateInfo fenceInfo = makeFenceCreateInfo();

    // Create per-frame semaphores and fences
    for (size_t i = 0; i < m_maxFramesInFlight; ++i) {
        if (vkCreateSemaphore(m_device.logical(), &semaphoreInfo, nullptr,
            &m_imageAvailable[i]) != VK_SUCCESS ||
            vkCreateFence(m_device.logical(), &fenceInfo, nullptr,
                &m_inFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create per-frame synchronization objects!");
        }
    }

    // Create per-image renderFinished semaphores
    m_renderFinished.resize(m_numImages);
    for (size_t i = 0; i < m_numImages; ++i) {
        if (vkCreateSemaphore(m_device.logical(), &semaphoreInfo, nullptr,
            &m_renderFinished[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create per-image renderFinished semaphores!");
        }
    }
}

void SyncObjects::recreate(uint32_t numImages) {
    // destroy old per-image semaphores
    for (size_t i = 0; i < m_renderFinished.size(); ++i) {
        vkDestroySemaphore(m_device.logical(), m_renderFinished[i], nullptr);
    }

    // resize tracking to new number of images
    m_numImages = numImages;
    m_renderFinished.clear();
    m_renderFinished.resize(m_numImages);

    m_imagesInFlight.clear();
    m_imagesInFlight.resize(m_numImages);
    for (auto& f : m_imagesInFlight) {
        f = VK_NULL_HANDLE;
    }

    VkSemaphoreCreateInfo semaphoreInfo = makeSemaphoreCreateInfo();
    for (size_t i = 0; i < m_numImages; ++i) {
        if (vkCreateSemaphore(m_device.logical(), &semaphoreInfo, nullptr,
            &m_renderFinished[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to recreate per-image renderFinished semaphores!");
        }
    }
}

SyncObjects::~SyncObjects() {
    // Destroy per-image renderFinished semaphores
    for (size_t i = 0; i < m_renderFinished.size(); ++i) {
        vkDestroySemaphore(m_device.logical(), m_renderFinished[i], nullptr);
    }

    // Destroy per-frame semaphores and fences
    for (size_t i = 0; i < m_maxFramesInFlight; ++i) {
        vkDestroySemaphore(m_device.logical(), m_imageAvailable[i], nullptr);
        vkDestroyFence(m_device.logical(), m_inFlightFences[i], nullptr);
    }
}
