import glsl;

[shader("fragment")]
void fragment_main() {
    mat4 matrix = mat4(1.0);
    vec4 vector = vec4(1.0);
    vec4 result0 = matrix * vector; 
    vec4 result1 = matrix * vector; 
}