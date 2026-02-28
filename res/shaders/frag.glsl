#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 WorldPos;
in vec3 Normal;

// texture samplers
uniform sampler2D textureSRC;
uniform vec4 uTint;
uniform float uMetallic;
uniform float uRoughness;
uniform bool uFullBrightOverride;
uniform vec3 uAmbientColor;
uniform float uAmbientIntensity;
uniform vec3 uCameraPos;

struct SceneLight {
	int type;
	vec3 position;
	vec3 direction;
	vec3 color;
	float intensity;
	float range;
};

uniform int uLightCount;
uniform SceneLight uLights[8];

const float PI = 3.14159265359;

float DistributionGGX(vec3 N, vec3 H, float roughness) {
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	return a2 / max(PI * denom * denom, 0.0001);
}

float GeometrySchlickGGX(float NdotV, float roughness) {
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;
	return NdotV / max((NdotV * (1.0 - k) + k), 0.0001);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);
	return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main()
{
	vec4 texColor = texture(textureSRC, TexCoord);
    if(texColor.a < 0.1)
        discard;

	vec4 baseColor = texColor * uTint;
	if (uFullBrightOverride) {
		FragColor = baseColor;
		return;
	}

	vec3 albedo = baseColor.rgb;
	float metallic = clamp(uMetallic, 0.0, 1.0);
	float roughness = clamp(uRoughness, 0.04, 1.0);
	vec3 N = normalize(Normal);
	vec3 V = normalize(uCameraPos - WorldPos);
	vec3 F0 = mix(vec3(0.04), albedo, metallic);

	vec3 Lo = vec3(0.0);
	for (int i = 0; i < uLightCount; ++i) {
		SceneLight light = uLights[i];
		vec3 L = vec3(0.0);
		float attenuation = 1.0;

		if (light.type == 0) {
			L = normalize(-light.direction);
		} else {
			vec3 toLight = light.position - WorldPos;
			float distance = length(toLight);
			if (distance <= 0.0001) {
				continue;
			}
			L = toLight / distance;
			float ratio = clamp(distance / max(light.range, 0.01), 0.0, 1.0);
			attenuation = 1.0 - (ratio * ratio);
			attenuation *= attenuation;
		}

		vec3 H = normalize(V + L);
		vec3 radiance = max(light.color, vec3(0.0)) * max(light.intensity, 0.0) * attenuation;

		float NDF = DistributionGGX(N, H, roughness);
		float G = GeometrySmith(N, V, L, roughness);
		vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

		vec3 numerator = NDF * G * F;
		float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
		vec3 specular = numerator / max(denominator, 0.0001);

		vec3 kS = F;
		vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);

		float NdotL = max(dot(N, L), 0.0);
		Lo += (kD * albedo / PI + specular) * radiance * NdotL;
	}

	vec3 ambient = uAmbientColor * uAmbientIntensity * albedo * (1.0 - metallic * 0.5);
	vec3 color = ambient + Lo;
	color = color / (color + vec3(1.0));
	color = pow(max(color, vec3(0.0)), vec3(1.0 / 2.2));

    FragColor = vec4(color, baseColor.a);
}
