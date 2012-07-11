
uniform sampler2D tex;
uniform float fadeAmount;
uniform float time;
uniform float warpAmp;
uniform float yMult;
uniform float maxU;
uniform float alphaAmount;

void main()
{
    float w = smoothstep(0.0, warpAmp, gl_TexCoord[0].x) * (1.0 - smoothstep(maxU - warpAmp, maxU, gl_TexCoord[0].x));
    float xOffset = sin(time + gl_TexCoord[0].y * yMult) * w * warpAmp;
    vec4 texColor = texture2D(tex, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y));
    float greyScale = dot(texColor.rgb, vec3(0.3, 0.59, 0.11)); 
    vec4 grey = vec4(greyScale, greyScale, greyScale, texColor.a); 
    gl_FragColor = mix(grey, texColor, fadeAmount) * alphaAmount;
}