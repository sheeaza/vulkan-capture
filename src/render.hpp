#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
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
        glm::vec3 color;

        static VkVertexInputBindingDescription getBindingDescription()
        {
            VkVertexInputBindingDescription bindingDescription = {};
            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(Vertex);
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return bindingDescription;
        }

        static std::array<VkVertexInputAttributeDescription, 2>
            getAttributeDescriptions()
        {
            std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};

            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[0].offset = offsetof(Vertex, pos);

            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[1].offset = offsetof(Vertex, color);

            return attributeDescriptions;
        }
    };

    struct UniformBufferObject
    {
        alignas(16) glm::mat4 model;
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
    };

    int init();
    int render();
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
        vkDeviceWaitIdle(m_device);
    }

    virtual ~Render();

private:
    static const std::vector<const char *> validationLayers;

    GLFWwindow *m_window;
    vk::UniqueInstance m_instance;
    vk::UniqueDebugReportCallbackEXT m_debugCallback;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device;
    VkQueue m_graphicsQueue;
    VkQueue m_presentQueue;
    VkSurfaceKHR m_surface;
    VkSwapchainKHR m_swapChain;
    std::vector<VkImage> m_swapChainImages;
    VkFormat m_swapChainImageFormat;
    VkExtent2D m_swapChainExtent;
    std::vector<VkImageView> m_swapChainImageViews;
    std::vector<VkFramebuffer> m_swapChainFramebuffers;

    VkRenderPass m_renderPass;
    VkPipelineLayout m_pipelineLayout;
    VkPipeline m_graphicsPipeline;

    VkCommandPool m_commandPool;
    std::vector<VkCommandBuffer> m_commandBuffers;

    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inFlightFences;
    size_t m_currentFrame = 0;
    bool framebufferResized = false;

    int initWindow();

    std::vector<const char*> getRequiredExtension();
    int createInstance();
    int setupDebugMessage();
    void destroyDebugMessage();
    static VkBool32 debugCallback(VkDebugReportFlagsEXT flags,
                                  VkDebugReportObjectTypeEXT objectType,
                                  uint64_t object, size_t location,
                                  int32_t messageCode, const char* pLayerPrefix,
                                  const char* pMessage, void* pUserData);
    int createSurface();

    struct QueueFamilyIndices {
        uint32_t graphicsFamily = -1;
        uint32_t presentFamily = -1;

        bool isComplete()
        {
            return graphicsFamily >= 0 &&
                   presentFamily >= 0;
        }
    };
    int pickPhysicalDevice();
    bool isDeviceSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

    int createLogicalDevice();

    static const std::vector<const char *> deviceExtensions;
    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };
    int createSwapChain();
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(
            const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(
            const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    int createImageViews();

    int createRenderPass();

    int createDescriptorSetLayout();

    int createGraphicsPipeline();
    static std::vector<char> readFile(const std::string& filename);
    VkShaderModule createShaderModule(const std::vector<char>& code);

    int createFramebuffers();

    int createCommandPool();
    int createCommandBuffers();
    int createSyncObjects();

    static void framebufferResizeCallback(GLFWwindow* window,
                                          int width, int height);
    void cleanupSwapChain();
    void recreateSwapChain();
};

