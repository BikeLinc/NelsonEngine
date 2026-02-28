#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec2 TexCoord;
out vec3 WorldPos;
out vec3 Normal;

uniform mat4 transform;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	vec4 worldPos = transform * vec4(aPos, 1.0);
	mat3 normalMatrix = mat3(transpose(inverse(transform)));
	WorldPos = worldPos.xyz;
	Normal = normalize(normalMatrix * aNormal);
	gl_Position = projection * view * worldPos;
	TexCoord = vec2(aTexCoord.x, aTexCoord.y);
}
