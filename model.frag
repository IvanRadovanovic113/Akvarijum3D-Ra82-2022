#version 330 core

out vec4 FragColor;

in vec3 chNormal;
in vec3 chFragPos;
in vec2 chUV;

// Glavni izvor svetlosti
uniform vec3 uLightPos;
uniform vec3 uLightColor;
uniform float uLightIntensity;

// Drugi izvor svetlosti (blago u kovčegu)
uniform vec3 uLight2Pos;
uniform vec3 uLight2Color;
uniform float uLight2Intensity;

uniform vec3 uViewPos;
uniform vec3 uFallbackColor;

uniform sampler2D uDiffMap1;

void main()
{
    // Uzmi boju iz teksture ili fallback
    vec4 texColor = texture(uDiffMap1, chUV);
    if (texColor.r < 0.01 && texColor.g < 0.01 && texColor.b < 0.01) {
        texColor = vec4(uFallbackColor, 1.0);
    }
    vec3 objectColor = texColor.rgb;

    vec3 norm = normalize(chNormal);
    vec3 viewDir = normalize(uViewPos - chFragPos);

    // ========== GLAVNI IZVOR SVETLOSTI ==========
    // Ambijentna komponenta (vrlo slaba)
    float ambientStrength = 0.05;
    vec3 ambient = ambientStrength * uLightColor;

    // Difuzna komponenta
    vec3 lightDir1 = normalize(uLightPos - chFragPos);
    float diff1 = max(dot(norm, lightDir1), 0.0);
    vec3 diffuse1 = diff1 * uLightColor * uLightIntensity;

    // Spekularna komponenta
    vec3 reflectDir1 = reflect(-lightDir1, norm);
    float spec1 = pow(max(dot(viewDir, reflectDir1), 0.0), 32.0);
    vec3 specular1 = 0.5 * spec1 * uLightColor * uLightIntensity;

    vec3 light1Result = ambient + diffuse1 + specular1;

    // ========== DRUGI IZVOR SVETLOSTI (BLAGO) ==========
    vec3 light2Result = vec3(0.0);
    if (uLight2Intensity > 0.01) {
        vec3 lightDir2 = normalize(uLight2Pos - chFragPos);

        // Difuzna
        float diff2 = max(dot(norm, lightDir2), 0.0);
        vec3 diffuse2 = diff2 * uLight2Color * uLight2Intensity;

        // Spekularna
        vec3 reflectDir2 = reflect(-lightDir2, norm);
        float spec2 = pow(max(dot(viewDir, reflectDir2), 0.0), 32.0);
        vec3 specular2 = 0.3 * spec2 * uLight2Color * uLight2Intensity;

        // Atenuacija (slabljenje sa udaljenoscu)
        float distance = length(uLight2Pos - chFragPos);
        float attenuation = 1.0 / (1.0 + 0.22 * distance + 0.20 * distance * distance);

        light2Result = (diffuse2 + specular2) * attenuation;
    }

    // Kombinuj sve
    vec3 result = objectColor * (light1Result + light2Result);
    FragColor = vec4(result, 1.0);
}
