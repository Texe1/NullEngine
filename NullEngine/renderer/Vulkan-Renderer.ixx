module;

#include "../utils/typedef.h"

export module Vulkan:Renderer;

import :CtxStructs;
import :Context;

import <vulkan/vulkan.h>;

export namespace render {
	export namespace vulkan {
		export class VulkanRenderer {
		private:
			VulkanContext* ctx;
			// TMP
			pipeline_data pipeline;
			// End of TMP
		public:
			VulkanRenderer(VulkanContext*, renderpass_data*);
			void render(VkCommandBuffer, int, int);
			~VulkanRenderer();
		};
	}
};

/*
 *
 *		DEFINITIONS
 *
 */

using namespace render::vulkan;

VulkanRenderer::VulkanRenderer(VulkanContext* ctx, renderpass_data* renderpass) {
	this->ctx = ctx;

	// TMP: create pipeline
	{
		VkShaderModule vert = ctx->createShaderModule("D:/Programming/C/NullEngine/NullEngine/shaders/vert.spv");
		//VkShaderModule vert = ctx->createShaderModule("../../NullEngine/shaders/vert.spv"); // some weird thing makes this only run standalone, but not through VS
		VkShaderModule frag = ctx->createShaderModule("D:/Programming/C/NullEngine/NullEngine/shaders/frag.spv");

		VkPipelineShaderStageCreateInfo shaderStages[2]{};
		{
			shaderStages[0] = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
			shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
			shaderStages[0].module = vert;
			shaderStages[0].pName = "main";

			shaderStages[1] = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
			shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			shaderStages[1].module = frag;
			shaderStages[1].pName = "main";
		}

		VkPipelineLayoutCreateInfo layoutInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };

		VkGraphicsPipelineCreateInfo info{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };

		VkPipelineVertexInputStateCreateInfo vertInputState{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
		{
			inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		}
		VkPipelineViewportStateCreateInfo viewPortState{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
		{
			viewPortState.viewportCount = 1;
			viewPortState.scissorCount = 1;
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

		VkPipelineDynamicStateCreateInfo dynamicState{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
		VkDynamicState dynStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		{
			dynamicState.dynamicStateCount = 2;
			dynamicState.pDynamicStates = dynStates;
		}

		{
			info.stageCount = 2;
			info.pStages = shaderStages;
			info.pVertexInputState = &vertInputState;
			info.pInputAssemblyState = &inputAssemblyState;
			info.pViewportState = &viewPortState;
			info.pRasterizationState = &rasterizationState;
			info.pMultisampleState = &multiSampleState;
			info.pColorBlendState = &colorBlendState;
			info.pDynamicState = &dynamicState;
			info.layout = this->pipeline.layout;
			info.renderPass = renderpass->handle;
			info.subpass = 0;
		}

		ctx->createGraphicsPipeline(&layoutInfo, &info, &pipeline);

		vkDestroyShaderModule(ctx->dvc.logic, vert, NULL);
		vkDestroyShaderModule(ctx->dvc.logic, frag, NULL);
	}
}

void VulkanRenderer::render(VkCommandBuffer cmdBuf, int width, int height) {
	vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipeline.handle);
	VkViewport viewport = { 0.0f, 0.0f, (float)width, (float)height };
	VkRect2D scissor = { {0,0 }, { width, height } };

	vkCmdSetViewport(cmdBuf, 0, 1, &viewport);
	vkCmdSetScissor(cmdBuf, 0, 1, &scissor);
	vkCmdDraw(cmdBuf, 3, 1, 0, 0);
}

VulkanRenderer::~VulkanRenderer() {
	pipeline.destroy(ctx->dvc.logic);
}