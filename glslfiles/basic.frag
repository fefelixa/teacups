// Fragment Shader

#version 400

in  vec3 ex_Color;  //colour arriving from the vertex
out vec4 out_Color; //colour for the pixel

uniform vec4 light_ambient;
uniform vec4 material_ambient;


void main(void)
{
	out_Color = light_ambient * material_ambient;
}