/**
 *
 * @brief Buffer class.
 * @author Ferran Barba, ESAT 2025-2026
 * @subject Engine Programming
 *
 */

#ifndef __VULKAN_API_HPP__
#define __VULKAN_API_HPP__ 1

#include <optional>
#include <vulkan/vulkan.h>
#include <vector>
#include "../deps/imgui/imgui_impl_vulkan.h"
#include <../include/MentatEngine/GraphicAPI.hpp>
#include <functional>

namespace ME {
    struct QueueFamilyIndices {
        std::optional<unsigned int> graphicsFamily;
        std::optional<unsigned int> presentFamily;
        bool IsCompleted() {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    class VulkanAPI : public GraphicAPI {
        public:
            void Init(GLFWwindow* window, bool usingImGui) override;
            void Draw(float dt) override;
            void StartRenderers() override;
            void Cleanup() override;

            void CleanupSwapChain();
            void RecreateSwapChain();
            static void FrameBufferResizeCallback(GLFWwindow* window, int height, int width);

            void CreateInstance();
            void PickPhysicalDevice();
            void CreateSurface();
            void CreateLogicalDevice();
            void CreateSwapChain();
            void CreateImageViews();
            void CreateRenderPass();
            void CreateGraphicsPipeline();
            void CreateFramebuffers();
            void CreateCommandPool();
            void CreateCommandBuffers();
            void CreateSyncObjects();
            void DrawFrame();

            bool IsDeviceSuitable(VkPhysicalDevice device);
            bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
            VkShaderModule CreateShaderModule(const std::vector<char>& code);
            QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
            VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
            VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
            VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
            SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
            bool framebufferResized_;

        protected:
            void InitImGuiEnvironment(GLFWwindow* window) override;
            void ImGuiFrame() override;
            void ImGuiShutdown() override;

        private:
            VkInstance instance_;
            VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;
            VkDevice device_;
            VkQueue graphicsQueue_, presentQueue_;
            VkSurfaceKHR surface_;
            VkSwapchainKHR swapChain_;
            std::vector<VkImage> swapChainImages_;
            VkFormat swapChainImageFormat_;
            VkExtent2D swapChainExtent_;
            std::vector<const char*> deviceExtensions_;
            std::vector<VkImageView> swapChainImageViews_;
            std::vector<VkFramebuffer> swapChainFrameBuffers_;
            VkRenderPass renderPass_;
            VkPipelineLayout pipelineLayout_;
            VkPipeline graphicsPipeline_;

            VkCommandPool commandPool_;
            std::vector<VkCommandBuffer> commandBuffers_;
            std::vector<VkSemaphore> imageAvailableSemaphores_;
            std::vector<VkSemaphore> renderFinishedSemaphores_;
            std::vector<VkFence> inFlightFences_;
            std::vector<VkFence> imagesInFlight_;
            unsigned long long currentFrame = 0;
            const int MAX_FRAMES_IN_FLIGHT = 2;
            GLFWwindow* window_;


            // IMGUI
            VkDescriptorPool CreateImGuiDescriptorTool();
            VkDescriptorPool imguiDescriptorPool_;
        };
}

#endif // __VULKAN_API_HPP__