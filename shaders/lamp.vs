#version 330
// Input vertex data, different for all executions of this shader.
in vec3 vertexPosition_Model;

// input data that is constant for whole mesh
uniform mat4 MVP;                   // model -> homogenous

void main()
{
    // vertex position in homogenous co-ords
    gl_Position = MVP * vec4(vertexPosition_Model, 1.0);
}

