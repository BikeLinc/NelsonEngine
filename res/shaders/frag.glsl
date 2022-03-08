#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

// texture samplers
uniform sampler2D textureSRC;

void main()
{
	vec4 texColor = texture(textureSRC, TexCoord);
    if(texColor.a < 0.1)
        discard;
    FragColor = texColor;
}