#version  330 core
layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;
uniform mat4 P;
uniform mat4 MV;
uniform mat4 View;
out vec3 fragNor;
out vec3 WPos;

void main()
{
	gl_Position = P * View * MV * vec4(vertPos, 1.0);
	fragNor = (MV * vec4(vertNor, 0.0)).xyz;
	WPos = vec3(MV*(vec4(vertPos, 1.0)));
}
