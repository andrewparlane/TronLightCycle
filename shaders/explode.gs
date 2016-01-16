#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in Data
{
    vec2 fragmentTextureUV;
    vec3 position_World;
    vec3 vertexPosition_Camera;
    vec3 normal_Camera;
    vec3 eyeDirection_Camera;
} vertex_in[];

uniform float explode;

out Data
{
    vec2 fragmentTextureUV;
    vec3 position_World;
    vec3 vertexPosition_Camera;
    vec3 normal_Camera;
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
    vec3 normal_Homogeneous = GetNormal();
    vec3 explodeDirection_Homogeneous = normal_Homogeneous * explode * 10;
    vec4 newPosition_Homogeneous[3] = vec4[](gl_in[0].gl_Position + explodeDirection_Homogeneous,
                                             gl_in[1].gl_Position + explodeDirection_Homogeneous,
                                             gl_in[2].gl_Position + explodeDirection_Homogeneous);

    vec4 newCentre_Homogeneous = (newPosition_Homogeneous[0] +
                                  newPosition_Homogeneous[1] +
                                  newPosition_Homogeneous[2]) / 3;

    float scaleFactor = 1.0f - explode;

    int i;
    for(i = 0;i < gl_in.length();i++)
    {
        geometry_out.fragmentTextureUV       = vertex_in[i].fragmentTextureUV;
        geometry_out.position_World          = vertex_in[i].position_World;
        geometry_out.vertexPosition_Camera   = vertex_in[i].vertexPosition_Camera;
        geometry_out.normal_Camera           = vertex_in[i].normal_Camera;
        geometry_out.eyeDirection_Camera     = vertex_in[i].eyeDirection_Camera;

        gl_Position = ((newPosition_Homogeneous[i] - newCentre_Homogeneous) * scaleFactor) + newCentre_Homogeneous;
        EmitVertex();
    }
    EndPrimitive();
}
