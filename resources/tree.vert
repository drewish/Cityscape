#version 150

uniform mat4 ciModelViewProjection;

in vec4 ciPosition;
in mat4 vInstanceModelMatrix; // per-instance position variable
in vec4 vInstanceColor;

out vec4 Color;

void main( void )
{
    gl_Position = ciModelViewProjection * vInstanceModelMatrix * ciPosition;
    Color       = vInstanceColor;
}