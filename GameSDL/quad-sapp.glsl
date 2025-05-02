@vs vs
layout(binding = 0) uniform Uniforms {
    mat4 view;
    mat4 proj;
};

in vec4 position;
in vec4 color0;
out vec4 color;

void main() {
    gl_Position = proj * view * position;
    color = color0;
}
@end

@fs fs
in vec4 color;
out vec4 frag_color;

void main() {
    frag_color = color;
}
@end

@program quad vs fs

