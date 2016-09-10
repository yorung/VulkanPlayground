#include "stdafx.h"

#pragma comment(lib, "vulkan-1.lib")

VkResult _afHandleVKError(const char* file, const char* func, int line, const char* command, VkResult result)
{
	const char *err = nullptr;
	switch (result)
	{
#define E(er) case er: err = #er; break
		E(VK_INCOMPLETE);
#undef E
	default:
		aflog("%s %s(%d): err=%d %s\n", file, func, line, result, command);
		return result;
	case VK_SUCCESS:
		return VK_SUCCESS;
	}
	aflog("%s %s(%d): %s %s\n", file, func, line, err, command);
	return result;
}

template <typename Deleter, class Object>
inline void afSafeDeleteVk(Deleter deleter, VkDevice device, Object& object)
{
	if (object)
	{
		deleter(device, object, nullptr);
		object = 0;
	}
}

static VkShaderModule CreateShaderModule(VkDevice device, const char* fileName)
{
	int size;
	void* file = LoadFile(fileName, &size);
	assert(file);
	VkShaderModuleCreateInfo info = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, nullptr, 0, (size_t)size, (uint32_t*)file };
	VkShaderModule module = 0;
	afHandleVKError(vkCreateShaderModule(device, &info, nullptr, &module));
	free(file);
	return module;
}

static void WriteBuffer(VkDevice device, VkBuffer buffer, VkPhysicalDeviceMemoryProperties memoryProperties, int size, const void* data)
{
	VkMemoryRequirements req;
	vkGetBufferMemoryRequirements(device, buffer, &req);
	uint32_t memoryTypeIndex = 0;

	VkMemoryAllocateInfo info = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, nullptr, req.size, memoryTypeIndex };

//	vkMapMemory(device, );
}

static VkBuffer CreateBuffer(VkDevice device, VkBufferUsageFlags usage, int size)
{
	const VkBufferCreateInfo info = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, nullptr, 0, (VkDeviceSize)size, usage };
	VkBuffer buffer;
	afHandleVKError(vkCreateBuffer(device, &info, nullptr, &buffer));
	return buffer;
}

static VkPipeline CreatePipeline(VkDevice device, VkPipelineLayout pipelineLayout, VkPipelineCache pipelineCache, VkRenderPass renderPass, const VkViewport* viewport, const VkRect2D* scissor)
{
	VkShaderModule vertexShader = CreateShaderModule(device, "test.vert.spv");
	VkShaderModule fragmentShader = CreateShaderModule(device, "test.frag.spv");
	const VkPipelineShaderStageCreateInfo shaderStageCreationInfos[] = { { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_VERTEX_BIT, vertexShader, "main" },{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_FRAGMENT_BIT, fragmentShader, "main" } };
	const VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
	const VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, nullptr, 0, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP };
	const VkPipelineViewportStateCreateInfo viewportStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, nullptr, 0, 1, viewport, 1, scissor };
	const VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, nullptr, 0, VK_FALSE, VK_FALSE, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_FALSE, 0, 0, 0, 1.0f };
	const VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO, nullptr, 0, VK_SAMPLE_COUNT_1_BIT };
	const VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
	const VkPipelineColorBlendAttachmentState colorBlendAttachmentStates[] = { { VK_FALSE, VK_BLEND_FACTOR_ZERO, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ZERO, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, 0xf } };
	const VkPipelineColorBlendStateCreateInfo colorBlendState = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, nullptr, 0, VK_FALSE, VK_LOGIC_OP_CLEAR, _countof(colorBlendAttachmentStates), colorBlendAttachmentStates };
	const VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	const VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, nullptr, 0, _countof(dynamicStates), dynamicStates };
	const VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfos[] = { { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO, nullptr, 0, _countof(shaderStageCreationInfos), shaderStageCreationInfos, &pipelineVertexInputStateCreateInfo, &pipelineInputAssemblyStateCreateInfo, nullptr, &viewportStateCreateInfo, &rasterizationStateCreateInfo, &multisampleStateCreateInfo, &depthStencilStateCreateInfo, &colorBlendState, &pipelineDynamicStateCreateInfo, pipelineLayout, renderPass } };
	VkPipeline pipeline = 0;
	afHandleVKError(vkCreateGraphicsPipelines(device, pipelineCache, _countof(graphicsPipelineCreateInfos), graphicsPipelineCreateInfos, nullptr, &pipeline));
	afSafeDeleteVk(vkDestroyShaderModule, device, vertexShader);
	afSafeDeleteVk(vkDestroyShaderModule, device, fragmentShader);
	return pipeline;
}

void VulkanTest(HWND hWnd)
{
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
	afHandleVKError(vkCreateInstance(&instInfo, nullptr, &inst));

	VkPhysicalDevice devices[16] = {};
	uint32_t numDevices = _countof(devices);
	afHandleVKError(vkEnumeratePhysicalDevices(inst, &numDevices, devices));

	float priorities[] = { 0 };
	const VkDeviceQueueCreateInfo devQueueInfos[] = { { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, nullptr, 0, 0, 1, priorities } };
	const char* deviceExtensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	const VkPhysicalDeviceFeatures features = {};
	const VkDeviceCreateInfo devInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, nullptr, 0, 1, devQueueInfos, 0, nullptr, _countof(deviceExtensions), deviceExtensions, &features };
	VkDevice device = nullptr;
	afHandleVKError(vkCreateDevice(devices[0], &devInfo, nullptr, &device));

	const VkWin32SurfaceCreateInfoKHR surfaceInfo = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR, nullptr, 0, GetModuleHandle(nullptr), hWnd };
	VkSurfaceKHR surface = 0;
	afHandleVKError(vkCreateWin32SurfaceKHR(inst, &surfaceInfo, nullptr, &surface));

	uint32_t numSurfaceFormats = 0;
	VkSurfaceFormatKHR surfaceFormats[32] = {};
	afHandleVKError(vkGetPhysicalDeviceSurfaceFormatsKHR(devices[0], surface, &numSurfaceFormats, nullptr));
	assert(numSurfaceFormats <= _countof(surfaceFormats));
	afHandleVKError(vkGetPhysicalDeviceSurfaceFormatsKHR(devices[0], surface, &numSurfaceFormats, surfaceFormats));

	uint32_t numPresentModes = 0;
	VkPresentModeKHR presentModes[32] = {};
	afHandleVKError(vkGetPhysicalDeviceSurfacePresentModesKHR(devices[0], surface, &numPresentModes, nullptr));
	assert(numPresentModes <= _countof(presentModes));
	afHandleVKError(vkGetPhysicalDeviceSurfacePresentModesKHR(devices[0], surface, &numPresentModes, presentModes));

	VkSurfaceCapabilitiesKHR surfaceCaps = {};
	afHandleVKError(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(devices[0], surface, &surfaceCaps));

	VkBool32 physicalDeviceSurfaceSupport = VK_FALSE;
	afHandleVKError(vkGetPhysicalDeviceSurfaceSupportKHR(devices[0], 0, surface, &physicalDeviceSurfaceSupport));
	assert(physicalDeviceSurfaceSupport);

	RECT rc;
	GetClientRect(hWnd, &rc);
	const VkSwapchainCreateInfoKHR swapchainInfo = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR, nullptr, 0, surface, surfaceCaps.minImageCount, surfaceFormats[0].format, surfaceFormats[0].colorSpace, { uint32_t(rc.right), uint32_t(rc.bottom) }, 1, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_SHARING_MODE_EXCLUSIVE, 0, nullptr, VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR, VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, presentModes[0], VK_TRUE };
	VkSwapchainKHR swapchain = 0;
	afHandleVKError(vkCreateSwapchainKHR(device, &swapchainInfo, nullptr, &swapchain));

	uint32_t swapChainCount = 0;
	VkImage images[8];
	afHandleVKError(vkGetSwapchainImagesKHR(device, swapchain, &swapChainCount, nullptr));
	assert(swapChainCount <= _countof(images));
	afHandleVKError(vkGetSwapchainImagesKHR(device, swapchain, &swapChainCount, images));

	const VkAttachmentReference colorAttachmentReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
	const VkSubpassDescription subpassDescriptions[] = { { 0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &colorAttachmentReference } };
	const VkAttachmentDescription attachments[1] = { { 0, swapchainInfo.imageFormat, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR } };
	const VkRenderPassCreateInfo renderPassInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, _countof(attachments), attachments, _countof(subpassDescriptions), subpassDescriptions };
	VkRenderPass renderPass;
	afHandleVKError(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass));

	VkImageView imageView = 0;
	const VkImageViewCreateInfo imageViewCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, nullptr, 0, images[0], VK_IMAGE_VIEW_TYPE_2D, surfaceFormats[0].format, { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A }, { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 } };
	afHandleVKError(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &imageView));

	const VkImageView frameBufferAttachmentImageView[1] = { { imageView }};
	assert(_countof(frameBufferAttachmentImageView) == renderPassInfo.attachmentCount);
	const VkFramebufferCreateInfo framebufferInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, renderPass, _countof(frameBufferAttachmentImageView), frameBufferAttachmentImageView, (uint32_t)rc.right, (uint32_t)rc.bottom, 1 };
	VkFramebuffer framebuffer;
	afHandleVKError(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffer));

	const VkCommandPoolCreateInfo commandPoolInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT };
	VkCommandPool commandPool;
	afHandleVKError(vkCreateCommandPool(device, &commandPoolInfo, nullptr, &commandPool));

	uint32_t numPhysicalDevices = 0;
	afHandleVKError(vkEnumeratePhysicalDevices(inst, &numPhysicalDevices, nullptr));

	assert(numPhysicalDevices == 1);
	VkPhysicalDevice physicalDevice;
	afHandleVKError(vkEnumeratePhysicalDevices(inst, &numPhysicalDevices, &physicalDevice));

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
	afHandleVKError(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer));

	const VkSemaphoreCreateInfo semaphoreCreateInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
	VkSemaphore semaphore;
	afHandleVKError(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphore));

	uint32_t imageIndex = 0;
	afHandleVKError(vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, semaphore, VK_NULL_HANDLE, &imageIndex));

	const VkCommandBufferBeginInfo commandBufferBeginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	afHandleVKError(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));

	const VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	VkPipelineLayout pipelineLayout = 0;
	afHandleVKError(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));

	const VkPipelineCacheCreateInfo pipelineCacheCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO };
	VkPipelineCache pipelineCache = 0;
	afHandleVKError(vkCreatePipelineCache(device, &pipelineCacheCreateInfo, nullptr, &pipelineCache));

	const VkViewport viewports[] = { { 0, 0, (float)rc.right, (float)rc.bottom, 0, 1 } };
	const VkRect2D scissors[] = { { 0, 0, (uint32_t)rc.right, (uint32_t)rc.bottom } };
	VkPipeline pipeline = CreatePipeline(device, pipelineLayout, pipelineCache, renderPass, viewports, scissors);

	VkBuffer buffer = CreateBuffer(device, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(Vec3) * 3);

	const VkClearValue clearValues[2] = { { 0.2f, 0.5f, 0.5f } };
	const VkRenderPassBeginInfo renderPassBeginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, nullptr, renderPass, framebuffer, { {}, {(uint32_t)rc.right, (uint32_t)rc.bottom} }, _countof(clearValues), clearValues };
	vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	vkCmdSetViewport(commandBuffer, 0, _countof(viewports), viewports);
	vkCmdSetScissor(commandBuffer, 0, _countof(scissors), scissors);
	vkCmdDraw(commandBuffer, 4, 1, 0, 0);
	vkCmdEndRenderPass(commandBuffer);
	afHandleVKError(vkEndCommandBuffer(commandBuffer));

	const VkSubmitInfo submitInfos[] = { { VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 0, nullptr, nullptr, 1, &commandBuffer } };
	afHandleVKError(vkQueueSubmit(queue, _countof(submitInfos), submitInfos, 0));

	afHandleVKError(vkQueueWaitIdle(queue));

	const VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, nullptr, 1, &semaphore, 1, &swapchain, &imageIndex };
	afHandleVKError(vkQueuePresentKHR(queue, &presentInfo));

	afSafeDeleteVk(vkDestroyBuffer, device, buffer);
	afSafeDeleteVk(vkDestroyPipeline, device, pipeline);
	afSafeDeleteVk(vkDestroyPipelineLayout, device, pipelineLayout);
	afSafeDeleteVk(vkDestroyPipelineCache, device, pipelineCache);
	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
	commandBuffer = 0;
	afSafeDeleteVk(vkDestroySemaphore, device, semaphore);
	afSafeDeleteVk(vkDestroyImageView, device, imageView);
	afSafeDeleteVk(vkDestroyCommandPool, device, commandPool);
	afSafeDeleteVk(vkDestroyFramebuffer, device, framebuffer);
	afSafeDeleteVk(vkDestroyRenderPass, device, renderPass);
	afSafeDeleteVk(vkDestroySwapchainKHR, device, swapchain);
	vkDestroySurfaceKHR(inst, surface, nullptr);
	surface = 0;
	vkDestroyDevice(device, nullptr);
	device = nullptr;
	vkDestroyInstance(inst, nullptr);
	inst = nullptr;
}
