#version 330

#define LIGHT_INTENSITY_CUT_OFF     0.01f

// const input per mesh
uniform vec2 screenResolution;
uniform sampler2D geometryTextureSampler;
uniform sampler2D normalTextureSampler;
uniform sampler2D colourTextureSampler;

// lights (const per mesh)
uniform vec3 lightPosition_Camera;
uniform vec3 lightColour;
uniform float lightAmbient;
uniform float lightDiffuse;
uniform float lightSpecular;
uniform float lightRadius;

vec3 calculatePointLight(vec3 vertexPosition_Camera, vec3 normal_Camera, vec3 materialColour, vec3 eyeDirection_Camera)
{
    // get the vector of the light source from the vertex in camera space
    // note: vertex -> light source, seems the wrong way round but makes the maths easier
    vec3 lightDirection_Camera = normalize(lightPosition_Camera - vertexPosition_Camera);

    // calculate how the light attenuates as we get further away
    float distance = length(lightPosition_Camera - vertexPosition_Camera);
    float attenuation = 1.0f / pow(1.0f + (distance / lightRadius), 2);
    attenuation = clamp((attenuation - LIGHT_INTENSITY_CUT_OFF) / (1.0f - LIGHT_INTENSITY_CUT_OFF), 0, 1);

    // calculate the ambient component
    vec3 ambientLighting = materialColour * lightColour * lightAmbient;

    // calculate the diffuse component
    // diffuse lighting reflects evenly at every angle.
    vec3 diffuseLighting = materialColour *
                           lightColour *
                           lightDiffuse *
                           clamp(dot(normal_Camera, lightDirection_Camera), 0, 1);

    // for specular lighting the light reflects off like a mirror
    // only scattering slightly
    vec3 r = reflect(-lightDirection_Camera, normal_Camera);
    vec3 specularLighting = lightColour *
                            lightSpecular *
                            pow(clamp(dot(eyeDirection_Camera, r), 0, 1), 32);    // change 32 to increase or decrease scattering angle

    return attenuation * (ambientLighting + diffuseLighting + specularLighting);
}

void main()
{
    vec2 fragmentTextureUV = vec2(gl_FragCoord) / screenResolution;
    vec3 vertexPosition_Camera = texture(geometryTextureSampler, fragmentTextureUV).rgb;
    vec3 normal_Camera = texture(normalTextureSampler, fragmentTextureUV).rgb;
    vec3 materialColour = texture(colourTextureSampler, fragmentTextureUV).rgb;

    // get a vector from the vertex to the camera in camera space.
    // in camera space, the camera is located at 0,0,0
    vec3 eyeDirection_Camera = normalize(-vertexPosition_Camera);

    vec3 result = calculatePointLight(vertexPosition_Camera, normal_Camera, materialColour, eyeDirection_Camera);

    gl_FragColor = vec4(result, 1.0f);
}
