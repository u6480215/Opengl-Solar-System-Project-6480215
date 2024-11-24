#version 330 core

layout(location = 0) in vec3 aPos;            // Vertex position
layout(location = 1) in vec3 aNormal;         // Vertex normal
layout(location = 2) in vec2 aTexCoords;      // Texture coordinates
layout(location = 3) in mat4 instanceMatrix;  // Instance-specific model matrix

out vec2 TexCoords;                           // Pass to fragment shader
out vec3 FragPos;                             // Pass to fragment shader
out vec3 Normal;                              // Pass to fragment shader

uniform mat4 view;                            // View matrix
uniform mat4 projection;                      // Projection matrix
uniform float asteroidRotationAngle;          // Rotation angle for the belt

void main() {
    // Compute rotation matrix around the Y-axis
    mat4 rotation = mat4(
        cos(asteroidRotationAngle), 0.0, sin(asteroidRotationAngle), 0.0,
        0.0,                        1.0, 0.0,                        0.0,
       -sin(asteroidRotationAngle), 0.0, cos(asteroidRotationAngle), 0.0,
        0.0,                        0.0, 0.0,                        1.0
    );

    // Combine rotation with instance matrix
    mat4 model = rotation * instanceMatrix;

    // Compute world position
    vec4 worldPos = model * vec4(aPos, 1.0);
    FragPos = vec3(worldPos);

    // Compute transformed normal
    mat3 normalMatrix = mat3(transpose(inverse(model)));
    Normal = normalize(normalMatrix * aNormal);

    // Pass texture coordinates
    TexCoords = aTexCoords;

    // Compute final vertex position
    gl_Position = projection * view * worldPos;
}
