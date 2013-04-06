uniform sampler2D tex;
uniform sampler2D texBlurred;
uniform float farClip;
uniform vec4 fogColor;

varying vec4 viewSpacePos;

// 0 -> -2.5, 1->0 -2.2
// -10->40, 0->1
void main()
{
	// Fakie DOF
	float nearbias = pow(smoothstep(-2.2, 0.0, viewSpacePos.z), 0.2);
	float farbias = pow(smoothstep(-4.0, -15.0, viewSpacePos.z), 0.2);
	float bias = nearbias + farbias;
	
	vec2 tc = vec2(gl_TexCoord[0].x, gl_TexCoord[0].y);
    vec4 texColor = texture2D(tex, tc);
    vec4 texColorBlurred = texture2D(texBlurred, tc);
    texColor = mix(texColor, texColorBlurred, bias);

    texColor *= gl_Color;
    vec4 pixelFogColor = fogColor;
    pixelFogColor.a = texColor.a;
    float distance = distance(viewSpacePos, vec4(0.0, 0.0, 0.0, 0.0));
    float density = clamp(farClip - distance, 0.0, farClip) / farClip;
    float fog = density*density;
    
    gl_FragColor = mix(fogColor, texColor, fog);
    //gl_FragColor = vec4(farbias);
    gl_FragColor.a = texColor.a;
}