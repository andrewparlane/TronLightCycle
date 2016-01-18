#version 330

#define LIGHT_INTENSITY_CUT_OFF     0.01f
#define MAX_LIGHTS                  20      // note: must sync with src/lamp.hp

// input data
in vec2 fragmentTextureUV;

// const input per mesh
uniform sampler2D geometryTextureSampler;
uniform sampler2D normalTextureSampler;
uniform sampler2D colourTextureSampler;

// lights (const per mesh)
// note: I wanted to use an array of structs
//       but that is difficult due to packing issues
uniform int numLights;
uniform vec3 lightPosition_Camera[MAX_LIGHTS];
uniform vec3 lightColour[MAX_LIGHTS];
uniform float lightAmbient[MAX_LIGHTS];
uniform float lightDiffuse[MAX_LIGHTS];
uniform float lightSpecular[MAX_LIGHTS];
uniform float lightRadius[MAX_LIGHTS];

vec3 calculatePointLight(int idx, vec3 vertexPosition_Camera, vec3 normal_Camera, vec3 materialColour, vec3 eyeDirection_Camera)
{
    // get the vector of the light source from the vertex in camera space
    // note: vertex -> light source, seems the wrong way round but makes the maths easier
    vec3 lightDirection_Camera = normalize(lightPosition_Camera[idx] - vertexPosition_Camera);

    // calculate how the light attenuates as we get further away
    float distance = length(lightPosition_Camera[idx] - vertexPosition_Camera);
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
                           clamp(dot(normal_Camera, lightDirection_Camera), 0, 1);

    // for specular lighting the light reflects off like a mirror
    // only scattering slightly
    vec3 r = reflect(-lightDirection_Camera, normal_Camera);
    vec3 specularLighting = lightColour[idx] *
                            lightSpecular[idx] *
                            pow(clamp(dot(eyeDirection_Camera, r), 0, 1), 32);    // change 32 to increase or decrease scattering angle

    return attenuation * (ambientLighting + diffuseLighting + specularLighting);
}

void main()
{
    vec3 vertexPosition_Camera = texture(geometryTextureSampler, fragmentTextureUV).rgb;
    vec3 normal_Camera = texture(normalTextureSampler, fragmentTextureUV).rgb;
    vec3 materialColour = texture(colourTextureSampler, fragmentTextureUV).rgb;

    // set background colour, if normal is 0,0,0 (impossible unless there's nothing there)
    if (normal_Camera == vec3(0.0f))
    {
        //gl_FragColor = vec4(0.0f, 0.0f, 0.4f, 1.0f);
        //return;
    }

    // get a vector from the vertex to the camera in camera space.
    // in camera space, the camera is located at 0,0,0
    vec3 eyeDirection_Camera = normalize(-vertexPosition_Camera);

    vec3 result = vec3(0,0,0);

    int i;
    for (i = 0; i < numLights; i++)
    {
        result += calculatePointLight(i, vertexPosition_Camera, normal_Camera, materialColour, eyeDirection_Camera);
    }

    gl_FragColor = vec4(result, 1.0f);
}