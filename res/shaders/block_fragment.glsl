#version 330 core

out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D texture1;

void main()
{
    vec3 texColor = vec3(texture(texture1, TexCoord));
    if (texColor == vec3(0.0, 0.0, 0.0))
        discard;
    FragColor = vec4(texColor, 1.0);
}