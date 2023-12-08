#pragma once

#include "../include/default.h"
#include "vk_state_structs.h"

#include <vulkan/vulkan.h>

struct vk_cfg {
	void* zero;
};

struct vk_inst_cfg {

	struct {
		int nReq;
		
		char** ptr;
	} ext;
	struct {
		int nReq;

		char** ptr;
	} layers;

	VkApplicationInfo* appInfo; // TODO replace with custom struct?
};

enum VK_DVC_CFG_FLAGS {
	VK_DVC_PREFER_INTEGRATED 	= 1 << 0, // TODO implement
	VK_DVC_PREFER_LOW_MEM 		= 1 << 1, // TODO implement
	VK_DVC_SURFACE_DONT_CARE	= 1 << 2, // TODO implement

	VK_DVC_NO_GRAPHICS			= 1 << 3, // TODO implement
	VK_DVC_COMPUTE				= 1 << 4, // TODO implement

	VK_DVC_CHOOSE_FIRST 		= 1 << 7,
};

struct vk_dvc_cfg {
	struct vk_inst* inst;
	struct {
		int nReq;
		char** ptr;
	} ext;

	u8 flags;
};