export module Renderer.Vulkan;

#include "../utils/typedef.h"

import :CfgStructs;
import :Context;

import <vulkan/vulkan.h>;
import <SDL2/SDL.h>;

export namespace render {
	export namespace vulkan {
		export class VulkanRenderer
		{
		public:
		private:
			// Vulkan stuff
			VulkanContext* ctx;

			/*
			VkInstance inst = 0;
			VkDevice logicalDevice = 0;
			VkPhysicalDevice physicalDevice = 0;
			VkSurfaceKHR surface = 0;
			struct {
				struct {
					i32 present;
					i32 graphics;
					i32 compute;
				} families;

				VkQueue* present;
				VkQueue* graphics;
				VkQueue* compute;
			} queues;
			*/
			struct {
				VkSwapchainKHR swapchain;
				u32 nImgs;
				VkImage* imgs;
				VkImageView* imgViews;
				VkFramebuffer* framebuffers;
				u32 width;
				u32 height;
				VkFormat format;
				VkColorSpaceKHR colorSpace;
			} swapchain;
			struct {
				VkRenderPass renderPass;
			} renderPass;
			struct {
				VkCommandPool handle;
				u32 nBuffers;
				VkCommandBuffer* buffers;
			} commandPool;
			struct {
				VkPipeline handle;
				VkPipelineLayout layout;

			} pipeline;

			struct {
				VkFence inFlightFence;
				VkSemaphore acquireSemaphore;
				VkSemaphore releaseSemaphore;
			} sync;

			// SDL window
			SDL_Window* wnd;
		public:
			VulkanRenderer(VkInstCfg& instCfg, VkDvcCfg& dvcCfg);
			VulkanRenderer(VkInstCfg& cfg) : VulkanRenderer(cfg, (VkDvcCfg&)VkDvcCfg::defaultCfg) {}
			VulkanRenderer(VkDvcCfg& cfg) : VulkanRenderer((VkInstCfg&)VkInstCfg::defaultCfg, cfg) {}
			VulkanRenderer() : VulkanRenderer((VkInstCfg&)VkInstCfg::defaultCfg, (VkDvcCfg&)VkDvcCfg::defaultCfg) {}

			int render();

			~VulkanRenderer();

		private:
			//void initInstance(const VkInstCfg&);
			//void initdevice(VkDvcCfg*);
			void addSwapchain();
			void addRenderPass();
			void addCommandPool();
			void addPipeline();
		};
	} // namespace vulkan
} // namespace render


module :private;

import <stdexcept>;
import <stdio.h>;
import <SDL2/SDL.h>;
import <SDL2/SDL_vulkan.h>;