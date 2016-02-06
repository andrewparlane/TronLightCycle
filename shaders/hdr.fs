#version 330

// const input per mesh
uniform vec2 screenResolution;
uniform sampler2D colourTextureSampler;
uniform sampler2D blurTextureSampler;

void main()
{
    vec2 fragmentTextureUV = vec2(gl_FragCoord) / screenResolution;
    vec3 colour = texture(colourTextureSampler, fragmentTextureUV).rgb;
    vec3 blurColour = texture(blurTextureSampler, fragmentTextureUV).rgb;

    vec3 result = colour + blurColour;

    // convert from HDR to LDR
    result = result / (result + vec3(1.0f));

    // gama correction
    const float gamma = 2.2f;
    result = pow(result, vec3(1.0f / gamma));

    gl_FragColor = vec4(result, 1.0f);
}
