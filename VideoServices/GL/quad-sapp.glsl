@vs vs
layout(binding = 0) uniform Uniforms {
    mat4 view;
    mat4 proj;
};

in ivec4 position;
in vec4 color0;
in vec2 texcoord0;
out vec4 color;
out vec2 uv;

void main() {
    gl_Position = proj * view * vec4(position);
    color = color0;
    uv = texcoord0;
}
@end

@fs fs
layout(binding = 1) uniform texture2D tex;
layout(binding = 2) uniform sampler smp;

in vec4 color;
in vec2 uv;
out vec4 frag_color;

void main() {
    vec4 tex_color = texture(sampler2D(tex, smp), uv);
    frag_color = tex_color * color;
}
@end

@program quad vs fs

