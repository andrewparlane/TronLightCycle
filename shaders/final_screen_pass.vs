#version 330
// Input vertex data, different for all executions of this shader.
in vec2 vertexPosition_Screen;

void main()
{
    gl_Position = vec4(vertexPosition_Screen, 0.0f, 1.0f);
}
