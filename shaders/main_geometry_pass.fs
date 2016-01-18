#version 330

// input data
in Data
{
    vec2 fragmentTextureUV;
    vec3 vertexPosition_Camera;
    vec3 normal_Camera;
};

// const input per mesh
uniform float fragmentIsTexture;
uniform sampler2D textureSampler;
uniform vec3 fragmentColour;

// outputs - MRT (multiple render targets)
layout (location = 0) out vec3 outVertexPosition_Camera;
layout (location = 1) out vec3 outNormal_Camera;
layout (location = 2) out vec3 outMaterialColour;

void main()
{
    outVertexPosition_Camera = vertexPosition_Camera;
    outNormal_Camera = normalize(normal_Camera);

    // get the colour of this point, either based on texture or input colour
    outMaterialColour.rgb = vec3((fragmentIsTexture > 0.5f) ?
                                    texture(textureSampler, fragmentTextureUV) :
                                    vec4(fragmentColour, 1));
}
