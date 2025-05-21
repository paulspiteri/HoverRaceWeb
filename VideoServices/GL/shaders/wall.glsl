@vs wall_vs

layout(binding = 0) uniform WallUniforms {
    mat4 view;
    mat4 proj;
    int time;
};
layout(binding = 1) uniform WallAtlasCoords {
    vec4 atlas_coords[32];
};

in ivec4 position;
in vec2 texcoord0;
in int textureIdx;
in int rotationSpeed;  // animation speed
in int rotationLength;  // animation segment length
in int segment;       // wall segment

out vec2 wall_uv;
flat out vec4 atlas_coord;

void main() {
    gl_Position = proj * view * vec4(position);
    wall_uv = texcoord0;

    if (rotationSpeed == 0) {
        atlas_coord = atlas_coords[textureIdx];
    } else {
        int lStartPos = ((time+40000) / rotationSpeed) % rotationLength;
        int segmentRepeat = segment % rotationLength;
        int idx = lStartPos == segmentRepeat ? textureIdx + 1 : textureIdx;
        atlas_coord = atlas_coords[idx];
    }
}
@end

@fs wall_fs
layout(binding = 0) uniform texture2D atlas_tex;
layout(binding = 0) uniform sampler wrap_sampler;
in vec2 wall_uv;
flat in vec4 atlas_coord;
out vec4 frag_color;

void main() {
    vec2 uv = fract(wall_uv);
    vec2 atlas_uv = mix(atlas_coord.xy, atlas_coord.zw, uv);
    frag_color = texture(sampler2D(atlas_tex, wrap_sampler), atlas_uv);
}
@end

@program wall wall_vs wall_fs
