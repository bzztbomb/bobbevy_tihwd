
uniform sampler2D tex;
uniform float fadeAmount;

void main()
{
    vec4 texColor = texture2D(tex, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y));
    float greyScale = dot(texColor.rgb, vec3(0.3, 0.59, 0.11)); 
    vec4 grey = vec4(greyScale, greyScale, greyScale, 1.0); 
    gl_FragColor = mix(grey, texColor, fadeAmount);
}