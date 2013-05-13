uniform vec3 iResolution;
uniform sampler2D newContour;
uniform sampler2D oldFeedback;

void main()
{
	vec2 q = gl_FragCoord.xy/iResolution.xy;
  vec2 p = -1.0+2.0*q;

  vec4 oldVal = vec4(0.0);
  vec2 tc = gl_TexCoord[0].xy;
  vec2 dir = normalize(p);
  dir *= vec2(1.0, 1.0) / iResolution.xy;
  for (int i = 0; i < 150; i++)
  {
    oldVal += texture2D(oldFeedback, tc);
    tc += dir;
  }
  oldVal *= 1.0/150.0;
  oldVal *= vec4(0.9, 0.7, 0.5, 0.0);
  vec4 newval = texture2D(newContour, gl_TexCoord[0].xy);
  newval *= vec4(1.0, 1.0, 0.5, 0.0);
  gl_FragColor = newval*0.5 + oldVal;
}
