module;

#include "../utils/typedef.h"
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <vulkan/vulkan.h>

export module Renderer.Vulkan:Window;

import :Context;
import :CtxStructs;
#define SDL_MAIN_HANDLED
import <SDL2/SDL.h>;
import <vulkan/vulkan.h>;
import <SDL2/SDL_vulkan.h>;
import <stdexcept>;

export namespace render {
	export namespace vulkan {
		export class VulkanWindow {
			VulkanContext* ctx;

			swapchain_data swapchain;
			renderpass_data renderpass;
			pipeline_data pipeline;
			command_pool_data command_pool;
			
			// TMP
			struct {
				u32 nFences;
				VkFence* fences;
				u32 nSemaphores;
				VkSemaphore* semaphores;
			} sync_objs;
			// end of TMP

			SDL_Window* wnd;
			VkSurfaceKHR surface;
		public:
			VulkanWindow(VulkanContext* ctx, const char* title, u32 width, u32 height);

			int render();// TMP

			~VulkanWindow();

		private:
			void resize();
		};
	}
}

/*
 *
 *		DEFINITIONS
 *
 */


using namespace render::vulkan;

VulkanWindow::VulkanWindow(VulkanContext* ctx, const char* title, u32 width, u32 height) {
	this->ctx = ctx;
	this->wnd = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
	SDL_Vulkan_CreateSurface(this->wnd, ctx->inst, &(this->surface));

	ctx->createSwapchain(this->surface, 3, VK_PRESENT_MODE_FIFO_KHR, &(this->swapchain));

	// TMP: renderPass
	{
		VkRenderPassCreateInfo info{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };

		VkAttachmentReference attachmentRef{ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

		VkAttachmentDescription attachment{};
		attachment.format = this->swapchain.format;
		attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkSubpassDescription subPass{};
		subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subPass.colorAttachmentCount = 1;
		subPass.pColorAttachments = &attachmentRef;

		info.attachmentCount = 1;
		info.pAttachments = &attachment; // <-
		info.subpassCount = 1;
		info.pSubpasses = &subPass; // <-

		ctx->createRenderPass(&info, &renderpass);
	}

	// TMP: framebuffers
	{
		VkFramebufferCreateInfo info{};
		info.layers = 1;
		info.renderPass = renderpass.handle;
		swapchain.createFramebuffers(ctx->dvc.logic, &info);
	}

	// TMP: pipeline
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

		{
			info.stageCount = 2;
			info.pStages = shaderStages;
			info.pVertexInputState = &vertInputState;
			info.pInputAssemblyState = &inputAssemblyState;
			info.pViewportState = &viewPortState;
			info.pRasterizationState = &rasterizationState;
			info.pMultisampleState = &multiSampleState;
			info.pColorBlendState = &colorBlendState;
			info.layout = this->pipeline.layout;
			info.renderPass = renderpass.handle;
			info.subpass = 0;
		}

		ctx->createGraphicsPipeline(&layoutInfo, &info, &pipeline);

		vkDestroyShaderModule(ctx->dvc.logic, vert, NULL);
		vkDestroyShaderModule(ctx->dvc.logic, frag, NULL);
	}

	// TMP: command_pool
	{	
		ctx->createCommandPool(1, &command_pool);
	}

	// TMP: sync
	{
		sync_objs.nFences = 1;
		sync_objs.fences = new VkFence[1];
		ctx->createFences(1, sync_objs.fences);
		sync_objs.nSemaphores = 2;
		sync_objs.semaphores = new VkSemaphore[2];
		ctx->createSemaphores(2, sync_objs.semaphores);
	}
 }

int VulkanWindow::render() {
	 // SDL events, TMP: only works when only 1 window is present
	 {
		 SDL_Event evt;
		 while (SDL_PollEvent(&evt)) {
			 if (evt.type == SDL_QUIT)
				 return 0;
		 }
	 }

	 // actual rendering
	 {
		 u32 imgIdx = 0;
		 VkResult result = vkAcquireNextImageKHR(ctx->dvc.logic, swapchain.handle, 0xffffffffffffffff, sync_objs.semaphores[0], 0, &imgIdx);
		 if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) { 
			 // window has been resized -> swapchain out of date
			 this->resize(); 
			 return 1;
		 }
		 if (result != VK_SUCCESS) {
			 throw new std::runtime_error("vkAcquireNextImageKHR returned with non-zero error code.");
			 exit(-1);
		 }

		 vkWaitForFences(ctx->dvc.logic, 1, &(sync_objs.fences[0]), 1U, 0xffffffffffffffff);
		 vkResetFences(ctx->dvc.logic, 1, &(sync_objs.fences[0]));

		 vkResetCommandPool(ctx->dvc.logic, command_pool.handle, 0);

		 VkCommandBuffer cmdBuf = command_pool.buffers[0];

		 VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		 beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		 vkBeginCommandBuffer(cmdBuf, &beginInfo);
		 {
			 VkClearValue clearVal = { .color = {0.0f, 0.0f, 0.1f, 1.0f} };

			 VkRenderPassBeginInfo renderPassBeginInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
			 renderPassBeginInfo.renderPass = renderpass.handle;
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
		 submitInfo.pCommandBuffers = &(command_pool.buffers[0]);
		 submitInfo.waitSemaphoreCount = 1;
		 submitInfo.pWaitSemaphores = &(sync_objs.semaphores[0]);
		 submitInfo.pWaitDstStageMask = &waitMask;

		 submitInfo.signalSemaphoreCount = 1;
		 submitInfo.pSignalSemaphores = &(sync_objs.semaphores[1]);

		 vkQueueSubmit(ctx->dvc.queues.graphics.queue[0], 1, &submitInfo, sync_objs.fences[0]);

		 VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
		 presentInfo.swapchainCount = 1;
		 presentInfo.pSwapchains = &(swapchain.handle);
		 presentInfo.pImageIndices = &imgIdx;
		 presentInfo.waitSemaphoreCount = 1;
		 presentInfo.pWaitSemaphores = &(sync_objs.semaphores[1]);
		 result = vkQueuePresentKHR(ctx->dvc.queues.graphics.queue[0], &presentInfo);
		 if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
			 this->resize();
		 }
	 }

	 return 1;
 }

VulkanWindow::~VulkanWindow() {

	vkDeviceWaitIdle(ctx->dvc.logic);

	for (u32 i = 0; i < sync_objs.nFences; i++) {
		vkDestroyFence(ctx->dvc.logic, sync_objs.fences[i], NULL);
	}
	free(sync_objs.fences);
	for (u32 i = 0; i < sync_objs.nSemaphores; i++) {
		vkDestroySemaphore(ctx->dvc.logic, sync_objs.semaphores[i], NULL);
	}
	free(sync_objs.semaphores);

	command_pool.destroy(ctx->dvc.logic);
	pipeline.destroy(ctx->dvc.logic);
	vkDestroyRenderPass(ctx->dvc.logic, renderpass.handle, NULL);

	swapchain.destroy(ctx->dvc.logic);
	vkDestroySurfaceKHR(ctx->inst, surface, NULL);
}

void VulkanWindow::resize() {


	VkSurfaceCapabilitiesKHR surfaceCaps;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(ctx->dvc.phys, surface, &surfaceCaps);
	if (surfaceCaps.currentExtent.width == 0 || surfaceCaps.currentExtent.height == 0) {
		return;
	}

	INF(printf("resizing Window:\n"));

	INF(printf(" > recreating Swapchain...\n"));
	swapchain_data new_swapchain;
	ctx->WaitIdle();
	(void)ctx->createSwapchain(surface, 3, VK_PRESENT_MODE_FIFO_KHR, &new_swapchain, swapchain.handle);

	swapchain.destroy(ctx->dvc.logic);
	swapchain = new_swapchain;

	// TMP: recreate render Pass
	{

		INF(printf(" > recreating RenderPass...\n"));
		if (renderpass.handle) {
			vkDestroyRenderPass(ctx->dvc.logic, renderpass.handle, NULL);
		}

		VkRenderPassCreateInfo info{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };

		VkAttachmentReference attachmentRef{ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

		VkAttachmentDescription attachment{};
		attachment.format = this->swapchain.format;
		attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkSubpassDescription subPass{};
		subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subPass.colorAttachmentCount = 1;
		subPass.pColorAttachments = &attachmentRef;

		info.attachmentCount = 1;
		info.pAttachments = &attachment; // <-
		info.subpassCount = 1;
		info.pSubpasses = &subPass; // <-

		ctx->createRenderPass(&info, &renderpass);
	}

	// TMP: create framebuffers
	{

		INF(printf(" > recreating frameBuffers...\n"));
		VkFramebufferCreateInfo info{};
		info.layers = 1;
		info.renderPass = renderpass.handle;
		swapchain.createFramebuffers(ctx->dvc.logic, &info);
	}

	// TMP: recreate Pipeline
	{
		INF(printf(" > recreating pipeline...\n"));
		pipeline.destroy(ctx->dvc.logic);
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

		{
			info.stageCount = 2;
			info.pStages = shaderStages;
			info.pVertexInputState = &vertInputState;
			info.pInputAssemblyState = &inputAssemblyState;
			info.pViewportState = &viewPortState;
			info.pRasterizationState = &rasterizationState;
			info.pMultisampleState = &multiSampleState;
			info.pColorBlendState = &colorBlendState;
			info.layout = this->pipeline.layout;
			info.renderPass = renderpass.handle;
			info.subpass = 0;
		}

		ctx->createGraphicsPipeline(&layoutInfo, &info, &pipeline);

		vkDestroyShaderModule(ctx->dvc.logic, vert, NULL);
		vkDestroyShaderModule(ctx->dvc.logic, frag, NULL);
	}
}