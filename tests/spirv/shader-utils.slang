#ifndef SHADER_UTILS_SLANG
#define SHADER_UTILS_SLANG

// Helper function to calculate distance from center
float calculateDistanceFromCenter(float2 uv)
{
    float2 center = float2(0.5, 0.5);
    return length(uv - center);
}

// Additional utility function - simple smoothstep fade
float createRadialFade(float2 uv, float radius)
{
    float dist = calculateDistanceFromCenter(uv);
    return smoothstep(radius, 0.0, dist);
}

#endif // SHADER_UTILS_SLANG