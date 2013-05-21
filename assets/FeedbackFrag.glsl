uniform vec3 iResolution;
uniform float iGlobalTime;
uniform float iInOut;
uniform float iNoiseAmp;
uniform sampler2D newContour;
uniform sampler2D oldFeedback;

float hash( float n )
{
    return fract(sin(n)*43758.5453123);
}

float noise( in vec3 x )
{
    vec3 p = floor(x);
    vec3 f = fract(x);

    f = f*f*(3.0-2.0*f);

    float n = p.x + p.y*57.0 + 113.0*p.z;

    float res = mix(mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
                        mix( hash(n+ 57.0), hash(n+ 58.0),f.x),f.y),
                    mix(mix( hash(n+113.0), hash(n+114.0),f.x),
                        mix( hash(n+170.0), hash(n+171.0),f.x),f.y),f.z);
    return res;
}

void main()
{
	vec2 q = gl_FragCoord.xy/iResolution.xy;
  vec2 p = -1.0+2.0*q;

  vec4 oldVal = vec4(0.0);
  vec2 tc = gl_TexCoord[0].xy;
  vec3 tc3 = vec3(tc+iGlobalTime*0.2, 1.0);
  vec2 noise_dir = normalize(vec2(noise(tc3 * 12.3), noise(tc3 * 14.3)));
  noise_dir = (-1.0+2.0*noise_dir) * iNoiseAmp;
  
  vec2 dir = iInOut * p + noise_dir;
  dir = normalize(dir);
  dir *= vec2(1.0, 1.0) / iResolution.xy;
  for (int i = 0; i < 150; i++)
  {
    oldVal += texture2D(oldFeedback, tc);
    tc += dir;
  }
  // tc = gl_TexCoord[0].xy;
  // dir = -p;
  // dir *= -vec2(1.0, 1.0) / iResolution.xy;
  // for (int i = 0; i < 75; i++)
  // {
  //   oldVal += texture2D(oldFeedback, tc);
  //   tc += dir;
  // }

  oldVal *= 1.0/150.0;
  oldVal *= vec4(0.9, 0.7, 0.5, 0.0);
  
  tc = gl_TexCoord[0].xy;
  vec4 newval = vec4(0.0);
  for (int i = 0; i < 25; i++)
  {
    newval += texture2D(newContour, tc);
    tc += dir;
  }
  newval *= 1.0/20.0;
  newval *= vec4(1.0, 1.0, 0.5, 0.0);
  
  gl_FragColor = newval + oldVal;
//  gl_FragColor = vec4(noise(vec3(gl_TexCoord[0].xy * 20.3, 1)));
}
