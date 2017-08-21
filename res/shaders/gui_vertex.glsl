#version 330 core

layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 texCoord;

uniform mat4 transform;

out vec2 frag_texCoord;

void main()
{
	gl_Position = transform * vec4(pos, 1.0, 1.0);
	frag_texCoord = texCoord;
}