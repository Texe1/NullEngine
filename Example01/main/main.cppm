#include "stdio.h"
import Vulkan;

using Mesh = render::vulkan::VulkanMesh;
using vert = Mesh::vertex;

vert verts[] = {
	{-0.55f, -0.55f, 0.0f},
	{ 0.55f,  0.55f, 0.0f},
	{-0.55f,  0.55f, 0.0f},
	{ 0.55f, -0.55f, 0.0f},
};

unsigned int indices[] = {
	0,1,2,
	0,3,1,
};

vert verts2[] = {
	{-0.4f, -0.55f,0.0f},
	{ 0.55f, 0.4f, 0.0f},
	{ 0.55f,-0.55f,0.0f},
};

unsigned int indices2[] = {
	0,1,2
};

using namespace render::vulkan;

int main() {
	printf("startup...\n");
	
	render::vulkan::VulkanContext ctx = render::vulkan::VulkanContext();
	render::vulkan::VulkanWindow wnd = render::vulkan::VulkanWindow(&ctx, "NullEngine", 800, 600);

	VulkanMesh m = VulkanMesh(verts, 4, indices, 3);
	wnd.renderer->addMesh(&m);
	VulkanMesh m2 = VulkanMesh(verts2, 3, indices2, 3);
	wnd.renderer->addMesh(&m2);

	while (wnd.render());

	printf("finished, cleaning up...\n");
}