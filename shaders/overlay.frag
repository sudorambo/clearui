/*
 * ClearUI overlay fragment shader (Vulkan / GLSL 450)
 *
 * Samples the ClearUI RGBA framebuffer texture and composites it onto the
 * scene with alpha blending (pipeline blend state should be configured for
 * pre-multiplied or standard src-alpha / one-minus-src-alpha).
 *
 * Push-constant toggle: set swizzle_rb to non-zero when the swapchain uses
 * a BGRA format (e.g. VK_FORMAT_B8G8R8A8_UNORM / _SRGB) so the shader
 * swaps the red and blue channels.
 */
#version 450

layout(location = 0) in  vec2 v_uv;
layout(location = 0) out vec4 o_color;

layout(set = 0, binding = 0) uniform sampler2D u_overlay;

layout(push_constant) uniform PushConstants {
    int swizzle_rb;
} pc;

void main() {
    vec4 texel = texture(u_overlay, v_uv);
    o_color = (pc.swizzle_rb != 0) ? texel.bgra : texel;
}
