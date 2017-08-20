#version 330 core

out vec4 FragColor;

in vec2 TexCoord;
in float frag_blockLight;
in float frag_sunLight;

uniform sampler2D texture1;
uniform float daylight;

void main()
{
    vec4 color = texture(texture1, TexCoord);
    if (color.a < 0.1)
        discard;

    float intensity = frag_blockLight + frag_sunLight * daylight;
    intensity = min(intensity, 1.0);
    FragColor = color * intensity;
}