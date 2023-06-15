module;

#include "../utils/typedef.h"
#include "vulkan/vulkan.h"
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

export module Renderer.Vulkan:Context;

import :CfgStructs;

namespace render {
	namespace vulkan {
		class VulkanContext {
		public:
			VkInstance inst;
			struct {
				VkDevice logic;
				VkPhysicalDevice phys;
				struct {
					struct {
						i32 family;
						u32 n;
						VkQueue* queue;
					} present;
					struct {
						i32 family;
						u32 n;
						VkQueue* queue;
					} graphics;
					struct {
						i32 family;
						u32 n;
						VkQueue* queue;
					} compute;
				} queues;
			} dvc;
			SDL_Window* wnd;
			VkSurfaceKHR surface;

		private:
			void initInst(VkInstCfg&);
			void initDvc(VkDvcCfg&);

		public:
			VulkanContext(VkInstCfg& instCfg, VkDvcCfg& dvcCfg, SDL_Window* wnd) { this->wnd = wnd; initInst(instCfg); initDvc(dvcCfg); }
			VulkanContext(VkInstCfg& cfg, SDL_Window* wnd) : VulkanContext(cfg, (VkDvcCfg&)VkDvcCfg::defaultCfg, wnd) {}
			VulkanContext(VkDvcCfg& cfg, SDL_Window* wnd) : VulkanContext((VkInstCfg&)VkInstCfg::defaultCfg, cfg, wnd) {}
			VulkanContext(SDL_Window* wnd) : VulkanContext((VkInstCfg&)VkInstCfg::defaultCfg, (VkDvcCfg&)VkDvcCfg::defaultCfg, wnd) {}
			VulkanContext() {}
			~VulkanContext();
		};
	}
}



/*
 *	Definitions
 */

using namespace render::vulkan;


import <stdexcept>;

void VulkanContext::initInst(VkInstCfg& cfg) {
	INF(printf("creating vulkan instance...\n"));

	// extensions
	u32 nExt = 0;
	{
		u32 nAvailableExt;
		vkEnumerateInstanceExtensionProperties(0, &nAvailableExt, 0);

		if (nAvailableExt != 0) {
			VkExtensionProperties* exts = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * nAvailableExt);
			vkEnumerateInstanceExtensionProperties(0, &nAvailableExt, exts);

			while (nExt < cfg.extensions.n) {
				u32 i = 0;
				for (; i < nAvailableExt; i++) {
					if (!strcmp(exts[i].extensionName, cfg.extensions.names[nExt])) {
						break;
					}
				}
				if (i == nAvailableExt)
					break;

				nExt++;
			}

			if (exts)free(exts);

		}
		if (nExt < cfg.extensions.nRequired) {
			throw std::runtime_error("Couldn't create Vulkan Instance: not all required Extensions are supported. Please update your graphics Driver and try again.");
		}
	}

	// layers
	u32 nLayers = 0;
	{
		if (cfg.layers.n != 0) {
			u32 nAvailableLayers;
			vkEnumerateInstanceLayerProperties(&nAvailableLayers, 0);
			if (nAvailableLayers != 0) {
				VkLayerProperties* layers = (VkLayerProperties*)malloc(sizeof(VkLayerProperties) * nAvailableLayers);
				vkEnumerateInstanceLayerProperties(&nAvailableLayers, layers);

				while (nLayers < cfg.layers.n) {
					u32 i = 0;
					for (; i < nAvailableLayers; i++) {
						if (!strcmp(layers[i].layerName, cfg.layers.names[nLayers])) {
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

		if (nLayers < cfg.layers.nRequired) {
			throw std::runtime_error("Couldn't create Vulkan Instance: not all required Layers are supported. Please update your graphics Driver and try again.");
		}
	}

	// creating the instance
	VkInstanceCreateInfo info{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,

		.enabledLayerCount = nLayers,
		.ppEnabledLayerNames = (nLayers) ? cfg.layers.names : 0,

		.enabledExtensionCount = nExt,
		.ppEnabledExtensionNames = (nExt) ? cfg.extensions.names : 0,
	};
	VkResult res = vkCreateInstance(&info, 0, &this->inst);
}

void VulkanContext::initDvc(VkDvcCfg& cfg) {
	INF(printf("Initializing logical device...\n > choosing physical device...\n"));
	
	// creating surface
	SDL_Vulkan_CreateSurface(this->wnd, this->inst, &(this->surface));

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

	// finding highest scoring device
	i64 maxScore = -1;
	VkPhysicalDevice bestDevice = 0;
	for (u32 i = 0; i < nPhysDevices; i++) {
		i64 score = cfg.ratePhysicalDevice(physDevices[i], this->surface);

		if (score > maxScore) {
			maxScore = score;
			bestDevice = physDevices[i];
		}
	}

	if (bestDevice == NULL) {
		throw std::runtime_error("coudn't find suitable device");
		return;
	}
	this->dvc.phys = bestDevice;

	INF(
		VkPhysicalDeviceProperties phDvcProps;
		vkGetPhysicalDeviceProperties(bestDevice, &phDvcProps);
		printf(" > chose '%s'\n", phDvcProps.deviceName);
	);

	// collecting the extensions that will be enabled
	u32 nEnableExts;
	char** enableExts;
	{
		// copying required extensions
		enableExts = (char**)malloc(sizeof(char*) * cfg.extensions.n);
		memcpy(enableExts, cfg.extensions.names, sizeof(char*) * cfg.extensions.nRequired);
		nEnableExts = cfg.extensions.nRequired;

		if (cfg.extensions.n > cfg.extensions.nRequired) {
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
			if (nAvailableExts && availableExts) {
				for (u32 i = cfg.extensions.nRequired; i < cfg.extensions.n; i++) {
					u32 j = 0;
					for (; j < nAvailableExts; j++) {
						if (!strcmp(cfg.extensions.names[i], availableExts[j].extensionName))
							break;
					}
					if (j < nAvailableExts) {
						enableExts[nEnableExts++] = (char*)cfg.extensions.names[i];
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
		enableLayers = (char**)malloc(sizeof(char*) * cfg.layers.n);
		memcpy(enableLayers, cfg.layers.names, sizeof(char*) * cfg.layers.nRequired);
		nEnableLayers = cfg.layers.nRequired;

		if (cfg.layers.n > cfg.layers.nRequired) {
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
				for (u32 i = cfg.layers.nRequired; i < cfg.layers.n; i++) {
					u32 j = 0;
					for (; j < nAvailableLayers; j++) {
						if (!strcmp(cfg.layers.names[i], availableLayers[j].layerName))
							break;
					}
					if (j < nAvailableLayers) {
						enableLayers[nEnableLayers++] = (char*)cfg.layers.names[i];
					}
				}
			}
		}
	}

	//TODO currenty, only one queue for present, graphics, and compute
	// queues
	float queuePriority = 1.0f;
	u32 nQueueInfos = 0;
	VkDeviceQueueCreateInfo queueInfos[3]{};
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

		u8 getGraphics = cfg.flags & VkDvcCfg::Flags::createQueuesGraphics;
		u8 getCompute = cfg.flags & VkDvcCfg::Flags::createQueuesCompute;

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

		this->dvc.queues.present.family = presentFam;
		this->dvc.queues.graphics.family = graphicsFam;
		this->dvc.queues.compute.family = computeFam;

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

	// creating logical device and saving physical device and surface
	{

		VkDeviceCreateInfo info{
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,

			.queueCreateInfoCount = nQueueInfos,
			.pQueueCreateInfos = queueInfos,

			.enabledLayerCount = nEnableLayers,
			.ppEnabledLayerNames = enableLayers,

			.enabledExtensionCount = nEnableExts,
			.ppEnabledExtensionNames = enableExts,
		};

		vkCreateDevice(bestDevice, &info, NULL, &(this->dvc.logic));
	}

	// getting Device Queues
	{
		this->dvc.queues.present.queue = 0;
		this->dvc.queues.graphics.queue = 0;
		this->dvc.queues.compute.queue = 0;

		this->dvc.queues.present.n = 0;
		this->dvc.queues.graphics.n = 0;
		this->dvc.queues.compute.n = 0;



		if (this->dvc.queues.present.family >= 0) {
			this->dvc.queues.present.queue = (VkQueue*)malloc(sizeof(VkQueue));
			this->dvc.queues.present.n = 1;
			vkGetDeviceQueue(this->dvc.logic, this->dvc.queues.present.family, 0, this->dvc.queues.present.queue);
		}

		if (this->dvc.queues.graphics.family >= 0) {
			if (this->dvc.queues.present.family == this->dvc.queues.present.family) {
				this->dvc.queues.graphics.queue = this->dvc.queues.present.queue;
				this->dvc.queues.graphics.n = this->dvc.queues.present.n;
			}
			else {

				this->dvc.queues.graphics.queue = (VkQueue*)malloc(sizeof(VkQueue));
				vkGetDeviceQueue(this->dvc.logic, this->dvc.queues.graphics.family, 0, this->dvc.queues.graphics.queue);
			}
		}

		if (this->dvc.queues.compute.family >= 0) {
			if (this->dvc.queues.compute.family == this->dvc.queues.present.family) {
				this->dvc.queues.compute.queue = this->dvc.queues.present.queue;
				this->dvc.queues.compute.n = this->dvc.queues.present.n;
			}
			else if (this->dvc.queues.compute.family == this->dvc.queues.graphics.family) {
				this->dvc.queues.compute.queue = this->dvc.queues.graphics.queue;
				this->dvc.queues.compute.n = this->dvc.queues.graphics.n;
			}
			else {
				this->dvc.queues.compute.queue = (VkQueue*)malloc(sizeof(VkQueue));
				vkGetDeviceQueue(this->dvc.logic, this->dvc.queues.compute.family, 0, this->dvc.queues.compute.queue);
			}
		}
	}
}

VulkanContext::~VulkanContext() {
	vkDeviceWaitIdle(dvc.logic);

	if (dvc.queues.present.queue) {
		free(dvc.queues.present.queue);
	}
	if (dvc.queues.graphics.queue
		&& (dvc.queues.graphics.queue != dvc.queues.present.queue)) {
		free(dvc.queues.graphics.queue);
	}
	if (dvc.queues.compute.queue
		&& (dvc.queues.compute.queue != dvc.queues.present.queue)
		&& (dvc.queues.compute.queue != dvc.queues.graphics.queue)) {
		free(dvc.queues.compute.queue);
	}

	INF(printf("destroying vulkan device...\n"));
	vkDestroyDevice(dvc.logic, 0);
	INF(printf("destroying vulkan surface...\n"));
	vkDestroySurfaceKHR(inst, surface, 0);
	INF(printf("destroying vulkan instance...\n"));
	vkDestroyInstance(inst, 0);
}