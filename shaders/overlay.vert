/*
 * ClearUI overlay vertex shader (Vulkan / GLSL 450)
 *
 * Draws a fullscreen triangle from gl_VertexIndex alone — no vertex buffer
 * needed.  Invoke with vkCmdDraw(cmd, 3, 1, 0, 0).
 *
 * The triangle covers [-1,1] in NDC; UV (0,0) is top-left, (1,1) bottom-right,
 * matching Vulkan's default viewport with minDepth=0 maxDepth=1.
 */
#version 450

layout(location = 0) out vec2 v_uv;

void main() {
    /*
     * Vertex 0: (-1, -1)  UV (0, 0)
     * Vertex 1: ( 3, -1)  UV (2, 0)
     * Vertex 2: (-1,  3)  UV (0, 2)
     *
     * The visible [0,1] portion of UV maps exactly to the screen.
     */
    vec2 pos = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    v_uv       = pos;
    gl_Position = vec4(pos * 2.0 - 1.0, 0.0, 1.0);
}
