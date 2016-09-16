class Sky
{
	BufferContext uniformBuffer;
	VkPipelineLayout pipelineLayout = 0;
	VkPipeline pipeline = 0;
	VkDescriptorSetLayout descriptorSetLayouts[1] = {};
	VkDescriptorSet descriptorSet = 0;
	VkDescriptorPool descriptorPool = 0;
public:
	void Create();
	void Draw();
	void Destroy();
};

extern Sky sky;
