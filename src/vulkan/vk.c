#include "vk.h"

#include "vk_init.h"

#include <stdio.h>
#include <Windows.h>
#include <vulkan/vulkan_win32.h>
#include "vk_state_structs.h"

#include <debugapi.h>


char* default_inst_extensions[] = {
	VK_KHR_SURFACE_EXTENSION_NAME,
	VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
	0
};

char* default_inst_layers[] = {
	"VK_LAYER_KHRONOS_profiles",
	0
};

struct vk_inst_cfg default_inst_cfg = {
	.ext = {
		.nReq = 2,
		.ptr = default_inst_extensions
	},
	.layers = {
		.nReq = 0,
		.ptr = default_inst_layers,
	},
	.appInfo = NULL,
} ;
const char* default_dvc_extensions[] = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	0
};
struct vk_dvc_cfg default_dvc_cfg = {
	.ext = {
		.nReq = 1,
		.ptr = default_dvc_extensions,
	},
	.flags = 0,
};


int vk_init(struct vk_cfg* _cfg){
	struct vk_inst inst;
	struct vk_dvc dvc;
	vk_init_inst(&default_inst_cfg, &inst);
	default_dvc_cfg.inst = &inst;
	vk_init_dvc(&default_dvc_cfg, &dvc);
	printf("Yo this works!\n");
	vkDestroyDevice(dvc.handle, NULL);
	vkDestroyInstance(inst.handle, NULL);
}

int vk_exit(void){
	return 0;
}