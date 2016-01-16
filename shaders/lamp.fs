#version 330

// const input per mesh
uniform vec3 fragmentColour;

void main()
{
    gl_FragColor = vec4(fragmentColour, 1.0f);
}
