#version 430

uniform mat4 ModelViewMatrix;
uniform mat4 ProjectionMatrix;

in  vec3 in_Position;  // Position coming in


void main(void)
{
	gl_Position = ProjectionMatrix * ModelViewMatrix * vec4(in_Position, 1.0);
}