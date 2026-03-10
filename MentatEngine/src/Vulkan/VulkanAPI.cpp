/**
 *
 * @brief Window opener with GLFW.
 * @author Ferran Barba, ESAT 2025-2026
 * @subject Engine Programming
 *
 */

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>
#include <../include/MentatEngine/Vulkan/VulkanAPI.hpp>
#include "../Include/MentatEngine/ImGuiManager.hpp"
#include <iostream>
#include <set>
#include <algorithm>
#include <fstream>

namespace ME {

    void VulkanAPI::Init(GLFWwindow* window, bool usingImGui) {
		window_ = window;
		imguiDescriptorPool_ = VK_NULL_HANDLE;
		glfwSetWindowUserPointer(window_, this);
		glfwSetFramebufferSizeCallback(window_, FrameBufferResizeCallback);

		CreateInstance();
		CreateSurface();
		PickPhysicalDevice();
		CreateLogicalDevice();
		CreateSwapChain();
		CreateImageViews();
		CreateRenderPass();
		CreateGraphicsPipeline();
		CreateFramebuffers();
		CreateCommandPool();
		CreateCommandBuffers();
		CreateSyncObjects();

		usingImGui_ = usingImGui;
		InitImGuiEnvironment(window);
    }

    void VulkanAPI::Draw(float dt)
	{
		ImGuiFrame();
		DrawFrame();
    }

	void VulkanAPI::StartRenderers() 
	{
		// AQUI SE RELLENARAN LOS RENDERIZADORES
	}

    void VulkanAPI::Cleanup() {
		
		CleanupSwapChain();
		ImGuiShutdown();
		
		for (unsigned long long i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroySemaphore(device_, renderFinishedSemaphores_[i], nullptr);
			vkDestroySemaphore(device_, imageAvailableSemaphores_[i], nullptr);
			vkDestroyFence(device_, inFlightFences_[i], nullptr);
		}

		vkDestroyCommandPool(device_, commandPool_, nullptr);

		vkDestroyDevice(device_, nullptr);
		vkDestroySurfaceKHR(instance_, surface_, nullptr);
		vkDestroyInstance(instance_, nullptr);
    }

	void VulkanAPI::FrameBufferResizeCallback(GLFWwindow* window, int height, int width) {
		auto app = reinterpret_cast<ME::VulkanAPI *>(glfwGetWindowUserPointer(window));
		app->framebufferResized_ = true;
	}

	void VulkanAPI::CleanupSwapChain() {

		for (auto framebuffer : swapChainFrameBuffers_)
		{
			vkDestroyFramebuffer(device_, framebuffer, nullptr);
		}

		vkFreeCommandBuffers(device_, commandPool_, static_cast<unsigned int>(commandBuffers_.size()), commandBuffers_.data());

		vkDestroyPipeline(device_, graphicsPipeline_, nullptr);
		vkDestroyPipelineLayout(device_, pipelineLayout_, nullptr);
		vkDestroyRenderPass(device_, renderPass_, nullptr);

		for (auto imageView : swapChainImageViews_)
		{
			vkDestroyImageView(device_, imageView, nullptr);
		}

		vkDestroySwapchainKHR(device_, swapChain_, nullptr);
	}

	void VulkanAPI::RecreateSwapChain() {
		int width = 0, height = 0;
		glfwGetFramebufferSize(window_, &width, &height);
	
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(window_, &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(device_);
		CleanupSwapChain();

		CreateSwapChain();
		CreateImageViews();
		CreateRenderPass();
		CreateGraphicsPipeline();
		CreateFramebuffers();
		CreateCommandBuffers();

		imagesInFlight_.resize(swapChainImages_.size(), VK_NULL_HANDLE);

	}

	// <-- ZONA DE INICIALIZACIÓN DE VULKAN (appInfo y createInfo)

	void VulkanAPI::CreateInstance() { // <- FUNCIÓN PÚBLICA

		deviceExtensions_.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Vulkan Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "MentatEngine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		unsigned int glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		createInfo.enabledExtensionCount = glfwExtensionCount;
		createInfo.ppEnabledExtensionNames = glfwExtensions;

		createInfo.enabledLayerCount = 0;

		if (vkCreateInstance(&createInfo, nullptr, &instance_) != VK_SUCCESS) {
			printf("%s", "ERROR LOADING VULKAN\n");
		} else {
			printf("%s", "VULKAN LOADED\n");
		}

		std::vector<VkExtensionProperties> vkExtensions_(glfwExtensionCount);

		vkEnumerateInstanceExtensionProperties(nullptr, &glfwExtensionCount, vkExtensions_.data());

		for (const auto& extension : vkExtensions_) {
			std::cout << '\t' << extension.extensionName << std::endl;
		}
	}

	// <-- FINAL DE ZONA DE INICIALIZACIÓN DE VULKAN (appInfo y createInfo)



	// <-- ZONA DE COMPROBACIÓN DE DISPOSITIVOS FÍSICOS

	QueueFamilyIndices VulkanAPI::FindQueueFamilies(VkPhysicalDevice device) {
		QueueFamilyIndices indices;
		unsigned int queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies) {
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphicsFamily = i;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface_, &presentSupport);

			if (presentSupport) {
				indices.presentFamily = i;
			}

			if (indices.IsCompleted()) {
				break;
			}

			i++;
		}

		return indices;
	}

	bool VulkanAPI::IsDeviceSuitable(VkPhysicalDevice device) {
		QueueFamilyIndices indices = FindQueueFamilies(device);

		bool extensionsSupported = CheckDeviceExtensionSupport(device);

		bool swapChainAdequate = false;
		if (extensionsSupported)
		{
			SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		return indices.IsCompleted() && extensionsSupported && swapChainAdequate;
	}

	void VulkanAPI::PickPhysicalDevice() { // <- FUNCIÓN PÚBLICA
		unsigned int deviceCount = 0;
		vkEnumeratePhysicalDevices(instance_, &deviceCount, nullptr);

		if (deviceCount == 0) {
			throw std::runtime_error("FAILED TO FIND ANY GPUS WITH VULKAN\n");
		}

		std::vector<VkPhysicalDevice> devices_(deviceCount);
		vkEnumeratePhysicalDevices(instance_, &deviceCount, devices_.data());

		for (const auto device : devices_) {
			if (IsDeviceSuitable(device)) {
				physicalDevice_ = device;
				printf("%s\n", "GRAPHICS FOUND");
				break;
			}
		}

		if (physicalDevice_ == VK_NULL_HANDLE) {
			throw std::runtime_error("FAILED TO FIND ANY SUITABLE GPU\n");
		}
	}

	// <-- FINAL DE ZONA DE COMPROBACIÓN DE DISPOSITIVOS FÍSICOS



	// <-- ZONA DE CREACIÓN DE SURFACE

	void VulkanAPI::CreateSurface() {
		VkResult res = glfwCreateWindowSurface(instance_, window_, nullptr, &surface_);
		if (res != VK_SUCCESS) {
			std::cerr << "FAILED TO CREATE A WINDOW SURFACE, VkResult = " << res << std::endl;
			throw std::runtime_error("FAILED TO CREATE A WINDOW SURFACE");
		} else {
			std::cout << "VULKAN SURFACE CREATED" << std::endl;
		}
	}

	// <-- FINAL DE ZONA DE CREACIÓN DE SURFACE



	// <-- ZONA DE INICIALIZACIÓN DE DISPOSITIVOS LÓGICOS

	void VulkanAPI::CreateLogicalDevice() {
		QueueFamilyIndices indices = FindQueueFamilies(physicalDevice_);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<unsigned int> uniqueQueueFamilies = {
			indices.graphicsFamily.value(), indices.presentFamily.value()
		};

		float queuePriority = 1.0f;

		for (unsigned int queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo{ };
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{ };

		VkDeviceCreateInfo createInfo{ };
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<unsigned int>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = static_cast<unsigned int>(deviceExtensions_.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions_.data();

		// TODO ENABLE LAYERS?
		createInfo.enabledLayerCount = 0;

		if (vkCreateDevice(physicalDevice_, &createInfo, nullptr, &device_) != VK_SUCCESS) {
			throw std::runtime_error("FAILED TO CREATE LOGICAL DEVICE");
		}

		vkGetDeviceQueue(device_, indices.graphicsFamily.value(), 0, &graphicsQueue_);
		vkGetDeviceQueue(device_, indices.presentFamily.value(), 0, &presentQueue_);
		printf("%s\n", "LOGICAL DEVICE CREATED");
	}

	// <-- FINAL DE ZONA DE INICIALIZACIÓN DE DISPOSITIVOS LÓGICOS



	// <-- ZONA DE SWAP CHAIN

	void VulkanAPI::CreateSwapChain() {
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(physicalDevice_);

		VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
		{
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface_;

		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		QueueFamilyIndices indices = FindQueueFamilies(physicalDevice_);
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		if (indices.graphicsFamily != indices.presentFamily)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;

		// createInfo.oldSwapchain = VK_NULL_HANDLE; <- removed in order to update the swap chain

		if (vkCreateSwapchainKHR(device_, &createInfo, nullptr, &swapChain_) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create swap chain!");
		}
		else {
			printf("%s", "SWAP CHAIN CREATED\n");
		}

		vkGetSwapchainImagesKHR(device_, swapChain_, &imageCount, nullptr);
		swapChainImages_.resize(imageCount);
		vkGetSwapchainImagesKHR(device_, swapChain_, &imageCount, swapChainImages_.data());

		swapChainImageFormat_ = surfaceFormat.format;
		swapChainExtent_ = extent;
	}

	VkSurfaceFormatKHR VulkanAPI::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& availableFormat : availableFormats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	VkPresentModeKHR VulkanAPI::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		for (const auto& availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return availablePresentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D VulkanAPI::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != UINT32_MAX)
		{
			return capabilities.currentExtent;
		}
		else
		{
			int width, height;
			glfwGetFramebufferSize(window_, &width, &height);

			VkExtent2D actualExtent =
			{
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}

	SwapChainSupportDetails VulkanAPI::QuerySwapChainSupport(VkPhysicalDevice device)
	{
		SwapChainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface_, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, nullptr);

		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, nullptr);

		if (presentModeCount != 0)
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	

	void VulkanAPI::InitImGuiEnvironment(GLFWwindow* window)
	{
		if (!usingImGui_) return;

		IMGUI_CHECKVERSION();
		if (ImGui::GetCurrentContext() == nullptr)
			ImGui::CreateContext();
		ImGui::StyleColorsDark();

		ImGui_ImplGlfw_InitForVulkan(window, true);

		if (imguiDescriptorPool_ == VK_NULL_HANDLE)
			imguiDescriptorPool_ = CreateImGuiDescriptorTool();

		QueueFamilyIndices qfi = FindQueueFamilies(physicalDevice_);

		ImGui_ImplVulkan_InitInfo init_info{};
		init_info.ApiVersion = VK_API_VERSION_1_2; // o el de tu app
		init_info.Instance = instance_;
		init_info.PhysicalDevice = physicalDevice_;
		init_info.Device = device_;
		init_info.QueueFamily = qfi.graphicsFamily.value();
		init_info.Queue = graphicsQueue_;
		init_info.DescriptorPool = imguiDescriptorPool_; // usamos nuestro pool
		init_info.DescriptorPoolSize = 0;                     // 0 = no crear pool interno
		init_info.MinImageCount = static_cast<uint32_t>(swapChainImages_.size());
		init_info.ImageCount = static_cast<uint32_t>(swapChainImages_.size());
		init_info.PipelineCache = VK_NULL_HANDLE;
		init_info.Allocator = nullptr;
		init_info.CheckVkResultFn = nullptr;

		// pipeline para el viewport principal
		init_info.PipelineInfoMain.RenderPass = renderPass_;
		init_info.PipelineInfoMain.Subpass = 0;
		init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

		ImGui_ImplVulkan_Init(&init_info);
	}

	void VulkanAPI::ImGuiFrame()
	{
		if (!usingImGui_) return;

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGuiManager::DrawRegisteredWindows();

		ImGui::Render();
	}

	void VulkanAPI::ImGuiShutdown()
	{
		if (!usingImGui_) return;

		if (device_ != VK_NULL_HANDLE) vkDeviceWaitIdle(device_);

		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();

		if (ImGui::GetCurrentContext() != nullptr)
			ImGui::DestroyContext();

		if (imguiDescriptorPool_ != VK_NULL_HANDLE) {
			vkDestroyDescriptorPool(device_, imguiDescriptorPool_, nullptr);
			imguiDescriptorPool_ = VK_NULL_HANDLE;
		}
	}

	VkDescriptorPool VulkanAPI::CreateImGuiDescriptorTool()
	{
		VkDescriptorPoolSize pool_sizes[] = {
		{ VK_DESCRIPTOR_TYPE_SAMPLER,                1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       1000 }
		};

		VkDescriptorPoolCreateInfo pool_info{};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT; // necesario para liberar sets individuales
		pool_info.maxSets = 1000 * static_cast<uint32_t>(std::size(pool_sizes));
		pool_info.poolSizeCount = static_cast<uint32_t>(std::size(pool_sizes));
		pool_info.pPoolSizes = pool_sizes;

		VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
		if (vkCreateDescriptorPool(device_, &pool_info, nullptr, &descriptorPool) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create ImGui descriptor pool");
		}
		else {
			printf("%s", "IMGUI DESCRIPTOR POOL CREATED\n");
		}

		return descriptorPool;
	}

	bool VulkanAPI::CheckDeviceExtensionSupport(VkPhysicalDevice device)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions_.begin(), deviceExtensions_.end());

		for (const auto& extension : availableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	// <-- FINAL DE ZONA DE SWAP CHAIN



	// <-- ZONA DE CREATE IMAGE VIEW

	void VulkanAPI::CreateImageViews() {
		swapChainImageViews_.resize(swapChainImages_.size());

		for (unsigned long long i = 0; i < swapChainImages_.size(); i++) {
			VkImageViewCreateInfo createInfo{ };
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = swapChainImages_[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = swapChainImageFormat_;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(device_, &createInfo, nullptr, &swapChainImageViews_[i]) != VK_SUCCESS) {
				printf("%s", "ERROR CREATING IMAGE VIEWS\n");
			} else {
				printf("%s", "IMAGE VIEWS CREATED\n");
			}
		}
	}

	// <-- FINAL DE ZONA DE CREATE IMAGE VIEW


	// <-- ZONA DE CREATE RENDER PASS

	void VulkanAPI::CreateRenderPass() {
		VkAttachmentDescription colorAttachment{ };
		colorAttachment.format = swapChainImageFormat_;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef{ };
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{ };
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkRenderPassCreateInfo renderPassInfo{ };
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;

		if (vkCreateRenderPass(device_, &renderPassInfo, nullptr, &renderPass_) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create pass");
		}
		else {
			printf("%s", "RENDER PASS CREATED\n");
		}
	}

	// <-- FINAL DE ZONA DE CREATE RENDER PASS



	// <-- ZONA DE CREATE GRAPHICS PIPELINE

	static std::vector<char> ReadFile(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open())
		{
			throw std::runtime_error("Failed to open file");
		} else {
			printf("%s%s%s", "FILE WITH ROUTE: ", filename.c_str(), " FOUND\n");
		}

		unsigned long long fileSize = (unsigned long long)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	void VulkanAPI::CreateGraphicsPipeline() {
		auto vertShaderCode = ReadFile("../../include/MentatEngine/Vulkan/res/vert.spv");
		auto fragShaderCode = ReadFile("../../include/MentatEngine/Vulkan/res/frag.spv");

		VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
		VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{ };
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{ };
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{ };
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.vertexAttributeDescriptionCount = 0;

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{ };
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport{ };
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float) swapChainExtent_.width;
		viewport.height = (float) swapChainExtent_.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{ };
		scissor.offset = { 0, 0 };
		scissor.extent = swapChainExtent_;

		VkPipelineViewportStateCreateInfo viewportState{ };
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer{ };
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisampling{ };
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineColorBlendAttachmentState colorBlendAttachment{ };
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlending{ };
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{ };
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pushConstantRangeCount = 0;

		if (vkCreatePipelineLayout(device_, &pipelineLayoutInfo, nullptr, &pipelineLayout_) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to Create Pipeline Layout");
		} else {
			printf("%s", "PIPELINE LAYOUT CREATED\n");
		}

		VkGraphicsPipelineCreateInfo pipelineInfo{ };
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.layout = pipelineLayout_;
		pipelineInfo.renderPass = renderPass_;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		if (vkCreateGraphicsPipelines(device_, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline_) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create a graphics pipeline");
		} else {
			printf("%s", "GRAPHICS PIPELINE CREATED\n");
		}

		vkDestroyShaderModule(device_, fragShaderModule, nullptr);
		vkDestroyShaderModule(device_, vertShaderModule, nullptr);
	}

	VkShaderModule VulkanAPI::CreateShaderModule(const std::vector<char>& code) {
		VkShaderModuleCreateInfo createInfo{ };
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const unsigned int*>(code.data());

		VkShaderModule shaderModule;

		if (vkCreateShaderModule(device_, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed To Create shader module");
		} else {
			printf("%s", "SHADER MODULE CREATED\n");
		}

		return shaderModule;
	}

	// <-- ZONA DE CREATE GRAPHICS PIPELINE



	// <-- ZONA DE CREATE FRAME BUFFERS 

	void VulkanAPI::CreateFramebuffers() {
		swapChainFrameBuffers_.resize(swapChainImageViews_.size());

		for (unsigned long long i = 0; i < swapChainImageViews_.size(); i++)
		{
			VkImageView attachments[] =
			{
				swapChainImageViews_[i]
			};

			VkFramebufferCreateInfo framebufferInfo{ };
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass_;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = swapChainExtent_.width;
			framebufferInfo.height = swapChainExtent_.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(device_, &framebufferInfo, nullptr, &swapChainFrameBuffers_[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create framebuffers");
			} else {
				printf("%s", "FRAME BUFFER CREATED\n");
			}
		}
	}

	// <-- FINAL DE ZONA DE CREATE FRAME BUFFERS 



	// <-- ZONA DE CREATE COMMAND POOL

	void VulkanAPI::CreateCommandPool() {
		QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(physicalDevice_);

		VkCommandPoolCreateInfo poolInfo{ };
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

		if (vkCreateCommandPool(device_, &poolInfo, nullptr, &commandPool_) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create command pool");
		} else {
			printf("%s", "COMMAND POOL CREATED\n");
		}
	}

	// <-- FINAL DE ZONA DE CREATE COMMAND POOL



	// <-- ZONA DE CREATE COMMAND BUFFERS

	void VulkanAPI::CreateCommandBuffers() {
		/*
		
		commandBuffers_.resize(swapChainFrameBuffers_.size());

		VkCommandBufferAllocateInfo allocInfo{ };
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool_;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (unsigned int)commandBuffers_.size();

		if (vkAllocateCommandBuffers(device_, &allocInfo, commandBuffers_.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate command buffers");
		} else {
			printf("%s", "FRAME BUFFER ALLOCATED\n");
		}

		for (unsigned long long i = 0; i < commandBuffers_.size(); i++)
		{
			VkCommandBufferBeginInfo beginInfo{ };
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

			if (vkBeginCommandBuffer(commandBuffers_[i], &beginInfo) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to beging recording command buffer");
			}

			VkRenderPassBeginInfo renderPassInfo{ };
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = renderPass_;
			renderPassInfo.framebuffer = swapChainFrameBuffers_[i];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = swapChainExtent_;

			VkClearValue clearColor = { { { 0.0f, 0.0f, 0.0f, 1.0f } } };
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;

			vkCmdBeginRenderPass(commandBuffers_[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(commandBuffers_[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline_);

			vkCmdDraw(commandBuffers_[i], 3, 1, 0, 0);

			vkCmdEndRenderPass(commandBuffers_[i]);

			if (vkEndCommandBuffer(commandBuffers_[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to record command buffer");
			}
		}*/

		commandBuffers_.resize(swapChainFrameBuffers_.size());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool_;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers_.size());

		if (vkAllocateCommandBuffers(device_, &allocInfo, commandBuffers_.data()) != VK_SUCCESS)
			throw std::runtime_error("Failed to allocate command buffers");
		else
			printf("%s", "COMMAND BUFFERS ALLOCATED\n");
	}

	// <-- FINAL DE ZONA DE CREATE COMMAND BUFFERS



	// <-- ZONA DE CREATE SYNC OBJECTS

	void VulkanAPI::CreateSyncObjects() {
		// Match the semaphore sizes with the maximum number of frames in flight
		imageAvailableSemaphores_.resize(MAX_FRAMES_IN_FLIGHT);
		renderFinishedSemaphores_.resize(MAX_FRAMES_IN_FLIGHT);
		inFlightFences_.resize(MAX_FRAMES_IN_FLIGHT);
		// Match the images in flight size to the swap chain images size
		imagesInFlight_.resize(swapChainImages_.size(), VK_NULL_HANDLE);

		// Create the semaphore create info object
		VkSemaphoreCreateInfo semaphoreInfo{ };
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		// Create the fence create info object
		VkFenceCreateInfo fenceInfo{ };
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		// Create all the semaphores and fences
		for (unsigned long long i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			if (vkCreateSemaphore(device_, &semaphoreInfo, nullptr, &imageAvailableSemaphores_[i]) != VK_SUCCESS ||
				vkCreateSemaphore(device_, &semaphoreInfo, nullptr, &renderFinishedSemaphores_[i]) != VK_SUCCESS ||
				vkCreateFence(device_, &fenceInfo, nullptr, &inFlightFences_[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create synchronization objects for a frame!");
			}
		}
	}



	// <-- FINAL DE ZONA DE CREATE SYNC OBJECTS



	// <-- ZONA DE CREATE DRAW FRAME

	void VulkanAPI::DrawFrame() {
		/*
		vkWaitForFences(device_, 1, &inFlightFences_[currentFrame], VK_TRUE, UINT64_MAX);

		unsigned int imageIndex;
		VkResult result = vkAcquireNextImageKHR(device_, swapChain_, UINT64_MAX, imageAvailableSemaphores_[currentFrame], VK_NULL_HANDLE, &imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			RecreateSwapChain();
			return;
		} else if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR){
			throw std::runtime_error("Failed to acquire swap chain!");
		}


		if (imagesInFlight_[imageIndex] != VK_NULL_HANDLE)
		{
			vkWaitForFences(device_, 1, &imagesInFlight_[imageIndex], VK_TRUE, UINT64_MAX);
		}

		imagesInFlight_[imageIndex] = inFlightFences_[currentFrame];

		VkSubmitInfo submitInfo{ };
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { imageAvailableSemaphores_[currentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers_[imageIndex];

		VkSemaphore signalSemaphores[] = { renderFinishedSemaphores_[currentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(device_, 1, &inFlightFences_[currentFrame]);

		if (vkQueueSubmit(graphicsQueue_, 1, &submitInfo, inFlightFences_[currentFrame]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to submit draw command buffer!");
		}

		VkPresentInfoKHR presentInfo{ };
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { swapChain_ };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;

		presentInfo.pImageIndices = &imageIndex;

		result = vkQueuePresentKHR(presentQueue_, &presentInfo);
		
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized_) {
			framebufferResized_ = false;
			RecreateSwapChain();
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("Failed to present swap chain!");
		}
		
		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;*/

		vkWaitForFences(device_, 1, &inFlightFences_[currentFrame], VK_TRUE, UINT64_MAX);

		uint32_t imageIndex = 0;
		VkResult result = vkAcquireNextImageKHR(
			device_, swapChain_, UINT64_MAX,
			imageAvailableSemaphores_[currentFrame], VK_NULL_HANDLE, &imageIndex
		);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) { RecreateSwapChain(); return; }
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
			throw std::runtime_error("Failed to acquire swap chain image!");

		if (imagesInFlight_[imageIndex] != VK_NULL_HANDLE)
			vkWaitForFences(device_, 1, &imagesInFlight_[imageIndex], VK_TRUE, UINT64_MAX);
		imagesInFlight_[imageIndex] = inFlightFences_[currentFrame];

		VkCommandBuffer cmd = commandBuffers_[imageIndex];
		vkResetCommandBuffer(cmd, 0);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		if (vkBeginCommandBuffer(cmd, &beginInfo) != VK_SUCCESS)
			throw std::runtime_error("Failed to begin cmd buffer");

		VkClearValue clearColor{};
		clearColor.color = { {0.0f, 0.0f, 0.0f, 1.0f} };

		VkRenderPassBeginInfo rpBegin{};
		rpBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		rpBegin.renderPass = renderPass_;
		rpBegin.framebuffer = swapChainFrameBuffers_[imageIndex];
		rpBegin.renderArea.offset = { 0, 0 };
		rpBegin.renderArea.extent = swapChainExtent_;
		rpBegin.clearValueCount = 1;
		rpBegin.pClearValues = &clearColor;

		vkCmdBeginRenderPass(cmd, &rpBegin, VK_SUBPASS_CONTENTS_INLINE);

		// tus draws
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline_);
		vkCmdDraw(cmd, 3, 1, 0, 0);

		// ahora ImGui, usando el draw data del frame actual
		if (usingImGui_) {
			ImDrawData* dd = ImGui::GetDrawData();
			if (dd) ImGui_ImplVulkan_RenderDrawData(dd, cmd);
		}

		vkCmdEndRenderPass(cmd);

		if (vkEndCommandBuffer(cmd) != VK_SUCCESS)
			throw std::runtime_error("Failed to record cmd buffer");

		VkSemaphore waitSemaphores[] = { imageAvailableSemaphores_[currentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkSemaphore signalSemaphores[] = { renderFinishedSemaphores_[currentFrame] };

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmd;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(device_, 1, &inFlightFences_[currentFrame]);
		if (vkQueueSubmit(graphicsQueue_, 1, &submitInfo, inFlightFences_[currentFrame]) != VK_SUCCESS)
			throw std::runtime_error("Failed to submit draw command buffer!");

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &swapChain_;
		presentInfo.pImageIndices = &imageIndex;

		result = vkQueuePresentKHR(presentQueue_, &presentInfo);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized_) {
			framebufferResized_ = false;
			RecreateSwapChain();
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("Failed to present swap chain!");
		}

		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	// <-- FINAL DE ZONA DE DRAW FRAME
}

