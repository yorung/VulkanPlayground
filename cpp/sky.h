class Sky
{
	TextureContext texture;
	VkSampler sampler = 0;
	VkPipelineLayout pipelineLayout = 0;
	VkPipeline pipeline = 0;
	VkDescriptorSetLayout descriptorSetLayouts[1] = {};
	VkDescriptorSet descriptorSet = 0;
public:
	void Create();
	void Draw();
	void Destroy();
};

extern Sky sky;
