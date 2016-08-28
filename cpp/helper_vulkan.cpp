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

	VkPhysicalDevice devices[16];
	uint32_t numDevices = _countof(devices);
	res = vkEnumeratePhysicalDevices(inst, &numDevices, devices);

	VkDeviceCreateInfo devInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
	VkDevice device = nullptr;
	res = vkCreateDevice(devices[0], &devInfo, nullptr, &device);

	VkWin32SurfaceCreateInfoKHR surfaceInfo = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR, nullptr, 0, GetModuleHandle(nullptr), hWnd };
	VkSurfaceKHR surface;
	res = vkCreateWin32SurfaceKHR(inst, &surfaceInfo, nullptr, &surface);

	VkSwapchainCreateInfoKHR swapchainInfo = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR, nullptr, 0, surface };
	VkSwapchainKHR swapchain;
	res = vkCreateSwapchainKHR(device, &swapchainInfo, nullptr, &swapchain);


	vkDestroySwapchainKHR(device, swapchain, nullptr);
	vkDestroySurfaceKHR(inst, surface, nullptr);
	vkDestroyDevice(device, nullptr);
	vkDestroyInstance(inst, nullptr);
}
