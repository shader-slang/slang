// Module containing utility functions for debug tests
module shader_utils_module;

// Helper function to calculate distance from center
public float calculateDistanceFromCenterModule(float2 uv)
{
    float2 center = float2(0.5, 0.5);
    return length(uv - center);
}

// Additional utility function - simple smoothstep fade
public float createRadialFadeModule(float2 uv, float radius)
{
    float dist = calculateDistanceFromCenterModule(uv);
    return smoothstep(radius, 0.0, dist);
}
