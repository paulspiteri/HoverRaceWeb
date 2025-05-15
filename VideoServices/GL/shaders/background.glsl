@vs background_vs

layout(binding = 0) uniform BackgroundUniforms {
    mat4 view;
    mat4 proj;
};

in ivec4 position;
in vec4 color0;
in vec2 texcoord0;

out vec2 uv;

void main() {
    gl_Position = proj * view * vec4(position);
    uv = texcoord0;
}
@end

@fs background_fs
layout(binding = 0) uniform texture2D tex;
layout(binding = 0) uniform sampler edge_sampler;
in vec2 uv;
out vec4 frag_color;

void main() {
    frag_color = texture(sampler2D(tex, edge_sampler), uv);
}
@end

@program background background_vs background_fs
