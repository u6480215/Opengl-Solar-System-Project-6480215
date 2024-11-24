#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BloomColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 lightPos;
uniform vec4 lightColor;
uniform vec3 viewPos;
uniform float ambientStrength;
uniform float specularStrength;
uniform float shininess;

uniform sampler2D texture1; // Diffuse texture
uniform sampler2D specularMap; // Specular map (optional)
uniform bool useSpecularMap; // Toggle for specular map usage
uniform sampler2D nightMap; 
uniform bool useNightMap; 


// Flashlight 
uniform bool flashlightOn;
uniform vec3 flashlightDir;
// use viewPos for flashlightPos

float cutoff = 0.95f;        // Narrower cone angle for smaller radius
float outerCutoff = 0.97f;   // Slightly larger for smoother transition

float a = 0.02;              // Refined decay factors for more realistic falloff
float b = 0.6;

uniform vec3 rimColor; // Rim light color
uniform float rimIntensity; // Rim light intensity

uniform vec3 terminatorColor; // Color of the light terminator line
uniform float terminatorBlendFactor; // Control the softness of the terminator blend

uniform vec3 edgeColor; // Color of the light terminator line
uniform float edgeIntensity; // Control the softness of the terminator blend

float ambientBase = 0.02; 
uniform float gamma;
uniform bool haveBloom; 
uniform float exposure;
void main() {
    // ------------------------Normal and Light Direction-----------------------
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 flashlightDirNorm = normalize(flashlightDir);
    vec3 viewVec = viewPos - FragPos;
    vec3 viewDir = normalize(viewVec);
    float normDotLight = dot(norm, lightDir);

    // Calculate attenuation based on distance to the light (sun)
    float distance = length(lightPos - FragPos);
    float sunAttenuation = 20.0 / distance;

    // -----------------------------Diffuse Lighting----------------------------
    float diff = max(normDotLight, 0.0);

    // -----------------------------Terminator Line-----------------------------
    vec3 terminatorLine = vec3(0.0, 0.0, 0.0);
    if(rimIntensity > 0) {
        float terminatorFactor = pow(1.0f - abs(normDotLight), terminatorBlendFactor*2); // Sharper falloff
        terminatorLine = terminatorFactor * terminatorColor * texture(texture1, TexCoords).rgb;
    }
    // -----------------------------Specular Lighting---------------------------
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

    // Use the specular map if available
    vec3 specular = useSpecularMap ? texture(specularMap, TexCoords).rgb : vec3(1.0);

    // -------------------------- Flashlight Effect ----------------------------
    vec3 spotlight = vec3(0.0);
    vec3 spotlightSpecularFinal = vec3(0.0);

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
        spotlight = attenuation * texture(texture1, TexCoords).rgb * (coreColor + ringColor1 + ringColor2) * intensity;
        
        vec3 flashlightColor = vec3(0.8, 0.81, 0.82);
        // Spotlight specular component
        vec3 spotlightReflectDir = reflect(-viewDir, norm);
        float spotlightSpec = pow(max(dot(viewDir, spotlightReflectDir), 0.0), shininess / attenuation);
          

        spotlightSpecularFinal = attenuation * intensity * specular * specularStrength * spotlightSpec * flashlightColor * spotlightSpec;
    }

    // ----------------------------- Night Lighting----------------------------
    vec3 nightLights = vec3(0.0);
    if (useNightMap) {
        // Fetch the night map texture value
        vec3 nightTex = texture(nightMap, TexCoords).rgb;
        
        float nightFactor = pow(1.0 - diff, 14.0);
        // Threshold to boost bright spots
        float lightThreshold = 0.3; // Only boost areas brighter than this
        if (nightTex.r > lightThreshold || nightTex.g > lightThreshold || nightTex.b > lightThreshold) {
            nightLights = nightTex * 1.2; // Apply boost factor to bright areas
        }
        nightLights *= nightFactor;
    }
    // ----------------------------- Rim Lighting ------------------------------
    float rimViewFactor = 1.0 - max(dot(norm, viewDir), 0.0);
    float rimLightFactor = diff;
    float rim = pow(rimViewFactor * rimLightFactor, 3.5); // Exponent for smoother falloff
    vec3 rimLight = rimColor * rim * rimIntensity;

    // ----------------------------- Back Light Effect ------------------------------
    float backViewFactor = pow(rimViewFactor, 15.0); // Smoothstep for soft transition
    vec3 backLight = rimColor * backViewFactor *0.09 * rimIntensity; 

     // ----------------------------- edge Effect ------------------------------
    float edgeViewFactor = rimViewFactor;
    float edgeLightFactor = diff;
    float edge = pow(edgeViewFactor * edgeLightFactor, 2.5); // Exponent for smoother falloff
    vec3 edgeLight = edgeColor * edge * edgeIntensity;


    // ----------------------------- Combine Results ---------------------------
    vec3 ambient = ambientStrength * texture(texture1, TexCoords).rgb;
    vec3 diffuse = diff * texture(texture1, TexCoords).rgb ;
    vec3 specularFinal = specular * specularStrength * spec;

    vec3 result = ((diffuse + specularFinal + terminatorLine + rimLight) * sunAttenuation*lightColor.rgb) + 
                (ambient + spotlight + spotlightSpecularFinal  + backLight + edgeLight+nightLights);

    // Add night map contribution
    //result += nightLights;

    // Apply gamma correction
    if (!haveBloom) result = pow(result, vec3(1.0 / gamma));
    result = vec3(1.0f) - exp(-result * exposure);
    FragColor = vec4(result, 1.0);

	// Calculate brightness by adding up all the channels with different weights each
	float brightness = dot(FragColor.rgb, vec3(0.2126f, 0.7152f, 0.0722f));
    if(brightness > 0.15f)
        BloomColor = vec4(FragColor.rgb, 1.0f);
    else
        BloomColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
}