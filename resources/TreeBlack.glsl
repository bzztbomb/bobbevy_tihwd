uniform sampler2D tex;
uniform float farClip;
uniform vec4 fogColor;

varying vec4 viewSpacePos;

void main()
{
    vec4 texColor = texture2D(tex, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y));    
    gl_FragColor = vec4(0.0, 0.0, 0.0, texColor.a);    
}