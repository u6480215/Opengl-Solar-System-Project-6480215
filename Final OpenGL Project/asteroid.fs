#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BloomColor;

in vec2 TexCoords;       // Texture coordinates from vertex shader
in vec3 Normal;          // Normal vector from vertex shader
in vec3 FragPos;         // Fragment position in world space

uniform sampler2D texture1; // Texture for the asteroid surface

uniform vec3 lightPos;   // Position of the light source
uniform vec4 lightColor; // Color of the light (can include intensity)
uniform vec3 viewPos;    // Position of the camera/viewer
uniform float ambientStrength;
uniform float specularStrength;
uniform float shininess;
uniform vec3 rimColor; // Rim light color
uniform float rimIntensity; // Rim light intensity
// Flashlight 
uniform bool flashlightOn;
uniform vec3 flashlightDir;
float cutoff = 0.95f;        // Narrower cone angle for smaller radius
float outerCutoff = 0.97f;   // Slightly larger for smoother transition

float a = 0.02;              // Refined decay factors for more realistic falloff
float b = 0.6;
uniform bool haveBloom; 
uniform bool flipped;

uniform float exposure;
void main()
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewVec = viewPos - FragPos;
    vec3 viewDir = normalize(viewVec);
    float normDotLight = dot(norm, lightDir);
    
    vec3 textureColor = texture(texture1, TexCoords).rgb;
    

    // Calculate attenuation based on distance to the light (sun)
    float distance = length(lightPos - FragPos);
    float sunAttenuation = 20.0 / distance;

    // -----------------------------Diffuse Lighting----------------------------
    float diff = max(normDotLight, 0.0);

    // ----------------------------- Rim Lighting ------------------------------
    float rimViewFactor = 1.0 - max(dot(norm, viewDir), 0.0);
    float rimLightFactor = diff;
    float rim = pow(rimViewFactor * rimLightFactor, 15.0); // Exponent for smoother falloff
    vec3 rimLight = rimColor * rim * rimIntensity;
    
    // ----------------------------- Back Light Effect ------------------------------
    float backViewFactor = pow(rimViewFactor, 8.0); // Smoothstep for soft transition
    vec3 backLight = rimColor * backViewFactor *0.09 * rimIntensity; 

    // -----------------------------Specular Lighting---------------------------
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

    vec3 specular = vec3(1.0);

     // -------------------------- Flashlight Effect ----------------------------
    vec3 spotlight = vec3(0.0);
    vec3 spotlightSpecularFinal = vec3(0.0);

    // Spotlight angle and blending
    vec3 flashlightDirNorm = normalize(flashlightDir);
    float theta = dot(normalize(viewVec), -flashlightDirNorm);

    if (flashlightOn && theta > cutoff) {
        float epsilon = outerCutoff - cutoff;
        float intensity = clamp((theta - cutoff) / epsilon, 0.0, 1.0);

        // green ring blending region
        float ringEpsilon1 = outerCutoff - cutoff - 0.18;
        float ringIntensity1 = clamp((theta - cutoff - 0.18) / ringEpsilon1, 0.0, 1.0);
        // red ring blending region
        float ringEpsilon2 = outerCutoff - cutoff - 0.08;
        float ringIntensity2 = clamp((theta - cutoff - 0.08) / ringEpsilon2, 0.0, 1.0);

        // Distance-based attenuation
        float dist = length(viewVec);
        float attenuation = 1.5f / (a * dist * dist + b * dist + 1.0f);

        // Spotlight diffuse component
        vec3 coreColor = vec3(0.0, 0.0, 0.82) * intensity; // Cyan core
        vec3 ringColor1 = vec3(0.0, 0.81, 0.0) * ringIntensity1; // Fading green ring
        vec3 ringColor2 = vec3(0.8, 0.0, 0.0) * ringIntensity2; // Fading red ring
        spotlight = attenuation * texture(texture1, TexCoords).rgb * (coreColor + ringColor1 + ringColor2) * intensity;
        
        vec3 flashlightColor = vec3(0.8, 0.81, 0.82);
        // Spotlight specular component
        vec3 spotlightReflectDir = reflect(-viewDir, norm);
        float spotlightSpec = pow(max(dot(viewDir, spotlightReflectDir), 0.0), shininess / attenuation);
          

        spotlightSpecularFinal = attenuation * intensity * specular * specularStrength * spotlightSpec * flashlightColor * spotlightSpec;
    }

    // ----------------------------- Combine Results ---------------------------
    vec3 ambient = ambientStrength * textureColor;
    vec3 diffuse = diff * textureColor;
    vec3 specularFinal = specular * specularStrength * spec * textureColor;

    vec3 result = ((rimLight + specularFinal + diffuse ) * sunAttenuation)+ ambient  + backLight + spotlight + spotlightSpecularFinal;

    if (!haveBloom) result = pow(result, vec3(1.0 / 2.2f));
    result = vec3(1.0f) - exp(-result * exposure);
    FragColor = vec4(result, 1.0);
    float brightness = dot(FragColor.rgb, vec3(0.2126f, 0.7152f, 0.0722f));
    if(brightness > 0.15f)
        BloomColor = vec4(FragColor.rgb, 1.0f);
    else
        BloomColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
}
