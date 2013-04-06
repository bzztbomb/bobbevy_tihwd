#define NUM_SAMPLES 25
#define weight 0.39
#define decay 0.751
#define density 0.50

uniform sampler2D tex;
//uniform float density;
uniform vec2 ssLightPos;

void main()
{	    
    vec2 deltaTexCoord = gl_TexCoord[0].xy - ssLightPos;
    float ns = float(NUM_SAMPLES);
    deltaTexCoord *= 1.0 / ns * density;
    vec2 texCoord = gl_TexCoord[0].xy;
    vec4 color = texture2D(tex, texCoord);
    float illuminationDecay = 1.0;
    for (int i = 0; i < NUM_SAMPLES; i++)
    {
    	texCoord -= deltaTexCoord;
    	vec4 sample = texture2D(tex, texCoord);
    	sample *= illuminationDecay * weight;
    	color += sample;
    	illuminationDecay *= decay;
    }    	       
    color *= 0.40;
    gl_FragColor = color;
}