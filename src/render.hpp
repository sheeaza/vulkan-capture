#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <array>
#include <cstddef>

class Render
{
public:
    struct Vertex
    {
        glm::vec2 pos;
        glm::vec2 texCoord;

        static vk::VertexInputBindingDescription getBindingDescription()
        {
            return vk::VertexInputBindingDescription(0, sizeof(Vertex),
                                                vk::VertexInputRate::eVertex);
        }

        static std::array<vk::VertexInputAttributeDescription, 2>
            getAttributeDescriptions()
        {
            std::array<vk::VertexInputAttributeDescription, 2> attributeDescriptions = {};

            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = vk::Format::eR32G32Sfloat;
            attributeDescriptions[0].offset = offsetof(Vertex, pos);

            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = vk::Format::eR32G32Sfloat;
            attributeDescriptions[1].offset = offsetof(Vertex, texCoord);

            return attributeDescriptions;
        }
    };

    struct UniformBufferObject
    {
        alignas(16) glm::mat4 model;
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
    };

    void init();
    void updateTexture(int index);
    void getBufferAddrs(std::vector<void *> &bufferMaps);
    void render();
    bool checkValidationLayerSupport();
    bool shouldStop()
    {
        return !glfwWindowShouldClose(m_window);
    }
    void setFbResized()
    {
        framebufferResized = true;
    }
    void waitIdle()
    {
        m_device->waitIdle();
    }

    virtual ~Render();

private:
    static const std::vector<const char *> validationLayers;

    GLFWwindow *m_window;
    vk::UniqueInstance m_instance;
    vk::UniqueDebugReportCallbackEXT m_debugCallback;
    vk::UniqueSurfaceKHR m_surface;
    vk::PhysicalDevice m_physicalDevice;
    vk::UniqueDevice m_device;
    vk::Queue m_graphicsQueue;
    vk::Queue m_presentQueue;
    vk::UniqueSwapchainKHR m_swapChain;
    std::vector<vk::Image> m_swapChainImages;
    vk::Format m_swapChainImageFormat;
    vk::Extent2D m_swapChainExtent;
    std::vector<vk::UniqueImageView> m_swapChainImageViews;
    std::vector<vk::UniqueFramebuffer> m_swapChainFramebuffers;

    vk::UniqueRenderPass m_renderPass;
    vk::UniqueDescriptorSetLayout m_descriptorSetLayout;
    vk::UniquePipelineLayout m_pipelineLayout;
    vk::UniquePipeline m_graphicsPipeline;

    vk::UniqueCommandPool m_commandPool;

    vk::UniqueImage m_utextureImage;
    vk::UniqueDeviceMemory m_utextureMem;
    vk::UniqueImageView m_utextureImageView;
    vk::UniqueSampler m_utextureSampler;
    std::vector<vk::UniqueBuffer> m_ustageBuffers;
    std::vector<vk::UniqueDeviceMemory> m_ustageMems;
    std::vector<void *> m_stageMemMaps;

    vk::UniqueBuffer m_vertexBuffer;
    vk::UniqueDeviceMemory m_vertexBufferMemory;
    vk::UniqueBuffer m_indexBuffer;
    vk::UniqueDeviceMemory m_indexBufferMemory;

    std::vector<vk::UniqueBuffer> m_uniformBuffers;
    std::vector<vk::UniqueDeviceMemory> m_uniformBuffersMemory;

    vk::UniqueDescriptorPool m_descriptorPool;
    std::vector<vk::UniqueDescriptorSet> m_descriptorSets;

    std::vector<vk::UniqueCommandBuffer> m_commandBuffers;

    std::vector<vk::UniqueSemaphore> m_imageAvailableSemaphores;
    std::vector<vk::UniqueSemaphore> m_renderFinishedSemaphores;
    std::vector<vk::UniqueFence> m_inFlightFences;
    size_t m_currentFrame = 0;
    bool framebufferResized = false;

    void initWindow();

    std::vector<const char*> getRequiredExtension();
    void createInstance();
    void setupDebugMessage();
    static VkBool32 debugCallback(VkDebugReportFlagsEXT flags,
                                  VkDebugReportObjectTypeEXT objectType,
                                  uint64_t object, size_t location,
                                  int32_t messageCode, const char* pLayerPrefix,
                                  const char* pMessage, void* pUserData);
    void createSurface();

    struct QueueFamilyIndices {
        uint32_t graphicsFamily = -1;
        uint32_t presentFamily = -1;

        bool isComplete()
        {
            return graphicsFamily >= 0 &&
                   presentFamily >= 0;
        }
    };
    void pickPhysicalDevice();
    bool isDeviceSuitable(vk::PhysicalDevice device);
    bool checkDeviceExtensionSupport(vk::PhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device);

    void createLogicalDevice();

    static const std::vector<const char *> deviceExtensions;
    struct SwapChainSupportDetails {
        vk::SurfaceCapabilitiesKHR capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> presentModes;
    };
    void createSwapChain();
    SwapChainSupportDetails querySwapChainSupport(vk::PhysicalDevice device);
    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(
            const std::vector<vk::SurfaceFormatKHR>& availableFormats);
    vk::PresentModeKHR chooseSwapPresentMode(
            const std::vector<vk::PresentModeKHR>& availablePresentModes);
    vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);

    void createImageViews();

    void createRenderPass();

    void createDescriptorSetLayout();

    void createGraphicsPipeline();
    static std::vector<char> readFile(const std::string& filename);
    vk::UniqueShaderModule createShaderModule(const std::vector<char>& code);

    void createFramebuffers();

    void createCommandPool();

    void transitionImageLayout(vk::Image image, vk::ImageLayout oldLayout,
                               vk::ImageLayout newLayout,
                               vk::PipelineStageFlags srcStageMask,
                               vk::PipelineStageFlags dstStageMask);
    void createTextureImage();
    void createTextureImageView();
    void createTextureSampler();

    uint32_t findMemoryType(uint32_t typeFilter,
                            vk::MemoryPropertyFlags properties);

    void createVertexBuffer();
    void createIndexBuffer();
    void createUniformBuffers();
    void updateUniformBuffer(uint32_t currentImage);
    void createDescriptorPool();
    void createDescriptorSets();
    void createCommandBuffers();
    void createSyncObjects();

    static void framebufferResizeCallback(GLFWwindow* window,
                                          int width, int height);
    void cleanupSwapChain();
    void recreateSwapChain();
};

