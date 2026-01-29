#version 330 core

in vec4 Color;
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D uTex;
uniform bool useTex;
uniform bool transparent;
uniform vec4 uColor;

void main()
{
    vec4 baseColor;

    if (useTex)
        baseColor = texture(uTex, TexCoord);
    else
        baseColor = uColor;

    if (transparent)
        FragColor = vec4(baseColor.rgb, baseColor.a);
    else
        FragColor = vec4(baseColor.rgb, 1.0);
}
