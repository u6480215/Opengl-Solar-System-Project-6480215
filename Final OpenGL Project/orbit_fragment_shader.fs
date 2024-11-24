#version 330 core
out vec4 FragColor;

uniform vec3 cameraPos; // Camera position in world space
in vec3 FragPos;        // Orbit point in world space
uniform float gamma;    // Gamma correction parameter
uniform bool haveBloom; 
void main() {
    float distance = length(cameraPos - FragPos);
    float alpha = clamp(1.0 / (distance * distance * 0.005), 0.1, 2.2);
    float darkness = clamp(1.0 / (0.005 * distance), 0.0, 2.2);

    vec3 color = vec3(darkness * 0.5);

    // Apply gamma correction
    vec3 finalColor = pow(color, vec3(1.0 / gamma));
    if (haveBloom) finalColor = pow(color, vec3(gamma));
    FragColor = vec4(finalColor, alpha);
}
