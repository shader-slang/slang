#version 450 core
//TEST:COMPARE_GLSL:
//TEST:COMPARE_GLSL:-DBINDING

#if defined(__SLANG__) && defined(BINDING)
#define LAYOUT(X) /* empty */
#else
#define LAYOUT(X) layout(X)
#endif

LAYOUT(location = 0) in vec2 inPos;
LAYOUT(location = 1) in vec2 inUV;

LAYOUT(location = 0) out vec2 outUV;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

void main(void)
{
	gl_Position = vec4(inPos, 0.0, 1.0);
	outUV = inUV;
}
