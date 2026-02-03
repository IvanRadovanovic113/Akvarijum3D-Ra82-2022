#version 330 core

in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uTex;

void main()
{
    vec4 texColor = texture(uTex, vUV);
    FragColor = texColor;
}
