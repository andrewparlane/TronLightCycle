#version 330
// Input vertex data, different for all executions of this shader.
in vec3 vertexPosition_Model;
in vec3 vertexNormal_Model;
in vec2 vertexTextureUV;

// input data that is constant for whole mesh
uniform mat4 ModelMatrix;           // model -> world transform
uniform mat4 ViewMatrix;            // world -> camera transform - note also in fragment shader, must be changed together
uniform mat4 MVP;                   // model -> homogenous

// output to fragment / geometry shader
out Data
{
    vec2 fragmentTextureUV;
    vec3 vertexPosition_Camera;
    vec3 normal_Camera;
};

void main()
{
    // vertex position in homogenous co-ords
    gl_Position = MVP * vec4(vertexPosition_Model, 1.0);

    // get the vertex position in world space and pass to fragment shader
    vec3 vertexPosition_World = (ModelMatrix * vec4(vertexPosition_Model, 1.0)).xyz;
    
    // get the vertex position in camera space
    vertexPosition_Camera = (ViewMatrix * vec4(vertexPosition_World, 1.0)).xyz;

    // get the normal vector in camera space and pass to the fragment shader
    normal_Camera = normalize(mat3(transpose(inverse(ViewMatrix * ModelMatrix))) * vertexNormal_Model);
    
    // pass values to fragment shader
    fragmentTextureUV = vertexTextureUV;
}

