module;

#include "../utils/typedef.h"

export module Vulkan:CtxStructs;

import <vulkan/vulkan.h>;
import <stdlib.h>;

namespace render {
	namespace vulkan {
		struct swapchain_data {
			VkSwapchainKHR handle;

			u32 nImgs;
			VkImage* imgs;
			VkImageView* imgViews;
			VkFramebuffer* framebuffers;
			u32 width;
			u32 height;
			VkFormat format;
			VkColorSpaceKHR colorSpace;

			swapchain_data* createImageViews(VkDevice, VkImageViewCreateInfo*);
			swapchain_data* createFramebuffers(VkDevice, VkFramebufferCreateInfo*);

			void destroy(VkDevice);
		};

		struct renderpass_data {
			VkRenderPass handle;
		};

		struct pipeline_data {
			VkPipeline handle;
			VkPipelineLayout layout;

			void destroy(VkDevice);
		};

		struct command_pool_data {
			VkCommandPool handle;
			u32 nBuffers;
			VkCommandBuffer* buffers;

			void destroy(VkDevice);

		};

		struct buffer_data {
			VkBuffer handle;
			VkDeviceMemory memory;
		};
	}
}

using namespace render::vulkan;

swapchain_data* swapchain_data::createImageViews(VkDevice dvc, VkImageViewCreateInfo* info) {
	info->sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	this->imgViews = new VkImageView[this->nImgs];

	for (u32 i = 0; i < this->nImgs; i++) {
		info->image = this->imgs[i];
		vkCreateImageView(dvc, info, 0, &(this->imgViews[i]));
	}
	return this;
}

swapchain_data* swapchain_data::createFramebuffers(VkDevice dvc, VkFramebufferCreateInfo* info) {
	if (!this->imgViews)
		return this;
	
	info->sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	info->width = this->width;
	info->height = this->height;

	this->framebuffers = new VkFramebuffer[this->nImgs];

	info->attachmentCount = 1;
	for (u32 i = 0; i < this->nImgs; i++) {
		info->pAttachments = &(this->imgViews[i]);
		vkCreateFramebuffer(dvc, info, 0, &(this->framebuffers[i]));
	}

	return this;
}

void swapchain_data::destroy(VkDevice dvc) {
	for (u32 i = 0; i < nImgs; i++) {
		if (framebuffers)vkDestroyFramebuffer(dvc, framebuffers[i], NULL);
		if (imgViews)vkDestroyImageView(dvc, imgViews[i], NULL);
	}

	if (framebuffers)free(framebuffers);
	if (imgViews)free(imgViews);

	vkDestroySwapchainKHR(dvc, handle, NULL);
}

void command_pool_data::destroy(VkDevice dvc) {
	if (buffers)vkFreeCommandBuffers(dvc, handle, nBuffers, buffers);
	vkDestroyCommandPool(dvc, handle, NULL);
}


void pipeline_data::destroy(VkDevice dvc) {
	vkDestroyPipeline(dvc, handle, NULL);
	vkDestroyPipelineLayout(dvc, layout, NULL);
}