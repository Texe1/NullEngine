module;

#include "../utils/typedef.h"

export module Vulkan:Mesh;

import <vulkan/vulkan.h>;
import <string.h>;

import :Context;

export namespace render {
	export namespace vulkan {
		export class VulkanMesh {
		public:
			u32 nVertices;
			u32 nIndices;
			u8 active;
		private:
			u32 vertexOffset;
			u32 indexOffset;

			float* vertices;
			u32* indices;
		public:
			VulkanMesh(float*, u32, u32*, u32);
			void writeVertices(VulkanContext*, u32, VkDeviceMemory);
			void writeIndices(VulkanContext*, u32, VkDeviceMemory);
		};
	}
}

/*
 *
 * DEFINITIONS
 * 
 */

using namespace render::vulkan;

VulkanMesh::VulkanMesh(float* _vertices, u32 _nVertices, u32* _indices, u32 _nIndices) {
	this->active = 1;
	this->nVertices = _nVertices;
	this->nIndices = _nIndices;

	this->vertices = _vertices;
	this->indices = _indices;
}


void VulkanMesh::writeVertices(VulkanContext* ctx, u32 offset, VkDeviceMemory mem) {
	void* data;
	vkMapMemory(ctx->dvc.logic, mem, offset, sizeof(float) * 3 * this->nVertices, 0, &data);
	memcpy(data, this->vertices, sizeof(float) * 3 * this->nVertices);
	vkUnmapMemory(ctx->dvc.logic, mem);
}
void VulkanMesh::writeIndices(VulkanContext* ctx, u32 offset, VkDeviceMemory mem) {
	void* data;
	vkMapMemory(ctx->dvc.logic, mem, offset, sizeof(u32) * this->nIndices, 0, &data);
	memcpy(data, this->indices, sizeof(u32) * this->nIndices);
	vkUnmapMemory(ctx->dvc.logic, mem);
}