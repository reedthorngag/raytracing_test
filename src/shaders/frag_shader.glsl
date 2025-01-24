#version 330 core
out vec4 FragColor;

uniform sampler3D tex;

in vec4 gl_FragCoord;





void main()
{
    FragColor = texture(tex, vec3(gl_FragCoord.xy/1000,0));
} 