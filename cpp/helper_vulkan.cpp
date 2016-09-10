#include "stdafx.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#pragma comment(lib, "vulkan-1.lib")

void VulkanTest(HWND hWnd)
{
	VkResult res;
	const char* extensions[] =
	{
		"VK_KHR_surface",
		"VK_KHR_win32_surface",
#ifndef NDEBUG
		"VK_EXT_debug_report",
#endif
	};
	const char* instanceLayers[] =
	{
		"VK_LAYER_LUNARG_standard_validation",
	};
	const VkInstanceCreateInfo instInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, nullptr, 0, nullptr,
#ifndef NDEBUG
		1,
#else
		0,
#endif
		instanceLayers, _countof(extensions), extensions };
	VkInstance inst = nullptr;
	res = vkCreateInstance(&instInfo, nullptr, &inst);
	assert(!res);

	VkPhysicalDevice devices[16] = {};
	uint32_t numDevices = _countof(devices);
	res = vkEnumeratePhysicalDevices(inst, &numDevices, devices);
	assert(!res);

	float priorities[] = { 0 };
	const VkDeviceQueueCreateInfo devQueueInfos[] = { { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, nullptr, 0, 0, 1, priorities } };
	const char* deviceExtensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	const VkPhysicalDeviceFeatures features = {};
	const VkDeviceCreateInfo devInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, nullptr, 0, 1, devQueueInfos, 0, nullptr, _countof(deviceExtensions), deviceExtensions, &features };
	VkDevice device = nullptr;
	res = vkCreateDevice(devices[0], &devInfo, nullptr, &device);
	assert(!res);

	const VkWin32SurfaceCreateInfoKHR surfaceInfo = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR, nullptr, 0, GetModuleHandle(nullptr), hWnd };
	VkSurfaceKHR surface = 0;
	res = vkCreateWin32SurfaceKHR(inst, &surfaceInfo, nullptr, &surface);
	assert(!res);

	uint32_t numSurfaceFormats = 0;
	VkSurfaceFormatKHR surfaceFormats[32] = {};
	res = vkGetPhysicalDeviceSurfaceFormatsKHR(devices[0], surface, &numSurfaceFormats, nullptr);
	assert(!res);
	assert(numSurfaceFormats <= _countof(surfaceFormats));
	res = vkGetPhysicalDeviceSurfaceFormatsKHR(devices[0], surface, &numSurfaceFormats, surfaceFormats);
	assert(!res);

	uint32_t numPresentModes = 0;
	VkPresentModeKHR presentModes[32] = {};
	res = vkGetPhysicalDeviceSurfacePresentModesKHR(devices[0], surface, &numPresentModes, nullptr);
	assert(!res);
	assert(numPresentModes <= _countof(presentModes));
	res = vkGetPhysicalDeviceSurfacePresentModesKHR(devices[0], surface, &numPresentModes, presentModes);
	assert(!res);

	VkSurfaceCapabilitiesKHR surfaceCaps = {};
	res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(devices[0], surface, &surfaceCaps);
	assert(!res);

	VkBool32 physicalDeviceSurfaceSupport = VK_FALSE;
	res = vkGetPhysicalDeviceSurfaceSupportKHR(devices[0], 0, surface, &physicalDeviceSurfaceSupport);
	assert(!res);
	assert(physicalDeviceSurfaceSupport);

	RECT rc;
	GetClientRect(hWnd, &rc);
	const VkSwapchainCreateInfoKHR swapchainInfo = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR, nullptr, 0, surface, surfaceCaps.minImageCount, surfaceFormats[0].format, surfaceFormats[0].colorSpace, { uint32_t(rc.right), uint32_t(rc.bottom) }, 1, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_SHARING_MODE_EXCLUSIVE, 0, nullptr, VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR, VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, presentModes[0], VK_TRUE };
	VkSwapchainKHR swapchain = 0;
	res = vkCreateSwapchainKHR(device, &swapchainInfo, nullptr, &swapchain);
	assert(!res);

	uint32_t swapChainCount = 0;
	VkImage images[8];
	res = vkGetSwapchainImagesKHR(device, swapchain, &swapChainCount, nullptr);
	assert(!res);
	assert(swapChainCount <= _countof(images));
	res = vkGetSwapchainImagesKHR(device, swapchain, &swapChainCount, images);
	assert(!res);

	const VkAttachmentReference colorAttachmentReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
	const VkSubpassDescription subpassDescriptions[] = { { 0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &colorAttachmentReference } };
	const VkAttachmentDescription attachments[1] = { { 0, swapchainInfo.imageFormat, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR } };
	const VkRenderPassCreateInfo renderPassInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, _countof(attachments), attachments, _countof(subpassDescriptions), subpassDescriptions };
	VkRenderPass renderPass;
	res = vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass);
	assert(!res);

	VkImageView imageView = 0;
	const VkImageViewCreateInfo imageViewCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, nullptr, 0, images[0], VK_IMAGE_VIEW_TYPE_2D, surfaceFormats[0].format, { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A }, { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 } };
	res = vkCreateImageView(device, &imageViewCreateInfo, nullptr, &imageView);
	assert(!res);

	const VkImageView frameBufferAttachmentImageView[1] = { { imageView }};
	assert(_countof(frameBufferAttachmentImageView) == renderPassInfo.attachmentCount);
	const VkFramebufferCreateInfo framebufferInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, renderPass, _countof(frameBufferAttachmentImageView), frameBufferAttachmentImageView, (uint32_t)rc.right, (uint32_t)rc.bottom, 1 };
	VkFramebuffer framebuffer;
	res = vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffer);
	assert(!res);

	const VkCommandPoolCreateInfo commandPoolInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT };
	VkCommandPool commandPool;
	res = vkCreateCommandPool(device, &commandPoolInfo, nullptr, &commandPool);
	assert(!res);

	uint32_t numPhysicalDevices = 0;
	res = vkEnumeratePhysicalDevices(inst, &numPhysicalDevices, nullptr);
	assert(!res);

	assert(numPhysicalDevices == 1);
	VkPhysicalDevice physicalDevice;
	res = vkEnumeratePhysicalDevices(inst, &numPhysicalDevices, &physicalDevice);
	assert(!res);

	uint32_t numQueueFamilyProperties = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &numQueueFamilyProperties, nullptr);
	VkQueueFamilyProperties queueFamilyProperties[2];
	assert(numQueueFamilyProperties == _countof(queueFamilyProperties));
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &numQueueFamilyProperties, queueFamilyProperties);
	assert(queueFamilyProperties[0].queueFlags & VK_QUEUE_GRAPHICS_BIT);

	VkPhysicalDeviceFeatures physicalDeviceFeatures;
	vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);
	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
	VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalDeviceMemoryProperties);

	VkQueue queue = 0;
	vkGetDeviceQueue(device, 0, 0, &queue);
	assert(queue);

	const VkCommandBufferAllocateInfo commandBufferAllocateInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, nullptr, commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1 };
	VkCommandBuffer commandBuffer;
	res = vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer);
	assert(!res);

	const VkSemaphoreCreateInfo semaphoreCreateInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
	VkSemaphore semaphore;
	res = vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphore);
	assert(!res);

	uint32_t imageIndex = 0;
	res = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, semaphore, VK_NULL_HANDLE, &imageIndex);
	assert(!res);

	const VkCommandBufferBeginInfo commandBufferBeginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	res = vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
	assert(!res);

	const VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	VkPipelineLayout pipelineLayout = 0;
	res = vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
	assert(!res);

	const VkPipelineCacheCreateInfo pipelineCacheCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO };
	VkPipelineCache pipelineCache = 0;
	res = vkCreatePipelineCache(device, &pipelineCacheCreateInfo, nullptr, &pipelineCache);
	assert(!res);

	const VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
	const VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, nullptr, 0, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST };
	const VkViewport viewports[] = { { 0, 0, (float)rc.right, (float)rc.bottom, 0, 1 } };
	const VkRect2D scissors[] = { { 0, 0, (uint32_t)rc.right, (uint32_t)rc.bottom } };
	const VkPipelineViewportStateCreateInfo viewportStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, nullptr, 0, _countof(viewports), viewports, _countof(scissors), scissors };
	const VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
	const VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO, nullptr, 0, VK_SAMPLE_COUNT_1_BIT };
	const VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
	const VkPipelineColorBlendAttachmentState colorBlendAttachmentStates[] = { { VK_FALSE, VK_BLEND_FACTOR_ZERO, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ZERO, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, 0xf } };
	const VkPipelineColorBlendStateCreateInfo colorBlendState = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, nullptr, 0, VK_FALSE, VK_LOGIC_OP_CLEAR, _countof(colorBlendAttachmentStates), colorBlendAttachmentStates };
	const VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
	const VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfos[] = { { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO, nullptr, 0, 0, nullptr, &pipelineVertexInputStateCreateInfo, &pipelineInputAssemblyStateCreateInfo, nullptr, &viewportStateCreateInfo, &rasterizationStateCreateInfo, &multisampleStateCreateInfo, &depthStencilStateCreateInfo, &colorBlendState, &pipelineDynamicStateCreateInfo, pipelineLayout, renderPass } };
	VkPipeline pipeline = 0;
//	res = vkCreateGraphicsPipelines(device, pipelineCache, _countof(graphicsPipelineCreateInfos), graphicsPipelineCreateInfos, nullptr, &pipeline);
	assert(!res);

	const VkClearValue clearValues[2] = { { 0.2f, 0.5f, 0.5f } };
	const VkRenderPassBeginInfo renderPassBeginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, nullptr, renderPass, framebuffer, { {}, {(uint32_t)rc.right, (uint32_t)rc.bottom} }, _countof(clearValues), clearValues };
	vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdEndRenderPass(commandBuffer);
	res = vkEndCommandBuffer(commandBuffer);
	assert(!res);

	const VkSubmitInfo submitInfos[] = { { VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 0, nullptr, nullptr, 1, &commandBuffer } };
	res = vkQueueSubmit(queue, _countof(submitInfos), submitInfos, 0);
	assert(!res);

	res = vkQueueWaitIdle(queue);
	assert(!res);

	const VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, nullptr, 1, &semaphore, 1, &swapchain, &imageIndex };
	res = vkQueuePresentKHR(queue, &presentInfo);
	assert(!res);

//	vkDestroyPipeline(device, pipeline, nullptr);
	pipeline = 0;
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	pipelineLayout = 0;
	vkDestroyPipelineCache(device, pipelineCache, nullptr);
	pipelineCache = 0;
	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
	commandBuffer = 0;
	vkDestroySemaphore(device, semaphore, nullptr);
	semaphore = 0;
	vkDestroyImageView(device, imageView, nullptr);
	imageView = 0;
	vkDestroyCommandPool(device, commandPool, nullptr);
	commandPool = 0;
	vkDestroyFramebuffer(device, framebuffer, nullptr);
	framebuffer = 0;
	vkDestroyRenderPass(device, renderPass, nullptr);
	renderPass = 0;
	vkDestroySwapchainKHR(device, swapchain, nullptr);
	swapchain = 0;
	vkDestroySurfaceKHR(inst, surface, nullptr);
	surface = 0;
	vkDestroyDevice(device, nullptr);
	device = nullptr;
	vkDestroyInstance(inst, nullptr);
	inst = nullptr;
}
