#version 330

in vec2 TexCoord;

uniform sampler2D ourTexture;

out vec4 Color;

void main() {
    Color = vec4( 1.0, 0.0, 0.0, texture(ourTexture, TexCoord).r );
    Color = texture(ourTexture, TexCoord);
}
