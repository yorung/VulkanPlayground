#version 450

/*
void main()
{
	vec2 vPosition = vec2((gl_VertexIndex & 2) != 0 ? 0.5 : -0.5, (gl_VertexIndex & 1) != 0 ? -0.5 : 0.5);
	gl_Position = vec4(vPosition.xy, 0, 1);
}
*/

layout(location = 0) in vec2 position;

void main()
{
	gl_Position = vec4(position, 0, 1);
}
