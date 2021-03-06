#version 120
// Input vertex data, different for all executions of this shader.
attribute vec2 vertexPosition_screenspace;
attribute vec2 vertexUV;
attribute vec3 vertexColour;

// output to fragment shader
varying vec2 uv;
varying vec3 colour;

void main()
{
    // Output position of the vertex, in clip space
    // map [0..800][0..600] to [-1..1][-1..1]
    vec2 vertexPosition_homoneneousspace = vertexPosition_screenspace - vec2(400,300); // [0..800][0..600] -> [-400..400][-300..300]
    vertexPosition_homoneneousspace /= vec2(400,300);
    gl_Position =  vec4(vertexPosition_homoneneousspace,0,1);

    uv = vertexUV;
    colour = vertexColour;
}

