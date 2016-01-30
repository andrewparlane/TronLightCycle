#version 330

// const input per mesh
uniform vec2 screenResolution;
uniform sampler2D colourTextureSampler;
uniform int horizontal;

// not to be used as a uniform, treat as const
// however better performance like this I think
uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main()
{
    vec2 uv = vec2(gl_FragCoord) / screenResolution;
    vec2 texOffset = 1.0 / textureSize(colourTextureSampler, 0); // gets size of single texel

    vec3 result = texture(colourTextureSampler, uv).rgb * weight[0];

    texOffset.x *= horizontal % 2;         // if (!horizontal) offset.x = 0;
    texOffset.y *= (horizontal + 1) % 2;   // if (horizontal) offset.y = 0;

    result += texture(colourTextureSampler, uv + (texOffset * 1)).rgb * weight[1];
    result += texture(colourTextureSampler, uv - (texOffset * 1)).rgb * weight[1];

    result += texture(colourTextureSampler, uv + (texOffset * 2)).rgb * weight[2];
    result += texture(colourTextureSampler, uv - (texOffset * 2)).rgb * weight[2];

    result += texture(colourTextureSampler, uv + (texOffset * 3)).rgb * weight[3];
    result += texture(colourTextureSampler, uv - (texOffset * 3)).rgb * weight[3];

    result += texture(colourTextureSampler, uv + (texOffset * 4)).rgb * weight[4];
    result += texture(colourTextureSampler, uv - (texOffset * 4)).rgb * weight[4];

    gl_FragColor = vec4(result, 1.0f);
}
