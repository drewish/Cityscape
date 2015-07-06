#version 150

in vec3     eyespacePosition;
out vec4    oColor;

void main()
{
    vec3 eyespaceNormal = normalize(cross(dFdx(eyespacePosition), dFdy(eyespacePosition)));
    oColor = vec4(eyespaceNormal, 1.0);
    
//    float sum = (eyespaceNormal.x + eyespaceNormal.y + eyespaceNormal.z) / 3.0;
//    oColor = vec4(sum, sum, sum, 1.0);
}