#version 330

// const input per mesh
uniform vec2 screenResolution;
uniform sampler2D colourTextureSampler;

void main()
{
    vec2 fragmentTextureUV = vec2(gl_FragCoord) / screenResolution;
    vec3 colour = texture(colourTextureSampler, fragmentTextureUV).rgb;

    // convert from HDR to LDR
    colour = colour / (colour + vec3(1.0f));

    // gama correction
    const float gamma = 2.2f;
    colour = pow(colour, vec3(1.0f / gamma));

    gl_FragColor = vec4(colour, 1.0f);
}
