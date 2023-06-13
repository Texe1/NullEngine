#include "../utils/typedef.h"
export module Renderer.Vulkan;

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
			VkInstance inst = NULL;
			VkDevice logicalDevice = NULL;
			VkPhysicalDevice physicalDevice = NULL;
			VkSurfaceKHR surface;
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
				u32 width;
				u32 height;
				VkFormat format;
				VkColorSpaceKHR colorSpace;
			} swapchain;

			// SDL window
			SDL_Window* wnd;
		public:
			VkContext(VkInstCfg&, VkDvcCfg&);
			VkContext(VkInstCfg& instCfg) : VkContext((VkInstCfg&)instCfg, (VkDvcCfg&)VkDvcCfg::defaultCfg) {}
			VkContext() : VkContext(VkInstCfg::defaultCfg) {}
			~VkContext();

		private:
			void addSwapchain();
			void initInstance(const VkInstCfg&);
			void initdevice(VkDvcCfg*);
		};
	} // namespace vulkan
} // namespace render
module :private;

import <SDL2/SDL.h>;
import <vulkan/vulkan.h>;
import <SDL2/SDL_vulkan.h>;

import <stdio.h>;
import <stdlib.h>;
import <string.h>;
import <stdexcept>;

using namespace render::vulkan;

VkContext::VkContext(VkInstCfg& instCfg, VkDvcCfg& dvcCfg) {
	this->initInstance(instCfg);

	this->wnd = SDL_CreateWindow("Window", 0, 0, 800, 600, SDL_WINDOW_VULKAN);
	SDL_Vulkan_CreateSurface(this->wnd, this->inst, &(this->surface));

	this->initdevice((VkDvcCfg*)&dvcCfg);

	if (dvcCfg.flags & VkDvcCfg::Flags::createQueuesGraphics) {
		this->addSwapchain();
	}
}

VkContext::~VkContext()
{
	vkDeviceWaitIdle(this->logicalDevice);

	if (this->swapchain.imgs)free(this->swapchain.imgs);

	INF(printf("destroying vulkan swapchain...\n"));
	if (this->swapchain.swapchain)vkDestroySwapchainKHR(this->logicalDevice, this->swapchain.swapchain, NULL);

	if (this->queues.present)free(this->queues.present);
	if (this->queues.graphics) {
		if(this->queues.graphics != this->queues.present)		
			free(this->queues.graphics);
	}
	if (this->queues.compute) {
		if ((this->queues.compute != this->queues.present)
			&& (this->queues.compute != this->queues.graphics))
			free(this->queues.compute);
	}

	INF(printf("destroying vulkan device...\n"));
	if (this->logicalDevice)vkDestroyDevice(this->logicalDevice, NULL);
	INF(printf("destroying vulkan surface...\n"));
	if (this->surface)vkDestroySurfaceKHR(this->inst, this->surface, NULL);
	INF(printf("destroying vulkan instance...\n"));
	if(this->inst)vkDestroyInstance(this->inst, NULL);
}


void VkContext::initInstance(const VkInstCfg& instCfg) {
	INF(printf("creating vulkan instance...\n"));

	// extensions
	u32 nExt = 0;
	{
		u32 nAvailableExt;
		vkEnumerateInstanceExtensionProperties(NULL, &nAvailableExt, NULL);

		if (nAvailableExt != 0) {
			VkExtensionProperties* exts = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * nAvailableExt);
			vkEnumerateInstanceExtensionProperties(NULL, &nAvailableExt, exts);

			while (nExt < instCfg.extensions.n) {
				u32 i = 0;
				for (; i < nAvailableExt; i++) {
					if (!strcmp(exts[i].extensionName, instCfg.extensions.names[nExt])) {
						break;
					}
				}
				if (i == nAvailableExt)
					break;

				nExt++;
			}

			if (exts)free(exts);

		}
		if (nExt < instCfg.extensions.nRequired) {
			throw std::runtime_error("Couldn't create Vulkan Instance: not all required Extensions are supported. Please update your graphics Driver and try again.");
		}
	}

	// layers
	u32 nLayers = 0;
	{
		if (instCfg.layers.n != 0) {
			u32 nAvailableLayers;
			vkEnumerateInstanceLayerProperties(&nAvailableLayers, NULL);
			if (nAvailableLayers != 0) {
				VkLayerProperties* layers = (VkLayerProperties*)malloc(sizeof(VkLayerProperties) * nAvailableLayers);
				vkEnumerateInstanceLayerProperties(&nAvailableLayers, layers);

				while (nLayers < instCfg.layers.n) {
					u32 i = 0;
					for (; i < nAvailableLayers; i++) {
						if (!strcmp(layers[i].layerName, instCfg.layers.names[nLayers])) {
							break;
						}
					}

					if (i == nAvailableLayers) {
						break;
					}

					nLayers++;
				}

				free(layers);
			}
		}

		if (nLayers < instCfg.layers.nRequired) {
			throw std::runtime_error("Couldn't create Vulkan Instance: not all required Layers are supported. Please update your graphics Driver and try again.");
		}
	}

	// creating the instance
	VkInstanceCreateInfo info{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,

		.enabledLayerCount = nLayers,
		.ppEnabledLayerNames = (nLayers) ? instCfg.layers.names : NULL,

		.enabledExtensionCount = nExt,
		.ppEnabledExtensionNames = (nExt) ? instCfg.extensions.names : NULL,
	};
	vkCreateInstance(&info, NULL, &this->inst);
}
void VkContext::initdevice(VkDvcCfg* cfg) {

	INF(printf("Initializing logical device...\n > choosing physical device...\n"));

	// get physical devices
	u32 nPhysDevices = 0;
	VkPhysicalDevice* physDevices = 0;
	{
		vkEnumeratePhysicalDevices(this->inst, &nPhysDevices, NULL);
		if (nPhysDevices == 0) 
			return;
		physDevices = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * nPhysDevices);
		if (!physDevices)
			throw std::runtime_error("Out of heap Memory (WTF THIS SHOULD NOT HAPPEN!)");
		vkEnumeratePhysicalDevices(this->inst, &nPhysDevices, physDevices);
	}


	// finding highest5 scoring device
	i64 maxScore = -1;
	VkPhysicalDevice bestDevice = NULL;
	for (u32 i = 0; i < nPhysDevices; i++) {
		i64 score = cfg->ratePhysicalDevice(physDevices[i], this->surface);

		if (score > maxScore) {
			maxScore = score;
			bestDevice = physDevices[i];
		}
	}

	if (bestDevice == NULL) {
		throw std::runtime_error("coudn't find suitable device");
		return;
	}

	// collecting the extensions that will be enabled
	u32 nEnableExts;
	char** enableExts;
	{
		// copying required extensions
		enableExts = (char**)malloc(sizeof(char*) * cfg->extensions.n);
		memcpy(enableExts, cfg->extensions.names, sizeof(char*) * cfg->extensions.nRequired);
		nEnableExts = cfg->extensions.nRequired;

		if (cfg->extensions.n > cfg->extensions.nRequired) {
			// getting available Extensions
			u32 nAvailableExts = 0;
			VkExtensionProperties* availableExts = NULL;
			{
				vkEnumerateDeviceExtensionProperties(bestDevice, NULL, &nAvailableExts, NULL);
				if (nAvailableExts) {
					availableExts = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * nAvailableExts);
					vkEnumerateDeviceExtensionProperties(bestDevice, NULL, &nAvailableExts, availableExts);
				}
			}

			// adding all available optional extensions to [enableExts]
			if(nAvailableExts && availableExts) {
				for (u32 i = cfg->extensions.nRequired; i < cfg->extensions.n; i++) {
					u32 j = 0;
					for (; j < nAvailableExts; j++) {
						if (!strcmp(cfg->extensions.names[i], availableExts[j].extensionName))
							break;
					}
					if (j < nAvailableExts) {
						enableExts[nEnableExts++] = (char*)cfg->extensions.names[i];
					}
				}
			}
		}
	}

	// collecting the layers that will be enabled
	u32 nEnableLayers;
	char** enableLayers;
	{
		// copying required layers
		enableLayers = (char**)malloc(sizeof(char*) * cfg->layers.n);
		memcpy(enableLayers, cfg->layers.names, sizeof(char*) * cfg->layers.nRequired);
		nEnableLayers = cfg->layers.nRequired;

		if (cfg->layers.n > cfg->layers.nRequired) {
			// getting available Layers
			u32 nAvailableLayers = 0;
			VkLayerProperties* availableLayers = NULL;
			{
				vkEnumerateDeviceLayerProperties(bestDevice, &nAvailableLayers, NULL);
				if (nAvailableLayers) {
					availableLayers = (VkLayerProperties*)malloc(sizeof(VkLayerProperties) * nAvailableLayers);
					vkEnumerateDeviceLayerProperties(bestDevice, &nAvailableLayers, availableLayers);
				}
			}

			// adding all available optional layers to [enableLayers]
			if (nAvailableLayers && availableLayers) {
				for (u32 i = cfg->layers.nRequired; i < cfg->layers.n; i++) {
					u32 j = 0;
					for (; j < nAvailableLayers; j++) {
						if (!strcmp(cfg->layers.names[i], availableLayers[j].layerName))
							break;
					}
					if (j < nAvailableLayers) {
						enableLayers[nEnableLayers++] = (char*)cfg->layers.names[i];
					}
				}
			}
		}
	}

	//TODO currenty, only one queue for present, graphics, and compute
	// queues
	float queuePriority = 1.0f;
	u32 nQueueInfos = 0;
	VkDeviceQueueCreateInfo queueInfos[3];
	{
		u32 nQueueFams = 0;
		VkQueueFamilyProperties* queueFams;
		{
			vkGetPhysicalDeviceQueueFamilyProperties(bestDevice, &nQueueFams, NULL);
			queueFams = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * nQueueFams);
			vkGetPhysicalDeviceQueueFamilyProperties(bestDevice, &nQueueFams, queueFams);
		}

		// getting the right graphics families (most likely they will all be == 1)
		i32 graphicsFam = -1;
		i32 presentFam = -1;
		i32 computeFam = -1;

		u8 getGraphics = cfg->flags & VkDvcCfg::Flags::createQueuesGraphics;
		u8 getCompute = cfg->flags & VkDvcCfg::Flags::createQueuesCompute;

		for (u32 i = 0; i < nQueueFams; i++) {
			u8 graphics = (queueFams[i].queueFlags & VK_QUEUE_GRAPHICS_BIT);
			u8 compute = (queueFams[i].queueFlags & VK_QUEUE_COMPUTE_BIT);
			u32 present = 0;
			vkGetPhysicalDeviceSurfaceSupportKHR(bestDevice, i, this->surface, &present);

			if (present && graphics && compute) {
				presentFam = i;
				graphicsFam = i;
				computeFam = i;
				break;
			}

			if (!getCompute && present && graphics) {
				presentFam = i;
				graphicsFam = i;
				break;
			}
			if (!getGraphics && present && graphics) {
				presentFam = i;
				graphicsFam = i;
				break;
			}

			if (presentFam < 0 && present) {
				presentFam = i;
			}
			if (graphicsFam < 0 && graphics) {
				graphicsFam = i;
			}
			if (computeFam < 0 && compute) {
				computeFam = i;
			}
		}

		this->queues.families.present = presentFam;
		this->queues.families.graphics = graphicsFam;
		this->queues.families.compute = computeFam;

		// createInfo for present queue
		{
			queueInfos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfos[0].pQueuePriorities = &queuePriority;
			queueInfos[0].pNext = NULL;
			queueInfos[0].flags = 0;
			queueInfos[0].queueCount = 1;// TODO figure out a better number (NVIDIA Vulkan do's and don'ts has some random formula)
			queueInfos[0].queueFamilyIndex = presentFam;

			nQueueInfos = 1;
		}

		// createInfo for graphics queue
		if (getGraphics && (graphicsFam != presentFam)) {

			queueInfos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfos[1].pQueuePriorities = &queuePriority;
			queueInfos[1].pNext = NULL;
			queueInfos[1].flags = 0;
			queueInfos[1].queueCount = 1;// TODO figure out a better number (NVIDIA Vulkan do's and don'ts has some random formula)
			queueInfos[1].queueFamilyIndex = graphicsFam;

			nQueueInfos = 2;
		}

		if (getCompute && (computeFam != presentFam) && (!getGraphics || (computeFam != graphicsFam))) {

			queueInfos[nQueueInfos].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfos[nQueueInfos].pQueuePriorities = &queuePriority;
			queueInfos[nQueueInfos].pNext = NULL;
			queueInfos[nQueueInfos].flags = 0;
			queueInfos[nQueueInfos].queueCount = 1;// TODO figure out a better number (NVIDIA Vulkan do's and don'ts has some random formula)
			queueInfos[nQueueInfos].queueFamilyIndex = computeFam;

			nQueueInfos++;
		}
	}

	// creating logical device and saving physical device
	{
		this->physicalDevice = bestDevice;

		VkDeviceCreateInfo info{
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,

			.queueCreateInfoCount = nQueueInfos,
			.pQueueCreateInfos = queueInfos,

			.enabledLayerCount = nEnableLayers,
			.ppEnabledLayerNames = enableLayers,

			.enabledExtensionCount = nEnableExts,
			.ppEnabledExtensionNames = enableExts,
		};

		vkCreateDevice(this->physicalDevice, &info, NULL, &(this->logicalDevice));
	}

	// getting Device Queues
	{
		this->queues.present = NULL;
		this->queues.graphics = NULL;
		this->queues.compute = NULL;

		if (this->queues.families.present >= 0) {
			this->queues.present = (VkQueue*)malloc(sizeof(VkQueue));
			vkGetDeviceQueue(this->logicalDevice, this->queues.families.present, 0, this->queues.present);
		}

		if (this->queues.families.graphics >= 0) {
			if (this->queues.families.graphics == this->queues.families.present) {
				this->queues.graphics = this->queues.present;
			}
			else {

				this->queues.graphics = (VkQueue*)malloc(sizeof(VkQueue));
				vkGetDeviceQueue(this->logicalDevice, this->queues.families.graphics, 0, this->queues.graphics);
			}
		}

		if (this->queues.families.compute >= 0) {
			if (this->queues.families.compute == this->queues.families.present) {
				this->queues.compute = this->queues.present;
			}
			else if (this->queues.families.compute == this->queues.families.graphics) {
				this->queues.compute = this->queues.graphics;
			}
			else {
				this->queues.compute = (VkQueue*)malloc(sizeof(VkQueue));
				vkGetDeviceQueue(this->logicalDevice, this->queues.families.compute, 0, this->queues.compute);
			}
		}
	}

}

void VkContext::addSwapchain() {
	if (this->queues.families.present != this->queues.families.graphics) {
		throw new std::runtime_error("present and graphics queue Are different, handling this is currently not supported");
		return;
	}


	VkSurfaceFormatKHR format = {};
	{
		u32 nFormats = 0;
		VkSurfaceFormatKHR* surfaceFormats = NULL;
		vkGetPhysicalDeviceSurfaceFormatsKHR(this->physicalDevice, this->surface, &nFormats, NULL);
		if (nFormats) { // Note: nFormats will essentially always be > 0
			surfaceFormats = (VkSurfaceFormatKHR*)malloc(sizeof(VkSurfaceFormatKHR) * nFormats);
			vkGetPhysicalDeviceSurfaceFormatsKHR(this->physicalDevice, this->surface, &nFormats, surfaceFormats);
			format = surfaceFormats[0];
			free(surfaceFormats);
		}
		else {
			throw new std::runtime_error("No surface Formats available for chosen physical Device.");
			return;
		}
	}

	VkExtent2D surfaceExtent = {};
	u32 minImgCount = 3;
	{
		VkSurfaceCapabilitiesKHR surfaceCaps;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(this->physicalDevice, this->surface, &surfaceCaps);

		if (surfaceCaps.maxImageCount) minImgCount = MIN(surfaceCaps.maxImageCount, minImgCount);
		minImgCount = MAX(minImgCount, surfaceCaps.minImageCount);
		surfaceExtent = surfaceCaps.currentExtent;

		if (surfaceExtent.width == 0xffffffff) {
			surfaceExtent.width = surfaceCaps.minImageExtent.width;
		}
		if (surfaceExtent.height == 0xffffffff) {
			surfaceExtent.height = surfaceCaps.minImageExtent.height;
		}
	}

	// creating Swapchain
	{
		VkSwapchainCreateInfoKHR info{};
		info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		info.surface = this->surface;
		info.minImageCount = minImgCount;
		info.imageFormat = format.format;
		info.imageColorSpace = format.colorSpace;
		info.imageExtent = surfaceExtent;
		info.imageArrayLayers = 1;
		info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
		vkCreateSwapchainKHR(this->logicalDevice, &info, 0, &(this->swapchain.swapchain));
	}

	// getting information about the created Swapchain
	{
		this->swapchain.format = format.format;
		this->swapchain.colorSpace = format.colorSpace;

		this->swapchain.width = surfaceExtent.width;
		this->swapchain.height = surfaceExtent.height;

		vkGetSwapchainImagesKHR(this->logicalDevice, this->swapchain.swapchain, &(this->swapchain.nImgs), NULL);
		if (this->swapchain.nImgs > 0) {
			this->swapchain.imgs = (VkImage*)malloc(sizeof(VkImage) * this->swapchain.nImgs);
			vkGetSwapchainImagesKHR(this->logicalDevice, this->swapchain.swapchain, &(this->swapchain.nImgs), this->swapchain.imgs);
		}
		else {
			this->swapchain.imgs = 0;
		}
	}
}