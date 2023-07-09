module;

#include "../utils/typedef.h"

export module Vulkan:Renderer;

import :CtxStructs;
import :Context;
import :Mesh;

import <vulkan/vulkan.h>;
import <vector>;

export namespace render {
	export namespace vulkan {
		export class VulkanRenderer {
		private:
			VulkanContext* ctx;
			struct {
				std::vector<VulkanMesh*> meshes;
				buffer_data vertex;
				buffer_data index;
			} buffers;
			// TMP
			pipeline_data pipeline;
			// End of TMP
		public:
			VulkanRenderer(VulkanContext*, renderpass_data*);
			inline void addMesh(VulkanMesh*);
			void render(VkCommandBuffer);
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

	buffers.vertex = { };
	buffers.vertex.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	buffers.vertex.props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

	buffers.index = { };
	buffers.index.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	buffers.index.props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
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
		VkVertexInputAttributeDescription attributeDesc{};
		VkVertexInputBindingDescription bindingDesc{};
		{
			bindingDesc.binding = 0;
			bindingDesc.stride = sizeof(float) * 3;
			bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			attributeDesc.binding = 0;
			attributeDesc.location = 0;
			attributeDesc.format = VK_FORMAT_R32G32_SFLOAT;
			attributeDesc.offset = 0;

			vertInputState.vertexAttributeDescriptionCount = 1;
			vertInputState.pVertexAttributeDescriptions = &attributeDesc;
			vertInputState.vertexBindingDescriptionCount = 1;
			vertInputState.pVertexBindingDescriptions = &bindingDesc;
		}
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
			colorBlendAttachment.blendEnable = 0;

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

inline void VulkanRenderer::addMesh(VulkanMesh* m) {
	buffers.meshes.push_back(m);

	u32 nVerts = 0;
	u32 nIdx = 0;
	for (u32 i = 0; i < buffers.meshes.size(); i++) {
		nVerts += buffers.meshes.at(i)->nVertices;
		nIdx += buffers.meshes.at(i)->nIndices;
	}

	ctx->resizeBuffer(buffers.vertex, sizeof(float) * 3 * nVerts);
	ctx->resizeBuffer(buffers.index, sizeof(int) * nIdx);

	nVerts = 0;
	nIdx = 0;
	for (u32 i = 0; i < buffers.meshes.size(); i++) {
		VulkanMesh* mesh = buffers.meshes.at(i);

		mesh->writeVertices(ctx, sizeof(float) * 3 * nVerts, buffers.vertex.memory);
		mesh->writeIndices(ctx, sizeof(int) * nIdx, buffers.index.memory);

		nVerts += mesh->nVertices;
		nIdx += mesh->nIndices;
	}
}

void VulkanRenderer::render(VkCommandBuffer cmdBuf) {
	vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipeline.handle);
	u32 nVerts = 0;
	u32 nIdx = 0;
	for (u32 i = 0; i < buffers.meshes.size(); i++) {
		VulkanMesh* mesh = buffers.meshes.at(i);

		if (mesh->active) {
			VkDeviceSize offset = 3 * sizeof(float) * nVerts;
			vkCmdBindVertexBuffers(cmdBuf, 0, 1, &(buffers.vertex.handle), &offset);
			vkCmdBindIndexBuffer(cmdBuf, buffers.index.handle, 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(cmdBuf, mesh->nIndices, 1, 0, 0, 0);
		}

		nVerts += mesh->nVertices;
		nIdx += mesh->nIndices;
	}
}

VulkanRenderer::~VulkanRenderer() {
	pipeline.destroy(ctx->dvc.logic);
	this->buffers.vertex.destroy(ctx->dvc.logic);
	this->buffers.index.destroy(ctx->dvc.logic);
}