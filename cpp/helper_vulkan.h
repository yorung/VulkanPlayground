#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

VkResult _afHandleVKError(const char* file, const char* func, int line, const char* command, VkResult result);
#define afHandleVKError(command) do{ _afHandleVKError(__FILE__, __FUNCTION__, __LINE__, #command, command); } while(0)

template <typename Deleter, class Object>
inline void afSafeDeleteVk(Deleter deleter, VkDevice device, Object& object)
{
	if (object)
	{
		deleter(device, object, nullptr);
		object = 0;
	}
}

struct BufferContext
{
	VkDevice device = 0;
	VkBuffer buffer = 0;
	VkDeviceMemory memory = 0;
	VkDeviceSize size = 0;
};

void DeleteBufer(BufferContext& buffer);
void WriteBuffer(BufferContext& buffer, int size, const void* srcData);
BufferContext CreateBuffer(VkDevice device, VkBufferUsageFlags usage, const VkPhysicalDeviceMemoryProperties& memoryProperties, int size, const void* srcData);

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
	VkSemaphore semaphore = 0;
	VkCommandPool commandPool = 0;
	VkRenderPass renderPass = 0;
	VkPipelineCache pipelineCache = 0;
	uint32_t frameIndex = 0;
	RECT rc = {};
	VkViewport viewport;
	VkRect2D scissor;
	void BeginScene();
public:
	VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
	VkDevice GetDevice() { return device; }
	VkCommandBuffer commandBuffer = 0;
	void Create(HWND hWnd);
	void Present();
	void Destroy();
	VkPipeline CreatePipeline(const char* name, VkPipelineLayout pipelineLayout, uint32_t numBindings, const VkVertexInputBindingDescription bindings[], uint32_t numAttributes, const VkVertexInputAttributeDescription attributes[]);
};
extern DeviceManVK deviceMan;
