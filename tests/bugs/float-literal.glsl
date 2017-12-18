//TEST:COMPARE_GLSL:-profile glsl_fragment_450 -no-checking
#version 450

// Confirm that we output floating-point literals in
// ways that are valid for GLSL syntax, on all platforms.

#ifdef __SLANG__
__import empty;
#endif

layout(location = 0)
out float r;

void main()
{
	r = 1.0;
}
