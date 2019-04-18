#include "render.hpp"

#include <vector>
#include <string>
#include <set>
#include <iostream>
#include <limits>
#include <fstream>
#include <cstring>

#define WIDTH 800
#define HEIGHT 600
static const int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<Render::Vertex> vertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
};

const std::vector<uint16_t> indices = {
    0, 1, 2, 2, 3, 0
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

Render::~Render()
{
    m_device->waitIdle();

    std::cout << __LINE__ << std::endl;
    // cleanupSwapChain();

    // for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        // vkDestroySemaphore(m_device, m_renderFinishedSemaphores.at(i), nullptr);
        // vkDestroySemaphore(m_device, m_imageAvailableSemaphores.at(i), nullptr);
        // vkDestroyFence(m_device, m_inFlightFences.at(i), nullptr);
    // }

    // vkDestroyCommandPool(m_device, m_commandPool, nullptr);

    // vkDestroyDevice(m_device, nullptr);

    glfwDestroyWindow(m_window);

    glfwTerminate();
}

int Render::init()
{
    int ret;

    initWindow();

    ret = createInstance();
    if (ret)
        return -1;

#ifndef NDDEBUG
    ret = setupDebugMessage();
    if (ret) {
        return -1;
    }
#endif

    ret = createSurface();
    if (ret) {
        std::cout << "failed to create window surface" << std::endl;
        return -1;
    }

    ret = pickPhysicalDevice();
    if (ret) {
        std::cout << "failed to pick physical device" << std::endl;
        return -1;
    }

    ret = createLogicalDevice();
    if (ret)
        return -1;

    ret = createSwapChain();
    if (ret) {
        std::cout << "failed to create swap chain" << std::endl;
        return -1;
    }

    ret = createImageViews();
    if (ret) {
        std::cout << "createImageViews failed" << std::endl;
        return -1;
    }

    ret = createRenderPass();
    if (ret) {
        std::cout << "createRenderPass failed" << std::endl;
        return -1;
    }

    ret = createGraphicsPipeline();
    if (ret) {
        std::cout << "createGraphicsPipeline failed" << std::endl;
        return -1;
    }

    ret = createFramebuffers();
    if (ret) {
        std::cout << "createFramebuffer failed" << std::endl;
        return -1;
    }

    ret = createCommandPool();
    if (ret) {
        std::cout << "createCommandPool failed" << std::endl;
        return -1;
    }

    createVertexBuffer();
    createIndexBuffer();

    ret = createCommandBuffers();
    if (ret) {
        std::cout << "createCommandBuffers failed" << std::endl;
        return -1;
    }

    ret = createSyncObjects();
    if (ret) {
        std::cout << "createSyncObjects failed" << std::endl;
        return -1;
    }
    return 0;
}

int Render::render()
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
        recreateSwapChain();
        return 0;
    } else if (result != vk::Result::eSuccess &&
               result != vk::Result::eSuboptimalKHR) {
        return -1;
    }

    vk::PipelineStageFlags waitStages[] =
        {vk::PipelineStageFlagBits::eColorAttachmentOutput};

    vk::SubmitInfo submitInfo(1, &*m_imageAvailableSemaphores.at(m_currentFrame),
                              waitStages, 1, &*m_commandBuffers.at(imageIndex),
                              1, &*m_renderFinishedSemaphores.at(m_currentFrame));

    m_device->resetFences(1, &*m_inFlightFences.at(m_currentFrame));

    result = m_graphicsQueue.submit(1, &submitInfo,
                                    *m_inFlightFences.at(m_currentFrame));
    if (result != vk::Result::eSuccess)
        return -1;

    vk::PresentInfoKHR
        presentInfo(1, &*m_renderFinishedSemaphores.at(m_currentFrame),
                    1, &*m_swapChain, &imageIndex);
    result = m_presentQueue.presentKHR(presentInfo);

    if (result == vk::Result::eErrorOutOfDateKHR ||
        result == vk::Result::eSuboptimalKHR ||
        framebufferResized) {
        framebufferResized = false;
        recreateSwapChain();
    } else if (result != vk::Result::eSuccess) {
        return -1;
    }

    m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    return 0;
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

int Render::initWindow()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    m_window = glfwCreateWindow(800, 600, "vulkan", nullptr, nullptr);
    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);

    return 0;
}

std::vector<const char*> Render::getRequiredExtension()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions,
                                        glfwExtensions + glfwExtensionCount);

#ifndef NDDEBUG
    extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif

    return extensions;
}

int Render::createInstance()
{
    int ret;

#ifndef NDDEBUG
    if (!checkValidationLayerSupport()) {
        std::cout << "validation layers not suppport!" << std::endl;
    }
#endif

    if (!glfwVulkanSupported()) {
        std::cout << "vulkan not supported!" << std::endl;
        return -1;
    }

    vk::ApplicationInfo appInfo("triangle", 1, "vulkan", 1, VK_API_VERSION_1_0);
    vk::InstanceCreateInfo instanceCreateInfo({}, &appInfo);

    ret = VK_SUCCESS;

#ifndef NDDEBUG
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

#ifndef NDDEBUG
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

    return 0;
}

int Render::setupDebugMessage()
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

    return 0;
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

int Render::createSurface()
{
    VkSurfaceKHR surface;

    int ret = glfwCreateWindowSurface(*m_instance, m_window, nullptr, &surface);

    if (ret != VK_SUCCESS) {
        return -1;
    }

    *m_surface = surface;

    return 0;
}

int Render::pickPhysicalDevice()
{
    int ret;

    std::vector<vk::PhysicalDevice> devices =
        m_instance->enumeratePhysicalDevices();

    for (const auto &device : devices) {
        if (isDeviceSuitable(device)) {
            m_physicalDevice = device;
            return 0;
        }
    }

    std::cout << "failed to find a suitable gpu" << std::endl;
    return -1;
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

    return indices.isComplete() && extensionsSupported && swapChainAdequate;
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

int Render::createLogicalDevice()
{
    int ret;
    QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice);

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies  = {indices.graphicsFamily,
                                               indices.presentFamily};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        queueCreateInfos.push_back(vk::DeviceQueueCreateInfo(
                                   {}, queueFamily, 1, &queuePriority));
    }

    vk::DeviceCreateInfo
        createInfo({}, static_cast<uint32_t>(queueCreateInfos.size()),
                   queueCreateInfos.data(),
#ifndef NDDEBUG
                   static_cast<uint32_t>(validationLayers.size()),
                   validationLayers.data(),
#else
                   0, nullptr,
#endif
                   static_cast<uint32_t>(deviceExtensions.size()),
                   deviceExtensions.data());
    m_device = m_physicalDevice.createDeviceUnique(createInfo);

    m_graphicsQueue = m_device->getQueue(indices.graphicsFamily, 0);
    m_presentQueue = m_device->getQueue(indices.presentFamily, 0);

    return 0;
}

int Render::createSwapChain()
{
    int ret;

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

    return 0;
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

int Render::createImageViews()
{
    int ret;

    m_swapChainImageViews.reserve(m_swapChainImages.size());

    for (size_t i = 0; i < m_swapChainImages.size(); i++) {
        vk::ImageViewCreateInfo
            createInfo({}, m_swapChainImages.at(i), vk::ImageViewType::e2D,
                       m_swapChainImageFormat,
                       vk::ComponentMapping(),
                       vk::ImageSubresourceRange(
                           vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

        m_swapChainImageViews.push_back(
                m_device->createImageViewUnique(createInfo));
    }

    return 0;
}

int Render::createDescriptorSetLayout()
{

}

int Render::createRenderPass()
{
    int ret;

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

    return 0;
}

int Render::createGraphicsPipeline()
{
    int ret;

    auto vertShaderCode = readFile("shader.vert.spv");
    auto fragShaderCode = readFile("shader.frag.spv");

    if (vertShaderCode.size() == 0 || fragShaderCode.size() == 0)
        return -1;

    vk::UniqueShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    vk::UniqueShaderModule fragShaderModule = createShaderModule(fragShaderCode);
    if (!vertShaderModule || !fragShaderModule)
        return -1;

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
            {}, vk::PrimitiveTopology::eTriangleList);

    vk::Viewport viewport(0.0f, 0.0f, (float)m_swapChainExtent.width,
                          (float)m_swapChainExtent.height, 0.0f, 1.0f);

    vk::Rect2D scissor(vk::Offset2D(), m_swapChainExtent);

    vk::PipelineViewportStateCreateInfo viewportState(
            {}, 1, &viewport, 1, &scissor);

    vk::PipelineRasterizationStateCreateInfo rasterizer(
            {}, VK_FALSE, VK_FALSE, vk::PolygonMode::eFill,
            vk::CullModeFlagBits::eBack, vk::FrontFace::eClockwise,
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
            vk::PipelineLayoutCreateInfo());

    vk::GraphicsPipelineCreateInfo pipelineInfo(
            {}, 2, shaderStages, &vertexInputInfo, &inputAssembly,
            nullptr, &viewportState, &rasterizer, &multisampling,
            nullptr, &colorBlending, nullptr, *m_pipelineLayout,
            *m_renderPass);

    m_graphicsPipeline = m_device->createGraphicsPipelineUnique(nullptr,
                                                                pipelineInfo);

    return 0;
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

int Render::createFramebuffers()
{
    int ret;

    m_swapChainFramebuffers.reserve(m_swapChainImageViews.size());

    for (auto const &view : m_swapChainImageViews) {
        m_swapChainFramebuffers.push_back(
            m_device->createFramebufferUnique(
                vk::FramebufferCreateInfo({}, *m_renderPass, 1, &*view,
                                          m_swapChainExtent.width,
                                          m_swapChainExtent.height,
                                          1)));
    }

    return 0;
}

int Render::createCommandPool()
{
    int ret;

    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(m_physicalDevice);

    m_commandPool = m_device->createCommandPoolUnique(
            vk::CommandPoolCreateInfo({}, queueFamilyIndices.graphicsFamily));

    return 0;
}

int Render::createVertexBuffer()
{
    uint32_t bufferSize = sizeof(vertices[0]) * vertices.size();

    m_vertexBuffer = m_device->createBufferUnique(
            vk::BufferCreateInfo({}, bufferSize,
                                 vk::BufferUsageFlagBits::eVertexBuffer));
    vk::MemoryRequirements memRequirements =
        m_device->getBufferMemoryRequirements(*m_vertexBuffer);

    uint32_t memoryTypeIndex =
        findMemoryType(memRequirements.memoryTypeBits,
                       vk::MemoryPropertyFlagBits::eHostVisible |
                       vk::MemoryPropertyFlagBits::eHostCoherent);
    m_vertexBufferMemory = m_device->allocateMemoryUnique(
            vk::MemoryAllocateInfo(memRequirements.size, memoryTypeIndex));

    void *data = m_device->mapMemory(*m_vertexBufferMemory, 0,
                                     memRequirements.size);
    memcpy(data, vertices.data(), bufferSize);
    m_device->unmapMemory(*m_vertexBufferMemory);

    m_device->bindBufferMemory(*m_vertexBuffer, *m_vertexBufferMemory, 0);

    return 0;
}

int Render::createIndexBuffer()
{
    uint32_t bufferSize = sizeof(indices[0]) * indices.size();

    m_indexBuffer = m_device->createBufferUnique(
            vk::BufferCreateInfo({}, bufferSize,
                                 vk::BufferUsageFlagBits::eIndexBuffer));
    vk::MemoryRequirements memRequirements =
        m_device->getBufferMemoryRequirements(*m_indexBuffer);

    uint32_t memoryTypeIndex =
        findMemoryType(memRequirements.memoryTypeBits,
                       vk::MemoryPropertyFlagBits::eHostVisible |
                       vk::MemoryPropertyFlagBits::eHostCoherent);
    m_indexBufferMemory = m_device->allocateMemoryUnique(
            vk::MemoryAllocateInfo(memRequirements.size, memoryTypeIndex));

    void *data = m_device->mapMemory(*m_indexBufferMemory, 0,
                                     memRequirements.size);
    memcpy(data, indices.data(), bufferSize);
    m_device->unmapMemory(*m_indexBufferMemory);

    m_device->bindBufferMemory(*m_indexBuffer, *m_indexBufferMemory, 0);

    return 0;
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

    return -1;
}

int Render::createCommandBuffers()
{
    int ret;

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
        vk::DeviceSize offset = 0;
        m_commandBuffers.at(i)->bindVertexBuffers(0, *m_vertexBuffer, offset);
        m_commandBuffers.at(i)->bindIndexBuffer(*m_indexBuffer, 0,
                                                vk::IndexType::eUint16);

        m_commandBuffers.at(i)->drawIndexed(static_cast<uint32_t>(indices.size()),
                                            1, 0, 0, 0);
        m_commandBuffers.at(i)->endRenderPass();
        m_commandBuffers.at(i)->end();
    }

    return 0;
}

int Render::createSyncObjects()
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

    return 0;
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
}

void Render::recreateSwapChain()
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
    createCommandBuffers();
}

