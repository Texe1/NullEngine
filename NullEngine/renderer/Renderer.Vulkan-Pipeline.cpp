module;

#include "../utils/typedef.h"
#include <stdio.h>
#include <vulkan/vulkan.h>

module Renderer.Vulkan;

using namespace render::vulkan;

VkShaderModule createShaderModule(const char* filename, VkDevice dvc) {
	char* code;
	u32 len;
	{
		FILE* f;
		fopen_s(&f, filename, "rb");

		fseek(f, 0, SEEK_END);
		u32 sz = ftell(f);

		char* s = (char*)malloc((u64)sz + 1);
		if (!s) {
			code = 0;
			len = 0;
			return 0;
		}

		rewind(f);

		for (u32 i = 0; i < sz; i++) {
			u32 x = ftell(f);
			s[i] = fgetc(f);
		}
		(s)[sz] = 0;

		code = s;
		len = sz;
		fclose(f);
	}

	if (!code || (len == 0))
		return NULL;

	VkShaderModule shader = 0;
	{
		VkShaderModuleCreateInfo info{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
		info.codeSize = len;
		info.pCode = (u32*)code;

		vkCreateShaderModule(dvc, &info, 0, &shader);

		free(code);
	}

	return shader;
}

void VulkanRenderer::addPipeline() {
	VkShaderModule vertShader = createShaderModule("D:/Programming/C/NullEngine/NullEngine/shaders/vert.spv", ctx->dvc.logic);
	VkShaderModule fragShader = createShaderModule("D:/Programming/C/NullEngine/NullEngine/shaders/frag.spv", ctx->dvc.logic);

	VkPipelineShaderStageCreateInfo shaderStages[2]{};
	{
		shaderStages[0] = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
		shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shaderStages[0].module = vertShader;
		shaderStages[0].pName = "main";
		shaderStages[1] = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
		shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderStages[1].module = fragShader;
		shaderStages[1].pName = "main";
	}

	{
		VkPipelineLayoutCreateInfo info = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
		vkCreatePipelineLayout(ctx->dvc.logic, &info, 0, &(this->pipeline.layout));
	}

	VkPipelineVertexInputStateCreateInfo vertInputState{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
	{
		inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	}
	VkPipelineViewportStateCreateInfo viewPortState{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
	VkViewport viewPort;
	VkRect2D scissor;
	{
		viewPort = { 0,0, (float)(this->swapchain.width), (float)(this->swapchain.height) };
		scissor = { 0,0, this->swapchain.width, this->swapchain.height };

		viewPortState.viewportCount = 1;
		viewPortState.pViewports = &viewPort;
		viewPortState.scissorCount = 1;
		viewPortState.pScissors = &scissor;
	}
	VkPipelineRasterizationStateCreateInfo rasterizationState{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
	{
		rasterizationState.lineWidth = 1.0f;
	}
	VkPipelineMultisampleStateCreateInfo multiSampleState{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
	{
		multiSampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	}

	VkPipelineColorBlendAttachmentState colorBlendAttachment{ };
	VkPipelineColorBlendStateCreateInfo colorBlendState{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
	{
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		colorBlendState.attachmentCount = 1;
		colorBlendState.pAttachments = &colorBlendAttachment;
	}

	// create pipeline
	{
		VkGraphicsPipelineCreateInfo info{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
		info.stageCount = 2;
		info.pStages = shaderStages;
		info.pVertexInputState = &vertInputState;
		info.pInputAssemblyState = &inputAssemblyState;
		info.pViewportState = &viewPortState;
		info.pRasterizationState = &rasterizationState;
		info.pMultisampleState = &multiSampleState;
		info.pColorBlendState = &colorBlendState;
		info.layout = this->pipeline.layout;
		info.renderPass = this->renderPass.renderPass;
		info.subpass = 0;
		vkCreateGraphicsPipelines(ctx->dvc.logic, 0, 1, &info, 0, &(this->pipeline.handle));
	}

	// Shader Modules are no longer needed
	{
		vkDestroyShaderModule(ctx->dvc.logic, vertShader, 0);
		vkDestroyShaderModule(ctx->dvc.logic, fragShader, 0);
	}
}