#version 330 core
layout(location = 0) in vec3 aPos;

uniform mat4 projection;
uniform mat4 view;

out vec3 FragPos; // Pass the world space position to the fragment shader

void main() {
    FragPos = aPos; // Directly pass vertex position 
    gl_Position = projection * view * vec4(aPos, 1.0);
}