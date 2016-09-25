#include "stdafx.h"

Sky sky;

void Sky::Draw()
{
	Mat matV, matP;
	matrixMan.Get(MatrixMan::VIEW, matV);
	matrixMan.Get(MatrixMan::PROJ, matP);
	matV._41 = matV._42 = matV._43 = 0;
	Mat invVP = inv(matV * matP);
	afBindBuffer(pipelineLayout, sizeof(invVP), &invVP, 0);
	VkCommandBuffer commandBuffer = deviceMan.commandBuffer;
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &textureDescriptorSet, 0, nullptr);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	afDraw(4);
}

void Sky::Create()
{
	VkDevice device = deviceMan.GetDevice();

	VkDescriptorSetLayout layouts[] = { deviceMan.commonUboDescriptorSetLayout, deviceMan.commonTextureDescriptorSetLayout };
	const VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0, arrayparam(layouts)};
	afHandleVKError(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));

	TexDesc desc;
	pipeline = deviceMan.CreatePipeline("sky_photosphere", pipelineLayout, 0, nullptr);
	//texture = afLoadTexture("yangjae.dds", desc);
	texture = afLoadTexture("yangjae_mip.dds", desc);
	//texture = afLoadTexture("hakodate.jpg", desc);

	//pipeline = deviceMan.CreatePipeline("sky_cubemap", pipelineLayout, 0, nullptr);
	//texture = afLoadTexture("cube.dds", desc);

	const VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, nullptr, deviceMan.descriptorPool, 1, &deviceMan.commonTextureDescriptorSetLayout };
	afHandleVKError(vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &textureDescriptorSet));
	const VkDescriptorImageInfo descriptorImageInfo = { deviceMan.sampler, texture.view, VK_IMAGE_LAYOUT_GENERAL };
	const VkWriteDescriptorSet writeDescriptorSets[] =
	{
		{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, textureDescriptorSet, 0, 0, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &descriptorImageInfo },
	};
	vkUpdateDescriptorSets(device, arrayparam(writeDescriptorSets), 0, nullptr);
}

void Sky::Destroy()
{
	DeleteTexture(texture);
	VkDevice device = deviceMan.GetDevice();
	afSafeDeleteVk(vkDestroyPipeline, device, pipeline);
	afSafeDeleteVk(vkDestroyPipelineLayout, device, pipelineLayout);
	if (textureDescriptorSet)
	{
		afHandleVKError(vkFreeDescriptorSets(device, deviceMan.descriptorPool, 1, &textureDescriptorSet));
		textureDescriptorSet = 0;
	}
}
