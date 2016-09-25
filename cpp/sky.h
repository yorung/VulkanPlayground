class Sky
{
	TextureContext texture;
	VkPipelineLayout pipelineLayout = 0;
	VkPipeline pipeline = 0;
	VkDescriptorSet textureDescriptorSet = 0;
public:
	void Create();
	void Draw();
	void Destroy();
};

extern Sky sky;
