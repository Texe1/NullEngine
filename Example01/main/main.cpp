#include "stdio.h"
import Renderer;

int main() {
	printf("startup...\n");
	render::vulkan::VkContext ctx = render::vulkan::VkContext();

	while (ctx.render());

	printf("finished, cleaning up...\n");
}