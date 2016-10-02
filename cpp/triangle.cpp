#include "stdafx.h"

Triangle triangle;

struct TriangleVertex
{
	Vec3 Pos;
	Vec3 Color;
};

static InputElement attributes[] =
{
	CInputElement(0, AFF_R32G32B32_FLOAT, 0),
	CInputElement(1, AFF_R32G32B32_FLOAT, 12),
};

void Triangle::Draw()
{
	Mat mat = q2m(Quat(Vec3(0, 0, 1), (float)GetTime()));
	afBindBuffer(pipelineLayout, sizeof(mat), &mat, 0);
	VkCommandBuffer commandBuffer = deviceMan.commandBuffer;
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	VkDeviceSize offsets[1] = {};
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer.buffer, offsets);
	afSetIndexBuffer(indexBuffer);
	afDrawIndexed(3);
//	afDraw(3);
}

void Triangle::Create()
{
	VkDevice device = deviceMan.GetDevice();
	const VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0, 1, &deviceMan.commonUboDescriptorSetLayout};
	afHandleVKError(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));

	pipeline = deviceMan.CreatePipeline("solid", pipelineLayout, arrayparam(attributes), BM_NONE, DSM_DEPTH_ENABLE);

	TriangleVertex vertexPositions[3];
	for (int i = 0; i < 3; i++)
	{
		vertexPositions[i] =
		{
			Vec3(sin(i * (float)M_PI * 2 / 3), cos(i * (float)M_PI * 2 / 3), 0) * 0.5,
			Vec3(i == 0, i == 1, i == 2),
		};
	}
	const unsigned short indexData[] = { 0, 1, 2 };
	vertexBuffer = afCreateVertexBuffer(sizeof(vertexPositions), vertexPositions);
	indexBuffer = afCreateIndexBuffer(arrayparam(indexData));
}

void Triangle::Destroy()
{
	afSafeDeleteBuffer(indexBuffer);
	afSafeDeleteBuffer(vertexBuffer);
	VkDevice device = deviceMan.GetDevice();
	afSafeDeleteVk(vkDestroyPipelineLayout, device, pipelineLayout);
	afSafeDeleteVk(vkDestroyPipeline, device, pipeline);
}
