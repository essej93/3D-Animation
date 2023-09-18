#version 330 core

// input data
layout(location = 0) in vec3 aPosition;

// uniform input data
uniform mat4 uModelViewProjectionMatrix;

void main()
{
	// set vertex position
    gl_Position = uModelViewProjectionMatrix * vec4(aPosition, 1.0f);
}
