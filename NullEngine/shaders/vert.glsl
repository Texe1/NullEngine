 #version 450 core

layout(location = 0) in vec3 in_position;

layout(location = 0) out vec3 vert_color;

 void main() {
	gl_Position = vec4(in_position.xy, 0.0, 1.0);

	vert_color = vec3((in_position.xy + vec2(1)) / 2, 0.0);
 }