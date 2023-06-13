export module Renderer.Vulkan;

#include "../utils/typedef.h"

import :CfgStructs;

import <vulkan/vulkan.h>;
import <SDL2/SDL.h>;

export namespace render {
	export namespace vulkan {
		export class VkContext
		{
		public:
		private:

			// Vulkan stuff
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

			// SDL window
			SDL_Window* wnd;
		public:
			VkContext(VkInstCfg&, VkDvcCfg&);
			VkContext(VkInstCfg& instCfg) : VkContext((VkInstCfg&)instCfg, (VkDvcCfg&)VkDvcCfg::defaultCfg) {}
			VkContext() : VkContext(VkInstCfg::defaultCfg) {}
			~VkContext();

		private:
			void addRenderPass();
			void addSwapchain();
			void initInstance(const VkInstCfg&);
			void initdevice(VkDvcCfg*);
		};
	} // namespace vulkan
} // namespace render


module :private;

import <stdexcept>;
import <stdio.h>;
import <SDL2/SDL.h>;
import <SDL2/SDL_vulkan.h>;