#version 330

#define LIGHT_INTENSITY_CUT_OFF     0.01f
#define MAX_LIGHTS                  20      // note: must sync with src/lamp.hp

// input data
in Data
{
    vec2 fragmentTextureUV;
    vec3 vertexPosition_Camera;
    vec3 normal_Camera;
    vec3 eyeDirection_Camera;
};

// const input per mesh
uniform mat4 ViewMatrix;            // world -> camera transform - note also in fragment shader, must be changed together
uniform float fragmentIsTexture;
uniform sampler2D textureSampler;
uniform vec3 fragmentColour;

// lights (const per mesh)
// note: I wanted to use an array of structs
//       but that is difficult due to packing issues
uniform int numLights;
uniform vec3 lightPosition_World[MAX_LIGHTS];
uniform vec3 lightColour[MAX_LIGHTS];
uniform float lightAmbient[MAX_LIGHTS];
uniform float lightDiffuse[MAX_LIGHTS];
uniform float lightSpecular[MAX_LIGHTS];
uniform float lightRadius[MAX_LIGHTS];

vec3 calculatePointLight(int idx, vec3 materialColour)
{
    // get the vector of the light source from the vertex in camera space
    // note: vertex -> light source, seems the wrong way round but makes the maths easier
    vec3 lightPosition_Camera = (ViewMatrix * vec4(lightPosition_World[idx], 1.0)).xyz;
    vec3 lightDirection_Camera = lightPosition_Camera - vertexPosition_Camera;

    // normalize the normal and the light direction vectors
    // since we want to use their dot product to get the cos of the angle between them
    vec3 n = normalize(normal_Camera);
    vec3 l = normalize(lightDirection_Camera);
    vec3 e = normalize(eyeDirection_Camera);

    // calculate how the light attenuates as we get further away
    float distance = length(lightPosition_Camera - vertexPosition_Camera);
    float attenuation = 1.0f / pow(1.0f + (distance / lightRadius[idx]), 2);
    attenuation = (attenuation - LIGHT_INTENSITY_CUT_OFF) / (1.0f - LIGHT_INTENSITY_CUT_OFF);
    if (attenuation <= 0.0f)
    {
        return vec3(0,0,0);
    }

    // calculate the ambient component
    vec3 ambientLighting = materialColour * lightColour[idx] * lightAmbient[idx];

    // calculate the diffuse component
    // diffuse lighting reflects evenly at every angle.
    vec3 diffuseLighting = materialColour *
                           lightColour[idx] *
                           lightDiffuse[idx] *
                           clamp(dot(n, l), 0, 1);

    // for specular lighting the light reflects off like a mirror
    // only scattering slightly
    vec3 r = reflect(-l,n);
    vec3 specularLighting = lightColour[idx] *
                            lightSpecular[idx] *
                            pow(clamp(dot(e, r), 0, 1), 32);    // change 32 to increase or decrease scattering angle

    return attenuation * (ambientLighting + diffuseLighting + specularLighting);
}

void main()
{
    // get the colour of this point, either based on texture or input colour
    vec3 materialColour = vec3((fragmentIsTexture > 0.5f) ?
                                    texture(textureSampler, fragmentTextureUV) :
                                    vec4(fragmentColour, 1));

    vec3 result = vec3(0,0,0);

    int i;
    for (i = 0; i < numLights; i++)
    {
        result += calculatePointLight(i, materialColour);
    }

    gl_FragColor = vec4(result, 1.0f);
}
