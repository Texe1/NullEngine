module;

#include "../utils/typedef.h"
#define SDL_MAIN_HANDLED
#include "SDL2/SDL.h"

module Renderer.Vulkan;

using namespace render::vulkan;

VulkanRenderer::VulkanRenderer(VkInstCfg& instCfg, VkDvcCfg& dvcCfg) {
	SDL_Init(SDL_INIT_VIDEO);
	this->wnd = SDL_CreateWindow("NullEngine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_VULKAN);
	this->ctx = new VulkanContext(instCfg, dvcCfg, wnd);

	if (dvcCfg.flags & VkDvcCfg::Flags::createQueuesGraphics) {
		this->addSwapchain();
		this->addRenderPass();
		// creating Frambuffers, temporary
		{

			INF(printf("Creating %d imageviews and framebuffers...\n", this->swapchain.nImgs));

			this->swapchain.framebuffers = (VkFramebuffer*)malloc(sizeof(VkFramebuffer) * this->swapchain.nImgs);
			this->swapchain.imgViews = (VkImageView*)malloc(sizeof(VkImageView) * this->swapchain.nImgs);
			for (u32 i = 0; i < this->swapchain.nImgs; i++) {
				VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
				viewInfo.image = this->swapchain.imgs[i];
				viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				viewInfo.format = this->swapchain.format;
				viewInfo.components = {};
				viewInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0,1,0,1 };

				vkCreateImageView(ctx->dvc.logic, &viewInfo, 0, &(this->swapchain.imgViews[i]));

				VkFramebufferCreateInfo info{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
				info.renderPass = this->renderPass.renderPass;
				info.attachmentCount = 1;
				info.pAttachments = &(this->swapchain.imgViews[i]);

				info.width = this->swapchain.width;
				info.height = this->swapchain.height;

				info.layers = 1;

				vkCreateFramebuffer(ctx->dvc.logic, &info, 0, &(this->swapchain.framebuffers[i]));
			}
		}

		this->addCommandPool();

		// sync objects, temporary
		{
			{
				VkFenceCreateInfo info{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
				info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
				vkCreateFence(ctx->dvc.logic, &info, 0, &(this->sync.inFlightFence));
			}

			{
				VkSemaphoreCreateInfo info{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
				vkCreateSemaphore(ctx->dvc.logic, &info, 0, &(this->sync.acquireSemaphore));
				vkCreateSemaphore(ctx->dvc.logic, &info, 0, &(this->sync.releaseSemaphore));
			}
		}

		this->addPipeline();
	}
}
VulkanRenderer::~VulkanRenderer()
{
	vkDeviceWaitIdle(ctx->dvc.logic);

	if (this->pipeline.handle)vkDestroyPipeline(ctx->dvc.logic, this->pipeline.handle, 0);
	if (this->pipeline.layout)vkDestroyPipelineLayout(ctx->dvc.logic, this->pipeline.layout, 0);

	if(this->sync.inFlightFence)vkDestroyFence(ctx->dvc.logic, this->sync.inFlightFence, 0);
	if (this->sync.acquireSemaphore)vkDestroySemaphore(ctx->dvc.logic, sync.acquireSemaphore, 0);
	if (this->sync.releaseSemaphore)vkDestroySemaphore(ctx->dvc.logic, sync.releaseSemaphore, 0);

	if (this->commandPool.handle) {
		vkFreeCommandBuffers(ctx->dvc.logic, this->commandPool.handle, 1, this->commandPool.buffers);
		free(this->commandPool.buffers);
		vkDestroyCommandPool(ctx->dvc.logic, this->commandPool.handle, 0);
	}

	for (u32 i = 0; i < this->swapchain.nImgs; i++) {
		if (this->swapchain.framebuffers)vkDestroyFramebuffer(ctx->dvc.logic, this->swapchain.framebuffers[i], 0);
		if (this->swapchain.imgViews)vkDestroyImageView(ctx->dvc.logic, this->swapchain.imgViews[i], 0);
	}

	if (this->swapchain.framebuffers) free(this->swapchain.framebuffers);
	if (this->swapchain.imgViews) free(this->swapchain.imgViews);

	if (this->renderPass.renderPass) {
		INF(printf("destroying vulkan render pass...\n"));
		vkDestroyRenderPass(ctx->dvc.logic, this->renderPass.renderPass, 0);
	}

	if (this->swapchain.imgs)free(this->swapchain.imgs);

	INF(printf("destroying vulkan swapchain...\n"));
	if (this->swapchain.swapchain)vkDestroySwapchainKHR(ctx->dvc.logic, this->swapchain.swapchain, 0);

	delete ctx;
	SDL_Quit();
}