#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in Data
{
    vec2 fragmentTextureUV;
    vec3 position_World;
    vec3 normal_Camera;
    vec3 lightDirection_Camera;
    vec3 eyeDirection_Camera;
} vertex_in[];

uniform float explode;

out Data
{
    vec2 fragmentTextureUV;
    vec3 position_World;
    vec3 normal_Camera;
    vec3 lightDirection_Camera;
    vec3 eyeDirection_Camera;
} geometry_out;

vec3 GetNormal()
{
   vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
   vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
   return normalize(cross(a, b));
}

void main()
{
    vec3 normal_World = GetNormal();
    vec3 explodeDirection_World = normal_World * explode * 10;
    vec4 newPosition_World[3] = vec4[](gl_in[0].gl_Position + explodeDirection_World,
                                       gl_in[1].gl_Position + explodeDirection_World,
                                       gl_in[2].gl_Position + explodeDirection_World);

    vec4 newCentre_World = (newPosition_World[0] +
                            newPosition_World[1] +
                            newPosition_World[2]) / 3;

    float scaleFactor = 1.0f - explode;

    int i;
    for(i = 0; i < 3; i++)
    {
        newPosition_World[i] = ((newPosition_World[i] - newCentre_World) * scaleFactor) + newCentre_World;
    }

    for(i = 0;i < gl_in.length();i++)
    {
        geometry_out.fragmentTextureUV       = vertex_in[i].fragmentTextureUV;
        geometry_out.position_World          = vertex_in[i].position_World;
        geometry_out.normal_Camera           = vertex_in[i].normal_Camera;
        geometry_out.lightDirection_Camera   = vertex_in[i].lightDirection_Camera;
        geometry_out.eyeDirection_Camera     = vertex_in[i].eyeDirection_Camera;

        gl_Position = newPosition_World[i];
        EmitVertex();
    }
    EndPrimitive();
}
