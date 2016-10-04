class Sky
{
	TextureContext texture;
	AFRenderStates renderStates;
public:
	void Create();
	void Draw();
	void Destroy();
};

extern Sky sky;
