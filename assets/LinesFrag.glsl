uniform vec3 iResolution;
uniform vec4 iMouse;
uniform float iGlobalTime;
uniform float iTimeMult;
uniform float iRays;
uniform float iPeriod;
uniform float iHalf_period;
uniform float iInner_radius;

//#define GRAY_SCALE
	
float fmod(float numerator, float denominator)
{
	return numerator - (floor(numerator / denominator) * denominator);
}

float on(vec2 origin_pt, vec2 test_pt, float m)
{
	test_pt -= origin_pt;
	test_pt.x *= iResolution.x/iResolution.y;
	
 	float a = atan(test_pt.y,test_pt.x);
    float r = sqrt(dot(test_pt,test_pt));
	a += 0.61 * sin(r+(iGlobalTime*iTimeMult)+m);
#ifndef GRAY_SCALE	
	a = fmod(a, iPeriod); // / iPeriod;		
	a = a > iHalf_period ? 0.0 : 1.0;
#else	
	a = fmod(a, iPeriod) / iPeriod;		
#endif	
	return r < iInner_radius ? 0.0 : a;
}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	uv = -1.0 + 2.0 * uv;

	vec2 mouse_uv = iMouse.xy / iResolution.xy;	
	mouse_uv = -1.0 + 2.0 * mouse_uv;

	vec2 mouse_uv2 = iMouse.zw / iResolution.xy;	
	mouse_uv2 = -1.0 + 2.0 * mouse_uv2;
	
	float c = on(mouse_uv, uv, 0.0);
	c = min(c, on(mouse_uv2, uv, -3.14));

	vec3 color = vec3(1.0, 0.0, 0.0);
	gl_FragColor = vec4(c * color, 1.0);
}