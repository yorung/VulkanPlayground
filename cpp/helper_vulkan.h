#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

VkResult _afHandleVKError(const char* file, const char* func, int line, const char* command, VkResult result);
#define afHandleVKError(command) do{ _afHandleVKError(__FILE__, __FUNCTION__, __LINE__, #command, command); } while(0)

class DeviceManVK
{
	VkDevice device = nullptr;
	VkInstance inst = nullptr;
	VkSurfaceKHR surface = 0;
	VkSwapchainKHR swapchain = 0;
	uint32_t swapChainCount = 0;
	VkImage swapChainImages[8] = {};
	VkImageView imageViews[8] = {};
	VkFramebuffer framebuffers[8] = {};
	VkCommandBuffer commandBuffer = 0;
	VkSemaphore semaphore = 0;
	VkCommandPool commandPool = 0;
	VkRenderPass renderPass = 0;
	VkPipelineCache pipelineCache = 0;
	VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
	uint32_t frameIndex = 0;
	RECT rc = {};
	void BeginScene();
public:
	void Create(HWND hWnd);
	void Test();
	void Present();
	void Destroy();
};
extern DeviceManVK deviceMan;
