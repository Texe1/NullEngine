#include "stdio.h"
import Renderer;

int main() {
	printf("startup...\n");
	render::vulkan::VkContext ctx = render::vulkan::VkContext();
	printf("finished, cleaning up...\n");
}