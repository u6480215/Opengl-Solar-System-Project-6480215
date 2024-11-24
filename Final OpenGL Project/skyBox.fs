#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;
uniform bool haveBloom; 
float gamma = 2.2f;
uniform float exposure;
void main()
{    
    vec3 finalColor = texture(skybox, TexCoords).rgb;
    if (haveBloom) finalColor = pow(finalColor, vec3(gamma));
    vec3 toneMapped = vec3(1.0f) - exp(-finalColor * exposure);
    FragColor = vec4(toneMapped,1.0f);

}