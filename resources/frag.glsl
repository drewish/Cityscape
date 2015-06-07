#version 120

varying vec3 ec_pos;

void main()
{
    vec3 ec_normal = normalize(cross(dFdx(ec_pos), dFdy(ec_pos)));

    gl_FragColor = vec4(ec_normal, 1.0);
}