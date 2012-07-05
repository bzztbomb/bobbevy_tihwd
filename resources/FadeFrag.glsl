uniform sampler2D tex;
uniform float fadeAmount;

void main()
{
    gl_FragColor = texture2D(tex, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y)) * fadeAmount;
}