#version 330 core

out vec4 FragColor;
in vec2 texCoords;

uniform sampler2D screenTexture;
uniform sampler2D bloomTexture;
uniform float exposure;
uniform bool haveBloom;
float gamma = 2.2f;

void main()
{
    vec3 fragment = texture(screenTexture, texCoords).rgb;
    vec3 bloom = texture(bloomTexture, texCoords).rgb;

    if (haveBloom) {fragment += bloom;}
    vec3 toneMapped = vec3(1.0f) - exp(-fragment * exposure);

    FragColor.rgb = pow(toneMapped, vec3(1.0f / gamma));
}