#include "stdio.h"

import Vulkan;

int main() {
	printf("startup...\n");
	
	render::vulkan::VulkanContext ctx = render::vulkan::VulkanContext();
	render::vulkan::VulkanWindow wnd = render::vulkan::VulkanWindow(&ctx, "NullEngine", 800, 600);

	while (wnd.render());

	printf("finished, cleaning up...\n");
}