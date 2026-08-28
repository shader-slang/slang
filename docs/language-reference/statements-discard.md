# Discard Statement

## Syntax

`discard` statement:

> **`'discard'`** **`';'`**

## Description

A `discard` statement disables the fragment shader thread by discarding its output and suppressing any
subsequent writes. It is [implementation-defined behavior](basics-behavior.md) whether the execution of a
disabled thread terminates or whether it continues with side effects suppressed.

Writes to buffers and textures performed before execution of the `discard` statement take effect.

A `discard` statement is valid only on the [fragment stage](shaders-and-kernels.md). It is an error to use it
on any other stage.

A `discard` statement does not trigger the execution of [deferred statements](statements-defer.md) before the
thread is disabled. Therefore, pending deferred statements have no effect.

See also [Program Execution](basics-program-execution.md) for the definition of inactive and helper threads.

## Examples

Alpha cutout, and the effect of a `discard` statement on writes:

```hlsl
Texture2D<float4> albedo;
SamplerState albedoSampler;

RWStructuredBuffer<uint> stats;

[shader("fragment")]
float4 fragMain(float2 uv : TEXCOORD) : SV_Target
{
    float4 color = albedo.Sample(albedoSampler, uv);

    if (color.a < 0.5f)
    {
        // This write is performed before the discard-statement,
        // so it takes effect.
        stats[0] = 1U;

        discard;

        // This write is suppressed, because it would be performed
        // after the discard-statement.
        stats[1] = 1U;
    }

    // The fragment output is discarded for the fragments that
    // executed the discard-statement.
    return color;
}
```
