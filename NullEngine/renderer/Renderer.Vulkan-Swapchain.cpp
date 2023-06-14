#include "../utils/typedef.h"

module Renderer.Vulkan;

using namespace render::vulkan;

void VkContext::addSwapchain() {
	if (this->queues.families.present != this->queues.families.graphics) {
		throw new std::runtime_error("present and graphics queue Are different, handling this is currently not supported");
		return;
	}


	VkSurfaceFormatKHR format = {};
	{
		u32 nFormats = 0;
		VkSurfaceFormatKHR* surfaceFormats = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(this->physicalDevice, this->surface, &nFormats, 0);
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

		vkGetSwapchainImagesKHR(this->logicalDevice, this->swapchain.swapchain, &(this->swapchain.nImgs), 0);
		if (this->swapchain.nImgs > 0) {
			this->swapchain.imgs = (VkImage*)malloc(sizeof(VkImage) * this->swapchain.nImgs);
			vkGetSwapchainImagesKHR(this->logicalDevice, this->swapchain.swapchain, &(this->swapchain.nImgs), this->swapchain.imgs);
		}
		else {
			this->swapchain.imgs = 0;
		}
	}
}