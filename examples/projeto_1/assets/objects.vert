#version 410

layout(location = 0) in vec2 inPosition;

uniform vec4 color;
uniform vec2 translation;

out vec4 fragColor;

void main() {
  gl_Position = vec4(inPosition.xy + translation, 0, 1);
  fragColor = color;
}