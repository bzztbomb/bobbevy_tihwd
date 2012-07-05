uniform vec4 lightColor;

void main()
{
    vec4 finalColor = lightColor;
    float t = gl_TexCoord[0].x;
    float blend = smoothstep(0.1, 0.3, t) * (1.0 - smoothstep(0.6, 0.9, t));
    finalColor *= blend;
    finalColor.a = smoothstep(0.0, 0.2, blend);
    gl_FragColor = finalColor;
}