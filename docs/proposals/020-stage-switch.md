# SP#020: `stage_switch`

## Status

Author: Yong He

Status: In Experiment

Implementation: [PR]()

Reviewed by:

## Background

We need to provide a mechanism for authoring stage-specific code that works with the capability system. For example, the user may want to define a function `ddx_or_zero(v)` that returns `ddx(v)` when called from a fragment shader, and return `0` when called from other shader stages. Without a mechanism for writing stage-specific code, there is no way to define a valid function that can be used from both a fragment shader and a compute shader in a single compilation.

The user can workaround this problem with the preprocessor:

```
float ddx_or_zero(float v)
{
#ifdef FRAGMENT_SHADER
    return ddx(v);
#else
    return 0.0;
#endif
}

[shader("compute")]
[numthread(1,1,1)]
void computeMain() { ddx_or_zero(...); }

[shader("fragment")]
float4 fragMain() { ddx_or_zero(...); }
```

However, this require the application to compile the source file twice with different pre-defined macros. It is impossible to use a single compilation to generate one SPIRV module that contains both the entrypoints.

## Proposed Approach

We propose to add a new construct, `__stage_switch` that works like `__target_switch` but switches on stages. With `__stage_switch` the above code can be written as:

```
float ddx_or_zero(float v)
{
    __stage_switch
    {
    case fragment:
        return ddx(v);
    default:
        return 0.0;
    }
}

[shader("compute")]
[numthread(1,1,1)]
void computeMain()
{
    ddx_or_zero(...); // returns 0.0
}

[shader("fragment")]
float4 fragMain()
{
    ddx_or_zero(...); // returns ddx(...)
}
```

With `__stage_switch`, the two entrypoints can be compiled into a single SPIRV in one go, without requiring setting up any preprocessor macros.

Unlike `switch`, there is no fallthrough between cases in a `__stage_switch`. All cases will implicitly end with a `break` if it is not written by the user. However, one special type of fallthrough is supported, that is when multiple `cases` are defined next to each other with nothing else in between, for example:

```
__stage_switch
{
case fragment:
case vertex:
case geometry:
    return 1.0;
case anyhit:
    return 2.0;
default:
    return 0.0;
}
```

## Alternatives Considered

We considered to reuse the existing `__target_switch` and extend it to allow switching between different stages. However this turns out to be difficult to implement, if ordinary capabilities are mixed together with stages, because specialization to stages needs to happen at a much later time in the compilation pipeline compared to specialization to capabilities. Using a separate switch allows us to easily tell apart the code that requires specialization at different phases of compilation, and also allow us to provide cleaner error messages.

## Conclusion

`__stage_switch` adds the missing functionality from `__target_switch` that allows the user to write stage-specific code that gets specialized for each unique entrypoint stage. This works together with the capability system to provide early type-system checks to ensure the correctness of user code, without requiring use of preprocessor to protect calls to stage specific functions.
