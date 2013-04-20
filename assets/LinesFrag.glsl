uniform vec3 iResolution;
uniform vec4 iMouse;
uniform float iGlobalTime;

const float rays = 64.0;
const float period = 6.28 / (rays / 2.0);
const float half_period = period / 1.3;
const float inner_radius = 0.004;
	
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
	a += 0.61 * sin(r+(iGlobalTime*0.5)+m);
	a = fmod(a, period); // / period;		
	a = a > half_period ? 0.0 : 1.0;
	
	return r < inner_radius ? 0.0 : a;
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