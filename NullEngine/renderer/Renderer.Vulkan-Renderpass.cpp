module Renderer.Vulkan;

using namespace render::vulkan;


// This is subject to change, currently the simplest renderPass is created
void VkContext::addRenderPass() {
	VkRenderPassCreateInfo info { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };

	VkAttachmentReference attachmentRef{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

	VkAttachmentDescription attachment{};
	attachment.format = this->swapchain.format;
	attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

	attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkSubpassDescription subPass{};
	subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subPass.colorAttachmentCount = 1;
	subPass.pColorAttachments = &attachmentRef;

	info.attachmentCount = 1;
	info.pAttachments = &attachment; // <-
	info.subpassCount = 1;
	info.pSubpasses = &subPass; // <-

	vkCreateRenderPass(this->logicalDevice, &info, NULL, &(this->renderPass.renderPass));
}