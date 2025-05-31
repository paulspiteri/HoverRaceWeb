@vs water_vs

layout(binding = 0) uniform WaterUniforms {
    mat4 view;
    mat4 proj;
};
layout(binding = 1) uniform WaterAtlasCoords {
    vec4 atlas_coords[32];
};

in ivec4 position;
in vec2 texcoord0;
in int textureIdx;

out vec2 water_uv;
flat out vec4 atlas_coord;

void main() {
    gl_Position = proj * view * vec4(position);
    water_uv = texcoord0;
    atlas_coord = atlas_coords[textureIdx];
}
@end

@fs water_fs
layout(binding = 0) uniform texture2D atlas_tex;
layout(binding = 0) uniform sampler wrap_sampler;
in vec2 water_uv;
flat in vec4 atlas_coord;
out vec4 frag_color;

void main() {
    vec2 uv = fract(water_uv);
    vec2 atlas_uv = mix(atlas_coord.xy, atlas_coord.zw, uv);
    frag_color = texture(sampler2D(atlas_tex, wrap_sampler), atlas_uv);
}
@end

@program water water_vs water_fs
