#version 410 core
in vec4 p0, p1;
uniform float t;
uniform mat4 mc;
void main(void)
{
  gl_Position = mc * mix(p0, p1, t);
}
