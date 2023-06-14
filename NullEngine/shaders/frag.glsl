#version 450 core

layout(location = 0) out vec4 color_out;
layout(location = 0) in vec3 col;

void main() {
	color_out = vec4(col, 1.0); 
}