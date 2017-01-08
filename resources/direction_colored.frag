#version 150

uniform vec3 darkColor;
uniform vec3 mediumColor;
uniform vec3 lightColor;

in vec3     eyespacePosition;
out vec4    oColor;

void main()
{
    vec3 eyespaceNormal = normalize(cross(dFdx(eyespacePosition), dFdy(eyespacePosition)));

    // Stylized
    if (eyespaceNormal.y > 0.5) {
        oColor = vec4(darkColor, 1);
    } else if (eyespaceNormal.x > 0.25) {
        oColor = vec4(mediumColor, 1);
    } else {
        oColor = vec4(lightColor, 1);
    }

//    // Grayscale
//    float sum = (eyespaceNormal.x + eyespaceNormal.y + eyespaceNormal.z) / 3.0;
//    oColor = vec4(sum, sum, sum, 1.0);
}
