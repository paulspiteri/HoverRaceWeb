@vs vs
layout(binding = 0) uniform Uniforms {
    mat4 view;
    mat4 proj;
};

in ivec4 position;
in vec4 color0;
in vec2 texcoord0;
in uint textureIdx;
out vec4 color;
out vec2 uv;
flat out uint tex_index;

void main() {
    gl_Position = proj * view * vec4(position);
    color = color0;
    uv = texcoord0;
    tex_index = textureIdx;
}
@end

@fs fs
layout(binding = 2) uniform texture2D texture0;
layout(binding = 3) uniform texture2D texture1;
layout(binding = 4) uniform texture2D texture2;
layout(binding = 5) uniform texture2D texture3;
layout(binding = 1) uniform sampler smp;

in vec4 color;
in vec2 uv;
flat in uint tex_index;
out vec4 frag_color;

void main() {
    vec4 tex_color;
    switch(tex_index) {
        case 0u: tex_color = texture(sampler2D(texture0, smp), uv); break;
        case 1u: tex_color = texture(sampler2D(texture1, smp), uv); break;
        case 2u: tex_color = texture(sampler2D(texture2, smp), uv); break;
        case 3u: tex_color = texture(sampler2D(texture3, smp), uv); break;
        case 4u: tex_color = texture(sampler2D(texture0, smp), uv); break;
        case 5u: tex_color = texture(sampler2D(texture0, smp), uv); break;
        case 6u: tex_color = texture(sampler2D(texture0, smp), uv); break;
        case 7u: tex_color = texture(sampler2D(texture0, smp), uv); break;
        default: tex_color = vec4(1.0, 0.0, 1.0, 1.0); break;
    }
    frag_color = tex_color * color;
}
@end

@program quad vs fs
