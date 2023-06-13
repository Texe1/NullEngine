#include "../utils/typedef.h"

export module Renderer.Vulkan:CfgStructs;

import <vulkan/vulkan.h>;

namespace render {
	namespace vulkan {

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

			static VkInstCfg defaultCfg;
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

			enum Flags {
				createQueuesGraphics = 1,
				createQueuesCompute = 2,
			};

			VkPhysicalDeviceType prefered_type;

			Flags flags;

			i64 ratePhysicalDevice(VkPhysicalDevice, VkSurfaceKHR);

			static const VkDvcCfg defaultCfg;
		};

	} // namespace vulkan
} // namespace render


/*
 * 
 * definitions
 *
 */ 

import <stdlib.h>;
import <string.h>;

using namespace render::vulkan;

const char* defaultDvcExt[] = { "VK_KHR_swapchain" };
const char* defaultDvcLayers[] = { "" };

const char* defaultInstExt[] = { "VK_KHR_surface", "VK_KHR_win32_surface" };
const char* defaultInstLayers[] = { "VK_LAYER_KHRONOS_validation" };

VkInstCfg VkInstCfg::defaultCfg {
	{2,2,defaultInstExt},
#ifdef _DEBUG
	{1,1, defaultInstLayers},
#else
	{0,0,0},
#endif // _DEBUG
};

const VkDvcCfg VkDvcCfg::defaultCfg {
	// extensions
	{1,1,defaultDvcExt},
	// layers
	{0,0,0},
	VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
	VkDvcCfg::Flags::createQueuesGraphics,
};

i64 VkDvcCfg::ratePhysicalDevice(VkPhysicalDevice dvc, VkSurfaceKHR surface) {

	i64 score = 0;

	// extensions
	if (this->extensions.n) {
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
	if (this->layers.n) {

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
			return -1;
		if (searchGraphicsFamily && !foundGraphicsFamily)
			return -1;
	}

	return score;
}

