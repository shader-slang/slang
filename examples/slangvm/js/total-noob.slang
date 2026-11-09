// SPDX-License-Identifier: CC-BY-NC-SA-3.0
// [SIG2014] - Total Noob by dynamite: https://www.shadertoy.com/view/XdlSDs
// Adapted from https://github.com/shader-slang/slang-playground/blob/main/public/demos/circle.slang
// License: https://creativecommons.org/licenses/by-nc-sa/3.0/
// This adaptation accepts pixel-center coordinates and elapsed seconds from the compute caller,
// and uses GLSL's positive modulo instead of the playground port's absolute-value remainder.

public float4 totalNoob(float2 fragCoord, float2 resolution, float time)
{
    float2 p = (2.0 * fragCoord - resolution) / resolution.y;
    float tau = 2.0 * float.getPi();
    float angle = atan2(p.x, p.y);
    float radius = length(p) * 0.75;
    float2 uv = float2(angle / tau, radius);

    float colorPhase = (uv.x - time / 3.0) * 3.0;
    colorPhase -= 3.0 * floor(colorPhase / 3.0);
    float3 color = float3(0.25);
    if (colorPhase < 1.0)
    {
        color.r += 1.0 - colorPhase;
        color.g += colorPhase;
    }
    else if (colorPhase < 2.0)
    {
        colorPhase -= 1.0;
        color.g += 1.0 - colorPhase;
        color.b += colorPhase;
    }
    else
    {
        colorPhase -= 2.0;
        color.b += 1.0 - colorPhase;
        color.r += colorPhase;
    }

    uv = 2.0 * uv - 1.0;
    float beam = (0.7 + 0.5 * cos(uv.x * 10.0 * tau * 0.15 *
        clamp(floor(5.0 + 10.0 * cos(time)), 0.0, 10.0))) * abs(1.0 / (30.0 * uv.y));
    return float4(beam * color, 1.0);
}
