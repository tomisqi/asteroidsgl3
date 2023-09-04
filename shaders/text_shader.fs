#version 330 core
out vec4 color;
  
in vec4 ourColor;
in vec2 TexCoord;

uniform sampler2D ourTexture;

void main()
{
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(ourTexture, TexCoord).r);
    color = ourColor * sampled;
}