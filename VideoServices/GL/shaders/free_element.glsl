@vs free_element_vs

layout(binding = 0) uniform FreeElementUniforms {
    mat4 view;
    mat4 proj;
    int time;
};
layout(binding = 1) uniform FreeElementAtlasCoords {
    vec4 atlas_coords[32];
};

// Per-vertex attributes (buffer 0)
in ivec4 position;
in vec2 texcoord0;
in int textureIdx;

// Per-instance attributes (buffer 1)
in ivec3 instancePosition;
in int type;

out vec2 free_element_uv;
flat out vec4 atlas_coord;

vec3 rotateY(vec3 pos, float angleRadians) {
    float c = cos(angleRadians);
    float s = sin(angleRadians);
    return vec3(
        pos.x * c + pos.z * s,
        pos.y,
        pos.x * -s + pos.z * c
    );
}

void main() {
    if (type == 22) {  // PowerUp rotates on the spot
        const float ANGLE_TO_RADIANS = 2.0 * 3.14159265359 / 4096.0;
        float wrappedTime = mod(float(time), 4096.0);
        float angleRadians = wrappedTime * ANGLE_TO_RADIANS;
        vec3 rotatedPos = rotateY(position.xyz, angleRadians);
        vec4 worldPos = vec4(rotatedPos.xyz + instancePosition, 1);
        gl_Position = proj * view * worldPos;
    } else {
        vec4 worldPos = vec4(position.xyz + instancePosition, 1);
        gl_Position = proj * view * worldPos;
    }
    free_element_uv = texcoord0;
    atlas_coord = atlas_coords[textureIdx];
}
@end

@fs free_element_fs
layout(binding = 0) uniform texture2D atlas_tex;
layout(binding = 0) uniform sampler wrap_sampler;
in vec2 free_element_uv;
flat in vec4 atlas_coord;
out vec4 frag_color;

void main() {
    vec2 uv = fract(free_element_uv);
    vec2 atlas_uv = mix(atlas_coord.xy, atlas_coord.zw, uv);
    frag_color = texture(sampler2D(atlas_tex, wrap_sampler), atlas_uv);
}
@end

@program free_element free_element_vs free_element_fs
