#include "stdafx.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#pragma comment(lib, "vulkan-1.lib")

void VulkanTest(HWND hWnd)
{
	VkResult res;
	const char* extensions[] = { "VK_KHR_surface", "VK_KHR_win32_surface" };
	VkInstanceCreateInfo instInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, nullptr, 0, nullptr, 0, nullptr, _countof(extensions), extensions };
	VkInstance inst = nullptr;
	res = vkCreateInstance(&instInfo, nullptr, &inst);
	assert(!res);

	VkPhysicalDevice devices[16] = {};
	uint32_t numDevices = _countof(devices);
	res = vkEnumeratePhysicalDevices(inst, &numDevices, devices);
	assert(!res);

	float priorities[] = { 0 };
	VkDeviceQueueCreateInfo devQueueInfos[] = { { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, nullptr, 0, 0, 1, priorities } };
	const char* deviceExtensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	VkPhysicalDeviceFeatures features = {};
	VkDeviceCreateInfo devInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, nullptr, 0, 1, devQueueInfos, 0, nullptr, _countof(deviceExtensions), deviceExtensions, &features };
	VkDevice device = nullptr;
	res = vkCreateDevice(devices[0], &devInfo, nullptr, &device);
	assert(!res);

	VkWin32SurfaceCreateInfoKHR surfaceInfo = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR, nullptr, 0, GetModuleHandle(nullptr), hWnd };
	VkSurfaceKHR surface = 0;
	res = vkCreateWin32SurfaceKHR(inst, &surfaceInfo, nullptr, &surface);
	assert(!res);

	VkSurfaceCapabilitiesKHR surfaceCaps = {};
	res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(devices[0], surface, &surfaceCaps);
	assert(!res);

	RECT rc;
	GetClientRect(hWnd, &rc);
	VkSwapchainCreateInfoKHR swapchainInfo = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR, nullptr, 0, surface, surfaceCaps.minImageCount, VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR, {uint32_t(rc.right - rc.left), uint32_t(rc.bottom - rc.top)} };
	VkSwapchainKHR swapchain = 0;
	res = vkCreateSwapchainKHR(device, &swapchainInfo, nullptr, &swapchain);
	assert(!res);

	uint32_t swapChainCount = 2;
	VkImage images[2];
	res = vkGetSwapchainImagesKHR(device, swapchain, &swapChainCount, images);
	assert(!res);

	VkRenderPassCreateInfo renderPassInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
	VkRenderPass renderPass;
	res = vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass);
	assert(!res);

	VkFramebufferCreateInfo framebufferInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
	VkFramebuffer framebuffer;
	res = vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffer);
	assert(!res);

	VkCommandPoolCreateInfo commandPoolInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT };
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

//	vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, 0, )
//	VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
//	vkQueuePresentKHR(, &presentInfo);


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
