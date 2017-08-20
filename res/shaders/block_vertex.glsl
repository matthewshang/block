#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in float blockLight;
layout (location = 3) in float sunLight;

uniform mat4 transform;

out vec2 TexCoord;
out float frag_blockLight;
out float frag_sunLight;

void main()
{
    gl_Position = transform * vec4(pos, 1.0);
    TexCoord = texCoord;
    frag_blockLight = blockLight;
    frag_sunLight = sunLight;
}