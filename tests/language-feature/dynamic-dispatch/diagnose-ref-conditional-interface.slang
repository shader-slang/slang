//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl

// Test that __ref and __constref parameters of `Conditional` types with
// interface payloads produce diagnostic errors in dynamic dispatch contexts,
// matching struct-with-interface-field and `__ref Optional<IFoo>` parameters.

[anyValueSize(16)]
interface IFoo
{
    float getValue();
}

struct ImplA : IFoo
{
    float v;
    float getValue() { return v; }
}

struct ImplB : IFoo
{
    float v;
    float getValue() { return v * 2; }
}

// Scenario 1: enabled Conditional with interface payload passed by __ref
float processEnabledRef(__ref Conditional<IFoo, true> c)
//CHECK:                                              ^ ref parameter incompatible with dynamic dispatch
//CHECK:                                              ^ '__ref' parameter of type 'Conditional<IFoo, true>' cannot be used in a dynamic dispatch context.
{
    if (let member = c.get())
        return member.getValue();
    return 0.0;
}

// Scenario 2: enabled Conditional with interface payload passed by __constref
float processEnabledConstref(__constref Conditional<IFoo, true> c)
//CHECK:                                                        ^ ref parameter incompatible with dynamic dispatch
//CHECK:                                                        ^ '__constref' parameter of type 'Conditional<IFoo, true>' cannot be used in a dynamic dispatch context.
{
    if (let member = c.get())
        return member.getValue();
    return 0.0;
}

// Scenario 3: disabled Conditional lowers to the empty struct, so __ref is allowed
float processDisabledRef(__ref Conditional<IFoo, false> c)
{
    return 1.0;
}

// Scenario 4: generic flag — instantiating with a literal-true flag produces the
// same rejection (a non-literal flag counts as potentially-true).
float processGenericRef<let E : bool>(__ref Conditional<IFoo, E> c)
//CHECK:                                                         ^ ref parameter incompatible with dynamic dispatch
//CHECK:                                                         ^ '__ref' parameter of type 'Conditional<IFoo, true>' cannot be used in a dynamic dispatch context.
{
    if (let member = c.get())
        return member.getValue();
    return 0.0;
}

// Link-time-constant flags (extern const) exercise the IntLit spelling of the
// flag that reaches the IR-level predicate.
extern static const bool EXTERN_ENABLED = true;
extern static const bool EXTERN_DISABLED = false;

float processExternEnabledRef(__ref Conditional<IFoo, EXTERN_ENABLED> c)
//CHECK:                                                              ^ ref parameter incompatible with dynamic dispatch
//CHECK:                                                              ^ '__ref' parameter of type 'Conditional<IFoo, true>' cannot be used in a dynamic dispatch context.
{
    if (let member = c.get())
        return member.getValue();
    return 0.0;
}

float processExternDisabledRef(__ref Conditional<IFoo, EXTERN_DISABLED> c)
{
    return 2.0;
}

IFoo factory(uint id)
{
    if (id == 0)
        return ImplA(1.0);
    return ImplB(2.0);
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    Conditional<IFoo, true> enabled = Conditional<IFoo, true>(factory(dispatchThreadID.x));
    outputBuffer[0] = processEnabledRef(enabled);
    outputBuffer[1] = processEnabledConstref(enabled);

    Conditional<IFoo, false> disabled;
    outputBuffer[2] = processDisabledRef(disabled);

    outputBuffer[3] = processGenericRef<true>(enabled);

    Conditional<IFoo, EXTERN_ENABLED> externEnabled =
        Conditional<IFoo, EXTERN_ENABLED>(factory(dispatchThreadID.x));
    outputBuffer[4] = processExternEnabledRef(externEnabled);

    Conditional<IFoo, EXTERN_DISABLED> externDisabled;
    outputBuffer[5] = processExternDisabledRef(externDisabled);
}
