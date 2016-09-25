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
	afBindTexture(pipelineLayout, texture, 1);
	VkCommandBuffer commandBuffer = deviceMan.commandBuffer;
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
}

void Sky::Destroy()
{
	DeleteTexture(texture);
	VkDevice device = deviceMan.GetDevice();
	afSafeDeleteVk(vkDestroyPipeline, device, pipeline);
	afSafeDeleteVk(vkDestroyPipelineLayout, device, pipelineLayout);
}
