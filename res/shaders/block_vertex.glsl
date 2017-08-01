#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in float light;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoord;
out float frag_light;

void main()
{
    gl_Position = projection * view * model * vec4(pos, 1.0);
    TexCoord = texCoord;
    frag_light = light;
}