module Renderer.Vulkan;

#include "../utils/typedef.h"

using namespace render::vulkan;

int VkContext::render() {
	// SDL Messages
	{
		SDL_Event evt;
		while (SDL_PollEvent(&evt)) {
			if (evt.type == SDL_QUIT) {
				return 0;
			}
		}
	}

	// actual rendering
	{
		u32 imgIdx = 0;
		vkAcquireNextImageKHR(this->logicalDevice, this->swapchain.swapchain, 0xffffffffffffffff, this->sync.acquireSemaphore, 0, &imgIdx);


		vkWaitForFences(this->logicalDevice, 1, &(this->sync.inFlightFence), 1U, 0xffffffffffffffff);
		vkResetFences(this->logicalDevice, 1, &(this->sync.inFlightFence));

		vkResetCommandPool(this->logicalDevice, this->commandPool.handle, 0);

		VkCommandBuffer cmdBuf = this->commandPool.buffers[0];

		VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(cmdBuf, &beginInfo);
		{
			VkClearValue clearVal = { .color = {0.0f, 0.0f, 0.1f, 1.0f} };

			VkRenderPassBeginInfo renderPassBeginInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
			renderPassBeginInfo.renderPass = this->renderPass.renderPass;
			renderPassBeginInfo.framebuffer = this->swapchain.framebuffers[imgIdx];
			renderPassBeginInfo.renderArea = { {0,0}, {this->swapchain.width, this->swapchain.height} };

			renderPassBeginInfo.clearValueCount = 1;
			renderPassBeginInfo.pClearValues = &clearVal;

			vkCmdBeginRenderPass(cmdBuf, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
			{
				vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipeline.handle);

				vkCmdDraw(cmdBuf, 3, 1, 0, 0);
			}
			vkCmdEndRenderPass(cmdBuf);
		}
		vkEndCommandBuffer(cmdBuf);

		VkPipelineStageFlags waitMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &(this->commandPool.buffers[0]);
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &this->sync.acquireSemaphore;
		submitInfo.pWaitDstStageMask = &waitMask;

		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &this->sync.releaseSemaphore;

		vkQueueSubmit(this->queues.graphics[0], 1, &submitInfo, this->sync.inFlightFence);

		//vkWaitForFences(this->logicalDevice, 1, &(this->sync.inFlightFence), 1U, 0xffffffffffffffff);
		//vkResetFences(this->logicalDevice, 1, &(this->sync.inFlightFence));

		VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &(this->swapchain.swapchain);
		presentInfo.pImageIndices = &imgIdx;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &this->sync.releaseSemaphore;
		vkQueuePresentKHR(this->queues.graphics[0], &presentInfo);
	}
	return 1;
}