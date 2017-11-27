#version  330 core
layout(location = 0) in vec3 vertPos;
uniform mat4 P;
uniform mat4 MV;
uniform mat4 View;


void main()
{
	gl_Position = P * View * MV * vec4(vertPos, 1.0);
}
