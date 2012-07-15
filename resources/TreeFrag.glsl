uniform sampler2D tex;
uniform float farClip;
uniform vec4 fogColor;
uniform float fogHeight;

varying vec4 viewSpacePos;

void main()
{
    vec4 texColor = texture2D(tex, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y));
    texColor *= gl_Color;
    vec4 pixelFogColor = fogColor;
    pixelFogColor.a = texColor.a;
    float distance = distance(viewSpacePos, vec4(0.0, 0.0, 0.0, 0.0));
    float density = clamp(farClip - distance, 0.0, farClip) / farClip;
    float fog = density*density;
//    float t = 1.0 - step(-200.0, -viewSpacePos.y);
//    float t = smoothstep(-600.0, -fogHeight, -viewSpacePos.y); 
//    fog *= t;
    gl_FragColor = mix(fogColor, texColor, fog);
    gl_FragColor.a = texColor.a;
  //  gl_FragColor = vec4(t, t, t, 1.0);
}