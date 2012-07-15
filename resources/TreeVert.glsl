varying vec4 viewSpacePos;

void main()
{
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_FrontColor = gl_Color;
    viewSpacePos = gl_ModelViewMatrix * gl_Vertex;
}