#include "stdafx.h"

Triangle triangle;

static VkVertexInputBindingDescription bindings[] = { { 0, sizeof(Vec2), VK_VERTEX_INPUT_RATE_VERTEX } };
static VkVertexInputAttributeDescription attributes[] = { { 0, 0, VK_FORMAT_R32G32_SFLOAT, 0 } };

void Triangle::Draw()
{
	VkCommandBuffer commandBuffer = deviceMan.commandBuffer;
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	VkDeviceSize offsets[1] = {};
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer.buffer, offsets);
	vkCmdDraw(commandBuffer, 4, 1, 0, 0);
}

void Triangle::Create()
{
	VkDevice device = deviceMan.GetDevice();
	const VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	afHandleVKError(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));

	pipeline = deviceMan.CreatePipeline("test", pipelineLayout, arrayparam(bindings), arrayparam(attributes));

	Vec2 vertexPositions[3];
	for (int i = 0; i < 3; i++)
	{
		vertexPositions[i] = Vec2(sin(i * (float)M_PI * 2 / 3), cos(i * (float)M_PI * 2 / 3));
	}
	vertexBuffer = CreateBuffer(device, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, deviceMan.physicalDeviceMemoryProperties, sizeof(vertexPositions), vertexPositions);
}

void Triangle::Destroy()
{
	DeleteBufer(vertexBuffer);
	VkDevice device = deviceMan.GetDevice();
	afSafeDeleteVk(vkDestroyPipeline, device, pipeline);
	afSafeDeleteVk(vkDestroyPipelineLayout, device, pipelineLayout);
}
