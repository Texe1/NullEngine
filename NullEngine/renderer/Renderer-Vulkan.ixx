#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>
#include "../utils/typedef.h"

export module Renderer:Vulkan;

namespace render {
	namespace vulkan {
		export class VkContext
		{
		public:
			struct VkInstCfg {
				struct {
					u32 n;
					u32 nRequired;
					const char** names;
				} extensions;
				struct {
					u32 n;
					u32 nRequired;
					const char** names;
				} layers;
			};

			struct VkDvcCfg {
				struct {
					u32 n;
					u32 nRequired;
					const char** names;
				} extensions;
				struct {
					u32 n;
					u32 nRequired;
					const char** names;
				} layers;

				static enum Flags {
					createQueuesGraphics = 1,
					createQueuesCompute = 2,
				};

				VkPhysicalDeviceType prefered_type;

				Flags flags;

				const i64 ratePhysicalDevice(VkPhysicalDevice, VkSurfaceKHR);
			};
		private:
			static const VkInstCfg defaultInstCfg;
			static const VkDvcCfg defaultDvcCfg;

			// Vulkan stuff
			VkInstance inst = NULL;
			VkDevice logicalDevice = NULL;
			VkPhysicalDevice physicalDevice = NULL;
			VkSurfaceKHR surface;

			// SDL window
			SDL_Window* wnd;
		public:
			VkContext(const VkInstCfg&, const VkDvcCfg&);
			VkContext(const VkInstCfg& instCfg) : VkContext(instCfg, defaultDvcCfg) {}
			VkContext() : VkContext(defaultInstCfg) {}
			~VkContext();

		private:
			void initInstance(const VkInstCfg&);
			void initdevice(VkDvcCfg*);
		};
	} // namespace vulkan
} // namespace render

using namespace render::vulkan;

const char* defaultInstExt[] = {"VK_KHR_surface", "VK_KHR_win32_surface"};
const char* defaultInstLayers[] = { "VK_LAYER_KHRONOS_validation" };

const char* defaultDvcExt[] = { "VK_KHR_swapchain" };
const char* defaultDvcLayers[] = { "" };

const VkContext::VkInstCfg VkContext::defaultInstCfg{
	{2,2,defaultInstExt},
#ifdef _DEBUG
	{1,1, defaultInstLayers},
#else
	{0,0,0},
#endif // _DEBUG
};
const VkContext::VkDvcCfg VkContext::defaultDvcCfg{
	// extensions
	{1,1,defaultDvcExt},
	// layers
	{0,0,0},
	VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
	VkContext::VkDvcCfg::Flags::createQueuesGraphics,
};

#include "../utils/typedef.h";

import <stdio.h>;
import <stdlib.h>;
import <string.h>;
import <stdexcept>;

import <SDL2/SDL_vulkan.h>;


VkContext::VkContext(const VkInstCfg& instCfg, const VkDvcCfg& dvcCfg) {
	initInstance(instCfg);

	this->wnd = SDL_CreateWindow("Window", 0, 0, 800, 600, SDL_WINDOW_VULKAN);
	SDL_Vulkan_CreateSurface(this->wnd, this->inst, &(this->surface));

	initdevice((VkDvcCfg*)&dvcCfg);
}

VkContext::~VkContext()
{
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

}

const i64 VkContext::VkDvcCfg::ratePhysicalDevice(VkPhysicalDevice dvc, VkSurfaceKHR surface) {

	i64 score = 0;

	// extensions
	if(this->extensions.n) {
		u32 nAvailableExt = 0;
		VkExtensionProperties* availableExtensions = NULL;
		{
			vkEnumerateDeviceExtensionProperties(dvc, NULL, &nAvailableExt, NULL);
			if (nAvailableExt < this->extensions.nRequired)
				return -1;

			if (nAvailableExt > 0) {
				availableExtensions = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * nAvailableExt);
				vkEnumerateDeviceExtensionProperties(dvc, NULL, &nAvailableExt, availableExtensions);
			}
		}

		if (nAvailableExt && availableExtensions) {

			// checking required Extensions
			{
				u32 i = 0;
				for (; i < this->extensions.nRequired; i++) {
					u32 j = 0;
					for (; j < nAvailableExt; j++) {
						if (!strcmp(this->extensions.names[i], availableExtensions[j].extensionName))
							break;
					}
					if (j == nAvailableExt)
						break;
				}
				if (i < this->extensions.nRequired) {
					free(availableExtensions);
					return -1;
				}
			}

			// checking optional extensions
			{

				for (u32 i = this->extensions.nRequired; i < this->extensions.n; i++) {
					for (u32 j = 0; j < nAvailableExt; j++) {
						if (!strcmp(this->extensions.names[i], availableExtensions[j].extensionName)) {
							score += nAvailableExt - i;
							break;
						}
					}
				}
			}

		}
	}

	// layers
	if(this->layers.n) {

		// getting available layers
		u32 nAvailableLayers = 0;
		VkLayerProperties* availableLayers = NULL;
		{
			vkEnumerateDeviceLayerProperties(dvc, &nAvailableLayers, NULL);
			if (nAvailableLayers) {
				availableLayers = (VkLayerProperties*)malloc(sizeof(VkLayerProperties) * nAvailableLayers);
				vkEnumerateDeviceLayerProperties(dvc, &nAvailableLayers, availableLayers);
			}
		}

		if (nAvailableLayers && availableLayers) {
			// checking required Layers
			{
				u32 i = 0;
				for (; i < this->layers.nRequired; i++) {
					u32 j = 0;
					for (; j < nAvailableLayers; j++) {
						if (!strcmp(this->layers.names[i], availableLayers[j].layerName))
							break;
					}
					if (j == nAvailableLayers)
						break;
				}
				if (i < this->layers.nRequired) {
					free(availableLayers);
					return -1;
				}
			}

			// checking optional layers
			{

				for (u32 i = this->layers.nRequired; i < this->layers.n; i++) {
					for (u32 j = 0; j < nAvailableLayers; j++) {
						if (!strcmp(this->layers.names[i], availableLayers[j].layerName)) {
							score += nAvailableLayers - i;
							break;
						}
					}
				}
			}
		}

	}

	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceProperties(dvc, &props);
	if (props.deviceType == this->prefered_type) {
		score += 4; // Don't know abt. this exact number, but sure, why not
	}

	// Queue Families
	{
		u32 nQueueFams = 0;
		VkQueueFamilyProperties* queueFams;

		vkGetPhysicalDeviceQueueFamilyProperties(dvc, &nQueueFams, NULL);
		if (nQueueFams == 0)
			return -1;

		queueFams = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * nQueueFams);
		vkGetPhysicalDeviceQueueFamilyProperties(dvc, &nQueueFams, queueFams);


		u8 foundPresentFamily = 0;
		u8 foundGraphicsFamily = 0;
		u8 foundGraphicsAndPresentFam = 0;
		u8 foundComputeFamily = 0;
		u8 foundComputeAndPresentFam = 0;

		u32 i = 0;
		for (u32 i = 0; i < nQueueFams; i++) {

			u32 presentSupport = 0;
			vkGetPhysicalDeviceSurfaceSupportKHR(dvc, i, surface, &presentSupport);

			if (presentSupport) {
				foundPresentFamily = 1;
			}

			if (queueFams[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				foundGraphicsFamily = 1;
				if (presentSupport) {
					foundGraphicsAndPresentFam = 1;
				}
			}

			if (queueFams[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
				foundComputeFamily = 1;
			}
		}

		u8 searchGraphicsFamily = (this->flags & Flags::createQueuesGraphics) ? 1 : 0;
		u8 searchComputeFamily = (this->flags & Flags::createQueuesCompute) ? 1 : 0;

		if (searchGraphicsFamily && foundGraphicsAndPresentFam) {
			score += 2;
		}
		if (searchComputeFamily && foundComputeAndPresentFam) {
			score += 2;
		}

		if (searchComputeFamily && !foundComputeFamily)
			return - 1;
		if (searchGraphicsFamily && !foundGraphicsFamily)
			return -1;
	}

	return score;
}