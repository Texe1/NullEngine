module;

#include "../utils/typedef.h"

module Renderer.Vulkan;

using namespace render::vulkan;

// assumes the same queue family for present and graphics
void VulkanRenderer::addCommandPool() {
	{
		VkCommandPoolCreateInfo info{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
		info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		info.queueFamilyIndex = ctx->dvc.queues.present.family;

		vkCreateCommandPool(ctx->dvc.logic, &info, 0, &(this->commandPool.handle));
	}

	{
		this->commandPool.buffers = (VkCommandBuffer*)malloc(sizeof(VkCommandBuffer));
		this->commandPool.nBuffers = 1;

		VkCommandBufferAllocateInfo info{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
		info.commandPool = this->commandPool.handle;
		info.commandBufferCount = 1;

		info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		vkAllocateCommandBuffers(ctx->dvc.logic, &info, (this->commandPool.buffers));
	}
}