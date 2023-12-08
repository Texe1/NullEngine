#pragma once

#include "../include/default.h"

struct vk_inst{
	VkInstance handle;
	
	u32 nExtensions;
	char** extensions;
	u32 nLayers;
	char** layers;
};

struct vk_dvc {
	VkDevice handle;
	VkPhysicalDevice phys;

	u32 nExtensions;
	char** extensions;
	u32 nLayers;
	char** layers;
};