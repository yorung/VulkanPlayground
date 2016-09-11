#include "stdafx.h"

#pragma comment(lib, "vulkan-1.lib")

DeviceManVK deviceMan;

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

static uint32_t GetCompatibleMemoryTypeIndex(const VkPhysicalDeviceMemoryProperties& memoryProperties, uint32_t typeBits, VkMemoryPropertyFlags propertyFlags)
{
	for (uint32_t i = 0; i < (int)memoryProperties.memoryTypeCount; i++)
	{
		if ((typeBits & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & propertyFlags) == propertyFlags)
		{
			return i;
		}
	}
	assert(0);
	return -1;	// dummy
}

void DeleteBufer(BufferContext& buffer)
{
	afSafeDeleteVk(vkDestroyBuffer, buffer.device, buffer.buffer);
	afSafeDeleteVk(vkFreeMemory, buffer.device, buffer.memory);
}

void WriteBuffer(BufferContext& buffer, int size, const void* srcData)
{
	void* mappedMemory = nullptr;
	afHandleVKError(vkMapMemory(buffer.device, buffer.memory, 0, size, 0, &mappedMemory));
	memcpy(mappedMemory, srcData, size);
	vkUnmapMemory(buffer.device, buffer.memory);
}

BufferContext CreateBuffer(VkDevice device, VkBufferUsageFlags usage, const VkPhysicalDeviceMemoryProperties& memoryProperties, int size, const void* srcData)
{
	const VkBufferCreateInfo bufferCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, nullptr, 0, (VkDeviceSize)size, usage };
	BufferContext buffer;
	buffer.device = device;
	afHandleVKError(vkCreateBuffer(device, &bufferCreateInfo, nullptr, &buffer.buffer));

	VkMemoryRequirements req = {};
	vkGetBufferMemoryRequirements(device, buffer.buffer, &req);
	const VkMemoryAllocateInfo memoryAllocateInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, nullptr, req.size, GetCompatibleMemoryTypeIndex(memoryProperties, req.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) };
	afHandleVKError(vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &buffer.memory));

	WriteBuffer(buffer, size, srcData);

	afHandleVKError(vkBindBufferMemory(device, buffer.buffer, buffer.memory, 0));

	return buffer;
}

VkPipeline DeviceManVK::CreatePipeline(const char* name, VkPipelineLayout pipelineLayout, uint32_t numBindings, const VkVertexInputBindingDescription bindings[], uint32_t numAttributes, const VkVertexInputAttributeDescription attributes[])
{
	char path[MAX_PATH];
	sprintf_s(path, sizeof(path), "%s.vert.spv", name);
	VkShaderModule vertexShader = CreateShaderModule(device, path);
	sprintf_s(path, sizeof(path), "%s.frag.spv", name);
	VkShaderModule fragmentShader = CreateShaderModule(device, path);
	const VkPipelineShaderStageCreateInfo shaderStageCreationInfos[] = { { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_VERTEX_BIT, vertexShader, "main" },{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_FRAGMENT_BIT, fragmentShader, "main" } };
	const VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, nullptr, 0, numBindings, bindings, numAttributes, attributes };
	const VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, nullptr, 0, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP };
	const VkPipelineViewportStateCreateInfo viewportStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, nullptr, 0, 1, &viewport, 1, &scissor };
	const VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, nullptr, 0, VK_FALSE, VK_FALSE, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_FALSE, 0, 0, 0, 1.0f };
	const VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO, nullptr, 0, VK_SAMPLE_COUNT_1_BIT };
	const VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
	const VkPipelineColorBlendAttachmentState colorBlendAttachmentStates[] = { { VK_FALSE, VK_BLEND_FACTOR_ZERO, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ZERO, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, 0xf } };
	const VkPipelineColorBlendStateCreateInfo colorBlendState = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, nullptr, 0, VK_FALSE, VK_LOGIC_OP_CLEAR, arrayparam(colorBlendAttachmentStates) };
	const VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	const VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, nullptr, 0, arrayparam(dynamicStates) };
	const VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfos[] = { { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO, nullptr, 0, arrayparam(shaderStageCreationInfos), &pipelineVertexInputStateCreateInfo, &pipelineInputAssemblyStateCreateInfo, nullptr, &viewportStateCreateInfo, &rasterizationStateCreateInfo, &multisampleStateCreateInfo, &depthStencilStateCreateInfo, &colorBlendState, &pipelineDynamicStateCreateInfo, pipelineLayout, renderPass } };
	VkPipeline pipeline = 0;
	afHandleVKError(vkCreateGraphicsPipelines(device, pipelineCache, arrayparam(graphicsPipelineCreateInfos), nullptr, &pipeline));
	afSafeDeleteVk(vkDestroyShaderModule, device, vertexShader);
	afSafeDeleteVk(vkDestroyShaderModule, device, fragmentShader);
	return pipeline;
}

void DeviceManVK::Create(HWND hWnd)
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
		instanceLayers, arrayparam(extensions) };
	afHandleVKError(vkCreateInstance(&instInfo, nullptr, &inst));

	VkPhysicalDevice devices[16] = {};
	uint32_t numDevices = _countof(devices);
	afHandleVKError(vkEnumeratePhysicalDevices(inst, &numDevices, devices));

	float priorities[] = { 0 };
	const VkDeviceQueueCreateInfo devQueueInfos[] = { { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, nullptr, 0, 0, 1, priorities } };
	const char* deviceExtensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	const VkPhysicalDeviceFeatures features = {};
	const VkDeviceCreateInfo devInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, nullptr, 0, 1, devQueueInfos, 0, nullptr, arrayparam(deviceExtensions), &features };
	afHandleVKError(vkCreateDevice(devices[0], &devInfo, nullptr, &device));

	const VkWin32SurfaceCreateInfoKHR surfaceInfo = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR, nullptr, 0, GetModuleHandle(nullptr), hWnd };
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

	GetClientRect(hWnd, &rc);
	const VkSwapchainCreateInfoKHR swapchainInfo = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR, nullptr, 0, surface, surfaceCaps.minImageCount, surfaceFormats[0].format, surfaceFormats[0].colorSpace,{ uint32_t(rc.right), uint32_t(rc.bottom) }, 1, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_SHARING_MODE_EXCLUSIVE, 0, nullptr, VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR, VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, presentModes[0], VK_TRUE };
	afHandleVKError(vkCreateSwapchainKHR(device, &swapchainInfo, nullptr, &swapchain));

	afHandleVKError(vkGetSwapchainImagesKHR(device, swapchain, &swapChainCount, nullptr));
	assert(swapChainCount <= _countof(swapChainImages));
	assert(swapChainCount <= _countof(imageViews));
	assert(swapChainCount <= _countof(framebuffers));
	afHandleVKError(vkGetSwapchainImagesKHR(device, swapchain, &swapChainCount, swapChainImages));

	const VkAttachmentReference colorAttachmentReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
	const VkSubpassDescription subpassDescriptions[] = { { 0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &colorAttachmentReference } };
	const VkAttachmentDescription attachments[1] = { { 0, swapchainInfo.imageFormat, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR } };
	const VkRenderPassCreateInfo renderPassInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, arrayparam(attachments), arrayparam(subpassDescriptions) };
	afHandleVKError(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass));

	for (int i = 0; i < (int)swapChainCount; i++)
	{
		const VkImageViewCreateInfo imageViewCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, nullptr, 0, swapChainImages[i], VK_IMAGE_VIEW_TYPE_2D, surfaceFormats[0].format,{ VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A },{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 } };
		afHandleVKError(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &imageViews[i]));

		const VkImageView frameBufferAttachmentImageView[1] = { { imageViews[i] } };
		assert(_countof(frameBufferAttachmentImageView) == renderPassInfo.attachmentCount);
		const VkFramebufferCreateInfo framebufferInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, renderPass, arrayparam(frameBufferAttachmentImageView), (uint32_t)rc.right, (uint32_t)rc.bottom, 1 };
		afHandleVKError(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffers[i]));
	}

	const VkCommandPoolCreateInfo commandPoolInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT };
	afHandleVKError(vkCreateCommandPool(device, &commandPoolInfo, nullptr, &commandPool));

	const VkCommandBufferAllocateInfo commandBufferAllocateInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, nullptr, commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1 };
	afHandleVKError(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer));

	const VkSemaphoreCreateInfo semaphoreCreateInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
	afHandleVKError(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphore));

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
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalDeviceMemoryProperties);

	const VkPipelineCacheCreateInfo pipelineCacheCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO };
	afHandleVKError(vkCreatePipelineCache(device, &pipelineCacheCreateInfo, nullptr, &pipelineCache));

	viewport = { 0, 0, (float)rc.right, (float)rc.bottom, 0, 1 };
	scissor = { 0, 0, (uint32_t)rc.right, (uint32_t)rc.bottom };

	BeginScene();
}

void DeviceManVK::BeginScene()
{
	afHandleVKError(vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, semaphore, VK_NULL_HANDLE, &frameIndex));

	const VkCommandBufferBeginInfo commandBufferBeginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	afHandleVKError(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));

	const VkClearValue clearValues[2] = { { 0.2f, 0.5f, 0.5f } };
	const VkRenderPassBeginInfo renderPassBeginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, nullptr, renderPass, framebuffers[frameIndex],{ {},{ (uint32_t)rc.right, (uint32_t)rc.bottom } }, arrayparam(clearValues) };
	vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void DeviceManVK::Present()
{
	vkCmdEndRenderPass(commandBuffer);
	afHandleVKError(vkEndCommandBuffer(commandBuffer));

	VkQueue queue = 0;
	vkGetDeviceQueue(device, 0, 0, &queue);
	assert(queue);

	const VkSubmitInfo submitInfos[] = { { VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 0, nullptr, nullptr, 1, &commandBuffer } };
	afHandleVKError(vkQueueSubmit(queue, arrayparam(submitInfos), 0));

	afHandleVKError(vkQueueWaitIdle(queue));

	const VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, nullptr, 1, &semaphore, 1, &swapchain, &frameIndex };
	afHandleVKError(vkQueuePresentKHR(queue, &presentInfo));

	BeginScene();
}

void DeviceManVK::Destroy()
{
	afSafeDeleteVk(vkDestroyPipelineCache, device, pipelineCache);
	if (commandBuffer)
	{
		vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
		commandBuffer = 0;
	}
	afSafeDeleteVk(vkDestroySemaphore, device, semaphore);
	for (auto& it : imageViews)
	{
		afSafeDeleteVk(vkDestroyImageView, device, it);
	}
	afSafeDeleteVk(vkDestroyCommandPool, device, commandPool);
	std::for_each(framebuffers, framebuffers + _countof(framebuffers), [&](VkFramebuffer& framebuffer) { afSafeDeleteVk(vkDestroyFramebuffer, device, framebuffer);	});
	afSafeDeleteVk(vkDestroyRenderPass, device, renderPass);
	afSafeDeleteVk(vkDestroySwapchainKHR, device, swapchain);
	if (surface)
	{
		vkDestroySurfaceKHR(inst, surface, nullptr);
		surface = 0;
	}
	if (device)
	{
		vkDestroyDevice(device, nullptr);
		device = nullptr;
	}
	if (inst)
	{
		vkDestroyInstance(inst, nullptr);
		inst = nullptr;
	}
}
