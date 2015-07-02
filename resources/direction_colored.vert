#version 150

uniform mat4    ciModelView;
uniform mat4    ciModelViewProjection;

in vec4         ciPosition;
in vec4         ciColor;
out vec3        eyespacePosition;

void main()
{
    eyespacePosition = (ciModelView * ciPosition).xyz;

//    gl_FrontColor = gl_Color * gl_Color;
    gl_Position = ciModelViewProjection * ciPosition;
}
