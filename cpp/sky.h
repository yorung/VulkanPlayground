class Sky
{
	TextureContext texture;
	VkSampler sampler = 0;
	VkPipelineLayout pipelineLayout = 0;
	VkPipeline pipeline = 0;
	VkDescriptorSet textureDescriptorSet = 0;
public:
	void Create();
	void Draw();
	void Destroy();
};

extern Sky sky;
