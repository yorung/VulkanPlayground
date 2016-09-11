class Triangle
{
	BufferContext vertexBuffer;
	VkPipelineLayout pipelineLayout = 0;
	VkPipeline pipeline = 0;
public:
	void Create();
	void Draw();
	void Destroy();
};

extern Triangle triangle;
