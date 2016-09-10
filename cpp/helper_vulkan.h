#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

void VulkanTest(HWND hWnd);

VkResult _afHandleVKError(const char* file, const char* func, int line, const char* command, VkResult result);
#define afHandleVKError(command) do{ _afHandleVKError(__FILE__, __FUNCTION__, __LINE__, #command, command); } while(0)
