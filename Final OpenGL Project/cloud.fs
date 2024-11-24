#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BloomColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform float ambientStrength;
uniform float specularStrength;
uniform float shininess;
uniform sampler2D texture1; // Black-and-white cloud texture

uniform vec3 rimColor;
uniform float rimIntensity;

uniform vec3 terminatorColor;
uniform float terminatorBlendFactor;

float gamma = 2.2f;

// Cloud-specific parameters
uniform vec3 cloudBaseColor;    // Base color of the cloud
uniform float transparency;     // Base cloud transparency factor
uniform float noiseScale;       // Scale of texture distortion (optional)

// Flashlight 
uniform bool flashlightOn;
uniform vec3 flashlightDir;
// use viewPos for flashlightPos

float cutoff = 0.95f;        // Narrower cone angle for smaller radius
float outerCutoff = 0.97f;   // Slightly larger for smoother transition

float a = 0.02;              // Refined decay factors for more realistic falloff
float b = 0.6;
uniform bool haveBloom; 
uniform float exposure;
void main() {
    // Normals and lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    
    // Texture-based alpha mask
    float cloudAlpha = texture(texture1, TexCoords).r*transparency;   // Mix transparency with texture value
    vec3 viewVec = viewPos - FragPos;
    float normDotLight = dot(norm, lightDir);
    float diff = max(normDotLight, 0.0);
    vec3 viewDir = normalize(viewVec);


        // Calculate attenuation based on distance to the light (sun)
    float distance = length(lightPos - FragPos);
    float sunAttenuation = 20.0 / distance;

    // Diffuse lighting
    vec3 diffuse = diff * cloudBaseColor;

    // Specular lighting
    vec3 reflectDir = reflect(-lightDir, norm);
    float specFactor = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = specFactor * specularStrength * vec3(1.0);

    // Flashlight Effect
    vec3 spotlight = vec3(0.0);
    vec3 spotlightSpecularFinal = vec3(0.0);
    vec3 flashlightDirNorm = normalize(flashlightDir);
    // Spotlight angle and blending
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
        spotlight = attenuation * cloudBaseColor * (coreColor + ringColor1 + ringColor2) * intensity;
        
        vec3 flashlightColor = vec3(0.8, 0.81, 0.82);
        // Spotlight specular component
        vec3 spotlightReflectDir = reflect(-viewDir, norm);
        float spotlightSpec = pow(max(dot(viewDir, spotlightReflectDir), 0.0), shininess / attenuation);
          

        spotlightSpecularFinal = attenuation * intensity * specular * specularStrength * spotlightSpec * flashlightColor * spotlightSpec;
    }

    // Rim lighting
    float rimViewFactor = 1.0 - max(dot(norm, viewDir), 0.0);
    float rimLightFactor = diff;
    float rim = pow(rimViewFactor * rimLightFactor, 2.5); // Exponent for smoother falloff
    vec3 rimLight = rimColor * rim * rimIntensity;

    // ----------------------------- Back Light Effect ------------------------------
    float backViewFactor = pow(rimViewFactor, 12.0); // Smoothstep for soft transition
    vec3 backLight = rimColor * backViewFactor * rimIntensity; // Rim light color and intensity

    // Terminator blending for soft edges
    float terminatorFactor = pow(1.0 - abs(dot(norm, lightDir)), terminatorBlendFactor);
    vec3 terminator = terminatorFactor * terminatorColor;

    // Combine all lighting effects
    vec3 directLighting = (rimLight + terminator + diffuse + specular) * sunAttenuation * 5.2f;
    vec3 ambientLighting = ambientStrength * cloudBaseColor;
    vec3 additionalLighting = backLight + spotlight + spotlightSpecularFinal;

    vec3 finalColor = directLighting + ambientLighting + additionalLighting;

    // Apply gamma correction
    if (!haveBloom) finalColor = pow(finalColor, vec3(1.0 / gamma));
    vec3 result = vec3(1.0f) - exp(-finalColor * exposure);
    FragColor = vec4(result, cloudAlpha);

    	float brightness = dot(FragColor.rgb, vec3(0.2126f, 0.7152f, 0.0722f));
    if(brightness > 0.15f)
        BloomColor = vec4(FragColor.rgb, cloudAlpha);
    else
        BloomColor = vec4(0.0f, 0.0f, 0.0f, cloudAlpha);
}
