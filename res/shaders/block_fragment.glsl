#version 330 core

out vec4 FragColor;

in vec2 TexCoord;
in float frag_light;

uniform sampler2D texture1;
uniform float daylight;

void main()
{
    vec4 texColor = texture(texture1, TexCoord);
    if (texColor.a < 0.1)
        discard;

    float intensity = min(1.0, daylight + frag_light);
    FragColor = texColor * intensity;
}