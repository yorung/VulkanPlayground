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

	VkDeviceCreateInfo devInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
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

	uint32_t swapChainCount = 0;
	res = vkGetSwapchainImagesKHR(device, swapchain, &swapChainCount, nullptr);
	assert(!res);

	VkRenderPassCreateInfo renderPassInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
	VkRenderPass renderPass;
	res = vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass);
	assert(!res);

	VkFramebufferCreateInfo framebufferInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
	VkFramebuffer framebuffer;
	res = vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffer);
	assert(!res);

	vkDestroyFramebuffer(device, framebuffer, nullptr);
	vkDestroyRenderPass(device, renderPass, nullptr);
	vkDestroySwapchainKHR(device, swapchain, nullptr);
	vkDestroySurfaceKHR(inst, surface, nullptr);
	vkDestroyDevice(device, nullptr);
	vkDestroyInstance(inst, nullptr);
}
