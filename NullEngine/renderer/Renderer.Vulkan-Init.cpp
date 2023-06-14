module;

#include "../utils/typedef.h"
#define SDL_MAIN_HANDLED
#include "SDL2/SDL.h"

module Renderer.Vulkan;

using namespace render::vulkan;

VkContext::VkContext(VkInstCfg& instCfg, VkDvcCfg& dvcCfg) {
	this->initInstance(instCfg);

	SDL_Init(SDL_INIT_VIDEO);
	this->wnd = SDL_CreateWindow("Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_VULKAN);
	SDL_Vulkan_CreateSurface(this->wnd, this->inst, &(this->surface));

	this->initdevice((VkDvcCfg*)&dvcCfg);

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
				viewInfo.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0,1,0,1};
				
				vkCreateImageView(this->logicalDevice, &viewInfo, 0, &(this->swapchain.imgViews[i]));

				VkFramebufferCreateInfo info{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
				info.renderPass = this->renderPass.renderPass;
				info.attachmentCount = 1;
				info.pAttachments = &(this->swapchain.imgViews[i]);

				info.width = this->swapchain.width;
				info.height = this->swapchain.height;

				info.layers = 1;

				vkCreateFramebuffer(this->logicalDevice, &info, 0, &(this->swapchain.framebuffers[i]));
			}
		}

		this->addCommandPool();
		{
			VkFenceCreateInfo info{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
			info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
			vkCreateFence(this->logicalDevice, &info, 0, &(this->sync.inFlightFence));
		}

		{
			VkSemaphoreCreateInfo info{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
			vkCreateSemaphore(this->logicalDevice, &info, 0, &(this->sync.acquireSemaphore));
			vkCreateSemaphore(this->logicalDevice, &info, 0, &(this->sync.releaseSemaphore));
		}

		this->addPipeline();
	}
}

VkContext::~VkContext()
{
	vkDeviceWaitIdle(this->logicalDevice);

	if (this->pipeline.handle)vkDestroyPipeline(this->logicalDevice, this->pipeline.handle, 0);
	if (this->pipeline.layout)vkDestroyPipelineLayout(this->logicalDevice, this->pipeline.layout, 0);

	if(this->sync.inFlightFence)vkDestroyFence(this->logicalDevice, this->sync.inFlightFence, 0);

	if (this->commandPool.handle) {
		vkFreeCommandBuffers(this->logicalDevice, this->commandPool.handle, 1, this->commandPool.buffers);
		free(this->commandPool.buffers);
		vkDestroyCommandPool(this->logicalDevice, this->commandPool.handle, 0);
	}

	for (u32 i = 0; i < this->swapchain.nImgs; i++) {
		if (this->swapchain.framebuffers)vkDestroyFramebuffer(this->logicalDevice, this->swapchain.framebuffers[i], 0);
		if (this->swapchain.imgViews)vkDestroyImageView(this->logicalDevice, this->swapchain.imgViews[i], 0);
	}

	if (this->swapchain.framebuffers) free(this->swapchain.framebuffers);
	if (this->swapchain.imgViews) free(this->swapchain.imgViews);

	if (this->renderPass.renderPass) {
		INF(printf("destroying vulkan render pass...\n"));
		vkDestroyRenderPass(this->logicalDevice, this->renderPass.renderPass, 0);
	}

	if (this->swapchain.imgs)free(this->swapchain.imgs);

	INF(printf("destroying vulkan swapchain...\n"));
	if (this->swapchain.swapchain)vkDestroySwapchainKHR(this->logicalDevice, this->swapchain.swapchain, 0);

	if (this->queues.present)free(this->queues.present);
	if (this->queues.graphics) {
		if (this->queues.graphics != this->queues.present)
			free(this->queues.graphics);
	}
	if (this->queues.compute) {
		if ((this->queues.compute != this->queues.present)
			&& (this->queues.compute != this->queues.graphics))
			free(this->queues.compute);
	}

	INF(printf("destroying vulkan device...\n"));
	if (this->logicalDevice)vkDestroyDevice(this->logicalDevice, 0);
	INF(printf("destroying vulkan surface...\n"));
	if (this->surface)vkDestroySurfaceKHR(this->inst, this->surface, 0);
	INF(printf("destroying vulkan instance...\n"));
	if (this->inst)vkDestroyInstance(this->inst, 0);

	SDL_Quit();
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
			if (nAvailableExts && availableExts) {
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