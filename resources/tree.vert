#version 150

uniform mat4 ciModelViewProjection;

in vec4 ciPosition;
in vec4 ciColor;
in mat4 vInstanceModelMatrix; // per-instance position variable

out lowp vec4 Color;

void main( void )
{
    gl_Position	= ciModelViewProjection * vInstanceModelMatrix * ciPosition;
    Color 		= ciColor;
}