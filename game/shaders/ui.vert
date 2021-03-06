#version 120

uniform vec2 viewport;

attribute vec3 pos;
attribute vec2 uv;
attribute float z_index;

varying vec2 uv_out;

void main()
{
	gl_Position =  vec4(pos.x * viewport.x - 1.0, pos.y  * -viewport.y + 1.0, 0.9 - (z_index / 10), 1);
    uv_out = uv;
};