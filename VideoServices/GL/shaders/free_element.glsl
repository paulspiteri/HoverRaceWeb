@vs free_element_vs

layout(binding = 0) uniform FreeElementUniforms {
    mat4 view;
    mat4 proj;
    int time;
    float scale;
};
layout(binding = 1) uniform FreeElementAtlasCoords {
    vec4 atlas_coords[64];
};

// Per-vertex attributes (buffer 0)
in ivec4 position;
in vec2 texcoord0;
in int textureIdx;
in int sequence;
in int frame;
in int is_variant_texture;

// Per-instance attributes (buffer 1)
in ivec3 instancePosition;
in int type;
in int orientation;
in int instance_sequence;
in int instance_frame;
in int instance_variant;         // only used for vehicles, to index into player# texture

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
    if (sequence != instance_sequence || frame != instance_frame) {
        gl_Position = vec4(0.0, 0.0, 0.0, -1.0);
        return;
    }
    const float ANGLE_TO_RADIANS = 2.0 * 3.14159265359 / 4096.0;
    float angleRadians;
    if (type == 22) {  // PowerUp rotates on the spot using the time rather than orientation parameter
        float wrappedTime = mod(float(time), 4096.0);
        angleRadians = wrappedTime * ANGLE_TO_RADIANS;

    } else {
        angleRadians = orientation * ANGLE_TO_RADIANS;
    }
    vec3 rotatedPos = rotateY(position.xyz, angleRadians);
    vec3 scaledPos = rotatedPos * scale;
    vec4 worldPos = vec4(scaledPos.xyz + instancePosition, 1);
    gl_Position = proj * view * worldPos;
    free_element_uv = texcoord0;
    atlas_coord = atlas_coords[textureIdx + (is_variant_texture == 0 ? 0 : instance_variant)];
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
