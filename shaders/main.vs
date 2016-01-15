#version 330
// Input vertex data, different for all executions of this shader.
in vec3 vertexPosition_Model;
in vec3 vertexNormal_Model;
in vec2 vertexTextureUV;

// input data that is constant for whole mesh
uniform mat4 ModelMatrix;           // model -> world transform
uniform mat4 ViewMatrix;            // world -> camera transform
uniform mat4 MVP;                   // model -> homogenous
uniform vec3 lightPosition_World;

// output to fragment / geometry shader
out Data
{
    vec2 fragmentTextureUV;
    vec3 position_World;
    vec3 normal_Camera;
    vec3 lightDirection_Camera;
    vec3 eyeDirection_Camera;
};

void main()
{
    // vertex position in homogenous co-ords
    gl_Position = MVP * vec4(vertexPosition_Model, 1.0);

    // get the vertex position in world space and pass to fragment shader
    position_World = (ModelMatrix * vec4(vertexPosition_Model, 1.0)).xyz;
    
    // get the vertex position in camera space
    vec3 position_Camera = (ViewMatrix * vec4(position_World, 1.0)).xyz;
    
    // get the vector of the light source from the vertex in camera space
    // note: vertex -> light source, seems the wrong way round but makes the maths easier
    vec3 lightPosition_Camera = (ViewMatrix * vec4(lightPosition_World, 1.0)).xyz;
    lightDirection_Camera = lightPosition_Camera - position_Camera;
    
    // get the normal vector in camera space and pass to the fragment shader
    // note: if the model matrix scales the model, then this dosen't work
    //       TODO
    normal_Camera = (ViewMatrix * ModelMatrix * vec4(vertexNormal_Model, 0.0f)).xyz;
    
    // get a vector from the vertex to the camera in camera space.
    // in camera space, the camera is located at 0,0,0
    eyeDirection_Camera = vec3(0,0,0) - position_Camera;
    
    // pass values to fragment shader
    fragmentTextureUV = vertexTextureUV;
}

