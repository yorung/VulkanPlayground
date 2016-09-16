#include "stdafx.h"

Triangle triangle;

struct TriangleVertex
{
	Vec3 Pos;
	Vec3 Color;
};

static VkVertexInputBindingDescription bindings[] = { { 0, sizeof(TriangleVertex), VK_VERTEX_INPUT_RATE_VERTEX } };
static VkVertexInputAttributeDescription attributes[] =
{
	{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 },
	{ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, 12 },
};

void Triangle::Draw()
{
	Mat mat = q2m(Quat(Vec3(0, 0, 1), (float)GetTime()));
	WriteBuffer(uniformBuffer, sizeof(mat), &mat);

	VkCommandBuffer commandBuffer = deviceMan.commandBuffer;
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	VkDeviceSize offsets[1] = {};
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer.buffer, offsets);
	vkCmdDraw(commandBuffer, 3, 1, 0, 0);
}

void Triangle::Create()
{
	VkDevice device = deviceMan.GetDevice();
	const VkDescriptorSetLayoutBinding descriptorSetLayoutBindings[] = { { 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT } };
	const VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, nullptr, 0, arrayparam(descriptorSetLayoutBindings) };
	afHandleVKError(vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayouts[0]));

	const VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0, arrayparam(descriptorSetLayouts) };
	afHandleVKError(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));

	const VkDescriptorPoolSize descriptorPoolSizes[1] = { { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 } };
	const VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, nullptr, 0, 1, arrayparam(descriptorPoolSizes) };
	afHandleVKError(vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, nullptr, &descriptorPool));

	const VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, nullptr, descriptorPool, arrayparam(descriptorSetLayouts) };
	afHandleVKError(vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &descriptorSet));

	pipeline = deviceMan.CreatePipeline("solid", pipelineLayout, arrayparam(bindings), arrayparam(attributes));

	TriangleVertex vertexPositions[3];
	for (int i = 0; i < 3; i++)
	{
		vertexPositions[i] =
		{
			Vec3(sin(i * (float)M_PI * 2 / 3), cos(i * (float)M_PI * 2 / 3), 0),
			Vec3(i == 0, i == 1, i == 2),
		};
	}
	vertexBuffer = CreateBuffer(device, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, deviceMan.physicalDeviceMemoryProperties, sizeof(vertexPositions), vertexPositions);
	uniformBuffer = CreateBuffer(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, deviceMan.physicalDeviceMemoryProperties, sizeof(Mat), nullptr);

	VkDescriptorBufferInfo descriptorBufferInfo = { uniformBuffer.buffer, 0, uniformBuffer.size };
	VkWriteDescriptorSet writeDescriptorSets[] = { { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, descriptorSet, 0, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, nullptr, &descriptorBufferInfo } };
	vkUpdateDescriptorSets(device, arrayparam(writeDescriptorSets), 0, nullptr);
}

void Triangle::Destroy()
{
	DeleteBufer(uniformBuffer);
	DeleteBufer(vertexBuffer);
	VkDevice device = deviceMan.GetDevice();
	afSafeDeleteVk(vkDestroyPipeline, device, pipeline);
	afSafeDeleteVk(vkDestroyPipelineLayout, device, pipelineLayout);

	afSafeDeleteVk(vkDestroyDescriptorPool, device, descriptorPool);
	for (auto& it : descriptorSetLayouts)
	{
		afSafeDeleteVk(vkDestroyDescriptorSetLayout, device, it);
	}
}
