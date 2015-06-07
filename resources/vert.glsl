#version 120

varying vec3 ec_pos;

void main()
{
    ec_pos = (gl_ModelViewMatrix * gl_Vertex).xyz;

//    gl_FrontColor = gl_Color * gl_Color;
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
