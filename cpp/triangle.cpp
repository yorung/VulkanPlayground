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
	WriteBuffer(uniformBuffer, sizeof(mat), &mat);

	VkCommandBuffer commandBuffer = deviceMan.commandBuffer;
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	VkDeviceSize offsets[1] = {};
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer.buffer, offsets);
	vkCmdBindIndexBuffer(commandBuffer, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);
	afDrawIndexed(3);
//	afDraw(3);
}

void Triangle::Create()
{
	VkDevice device = deviceMan.GetDevice();
	const VkDescriptorSetLayoutBinding descriptorSetLayoutBindings[] = { { 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT } };
	const VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, nullptr, 0, arrayparam(descriptorSetLayoutBindings) };
	afHandleVKError(vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayouts[0]));

	const VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0, arrayparam(descriptorSetLayouts) };
	afHandleVKError(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));

	const VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, nullptr, deviceMan.descriptorPool, arrayparam(descriptorSetLayouts) };
	afHandleVKError(vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &descriptorSet));

	pipeline = deviceMan.CreatePipeline("solid", pipelineLayout, arrayparam(attributes));

	TriangleVertex vertexPositions[3];
	for (int i = 0; i < 3; i++)
	{
		vertexPositions[i] =
		{
			Vec3(sin(i * (float)M_PI * 2 / 3), cos(i * (float)M_PI * 2 / 3), 0) * 0.5,
			Vec3(i == 0, i == 1, i == 2),
		};
	}
	vertexBuffer = afCreateVertexBuffer(sizeof(vertexPositions), vertexPositions);
	uniformBuffer = afCreateUBO(sizeof(Mat));
	unsigned short indexData[] = {0, 1, 2};
	indexBuffer = CreateBuffer(deviceMan.GetDevice(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, deviceMan.physicalDeviceMemoryProperties, sizeof(indexData), indexData);

	VkDescriptorBufferInfo descriptorBufferInfo = { uniformBuffer.buffer, 0, uniformBuffer.size };
	VkWriteDescriptorSet writeDescriptorSets[] = { { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, descriptorSet, 0, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, nullptr, &descriptorBufferInfo } };
	vkUpdateDescriptorSets(device, arrayparam(writeDescriptorSets), 0, nullptr);
}

void Triangle::Destroy()
{
	afSafeDeleteBufer(uniformBuffer);
	afSafeDeleteBufer(indexBuffer);
	afSafeDeleteBufer(vertexBuffer);
	VkDevice device = deviceMan.GetDevice();
	afSafeDeleteVk(vkDestroyPipeline, device, pipeline);
	afSafeDeleteVk(vkDestroyPipelineLayout, device, pipelineLayout);

	for (auto& it : descriptorSetLayouts)
	{
		afSafeDeleteVk(vkDestroyDescriptorSetLayout, device, it);
	}
}
