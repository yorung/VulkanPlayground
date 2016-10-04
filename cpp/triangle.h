class Triangle
{
	VBOID vertexBuffer;
	IBOID indexBuffer;
	AFRenderStates renderStates;
public:
	void Create();
	void Draw();
	void Destroy();
};

extern Triangle triangle;
