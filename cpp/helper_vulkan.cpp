#include "stdafx.h"

#include <vulkan/vulkan.h>

#pragma comment(lib, "vulkan-1.lib")

void VulkanTest()
{
	VkInstanceCreateInfo instInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
	VkInstance inst = nullptr;
	vkCreateInstance(&instInfo, nullptr, &inst);

	VkPhysicalDevice devices[16];
	uint32_t numDevices = _countof(devices);
	vkEnumeratePhysicalDevices(inst, &numDevices, devices);

	VkDeviceCreateInfo devInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
	VkDevice device = nullptr;
	vkCreateDevice(devices[0], &devInfo, nullptr, &device);

	vkDestroyDevice(device, nullptr);
	vkDestroyInstance(inst, nullptr);
}
