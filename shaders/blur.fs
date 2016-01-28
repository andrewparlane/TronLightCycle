#version 330

// const input per mesh
uniform vec2 screenResolution;
uniform sampler2D colourTextureSampler;
uniform bool horizontal;

// not to be used as a uniform, treat as const
// however better performance like this I think
uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main()
{
    vec2 uv = vec2(gl_FragCoord) / screenResolution;
    vec2 tex_offset = 1.0 / textureSize(colourTextureSampler, 0); // gets size of single texel

    vec3 result = texture(colourTextureSampler, uv).rgb * weight[0];

    if(horizontal)
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(colourTextureSampler, uv + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
            result += texture(colourTextureSampler, uv - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
        }
    }
    else
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(colourTextureSampler, uv + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
            result += texture(colourTextureSampler, uv - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
        }
    }

    gl_FragColor = vec4(result, 1.0f);
}
