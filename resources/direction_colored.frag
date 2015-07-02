#version 150

in vec3     eyespacePosition;
out vec4    oColor;

void main()
{
    vec3 eyespaceNormal = normalize(cross(dFdx(eyespacePosition), dFdy(eyespacePosition)));
    oColor = vec4(eyespaceNormal, 1.0);
}