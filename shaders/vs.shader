#version 330 core

layout(location = 0) in vec3 aPosition;
//layout(location = 1) in vec3 aNormal;
layout (location = 1) in vec3 aColor; // Vertex color input

out vec3 FragColor; // Fragment color output

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
//uniform vec3 color;

//out vec3 WorldPos;
//out vec3 Normal;
//out vec4 Color;

void main()
{
  FragColor = aColor;
  //FragColor = vec3(0.572549f,0.584314f,0.611765f)
  //WorldPos = vec3(model * vec4(aPosition, 1.0));
  //Normal = aNormal;

  gl_Position = projection * view * model * vec4(aPosition, 1.0f);

}