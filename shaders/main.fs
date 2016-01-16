#version 330

// input data
in Data
{
    vec2 fragmentTextureUV;
    vec3 position_World;
    vec3 normal_Camera;
    vec3 lightDirection_Camera;
    vec3 eyeDirection_Camera;
};

struct Light
{
    vec3 colour;
    float ambient;
    float diffuse;
    float specular;
};

// const input per mesh
uniform float fragmentIsTexture;
uniform sampler2D textureSampler;
uniform vec3 fragmentColour;
uniform vec3 lightPosition_World;   // note also in vertex shader, must be changed together
uniform Light light;

void main()
{
    // get the colour of this point, either based on texture or input colour
    vec3 materialColour = vec3((fragmentIsTexture > 0.5f) ?
                                    texture(textureSampler, fragmentTextureUV) :
                                    vec4(fragmentColour, 1));

    // normalize the normal and the light direction vectors
    // since we want to use their dot product to get the cos of the angle between them
    vec3 n = normalize(normal_Camera);
    vec3 l = normalize(lightDirection_Camera);
    vec3 e = normalize(eyeDirection_Camera);

    // calculate how the light attenuates as we get further away
    float distance = length(lightPosition_World - position_World);
    float attenuation = 1.0f / (1.0f + (0.045f * distance) + (0.0075f * pow(distance, 2)));

    // calculate the ambient component
    vec3 ambientLighting = materialColour * light.colour * light.ambient;

    // calculate the diffuse component
    // diffuse lighting reflects evenly at every angle.
    vec3 diffuseLighting = materialColour *
                           light.colour *
                           light.diffuse *
                           clamp(dot(n, l), 0, 1);

    // for specular lighting the light reflects off like a mirror
    // only scattering slightly
    vec3 r = reflect(-l,n);
    vec3 materialSpecularColour = light.colour / 4;
    vec3 specularLighting = materialSpecularColour *
                            light.colour *
                            light.specular *
                            pow(clamp(dot(e, r), 0, 1), 32);    // change 32 to increase or decrease scattering angle

    gl_FragColor = vec4(attenuation * (ambientLighting + diffuseLighting + specularLighting), 1.0f);
}
