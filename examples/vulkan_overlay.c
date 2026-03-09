/**
 * ClearUI Vulkan Overlay — Integration Reference
 *
 * This file shows how to composite the ClearUI software-rendered RGBA
 * framebuffer on top of an existing Vulkan scene.  It is a *reference*, not a
 * standalone runnable application: it assumes you already have a VkDevice,
 * VkQueue, VkSwapchainKHR, and a VkRenderPass for your scene.
 *
 * Workflow each frame:
 *   1. Call cui_begin_frame / emit widgets / cui_end_frame as usual.
 *   2. Obtain the RGBA framebuffer via cui_rdi_soft_get_framebuffer().
 *   3. Upload it to a Vulkan texture.
 *   4. In your render pass, bind the overlay pipeline and draw a fullscreen
 *      triangle that samples the texture.
 *
 * The shipped shaders (shaders/overlay.vert, shaders/overlay.frag) and their
 * pre-compiled SPIR-V (include/clearui_overlay_spv.h) are used here.
 *
 * Compile: this file is not meant to be compiled on its own — copy the
 * relevant sections into your Vulkan renderer.
 */

#include <string.h>
#include <vulkan/vulkan.h>

#include "clearui.h"
#include "clearui_rdi.h"
#include "clearui_overlay_spv.h"

/* --------------------------------------------------------------------------
 * Structures
 * ----------------------------------------------------------------------- */

typedef struct cui_vk_overlay {
	VkDevice            device;
	VkPhysicalDevice    physical_device;

	/* Overlay texture (ClearUI framebuffer) */
	VkImage             image;
	VkDeviceMemory      image_memory;
	VkImageView         image_view;
	VkSampler           sampler;
	int                 tex_width;
	int                 tex_height;

	/* Pipeline */
	VkDescriptorSetLayout desc_layout;
	VkPipelineLayout      pipeline_layout;
	VkPipeline            pipeline;

	/* Descriptors */
	VkDescriptorPool      desc_pool;
	VkDescriptorSet       desc_set;
} cui_vk_overlay;

/* --------------------------------------------------------------------------
 * Helpers
 * ----------------------------------------------------------------------- */

static uint32_t find_memory_type(VkPhysicalDevice gpu, uint32_t type_filter,
                                 VkMemoryPropertyFlags props) {
	VkPhysicalDeviceMemoryProperties mem;
	vkGetPhysicalDeviceMemoryProperties(gpu, &mem);
	for (uint32_t i = 0; i < mem.memoryTypeCount; i++) {
		if ((type_filter & (1u << i)) &&
		    (mem.memoryTypes[i].propertyFlags & props) == props)
			return i;
	}
	return UINT32_MAX;
}

static VkShaderModule create_shader_module(VkDevice dev,
                                           const unsigned char *code,
                                           size_t size) {
	VkShaderModuleCreateInfo ci = {0};
	ci.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	ci.codeSize = size;
	ci.pCode    = (const uint32_t *)code;
	VkShaderModule mod = VK_NULL_HANDLE;
	vkCreateShaderModule(dev, &ci, NULL, &mod);
	return mod;
}

/* --------------------------------------------------------------------------
 * Create the overlay texture (call once, or when the ClearUI viewport resizes)
 * ----------------------------------------------------------------------- */

static void create_overlay_texture(cui_vk_overlay *ov, int width, int height) {
	ov->tex_width  = width;
	ov->tex_height = height;

	VkImageCreateInfo ici = {0};
	ici.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	ici.imageType     = VK_IMAGE_TYPE_2D;
	ici.format        = VK_FORMAT_R8G8B8A8_UNORM;
	ici.extent.width  = (uint32_t)width;
	ici.extent.height = (uint32_t)height;
	ici.extent.depth  = 1;
	ici.mipLevels     = 1;
	ici.arrayLayers   = 1;
	ici.samples       = VK_SAMPLE_COUNT_1_BIT;
	ici.tiling        = VK_IMAGE_TILING_LINEAR;
	ici.usage         = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	vkCreateImage(ov->device, &ici, NULL, &ov->image);

	VkMemoryRequirements req;
	vkGetImageMemoryRequirements(ov->device, ov->image, &req);

	VkMemoryAllocateInfo mai = {0};
	mai.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mai.allocationSize   = req.size;
	mai.memoryTypeIndex = find_memory_type(
		ov->physical_device, req.memoryTypeBits,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	vkAllocateMemory(ov->device, &mai, NULL, &ov->image_memory);
	vkBindImageMemory(ov->device, ov->image, ov->image_memory, 0);

	VkImageViewCreateInfo vci = {0};
	vci.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	vci.image    = ov->image;
	vci.viewType = VK_IMAGE_VIEW_TYPE_2D;
	vci.format   = VK_FORMAT_R8G8B8A8_UNORM;
	vci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	vci.subresourceRange.levelCount = 1;
	vci.subresourceRange.layerCount = 1;
	vkCreateImageView(ov->device, &vci, NULL, &ov->image_view);

	VkSamplerCreateInfo sci = {0};
	sci.sType     = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sci.magFilter = VK_FILTER_NEAREST;
	sci.minFilter = VK_FILTER_NEAREST;
	sci.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sci.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	vkCreateSampler(ov->device, &sci, NULL, &ov->sampler);
}

/* --------------------------------------------------------------------------
 * Create the overlay graphics pipeline
 * ----------------------------------------------------------------------- */

static void create_overlay_pipeline(cui_vk_overlay *ov,
                                    VkRenderPass render_pass) {
	/* Descriptor set layout: one combined image sampler at binding 0 */
	VkDescriptorSetLayoutBinding binding = {0};
	binding.binding         = 0;
	binding.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	binding.descriptorCount = 1;
	binding.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutCreateInfo dslci = {0};
	dslci.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	dslci.bindingCount = 1;
	dslci.pBindings    = &binding;
	vkCreateDescriptorSetLayout(ov->device, &dslci, NULL, &ov->desc_layout);

	/* Push constant: int swizzle_rb (4 bytes, fragment stage) */
	VkPushConstantRange pcr = {0};
	pcr.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	pcr.offset     = 0;
	pcr.size       = sizeof(int);

	VkPipelineLayoutCreateInfo plci = {0};
	plci.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	plci.setLayoutCount         = 1;
	plci.pSetLayouts            = &ov->desc_layout;
	plci.pushConstantRangeCount = 1;
	plci.pPushConstantRanges    = &pcr;
	vkCreatePipelineLayout(ov->device, &plci, NULL, &ov->pipeline_layout);

	/* Shader modules from the pre-compiled SPIR-V */
	VkShaderModule vert = create_shader_module(
		ov->device, clearui_overlay_vert_spv, sizeof(clearui_overlay_vert_spv));
	VkShaderModule frag = create_shader_module(
		ov->device, clearui_overlay_frag_spv, sizeof(clearui_overlay_frag_spv));

	VkPipelineShaderStageCreateInfo stages[2] = {{0}, {0}};
	stages[0].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[0].stage  = VK_SHADER_STAGE_VERTEX_BIT;
	stages[0].module = vert;
	stages[0].pName  = "main";
	stages[1].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[1].stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
	stages[1].module = frag;
	stages[1].pName  = "main";

	/* No vertex input — the vertex shader generates positions from gl_VertexIndex */
	VkPipelineVertexInputStateCreateInfo vi = {0};
	vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	VkPipelineInputAssemblyStateCreateInfo ia = {0};
	ia.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	/* Dynamic viewport and scissor */
	VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
	VkPipelineDynamicStateCreateInfo dyn = {0};
	dyn.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dyn.dynamicStateCount = 2;
	dyn.pDynamicStates    = dyn_states;

	VkPipelineViewportStateCreateInfo vp = {0};
	vp.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	vp.viewportCount = 1;
	vp.scissorCount  = 1;

	VkPipelineRasterizationStateCreateInfo rs = {0};
	rs.sType       = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rs.polygonMode = VK_POLYGON_MODE_FILL;
	rs.cullMode    = VK_CULL_MODE_NONE;
	rs.frontFace   = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rs.lineWidth   = 1.0f;

	VkPipelineMultisampleStateCreateInfo ms = {0};
	ms.sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	/* Alpha blending: srcAlpha / oneMinusSrcAlpha */
	VkPipelineColorBlendAttachmentState blend_att = {0};
	blend_att.blendEnable         = VK_TRUE;
	blend_att.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	blend_att.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	blend_att.colorBlendOp        = VK_BLEND_OP_ADD;
	blend_att.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	blend_att.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	blend_att.alphaBlendOp        = VK_BLEND_OP_ADD;
	blend_att.colorWriteMask      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
	                                VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

	VkPipelineColorBlendStateCreateInfo cb = {0};
	cb.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	cb.attachmentCount = 1;
	cb.pAttachments    = &blend_att;

	VkGraphicsPipelineCreateInfo gci = {0};
	gci.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	gci.stageCount          = 2;
	gci.pStages             = stages;
	gci.pVertexInputState   = &vi;
	gci.pInputAssemblyState = &ia;
	gci.pViewportState      = &vp;
	gci.pRasterizationState = &rs;
	gci.pMultisampleState   = &ms;
	gci.pColorBlendState    = &cb;
	gci.pDynamicState       = &dyn;
	gci.layout              = ov->pipeline_layout;
	gci.renderPass          = render_pass;
	gci.subpass             = 0;
	vkCreateGraphicsPipelines(ov->device, VK_NULL_HANDLE, 1, &gci, NULL,
	                          &ov->pipeline);

	vkDestroyShaderModule(ov->device, vert, NULL);
	vkDestroyShaderModule(ov->device, frag, NULL);
}

/* --------------------------------------------------------------------------
 * Allocate and update the descriptor set
 * ----------------------------------------------------------------------- */

static void create_overlay_descriptors(cui_vk_overlay *ov) {
	VkDescriptorPoolSize pool_size = {0};
	pool_size.type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	pool_size.descriptorCount = 1;

	VkDescriptorPoolCreateInfo dpci = {0};
	dpci.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	dpci.maxSets       = 1;
	dpci.poolSizeCount = 1;
	dpci.pPoolSizes    = &pool_size;
	vkCreateDescriptorPool(ov->device, &dpci, NULL, &ov->desc_pool);

	VkDescriptorSetAllocateInfo dsai = {0};
	dsai.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	dsai.descriptorPool     = ov->desc_pool;
	dsai.descriptorSetCount = 1;
	dsai.pSetLayouts        = &ov->desc_layout;
	vkAllocateDescriptorSets(ov->device, &dsai, &ov->desc_set);

	VkDescriptorImageInfo dii = {0};
	dii.sampler     = ov->sampler;
	dii.imageView   = ov->image_view;
	dii.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkWriteDescriptorSet wds = {0};
	wds.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	wds.dstSet          = ov->desc_set;
	wds.dstBinding      = 0;
	wds.descriptorCount = 1;
	wds.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	wds.pImageInfo      = &dii;
	vkUpdateDescriptorSets(ov->device, 1, &wds, 0, NULL);
}

/* --------------------------------------------------------------------------
 * Upload the ClearUI RGBA framebuffer to the Vulkan texture
 * ----------------------------------------------------------------------- */

static void upload_overlay(cui_vk_overlay *ov, const void *rgba,
                           int width, int height) {
	if (width != ov->tex_width || height != ov->tex_height) return;

	void *mapped = NULL;
	vkMapMemory(ov->device, ov->image_memory, 0, VK_WHOLE_SIZE, 0, &mapped);
	memcpy(mapped, rgba, (size_t)width * (size_t)height * 4);
	vkUnmapMemory(ov->device, ov->image_memory);
}

/* --------------------------------------------------------------------------
 * Record overlay draw commands into a command buffer
 *
 * Call this *inside* an active render pass (after your scene geometry).
 * Set swizzle_rb = 1 when the swapchain uses a BGRA format.
 * ----------------------------------------------------------------------- */

static void record_overlay(cui_vk_overlay *ov, VkCommandBuffer cmd,
                            int fb_width, int fb_height, int swizzle_rb) {
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ov->pipeline);
	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
	                        ov->pipeline_layout, 0, 1, &ov->desc_set, 0, NULL);
	vkCmdPushConstants(cmd, ov->pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT,
	                   0, sizeof(int), &swizzle_rb);

	VkViewport viewport = {0};
	viewport.width    = (float)fb_width;
	viewport.height   = (float)fb_height;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(cmd, 0, 1, &viewport);

	VkRect2D scissor = {0};
	scissor.extent.width  = (uint32_t)fb_width;
	scissor.extent.height = (uint32_t)fb_height;
	vkCmdSetScissor(cmd, 0, 1, &scissor);

	vkCmdDraw(cmd, 3, 1, 0, 0);
}

/* --------------------------------------------------------------------------
 * Cleanup
 * ----------------------------------------------------------------------- */

static void destroy_overlay(cui_vk_overlay *ov) {
	vkDestroyPipeline(ov->device, ov->pipeline, NULL);
	vkDestroyPipelineLayout(ov->device, ov->pipeline_layout, NULL);
	vkDestroyDescriptorPool(ov->device, ov->desc_pool, NULL);
	vkDestroyDescriptorSetLayout(ov->device, ov->desc_layout, NULL);
	vkDestroySampler(ov->device, ov->sampler, NULL);
	vkDestroyImageView(ov->device, ov->image_view, NULL);
	vkDestroyImage(ov->device, ov->image, NULL);
	vkFreeMemory(ov->device, ov->image_memory, NULL);
}

/* --------------------------------------------------------------------------
 * Usage sketch (not a complete main — fills in the gaps above)
 *
 *   cui_vk_overlay ov = {0};
 *   ov.device          = your_device;
 *   ov.physical_device = your_gpu;
 *
 *   create_overlay_texture(&ov, overlay_width, overlay_height);
 *   create_overlay_pipeline(&ov, your_render_pass);
 *   create_overlay_descriptors(&ov);
 *
 *   // Each frame:
 *   cui_begin_frame(ctx);
 *   // ... emit widgets ...
 *   cui_end_frame(ctx);
 *
 *   const void *rgba; int w, h;
 *   cui_rdi_soft_get_framebuffer(rdi_ctx, &rgba, &w, &h);
 *   upload_overlay(&ov, rgba, w, h);
 *
 *   // Inside your command buffer recording, after scene geometry:
 *   int swizzle = 1;  // set to 1 for BGRA swapchains
 *   record_overlay(&ov, cmd, swapchain_width, swapchain_height, swizzle);
 *
 *   // At shutdown:
 *   destroy_overlay(&ov);
 * ----------------------------------------------------------------------- */
