#include "../include/default.h"
#include "vk_init.h"

#include <vulkan/vulkan.h>
#include <string.h>

#include "vk_cfg_structs.h"
#include "vk_state_structs.h"

i32 getSupportedExtensionCount(const char** _requested, i32 _max, VkExtensionProperties* _props){
	if(*_requested == NULL){
		return 0;
	}
	for(int i = 0; i < _max; i++){
		if(!strcmp(*_requested, _props[i].extensionName)){
			return getSupportedExtensionCount(_requested +1, _max, _props) +1;
		}
	}
	return 0;
}

i32 getSupportedLayerCount(const char** _requested, i32 _max, VkLayerProperties* _props){
	if(*_requested == NULL){
		return 0;
	}
	for(int i = 0; i < _max; i++){
		if(!strcmp(*_requested, _props[i].layerName)){
			return getSupportedLayerCount(_requested +1, _max, _props) +1;
		}
	}
	return 0;
}

i32 vk_init_inst(struct vk_inst_cfg* _cfg, struct vk_inst* _inst){

	VkInstanceCreateInfo info = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,

		.enabledExtensionCount = _cfg->ext.nReq, // TODO check for availability of optional instatnce extensions
		.ppEnabledExtensionNames = _cfg->ext.ptr,

		.enabledLayerCount = _cfg->layers.nReq, // TODO check for availability of optional layers
		.ppEnabledLayerNames = _cfg->layers.ptr,

		.pApplicationInfo = _cfg->appInfo,

		.pNext = NULL,
		.flags = 0, // TODO read into the flag that is available here 
	};

	if(_cfg->ext.ptr != 0){
		int nSupportedExts = 0;
		vkEnumerateInstanceExtensionProperties(NULL, &nSupportedExts, NULL);
		u32 sz = sizeof(VkExtensionProperties) * nSupportedExts;
		VkExtensionProperties* extProps = malloc(sz);
		if(extProps == 0){
			LOGERR("Error: out of Memory (requested block minimum size: %d)\n", sz);
			return -1;
		}

		vkEnumerateInstanceExtensionProperties(NULL, &nSupportedExts, extProps);
		int n = getSupportedExtensionCount(_cfg->ext.ptr, nSupportedExts, extProps);

		if(n < _cfg->ext.nReq){
			LOGERR("Error: one or more ofthe required instance extensions is not supported!\n\t(extension name '%s')\n", _cfg->ext.ptr[n]);
			return -1;
		}
		info.enabledExtensionCount = n;
		free(extProps);
	}

	if(_cfg->layers.ptr != 0){
		int nSupportedLayers = 0;
		vkEnumerateInstanceLayerProperties(&nSupportedLayers, NULL);
		u32 sz = sizeof(VkLayerProperties) * nSupportedLayers;
		VkLayerProperties* layerProps = malloc(sz);
		if(layerProps == 0){
			LOGERR("Error: out of Memory (requested block minimum size: %d)\n", sz);
			return -1;
		}

		vkEnumerateInstanceLayerProperties(&nSupportedLayers, layerProps);
		int n = getSupportedLayerCount(_cfg->layers.ptr, nSupportedLayers, layerProps);

		if(n < _cfg->layers.nReq){
			LOGERR("Error: one or more ofthe required instance extensions is not supported!\n\t(extension name '%s')\n", _cfg->layers.ptr[n]);
			return -1;
		}
		info.enabledLayerCount = n;
		free(layerProps);
	}


	VkInstance inst = NULL;

	i32 result = vkCreateInstance(&info, NULL, &inst);

	if(result != VK_SUCCESS) {
		LOGERR("ERR: Couldn't create Instance;\n\tReturn value: %d\n", result);
		return result - VK_SUCCESS;
	}

	_inst->handle = inst;
	_inst->nExtensions = info.enabledExtensionCount;
	_inst->extensions = info.ppEnabledExtensionNames;
	_inst->nLayers = info.enabledLayerCount;
	_inst->layers = info.ppEnabledLayerNames;

	return result - VK_SUCCESS;
}

i32 vk_init_dvc(struct vk_dvc_cfg* _cfg, struct vk_dvc* _dvc){
	VkPhysicalDevice phys;
	int n = 1;

	vkEnumeratePhysicalDevices(_cfg->inst->handle, &n, &phys);

	float queuePrio = 1.0; 

	VkDeviceQueueCreateInfo queueInfo = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		
		.queueCount = 1,
		.pQueuePriorities = &queuePrio,
		.queueFamilyIndex = 0,

		.flags = 0,
		.pNext = NULL,
	};

	VkDeviceCreateInfo info = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,

		.enabledExtensionCount = _cfg->ext.nReq,
		.ppEnabledExtensionNames = _cfg->ext.ptr,

		.enabledLayerCount = 2753745,
		.ppEnabledLayerNames = NULL,

		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &queueInfo,

		.flags = 0,
		.pNext = NULL,
	};

	if(_cfg->ext.ptr != 0){
		int nSupportedExts = 0;
		vkEnumerateDeviceExtensionProperties(phys, NULL, &nSupportedExts, NULL);
		u32 sz = sizeof(VkExtensionProperties) * nSupportedExts;
		VkExtensionProperties* extProps = malloc(sz);
		if(extProps == 0){
			LOGERR("Error: out of Memory (requested block minimum size: %d)\n", sz);
			return -1;
		}

		vkEnumerateDeviceExtensionProperties(phys, NULL, &nSupportedExts, extProps);
		int n = getSupportedExtensionCount(_cfg->ext.ptr, nSupportedExts, extProps);

		if(n < _cfg->ext.nReq){
			LOGERR("Error: one or more of the required device extensions is not supported!\n\t(extension name '%s')\n", _cfg->ext.ptr[n]);
			return -1;
		}
		info.enabledExtensionCount = n;
		free(extProps);
	}
	

	VkDevice handle = NULL;
	i32 result = vkCreateDevice(phys, &info, NULL, &handle);

	if(result != VK_SUCCESS){
		LOGERR("ERR: Couldn't create Device;\n\tReturn value: %d\n", result);
		return result - VK_SUCCESS;
	}

	*_dvc = (struct vk_dvc){
		.handle = handle,
		.phys = phys,

		.nExtensions = info.enabledExtensionCount,
		.extensions = info.ppEnabledExtensionNames,
		
		.nLayers = info.enabledLayerCount,
		.layers = info.ppEnabledLayerNames,
	};

	return 0;
}