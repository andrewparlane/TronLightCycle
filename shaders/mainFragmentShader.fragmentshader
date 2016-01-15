#version 330

// input data
in vec2 fragmentTextureUV;
in vec3 position_World;
in vec3 normal_Camera;
in vec3 lightDirection_Camera;
in vec3 eyeDirection_Camera;

// const input per mesh
uniform float fragmentIsTexture;
uniform sampler2D textureSampler;
uniform vec3 fragmentColour;
uniform vec3 lightPosition_World;
uniform vec3 lightColour;
uniform float lightPower;
uniform vec3 lightAmbientColour;

void main()
{
    // get the colour of this point, either based on texture or input colour
    vec4 materialColour = (fragmentIsTexture > 0.5f) ?
                                texture(textureSampler, fragmentTextureUV) :
                                vec4(fragmentColour, 1);
    
    // normalize the normal and the light direction vectors
    // since we want to use their dot product to get the cos of the angle between them
    vec3 n = normalize(normal_Camera);
    vec3 l = normalize(lightDirection_Camera);
    vec3 e = normalize(eyeDirection_Camera);

    // the brightens is inversly proportional to the
    // square of the distance from the light to the point
    float distanceSquared = pow(length(lightPosition_World - position_World),2);
    
    // calculate the diffuse component
    // diffuse lighting reflects evenly at every angle.
    vec3 diffuseLighting = lightColour * 
                           lightPower *
                           clamp(dot(n, l), 0, 1) / 
                           distanceSquared;
    
    // for specular lighting the light reflects off like a mirror
    // only scattering slightly
    vec3 r = reflect(-l,n);
    vec3 specularLighting = lightColour * 
                            lightPower * 
                            pow(clamp(dot(e, r), 0, 1), 5) /    // change 5 to increase or decrease scattering angle
                            distanceSquared;
                            
    vec4 materialSpecularColour = vec4(lightColour / 4, 1.0f);
    
    gl_FragColor = (materialColour *
                        (vec4(lightAmbientColour,1.0) +         // ambient
                         vec4(diffuseLighting,1.0)))  +         // diffuse
                   (materialSpecularColour *
                         vec4(specularLighting,1.0));           // specular
}
