#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec2 aTex;
layout (location = 3) in vec3 aNormal;

uniform mat4 uM;
uniform mat4 uV;
uniform mat4 uP;

out vec4 Color;
out vec2 TexCoord;

void main()
{
    gl_Position = uP * uV * uM * vec4(aPos, 1.0);
    Color = aColor;
    TexCoord = aTex;
}
