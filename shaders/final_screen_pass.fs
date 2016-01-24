#version 330

// const input per mesh
uniform vec2 screenResolution;
uniform sampler2D colourTextureSampler;

void main()
{
    vec2 fragmentTextureUV = vec2(gl_FragCoord) / screenResolution;
    vec3 colour = texture(colourTextureSampler, fragmentTextureUV).rgb;
    gl_FragColor = vec4(colour, 1.0f);
}
