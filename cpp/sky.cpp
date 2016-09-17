#include "stdafx.h"

Sky sky;

void Sky::Draw()
{
	Mat mat = q2m(Quat(Vec3(0, 0, 1), (float)GetTime()));
	WriteBuffer(uniformBuffer, sizeof(mat), &mat);

	VkCommandBuffer commandBuffer = deviceMan.commandBuffer;
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	vkCmdDraw(commandBuffer, 4, 1, 0, 0);
}

void Sky::Create()
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

	pipeline = deviceMan.CreatePipeline("sky_photosphere", pipelineLayout, 0, nullptr, 0, nullptr);

	uniformBuffer = CreateBuffer(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, deviceMan.physicalDeviceMemoryProperties, sizeof(Mat), nullptr);

	VkDescriptorBufferInfo descriptorBufferInfo = { uniformBuffer.buffer, 0, uniformBuffer.size };
	VkWriteDescriptorSet writeDescriptorSets[] = { { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, descriptorSet, 0, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, nullptr, &descriptorBufferInfo } };
	vkUpdateDescriptorSets(device, arrayparam(writeDescriptorSets), 0, nullptr);

	TexDesc desc;
//	texture = afLoadTexture("yangjae.dds", desc);
	texture = afLoadTexture("hakodate.jpg", desc);

	const VkSamplerCreateInfo samplerCreateInfo = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
	vkCreateSampler(device, &samplerCreateInfo, nullptr, &sampler);
}

void Sky::Destroy()
{
	DeleteTexture(texture);
	DeleteBufer(uniformBuffer);
	VkDevice device = deviceMan.GetDevice();
	afSafeDeleteVk(vkDestroySampler, device, sampler);
	afSafeDeleteVk(vkDestroyPipeline, device, pipeline);
	afSafeDeleteVk(vkDestroyPipelineLayout, device, pipelineLayout);

	afSafeDeleteVk(vkDestroyDescriptorPool, device, descriptorPool);
	for (auto& it : descriptorSetLayouts)
	{
		afSafeDeleteVk(vkDestroyDescriptorSetLayout, device, it);
	}
}
