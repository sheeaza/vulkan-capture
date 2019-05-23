#include "render.hpp"

#include <vector>
#include <string>
#include <array>
#include <set>
#include <iostream>
#include <limits>
#include <fstream>
#include <cstring>
#include <chrono>

#include <opencv2/opencv.hpp>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define WIDTH 800
#define HEIGHT 600
static const int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<std::vector<Render::Vertex>>  vertices = {
    {
        {{-1.0f, -1.0f}, {1.0f, 0.0f}},
        {{0.0f, -1.0f}, {0.0f, 0.0f}},
        {{-1.0f, 0.0f},  {1.0f, 1.0f}},
        {{0.0f, 0.0f}, {0.0f, 1.0f}}
    }, {
        {{0.0f, -1.0f}, {1.0f, 0.0f}},
        {{1.0f, -1.0f}, {0.0f, 0.0f}},
        {{0.0f, 0.0f},  {1.0f, 1.0f}},
        {{1.0f, 0.0f}, {0.0f, 1.0f}}
    }, {
        {{-1.0f, 0.0f}, {1.0f, 0.0f}},
        {{0.0f, 0.0f}, {0.0f, 0.0f}},
        {{-1.0f, 1.0f},  {1.0f, 1.0f}},
        {{0.0f, 1.0f}, {0.0f, 1.0f}}
    }, {
        {{0.0f, 0.0f}, {1.0f, 0.0f}},
        {{1.0f, 0.0f}, {0.0f, 0.0f}},
        {{0.0f, 1.0f},  {1.0f, 1.0f}},
        {{1.0f, 1.0f}, {0.0f, 1.0f}}
    }
};

const std::vector<const char*> Render::validationLayers = {
    "VK_LAYER_LUNARG_standard_validation"
};

const std::vector<const char*> Render::deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

static PFN_vkDestroyDebugReportCallbackEXT pfn_vkDestroyDebugReportCallbackEXT;
void vkDestroyDebugReportCallbackEXT(
        VkInstance                                  instance,
        VkDebugReportCallbackEXT                    callback,
        const VkAllocationCallbacks*                pAllocator)
{
    pfn_vkDestroyDebugReportCallbackEXT(
            instance,
            callback,
            pAllocator
            );
}

static PFN_vkCreateDebugReportCallbackEXT pfn_vkCreateDebugReportCallbackEXT;
VkResult vkCreateDebugReportCallbackEXT(
        VkInstance                                  instance,
        const VkDebugReportCallbackCreateInfoEXT*   pCreateInfo,
        const VkAllocationCallbacks*                pAllocator,
        VkDebugReportCallbackEXT*                   pCallback)
{
    return pfn_vkCreateDebugReportCallbackEXT(
            instance,
            pCreateInfo,
            pAllocator,
            pCallback
            );
}

Render::Render()
{
}

Render::~Render()
{
    for (auto &stageMem : m_ustageMems) {
        for (auto &subStageMem : stageMem) {
            m_device->unmapMemory(*subStageMem);
        }
    }
    // m_device->waitIdle();

    // std::cout << __LINE__ << std::endl;
    // cleanupSwapChain();

    // m_device->destroyBuffer(*m_indexBuffer);
    // m_device->freeMemory(*m_indexBufferMemory);

    // m_device->destroyBuffer(*m_uVertexBuffers);
    // m_device->freeMemory(*m_uVertexBufferMems);

    // std::cout << "size " << m_inFlightFences.size() << std::endl;

    // for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        // // vkDestroySemaphore(m_device, m_renderFinishedSemaphores.at(i), nullptr);
        // // vkDestroySemaphore(m_device, m_imageAvailableSemaphores.at(i), nullptr);
        // // vkDestroyFence(m_device, m_inFlightFences.at(i), nullptr);
        // m_device->destroySemaphore(*m_renderFinishedSemaphores.at(i));
        // m_device->destroySemaphore(*m_imageAvailableSemaphores.at(i));
        // m_device->destroyFence(*m_inFlightFences.at(i));
    // }

    // // vkDestroyCommandPool(m_device, m_commandPool, nullptr);
    // m_device->destroyCommandPool(*m_commandPool);

    // // vkDestroyDevice(m_device, nullptr);
    // m_device->destroy();

#ifndef NDEBUG
    // m_instance->destroyDebugReportCallbackEXT(*m_debugCallback);
#endif

    // m_instance->destroySurfaceKHR(*m_surface);
    // m_instance->destroy();

    // for (auto &imageView : m_swapChainImageViews) {
        // m_device->destroyImageView(*imageView);
    // }
    // m_device->destroySwapchainKHR(*m_swapChain);
    // m_device->destroy();
    // m_instance->destroySurfaceKHR(*m_surface);
    // m_instance->destroy();

    // glfwDestroyWindow(m_window);

    // glfwTerminate();
    // std::cout << __LINE__ << std::endl;
}

void Render::init()
{
    initWindow();

    createInstance();

#ifndef NDEBUG
    setupDebugMessage();
#endif

    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createImageViews();
    createRenderPass();
    createDescriptorSetLayout();
    createGraphicsPipeline();
    createFramebuffers();
    createCommandPool();
    createTextureImage();
    createTextureImageView();
    createTextureSampler();
    createVertexBuffer();
    // createIndexBuffer();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffers(0);
    createSyncObjects();
}

void Render::updateTexture(int index, int subIndex)
{
    int imageWidth = 1280;
    int imageHeight = 800;

    transitionImageLayout(*m_utextureImage, vk::ImageLayout::eUndefined,
                          vk::ImageLayout::eTransferDstOptimal,
                          vk::PipelineStageFlagBits::eTopOfPipe,
                          vk::PipelineStageFlagBits::eTransfer);
    vk::BufferImageCopy copyRegion(0, 0, 0,
                                   vk::ImageSubresourceLayers(
                                       vk::ImageAspectFlagBits::eColor,
                                       0, index, 1), vk::Offset3D(0, 0, 0),
                                   vk::Extent3D(imageWidth, imageHeight, 1));
    std::vector<vk::UniqueCommandBuffer> ucmdBuffers =
        m_device->allocateCommandBuffersUnique(
                vk::CommandBufferAllocateInfo(*m_commandPool,
                                              vk::CommandBufferLevel::ePrimary,
                                              1));
    ucmdBuffers[0]->begin(
            vk::CommandBufferBeginInfo(
                vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
    ucmdBuffers[0]->copyBufferToImage(*m_ustageBuffers[index][subIndex], *m_utextureImage,
                                         vk::ImageLayout::eTransferDstOptimal,
                                         copyRegion);
    ucmdBuffers[0]->end();
    m_graphicsQueue.submit(vk::SubmitInfo(0, nullptr, nullptr, 1,
                                          &*ucmdBuffers[0]), {});
    m_graphicsQueue.waitIdle();
    transitionImageLayout(*m_utextureImage, vk::ImageLayout::eTransferDstOptimal,
                          vk::ImageLayout::eShaderReadOnlyOptimal,
                          vk::PipelineStageFlagBits::eTransfer,
                          vk::PipelineStageFlagBits::eFragmentShader);

}

void Render::getBufferAddrs(int index, std::array<void *, 4> &bufferMaps)
{
    bufferMaps = m_stageMemMaps[index];
}

void Render::render(int index)
{
    m_device->waitForFences(1, &*m_inFlightFences.at(m_currentFrame),
                            VK_TRUE, std::numeric_limits<uint64_t>::max());

    uint32_t imageIndex;
    vk::Result result =
        m_device->acquireNextImageKHR(*m_swapChain,
                                      std::numeric_limits<uint64_t>::max(),
                                      *m_imageAvailableSemaphores.at(m_currentFrame),
                                      nullptr, &imageIndex);

    if (result == vk::Result::eErrorOutOfDateKHR) {
        recreateSwapChain(index);
        std::cout << "recreating" << std::endl;
        return;
    } else if (result != vk::Result::eSuccess &&
               result != vk::Result::eSuboptimalKHR) {
        throw std::runtime_error("failed to acquire swap chain image");
    }

    vk::ClearValue clearColor(
            vk::ClearColorValue(
                std::array<float, 4>({0.0f, 0.0f, 0.0f, 1.0f})));
    updateUniformBuffer(imageIndex);
    // for (size_t i = 0; i < m_commandBuffers.size(); i++) {
        // m_commandBuffers[i]->begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eSimultaneousUse));
        // m_commandBuffers[i]->beginRenderPass(
            // vk::RenderPassBeginInfo(*m_renderPass,
                                    // *m_swapChainFramebuffers.at(i),
                                    // vk::Rect2D(vk::Offset2D(0, 0),
                                               // m_swapChainExtent),
                                    // 1, &clearColor),
            // vk::SubpassContents::eInline);
        // m_commandBuffers[i]->bindPipeline(vk::PipelineBindPoint::eGraphics,*m_graphicsPipeline);
        // vk::DeviceSize offset = 0;
        // m_commandBuffers[i]->bindVertexBuffers(0, *m_uVertexBuffers[index], offset);
        // m_commandBuffers[i]->draw(static_cast<uint32_t>(vertices[0].size()), 1, 0, 0);
        // m_commandBuffers.at(i)->endRenderPass();
        // m_commandBuffers[i]->end();
    // }

    vk::PipelineStageFlags waitStages[] =
        {vk::PipelineStageFlagBits::eColorAttachmentOutput};

    vk::SubmitInfo submitInfo(1, &*m_imageAvailableSemaphores.at(m_currentFrame),
                              waitStages, 1, &*m_commandBuffers.at(imageIndex),
                              1, &*m_renderFinishedSemaphores.at(m_currentFrame));

    m_device->resetFences(1, &*m_inFlightFences.at(m_currentFrame));

    result = m_graphicsQueue.submit(1, &submitInfo,
                                    *m_inFlightFences.at(m_currentFrame));
    if (result != vk::Result::eSuccess)
        throw std::runtime_error("failed to submit draw command buffer!");

    vk::PresentInfoKHR
        presentInfo(1, &*m_renderFinishedSemaphores.at(m_currentFrame),
                    1, &*m_swapChain, &imageIndex);
    result = m_presentQueue.presentKHR(presentInfo);

    if (result == vk::Result::eErrorOutOfDateKHR ||
        result == vk::Result::eSuboptimalKHR ||
        framebufferResized) {
        framebufferResized = false;
        recreateSwapChain(index);
        std::cout << "recreating" << std::endl;
    } else if (result != vk::Result::eSuccess) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    m_device->waitIdle();
}

bool Render::checkValidationLayerSupport()
{
    std::vector<vk::LayerProperties> availableLayers =
        vk::enumerateInstanceLayerProperties();

    if (availableLayers.empty())
        return false;

    for (const char *layerName : validationLayers) {
        bool found = false;

        for (const auto &layerProperty : availableLayers) {
            if (strcmp(layerName, layerProperty.layerName) == 0) {
                found = true;
                break;
            }
        }

        if (!found)
            return false;
    }

    return true;
}

void Render::initWindow()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    m_window = glfwCreateWindow(800, 600, "vulkan", nullptr, nullptr);
    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
}

std::vector<const char*> Render::getRequiredExtension()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions,
                                        glfwExtensions + glfwExtensionCount);

#ifndef NDEBUG
    extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif

    return extensions;
}

void Render::createInstance()
{
#ifndef NDEBUG
    if (!checkValidationLayerSupport()) {
        std::cout << "validation layers not suppport!" << std::endl;
    }
#endif

    if (!glfwVulkanSupported()) {
        throw std::runtime_error("vulkan not supported!");
    }

    vk::ApplicationInfo appInfo("triangle", 1, "vulkan", 1, VK_API_VERSION_1_0);
    vk::InstanceCreateInfo instanceCreateInfo({}, &appInfo);

#ifndef NDEBUG
    {
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount,
                                           extensions.data());
    std::cout << "available extensions:" << std::endl;
    for (const auto &ext : extensions) {
        std::cout << "\t" << ext.extensionName << std::endl;
    }
    }
#endif

#ifndef NDEBUG
    instanceCreateInfo.enabledLayerCount =
        static_cast<uint32_t>(validationLayers.size());
    instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
#else
    instanceCreateInfo.enabledLayerCount = 0;
#endif

    auto extensions = getRequiredExtension();
    instanceCreateInfo.enabledExtensionCount =
        static_cast<uint32_t>(extensions.size());
    instanceCreateInfo.ppEnabledExtensionNames = extensions.data();

    m_instance = vk::createInstanceUnique(instanceCreateInfo);
}

void Render::setupDebugMessage()
{
    pfn_vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)
        vkGetInstanceProcAddr(*m_instance, "vkCreateDebugReportCallbackEXT");
    pfn_vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)
        vkGetInstanceProcAddr(*m_instance, "vkDestroyDebugReportCallbackEXT");

    vk::DebugReportCallbackCreateInfoEXT createInfo(
            vk::DebugReportFlagBitsEXT::eWarning |
            vk::DebugReportFlagBitsEXT::eError, debugCallback);

    m_debugCallback =
        m_instance->createDebugReportCallbackEXTUnique(createInfo);
}

VkBool32 Render::debugCallback(VkDebugReportFlagsEXT flags,
                               VkDebugReportObjectTypeEXT objectType,
                               uint64_t object, size_t location,
                               int32_t messageCode, const char *pLayerPrefix,
                               const char *pMessage, void *pUserData)
{
    std::cerr << "error: " << pMessage << std::endl;

    return VK_FALSE;
}

void Render::createSurface()
{
    VkSurfaceKHR surface;

    int ret = glfwCreateWindowSurface(*m_instance, m_window, nullptr, &surface);

    if (ret != VK_SUCCESS) {
        throw std::runtime_error("createSurface failed");
    }

    m_surface = vk::UniqueSurfaceKHR(surface,
                                     vk::SurfaceKHRDeleter(*m_instance));
}

void Render::pickPhysicalDevice()
{
    std::vector<vk::PhysicalDevice> devices =
        m_instance->enumeratePhysicalDevices();

    for (const auto &device : devices) {
        if (isDeviceSuitable(device)) {
            m_physicalDevice = device;
            return;
        }
    }

    throw std::runtime_error("failed to find a suitable gpu");
}

bool Render::isDeviceSuitable(vk::PhysicalDevice device)
{
    QueueFamilyIndices indices = findQueueFamilies(device);

    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() &&
                            !swapChainSupport.presentModes.empty();
    }

    vk::PhysicalDeviceFeatures supportedFeatures =
        device.getFeatures();

    return indices.isComplete() && extensionsSupported && swapChainAdequate &&
           supportedFeatures.samplerAnisotropy;
}

bool Render::checkDeviceExtensionSupport(vk::PhysicalDevice device)
{
    std::vector<vk::ExtensionProperties> availableExtensions =
        device.enumerateDeviceExtensionProperties();

    std::set<std::string> requiredExtensions(deviceExtensions.begin(),
                                             deviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

Render::QueueFamilyIndices Render::findQueueFamilies(vk::PhysicalDevice device)
{
    QueueFamilyIndices indices;

    std::vector<vk::QueueFamilyProperties> queueFamilies =
        device.getQueueFamilyProperties();

    int i = 0;
    for (const auto &queueFamily : queueFamilies) {
        if (queueFamily.queueCount > 0 &&
            queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = device.getSurfaceSupportKHR(i, *m_surface);

        if (queueFamily.queueCount > 0 && presentSupport) {
            indices.presentFamily = i;
        }

        if (indices.isComplete()) {
            break;
        }

        i++;
    }

    return indices;
}

void Render::createLogicalDevice()
{
    QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice);

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies  = {indices.graphicsFamily,
                                               indices.presentFamily};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        queueCreateInfos.push_back(vk::DeviceQueueCreateInfo(
                                   {}, queueFamily, 1, &queuePriority));
    }

    vk::PhysicalDeviceFeatures deviceFeatures;
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    vk::DeviceCreateInfo
        createInfo({}, static_cast<uint32_t>(queueCreateInfos.size()),
                   queueCreateInfos.data(),
#ifndef NDEBUG
                   static_cast<uint32_t>(validationLayers.size()),
                   validationLayers.data(),
#else
                   0, nullptr,
#endif
                   static_cast<uint32_t>(deviceExtensions.size()),
                   deviceExtensions.data(),
                   &deviceFeatures);

    m_device = m_physicalDevice.createDeviceUnique(createInfo);

    m_graphicsQueue = m_device->getQueue(indices.graphicsFamily, 0);
    m_presentQueue = m_device->getQueue(indices.presentFamily, 0);
}

void Render::createSwapChain()
{
    SwapChainSupportDetails swapChainSupport =
        querySwapChainSupport(m_physicalDevice);

    vk::SurfaceFormatKHR surfaceFormat =
        chooseSwapSurfaceFormat(swapChainSupport.formats);
    vk::PresentModeKHR presentMode =
        chooseSwapPresentMode(swapChainSupport.presentModes);
    vk::Extent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 &&
        imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR
        createInfo({}, *m_surface, imageCount, surfaceFormat.format,
                   surfaceFormat.colorSpace, extent, 1,
                   vk::ImageUsageFlagBits::eColorAttachment);

    QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily,
                                     indices.presentFamily};

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = vk::SharingMode::eExclusive;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    m_swapChain = m_device->createSwapchainKHRUnique(createInfo);
    m_swapChainImages = m_device->getSwapchainImagesKHR(*m_swapChain);

    m_swapChainImageFormat = surfaceFormat.format;
    m_swapChainExtent = extent;
}

Render::SwapChainSupportDetails
    Render::querySwapChainSupport(vk::PhysicalDevice device)
{
    SwapChainSupportDetails details;

    details.capabilities = device.getSurfaceCapabilitiesKHR(*m_surface);
    details.formats = device.getSurfaceFormatsKHR(*m_surface);
    details.presentModes = device.getSurfacePresentModesKHR(*m_surface);

    return details;
}

vk::SurfaceFormatKHR Render::chooseSwapSurfaceFormat(
        const std::vector<vk::SurfaceFormatKHR>& availableFormats)
{
    if (availableFormats.size() == 1 &&
        availableFormats[0].format == vk::Format::eUndefined) {
        return {vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear};
    }

    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == vk::Format::eB8G8R8A8Unorm &&
            availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

vk::PresentModeKHR Render::chooseSwapPresentMode(
        const std::vector<vk::PresentModeKHR>& availablePresentModes)
{
    vk::PresentModeKHR bestMode = vk::PresentModeKHR::eFifo;

    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
            return availablePresentMode;
        } else if (availablePresentMode == vk::PresentModeKHR::eImmediate) {
            bestMode = availablePresentMode;
        }
    }

    return bestMode;
}

vk::Extent2D Render::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR&
                                    capabilities)
{
    if (capabilities.currentExtent.width !=
            std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(m_window, &width, &height);
        vk::Extent2D actualExtent(static_cast<uint32_t>(width),
                                  static_cast<uint32_t>(height));

        actualExtent.width =
            std::max(capabilities.minImageExtent.width,
                     std::min(capabilities.maxImageExtent.width,
                              actualExtent.width));
        actualExtent.height =
            std::max(capabilities.minImageExtent.height,
                     std::min(capabilities.maxImageExtent.height,
                              actualExtent.height));

        return actualExtent;
    }
}

void Render::createImageViews()
{
    m_swapChainImageViews.resize(m_swapChainImages.size());

    for (size_t i = 0; i < m_swapChainImages.size(); i++) {
        vk::ImageViewCreateInfo
            createInfo({}, m_swapChainImages.at(i), vk::ImageViewType::e2D,
                       m_swapChainImageFormat,
                       vk::ComponentMapping(),
                       vk::ImageSubresourceRange(
                           vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

        m_swapChainImageViews.at(i) =
                m_device->createImageViewUnique(createInfo);
    }
}

void Render::createDescriptorSetLayout()
{
    vk::DescriptorSetLayoutBinding uboLayoutBinding(
            0, vk::DescriptorType::eUniformBuffer, 1,
            vk::ShaderStageFlagBits::eVertex);

    vk::DescriptorSetLayoutBinding samplerLayoutBinding(
            1, vk::DescriptorType::eCombinedImageSampler, 1,
            vk::ShaderStageFlagBits::eFragment);

    std::array<vk::DescriptorSetLayoutBinding, 2> bindings = {
        uboLayoutBinding, samplerLayoutBinding};

    m_descriptorSetLayout = m_device->createDescriptorSetLayoutUnique(
            vk::DescriptorSetLayoutCreateInfo({}, 2, bindings.data()));
}

void Render::createRenderPass()
{
    vk::AttachmentDescription colorAttachment({}, m_swapChainImageFormat,
                                              vk::SampleCountFlagBits::e1,
                                              vk::AttachmentLoadOp::eClear,
                                              vk::AttachmentStoreOp::eStore,
                                              vk::AttachmentLoadOp::eDontCare,
                                              vk::AttachmentStoreOp::eDontCare,
                                              vk::ImageLayout::eUndefined,
                                              vk::ImageLayout::ePresentSrcKHR);

    vk::AttachmentReference
        colorAttachmentRef(0, vk::ImageLayout::eColorAttachmentOptimal);

    vk::AttachmentReference colorReference(0, vk::ImageLayout::eUndefined);
    vk::SubpassDescription subpass({}, vk::PipelineBindPoint::eGraphics,
                                   0, nullptr, 1, &colorAttachmentRef);

    vk::RenderPassCreateInfo renderPassInfo({}, 1, &colorAttachment,
                                            1, &subpass);

    m_renderPass = m_device->createRenderPassUnique(renderPassInfo);
}

void Render::createGraphicsPipeline()
{
    auto vertShaderCode = readFile("shader.vert.spv");
    auto fragShaderCode = readFile("shader.frag.spv");

    if (vertShaderCode.size() == 0 || fragShaderCode.size() == 0) {
        throw std::runtime_error("createGraphicsPipeline failed");
    }

    vk::UniqueShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    vk::UniqueShaderModule fragShaderModule = createShaderModule(fragShaderCode);
    if (!vertShaderModule || !fragShaderModule) {
        throw std::runtime_error("createGraphicsPipeline failed");
    }

    vk::PipelineShaderStageCreateInfo shaderStages[2] = {
        vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eVertex,
                                          *vertShaderModule, "main"),
        vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eFragment,
                                          *fragShaderModule, "main")};

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();
    vk::PipelineVertexInputStateCreateInfo
        vertexInputInfo({}, 1, &bindingDescription,
                        static_cast<uint32_t>(attributeDescriptions.size()),
                        attributeDescriptions.data());

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly(
            {}, vk::PrimitiveTopology::eTriangleStrip);

    vk::Viewport viewport(0.0f, 0.0f, (float)m_swapChainExtent.width,
                          (float)m_swapChainExtent.height, 0.0f, 1.0f);

    vk::Rect2D scissor(vk::Offset2D(), m_swapChainExtent);

    vk::PipelineViewportStateCreateInfo viewportState(
            {}, 1, &viewport, 1, &scissor);

    vk::PipelineRasterizationStateCreateInfo rasterizer(
            {}, VK_FALSE, VK_FALSE, vk::PolygonMode::eFill,
            vk::CullModeFlagBits::eNone, vk::FrontFace::eClockwise,
            VK_FALSE, 0, 0, 0, 1.0f);

    vk::PipelineMultisampleStateCreateInfo multisampling(
            {}, vk::SampleCountFlagBits::e1, VK_FALSE);

    vk::PipelineColorBlendAttachmentState colorBlendAttachment;
    colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR |
                                          vk::ColorComponentFlagBits::eG |
                                          vk::ColorComponentFlagBits::eB |
                                          vk::ColorComponentFlagBits::eA;

    vk::PipelineColorBlendStateCreateInfo colorBlending(
            {}, VK_FALSE, vk::LogicOp::eCopy, 1, &colorBlendAttachment);

    m_pipelineLayout = m_device->createPipelineLayoutUnique(
            vk::PipelineLayoutCreateInfo({}, 1, &*m_descriptorSetLayout));

    vk::GraphicsPipelineCreateInfo pipelineInfo(
            {}, 2, shaderStages, &vertexInputInfo, &inputAssembly,
            nullptr, &viewportState, &rasterizer, &multisampling,
            nullptr, &colorBlending, nullptr, *m_pipelineLayout,
            *m_renderPass);

    m_graphicsPipeline = m_device->createGraphicsPipelineUnique(nullptr,
                                                                pipelineInfo);
}

std::vector<char> Render::readFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        return std::vector<char>();
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

vk::UniqueShaderModule Render::createShaderModule(const std::vector<char>& code)
{
    int ret;

    vk::ShaderModuleCreateInfo
        createInfo({}, code.size(),
                   reinterpret_cast<const uint32_t*>(code.data()));

    return m_device->createShaderModuleUnique(createInfo);
}

void Render::createFramebuffers()
{
    m_swapChainFramebuffers.resize(m_swapChainImageViews.size());

    for (size_t i = 0; i < m_swapChainImageViews.size(); i++) {
        m_swapChainFramebuffers.at(i) =
            m_device->createFramebufferUnique(
                vk::FramebufferCreateInfo({}, *m_renderPass,
                                          1, &*m_swapChainImageViews.at(i),
                                          m_swapChainExtent.width,
                                          m_swapChainExtent.height,
                                          1));
    }
}

void Render::createCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(m_physicalDevice);

    m_commandPool = m_device->createCommandPoolUnique(
            vk::CommandPoolCreateInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
									  queueFamilyIndices.graphicsFamily));
}

void Render::transitionImageLayout(vk::Image image, vk::ImageLayout oldLayout,
                           vk::ImageLayout newLayout,
                           vk::PipelineStageFlags srcStageMask,
                           vk::PipelineStageFlags dstStageMask)
{
    std::vector<vk::UniqueCommandBuffer> ucmdBuffers =
        m_device->allocateCommandBuffersUnique(
                vk::CommandBufferAllocateInfo(*m_commandPool,
                                              vk::CommandBufferLevel::ePrimary,
                                              1));
    ucmdBuffers[0]->begin(
            vk::CommandBufferBeginInfo(
                vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

    vk::AccessFlags  srcAccessMask;
    switch (oldLayout) {
    case vk::ImageLayout::eColorAttachmentOptimal:
        srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        break;
    case vk::ImageLayout::eTransferDstOptimal:
        srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        break;
    case vk::ImageLayout::ePreinitialized:
        srcAccessMask = vk::AccessFlagBits::eHostWrite;
        break;
    default:
        break;
    }

    vk::AccessFlags dstAccessMask;
    switch (newLayout) {
    case vk::ImageLayout::eTransferDstOptimal:
        dstAccessMask = vk::AccessFlagBits::eTransferWrite;
        break;
    case vk::ImageLayout::eTransferSrcOptimal:
        dstAccessMask = vk::AccessFlagBits::eTransferRead;
        break;
    case vk::ImageLayout::eShaderReadOnlyOptimal:
        dstAccessMask = vk::AccessFlagBits::eShaderRead;
        break;
    case vk::ImageLayout::eColorAttachmentOptimal:
        dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        break;
    case vk::ImageLayout::eDepthStencilAttachmentOptimal:
        dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        break;
    default:
        break;
    }

    vk::ImageSubresourceRange
        imageSubresourceRange(vk::ImageAspectFlagBits::eColor,
                              0, 1, 0, 4); //TODO layer count dyn
    vk::ImageMemoryBarrier imageMemoryBarrier(srcAccessMask, dstAccessMask,
                                              oldLayout, newLayout,
                                              VK_QUEUE_FAMILY_IGNORED,
                                              VK_QUEUE_FAMILY_IGNORED,
                                              image, imageSubresourceRange);
    ucmdBuffers[0]->pipelineBarrier(srcStageMask, dstStageMask, {}, nullptr,
                                nullptr, imageMemoryBarrier);

    ucmdBuffers[0]->end();

    m_graphicsQueue.submit(vk::SubmitInfo(0, nullptr, nullptr, 1,
                                          &*ucmdBuffers[0]), {});
    m_graphicsQueue.waitIdle();
}

void Render::createTextureImage()
{
    int imageWidth = 1280;
    int imageHeight = 800;

    // cv::Mat image = cv::imread("src_1.jpg");
    // if (image.empty()) {
        // throw std::runtime_error("failed to openg image");
    // }
    // cv::cvtColor(image, image, cv::COLOR_BGR2BGRA);

    // vk::UniqueBuffer stageBuffer;
    // vk::UniqueDeviceMemory stageMem;
    // vk::MemoryRequirements stageMemReq;
    // stageBuffer = m_device->createBufferUnique(
            // vk::BufferCreateInfo({}, image.cols * image.rows * 4,
                                 // vk::BufferUsageFlagBits::eTransferSrc));
    // stageMemReq = m_device->getBufferMemoryRequirements(*stageBuffer);
    // uint32_t stageMemTypeIndex =
        // findMemoryType(stageMemReq.memoryTypeBits,
                       // vk::MemoryPropertyFlagBits::eHostVisible |
                       // vk::MemoryPropertyFlagBits::eHostCoherent);
    // stageMem = m_device->allocateMemoryUnique(
            // vk::MemoryAllocateInfo(stageMemReq.size, stageMemTypeIndex));
    // m_device->bindBufferMemory(*stageBuffer, *stageMem, 0);

    // void *data = m_device->mapMemory(*stageMem, 0, stageMemReq.size);
    // memcpy(data, image.data, stageMemReq.size);
    // m_device->unmapMemory(*stageMem);

    m_ustageBuffers.resize(4);
    m_ustageMems.resize(4);
    m_stageMemMaps.resize(4);

    for (size_t i = 0; i < m_ustageBuffers.size(); i++) {
        for (size_t j = 0; j < m_ustageBuffers[i].size(); j++) {
            vk::UniqueBuffer stageBuffer;
            vk::UniqueDeviceMemory stageMem;
            vk::MemoryRequirements stageMemReq;

            stageBuffer = m_device->createBufferUnique(
                    vk::BufferCreateInfo({}, imageWidth * imageHeight * 4,
                        vk::BufferUsageFlagBits::eTransferSrc));
            stageMemReq = m_device->getBufferMemoryRequirements(*stageBuffer);
            uint32_t stageMemTypeIndex =
                findMemoryType(stageMemReq.memoryTypeBits,
                        vk::MemoryPropertyFlagBits::eHostVisible |
                        vk::MemoryPropertyFlagBits::eHostCoherent);
            stageMem = m_device->allocateMemoryUnique(
                    vk::MemoryAllocateInfo(stageMemReq.size, stageMemTypeIndex));
            m_device->bindBufferMemory(*stageBuffer, *stageMem, 0);

            void *data = m_device->mapMemory(*stageMem, 0, stageMemReq.size);

            m_ustageBuffers[i][j] = std::move(stageBuffer);
            m_ustageMems[i][j] = std::move(stageMem);
            m_stageMemMaps[i][j] = data;
        }
    }

    m_utextureImage = m_device->createImageUnique(
            vk::ImageCreateInfo({}, vk::ImageType::e2D,
                vk::Format::eR8G8B8A8Unorm,
                vk::Extent3D(imageWidth, imageHeight, 1),
                1, 4, vk::SampleCountFlagBits::e1, // TODO layout number dynamic
                vk::ImageTiling::eOptimal,
                vk::ImageUsageFlagBits::eSampled |
                vk::ImageUsageFlagBits::eTransferDst,
                vk::SharingMode::eExclusive,
                0, nullptr, vk::ImageLayout::eUndefined));

    vk::MemoryRequirements memoryRequirements =
        m_device->getImageMemoryRequirements(*m_utextureImage);
    uint32_t memoryTypeIndex =
        findMemoryType(memoryRequirements.memoryTypeBits,
                vk::MemoryPropertyFlagBits::eDeviceLocal);
    m_utextureMem = m_device->allocateMemoryUnique(
            vk::MemoryAllocateInfo(memoryRequirements.size, memoryTypeIndex));
    m_device->bindImageMemory(*m_utextureImage, *m_utextureMem, 0);

    // transitionImageLayout(*m_utextureImage, vk::ImageLayout::eUndefined,
                          // vk::ImageLayout::eTransferDstOptimal,
                          // vk::PipelineStageFlagBits::eTopOfPipe,
                          // vk::PipelineStageFlagBits::eTransfer);
    // vk::BufferImageCopy copyRegion(0, 0, 0,
                                   // vk::ImageSubresourceLayers(
                                       // vk::ImageAspectFlagBits::eColor,
                                       // 0, 0, 1), vk::Offset3D(0, 0, 0),
                                   // vk::Extent3D(imageWidth, imageHeight, 1));
    // std::vector<vk::UniqueCommandBuffer> ucmdBuffers =
        // m_device->allocateCommandBuffersUnique(
                // vk::CommandBufferAllocateInfo(*m_commandPool,
                                              // vk::CommandBufferLevel::ePrimary,
                                              // 1));
    // ucmdBuffers[0]->begin(
            // vk::CommandBufferBeginInfo(
                // vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
    // ucmdBuffers[0]->copyBufferToImage(*stageBuffer, *m_utextureImage,
                                         // vk::ImageLayout::eTransferDstOptimal,
                                         // copyRegion);
    // ucmdBuffers[0]->end();
    // m_graphicsQueue.submit(vk::SubmitInfo(0, nullptr, nullptr, 1,
                                          // &*ucmdBuffers[0]), {});
    // m_graphicsQueue.waitIdle();
    // transitionImageLayout(*m_utextureImage, vk::ImageLayout::eTransferDstOptimal,
                          // vk::ImageLayout::eShaderReadOnlyOptimal,
                          // vk::PipelineStageFlagBits::eTransfer,
                          // vk::PipelineStageFlagBits::eFragmentShader);
}

void Render::createTextureImageView()
{
    m_utextureImageView = m_device->createImageViewUnique(
            vk::ImageViewCreateInfo({}, *m_utextureImage,
                vk::ImageViewType::e2DArray,
                vk::Format::eR8G8B8A8Unorm, {},
                vk::ImageSubresourceRange(
                    vk::ImageAspectFlagBits::eColor,
                    0, 1, 0, 4)));
}

void Render::createTextureSampler()
{
    m_utextureSampler = m_device->createSamplerUnique(
            vk::SamplerCreateInfo({}, vk::Filter::eLinear, vk::Filter::eLinear,
                                  vk::SamplerMipmapMode::eLinear,
                                  vk::SamplerAddressMode::eClampToBorder,
                                  vk::SamplerAddressMode::eClampToBorder,
                                  vk::SamplerAddressMode::eClampToBorder,
                                  0, VK_TRUE, 16, VK_FALSE,
                                  vk::CompareOp::eAlways));
}

void Render::createVertexBuffer()
{
    m_uVertexBuffers.resize(4);
    m_uVertexBufferMems.resize(4);

    for (size_t i = 0; i < m_uVertexBuffers.size(); i++) {
        auto &vertex = vertices[i];

        uint32_t bufferSize = sizeof(vertex[0]) * vertex.size();

        m_uVertexBuffers[i] = m_device->createBufferUnique(
                vk::BufferCreateInfo({}, bufferSize,
                    vk::BufferUsageFlagBits::eVertexBuffer));
        vk::MemoryRequirements memRequirements =
            m_device->getBufferMemoryRequirements(*m_uVertexBuffers[i]);

        uint32_t memoryTypeIndex =
            findMemoryType(memRequirements.memoryTypeBits,
                    vk::MemoryPropertyFlagBits::eHostVisible |
                    vk::MemoryPropertyFlagBits::eHostCoherent);
        m_uVertexBufferMems[i] = m_device->allocateMemoryUnique(
                vk::MemoryAllocateInfo(memRequirements.size, memoryTypeIndex));

        void *data = m_device->mapMemory(*m_uVertexBufferMems[i], 0,
                memRequirements.size);
        memcpy(data, vertex.data(), bufferSize);
        m_device->unmapMemory(*m_uVertexBufferMems[i]);

        m_device->bindBufferMemory(*m_uVertexBuffers[i], *m_uVertexBufferMems[i], 0);
    }

}

void Render::createIndexBuffer()
{
    // uint32_t bufferSize = sizeof(indices[0]) * indices.size();

    // m_indexBuffer = m_device->createBufferUnique(
            // vk::BufferCreateInfo({}, bufferSize,
                                 // vk::BufferUsageFlagBits::eIndexBuffer));
    // vk::MemoryRequirements memRequirements =
        // m_device->getBufferMemoryRequirements(*m_indexBuffer);

    // uint32_t memoryTypeIndex =
        // findMemoryType(memRequirements.memoryTypeBits,
                       // vk::MemoryPropertyFlagBits::eHostVisible |
                       // vk::MemoryPropertyFlagBits::eHostCoherent);
    // m_indexBufferMemory = m_device->allocateMemoryUnique(
            // vk::MemoryAllocateInfo(memRequirements.size, memoryTypeIndex));

    // void *data = m_device->mapMemory(*m_indexBufferMemory, 0,
                                     // memRequirements.size);
    // memcpy(data, indices.data(), bufferSize);
    // m_device->unmapMemory(*m_indexBufferMemory);

    // m_device->bindBufferMemory(*m_indexBuffer, *m_indexBufferMemory, 0);
}

uint32_t Render::findMemoryType(uint32_t typeFilter,
                                vk::MemoryPropertyFlags properties)
{
    vk::PhysicalDeviceMemoryProperties memProperties =
        m_physicalDevice.getMemoryProperties();

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter  & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties)
                == properties) {
            return i;
        }
    }

    throw std::runtime_error("findMemoryType failed");
}

void Render::createUniformBuffers()
{
    uint32_t bufferSize = sizeof(UniformBufferObject);

    m_uniformBuffers.resize(m_swapChainImages.size());
    m_uniformBuffersMemory.resize(m_swapChainImages.size());

    for (size_t i = 0; i < m_swapChainImages.size(); i++) {
        m_uniformBuffers.at(i) = m_device->createBufferUnique(
                vk::BufferCreateInfo({}, bufferSize,
                                     vk::BufferUsageFlagBits::eUniformBuffer));
        vk::MemoryRequirements memRequirements =
            m_device->getBufferMemoryRequirements(*m_uniformBuffers.at(i));

        uint32_t memoryTypeIndex =
            findMemoryType(memRequirements.memoryTypeBits,
                           vk::MemoryPropertyFlagBits::eHostVisible |
                           vk::MemoryPropertyFlagBits::eHostCoherent);
        m_uniformBuffersMemory.at(i) = m_device->allocateMemoryUnique(
                vk::MemoryAllocateInfo(memRequirements.size, memoryTypeIndex));

        m_device->bindBufferMemory(*m_uniformBuffers.at(i),
                                   *m_uniformBuffersMemory.at(i), 0);
    }
}

void Render::updateUniformBuffer(uint32_t currentImage)
{
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>
                    (currentTime - startTime).count();

    UniformBufferObject ubo = {};
    ubo.model = glm::rotate(glm::mat4(1.0f),
                            time * glm::radians(90.0f),
                            glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f),
                           glm::vec3(0.0f, 0.0f, 0.0f),
                           glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f),
                                m_swapChainExtent.width /
                                    (float)m_swapChainExtent.height,
                                0.001f, 256.0f);
    ubo.proj[1][1] *= -1;

    ubo.model = glm::mat4(1.0f);
    ubo.view = glm::mat4(1.0f);
    ubo.proj = glm::mat4(1.0f);

    void *data = m_device->mapMemory(*m_uniformBuffersMemory.at(currentImage),
                                     0, sizeof(ubo));
    memcpy(data, &ubo, sizeof(ubo));
    m_device->unmapMemory(*m_uniformBuffersMemory.at(currentImage));
}

void Render::createDescriptorPool()
{
    uint32_t descriptCnt = static_cast<uint32_t>(m_swapChainImages.size());
    descriptCnt *= 4;

    std::array<vk::DescriptorPoolSize, 2> poolSizes = {
        vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer,
                               descriptCnt),
        vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler,
                               descriptCnt)};

    m_descriptorPool = m_device->createDescriptorPoolUnique(
            vk::DescriptorPoolCreateInfo(
                vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
                descriptCnt, poolSizes.size(), poolSizes.data()));
}

void Render::createDescriptorSets()
{
    std::vector<vk::DescriptorSetLayout> layouts(m_swapChainImages.size(),
                                                 *m_descriptorSetLayout);

    m_descriptorSets = m_device->allocateDescriptorSetsUnique(
            vk::DescriptorSetAllocateInfo(
                *m_descriptorPool,
                static_cast<uint32_t>(m_swapChainImages.size()),
                layouts.data()));

    for (size_t i = 0; i < m_swapChainImages.size(); i++) {
        vk::DescriptorBufferInfo bufferInfo(*m_uniformBuffers.at(i),
                0, sizeof(UniformBufferObject));

        vk::DescriptorImageInfo
            imageInfo(*m_utextureSampler, *m_utextureImageView,
                    vk::ImageLayout::eShaderReadOnlyOptimal);

        std::array<vk::WriteDescriptorSet, 2> descriptorWrites = {
            vk::WriteDescriptorSet(*m_descriptorSets.at(i), 0, 0, 1,
                    vk::DescriptorType::eUniformBuffer,
                    nullptr, &bufferInfo),
            vk::WriteDescriptorSet(*m_descriptorSets.at(i), 1, 0, 1,
                    vk::DescriptorType::eCombinedImageSampler,
                    &imageInfo, nullptr) };
        m_device->updateDescriptorSets(descriptorWrites, {});
    }
}

void Render::createCommandBuffers(int index)
{
    m_commandBuffers = m_device->allocateCommandBuffersUnique(
            vk::CommandBufferAllocateInfo(*m_commandPool,
                                          vk::CommandBufferLevel::ePrimary,
                                          (uint32_t)m_swapChainFramebuffers.size()));

    for (size_t i = 0; i < m_commandBuffers.size(); i++) {
        m_commandBuffers.at(i)->begin(
            vk::CommandBufferBeginInfo(
                vk::CommandBufferUsageFlagBits::eSimultaneousUse));

        vk::ClearValue clearColor(
                vk::ClearColorValue(
                    std::array<float, 4>({0.0f, 0.0f, 0.0f, 1.0f})));
        m_commandBuffers.at(i)->beginRenderPass(
            vk::RenderPassBeginInfo(*m_renderPass,
                                    *m_swapChainFramebuffers.at(i),
                                    vk::Rect2D(vk::Offset2D(0, 0),
                                               m_swapChainExtent),
                                    1, &clearColor),
            vk::SubpassContents::eInline);
        m_commandBuffers.at(i)->bindPipeline(vk::PipelineBindPoint::eGraphics,
                                             *m_graphicsPipeline);

        for (int xx = 0; xx < m_uVertexBuffers.size(); xx++) {
            vk::DeviceSize offset = 0;
            m_commandBuffers.at(i)->bindVertexBuffers(0, *m_uVertexBuffers[xx], offset);
            // m_commandBuffers.at(i)->bindVertexBuffers(0, *m_uVertexBuffers[index], offset);
            // m_commandBuffers.at(i)->bindIndexBuffer(*m_indexBuffer, 0,
            // vk::IndexType::eUint16);

            m_commandBuffers.at(i)->bindDescriptorSets(
                    vk::PipelineBindPoint::eGraphics, *m_pipelineLayout, 0, 1,
                    &*m_descriptorSets.at(i), 0, nullptr);

            // m_commandBuffers.at(i)->draw(static_cast<uint32_t>(vertices[0].size()),
                    // 1, 0, 0);
            m_commandBuffers.at(i)->draw(static_cast<uint32_t>(vertices[0].size()),
                    1, 0, xx);
        }
        m_commandBuffers.at(i)->endRenderPass();
        m_commandBuffers.at(i)->end();
    }
}

void Render::createSyncObjects()
{
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        m_imageAvailableSemaphores.push_back(
            m_device->createSemaphoreUnique(vk::SemaphoreCreateInfo()));

        m_renderFinishedSemaphores.push_back(
            m_device->createSemaphoreUnique(vk::SemaphoreCreateInfo()));

        m_inFlightFences.push_back(
            m_device->createFenceUnique(
                vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled)));
    }
}

void Render::framebufferResizeCallback(GLFWwindow *window,
                                       int width, int height)
{
    auto app = reinterpret_cast<Render*>(glfwGetWindowUserPointer(window));
    app->setFbResized();
}

void Render::cleanupSwapChain()
{
    for (auto &framebuffer : m_swapChainFramebuffers) {
        m_device->destroyFramebuffer(*framebuffer);
    }

    for (auto &commandBuffer : m_commandBuffers) {
        commandBuffer.reset();
    }

    m_device->destroyPipeline(*m_graphicsPipeline);
    m_device->destroyPipelineLayout(*m_pipelineLayout);
    m_device->destroyRenderPass(*m_renderPass);

    for (auto &imageView : m_swapChainImageViews) {
        m_device->destroyImageView(*imageView);
    }

    m_device->destroySwapchainKHR(*m_swapChain);

    for (size_t i = 0; i < m_swapChainImages.size(); i++) {
        m_uniformBuffers.at(i).reset();
        m_uniformBuffersMemory.at(i).reset();
    }

    m_descriptorPool.reset();
}

void Render::recreateSwapChain(int index)
{
    int width = 0, height = 0;
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(m_window, &width, &height);
        glfwWaitEvents();
    }

    m_device->waitIdle();

    cleanupSwapChain();

    createSwapChain();
    createImageViews();
    createRenderPass();
    createGraphicsPipeline();
    createFramebuffers();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffers(index);
}

