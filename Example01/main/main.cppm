#include "stdio.h"
import Renderer;

int main() {
	printf("startup...\n");
	render::vulkan::VulkanRenderer ctx = render::vulkan::VulkanRenderer();

	while (ctx.render());

	printf("finished, cleaning up...\n");
}