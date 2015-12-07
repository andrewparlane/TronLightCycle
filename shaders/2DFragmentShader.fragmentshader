#version 120

// input data
varying vec2 uv;
varying vec3 colour;

// const input per mesh
uniform float fragmentIsTexture;
uniform sampler2D myTextureSampler;

void main()
{
    gl_FragColor = (fragmentIsTexture > 0.5f) ?
                        texture2D(myTextureSampler, uv) :
                        vec4(colour, 1);
}
