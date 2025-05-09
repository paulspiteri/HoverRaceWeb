@vs vs

layout(binding = 0) uniform Uniforms {
    mat4 view;
    mat4 proj;
};
layout(binding = 1) uniform AtlasCoords {
    vec4 atlas_coords[64];
};

in ivec4 position;
in vec4 color0;
in vec2 texcoord0;
in uint textureIdx;

out vec4 color;
out vec2 world_uv;
flat out vec4 atlas_coord;

void main() {
    gl_Position = proj * view * vec4(position);
    color = color0;
    world_uv = texcoord0;
    atlas_coord = atlas_coords[textureIdx];
}
@end

@fs fs
layout(binding = 2) uniform texture2D atlas_tex;
layout(binding = 3) uniform sampler smp;
in vec4 color;
in vec2 world_uv;
flat in vec4 atlas_coord;
out vec4 frag_color;

void main() {
    vec2 uv = fract(world_uv);
    vec2 atlas_uv = mix(atlas_coord.xy, atlas_coord.zw, uv);
    frag_color = texture(sampler2D(atlas_tex, smp), atlas_uv);
}
@end

@program quad vs fs
