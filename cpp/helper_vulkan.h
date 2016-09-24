typedef VkFormat AFFFormat;
#define AFF_INVALID VK_FORMAT_UNDEFINED
#define AFF_BC1_UNORM VK_FORMAT_BC1_RGBA_UNORM_BLOCK
#define AFF_BC2_UNORM VK_FORMAT_BC2_UNORM_BLOCK
#define AFF_BC3_UNORM VK_FORMAT_BC3_UNORM_BLOCK
#define AFF_R8G8B8A8_UNORM VK_FORMAT_R8G8B8A8_UNORM
#define AFF_R32G32B32_FLOAT VK_FORMAT_R32G32B32_SFLOAT

typedef VkVertexInputAttributeDescription InputElement;
class CInputElement : public InputElement
{
public:
	CInputElement(uint32_t location_, VkFormat format_, int offset_)
	{
		location = location_;
		format = format_;
		offset = offset_;
		binding = 0;
	}
};

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

void afSafeDeleteBufer(BufferContext& buffer);
void WriteBuffer(BufferContext& buffer, int size, const void* srcData);
BufferContext CreateBuffer(VkDevice device, VkBufferUsageFlags usage, const VkPhysicalDeviceMemoryProperties& memoryProperties, int size, const void* srcData);

typedef BufferContext VBOID;
typedef BufferContext UBOID;
VBOID afCreateVertexBuffer(int size, const void* srcData);
UBOID afCreateUBO(int size, const void* srcData = nullptr);

struct AFTexSubresourceData
{
	const void* ptr;
	uint32_t pitch;
	uint32_t pitchSlice;
};

struct TextureContext
{
	VkDevice device = 0;
	VkImage image = 0;
	VkDeviceMemory memory = 0;
	VkImageView view = 0;
	bool operator !() { return !image; }
};
typedef TextureContext SRVID;

SRVID afLoadTexture(const char* name, TexDesc& desc);
SRVID LoadTextureViaOS(const char* name, IVec2& size);
SRVID afCreateTexture2D(VkFormat format, const IVec2& size, void *image);
SRVID afCreateTexture2D(AFFFormat format, const struct TexDesc& desc, int mipCount, const AFTexSubresourceData datas[]);
void DeleteTexture(TextureContext& textureContext);

void afDrawIndexed(int numIndices, int start = 0, int instanceCount = 1);
void afDraw(int numVertices, int start = 0, int instanceCount = 1);

inline void afSetTextureName(const TextureContext& tex, const char* name)
{
}

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
public:
	VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
	VkPhysicalDevice physicalDevice = nullptr;
	VkDevice GetDevice() { return device; }
	VkCommandBuffer commandBuffer = 0;
	VkDescriptorPool descriptorPool = 0;
	void Create(HWND hWnd);
	void Present();
	void Destroy();
	void BeginScene();
	void Flush();
	VkPipeline CreatePipeline(const char* name, VkPipelineLayout pipelineLayout, uint32_t numAttributes, const VkVertexInputAttributeDescription attributes[]);
};
extern DeviceManVK deviceMan;
