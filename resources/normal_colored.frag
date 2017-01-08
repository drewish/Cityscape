#version 150

in vec3     eyespacePosition;
out vec4    oColor;

void main()
{
    vec3 eyespaceNormal = normalize(cross(dFdx(eyespacePosition), dFdy(eyespacePosition)));

    // Color based on normal
    oColor = vec4(eyespaceNormal, 0.75);
}
