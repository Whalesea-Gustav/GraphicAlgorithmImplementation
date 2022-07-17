#version 330 core
out vec4 FragColor;

void main() {
    float depth = gl_FragCoord.z;
    float depth2 = depth * depth;
    FragColor = vec4(depth, depth2, 0.0, 1.0);
}
