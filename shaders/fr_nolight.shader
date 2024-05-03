#version 330

in vec3 FragColor; // Fragment color input from vertex shader
out vec4 fragColor; // Output color of the fragment

void main()
{
    fragColor = vec4(FragColor, 1.0); // Set the output color to the input color
}
