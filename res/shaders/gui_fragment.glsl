#version 330 core

out vec4 FragColor;

in vec2 frag_texCoord;

uniform sampler2D tex;

void main()
{
	vec4 color = texture(tex, frag_texCoord);
	if (color.a < 0.1)
		discard;	

	FragColor = vec4(color.rgb, 0.5);
}