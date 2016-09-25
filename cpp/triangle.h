class Triangle
{
	VBOID vertexBuffer;
	BufferContext indexBuffer;
	VkPipelineLayout pipelineLayout = 0;
	VkPipeline pipeline = 0;
public:
	void Create();
	void Draw();
	void Destroy();
};

extern Triangle triangle;
