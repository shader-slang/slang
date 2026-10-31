//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -xslang -std -xslang legacy -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -xslang -std -xslang 2025 -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -xslang -std -xslang 2026 -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -xslang -std -xslang 202c -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out
RWStructuredBuffer<int> outputBuffer;

interface IBase
{
    int getValue();
}

interface IDerived : IBase
{
}

interface IGrandchild : IDerived
{
}

struct First : IGrandchild
{
    int payload;

    int getValue()
    {
        return payload;
    }
}

struct Second : IDerived
{
    int payload;

    int getValue()
    {
        return payload * 2;
    }
}

interface IGenericBase<T>
{
    T getValue();
}

interface IGenericDerived<T> : IGenericBase<T>
{
}

struct GenericValue<T> : IGenericDerived<T>
{
    T payload;

    T getValue()
    {
        return payload;
    }
}

dyn IBase pack<T : IBase>(T value)
{
    return value;
}

dyn IDerived makePure(int payload)
{
    return First(payload);
}

// The checker wraps `makeNoDiffBox`'s result type in `no_diff` because `IDerived` does not
// refine `IDifferentiable`.
[Differentiable]
dyn IDerived makeNoDiffBox(int payload)
{
    return First(payload);
}

no_diff First makeNoDiffValue(int payload)
{
    return First(payload);
}

typealias BaseContract = IBase;
typealias BaseBox = dyn BaseContract;

dyn IDerived makeField()
{
    outputBuffer[17] += 1;
    return First(71);
}

struct FieldHolder
{
    dyn IBase value = makeField();
}

struct GenericFieldHolder<T>
{
    T tag;
    dyn IBase value = makeField();
}

struct AccessorStorage
{
    int unused;

    property dyn IDerived item
    {
        get
        {
            outputBuffer[19] += 1;
            return First(37);
        }
        set(dyn IDerived value)
        {
            outputBuffer[20] += 1;
        }
    }
}

dyn IDerived makeEffectful(int kind)
{
    outputBuffer[0] += 1;
    if ((kind & 1) == 0)
    {
        return First(17);
    }
    return Second(17);
}

int firstEffect()
{
    outputBuffer[15] = outputBuffer[15] * 10 + 1;
    return 7;
}

dyn IDerived secondEffect()
{
    outputBuffer[15] = outputBuffer[15] * 10 + 2;
    return First(23);
}

int combine(int value, dyn IBase boxed)
{
    return value + boxed.getValue();
}

[numthreads(1, 1, 1)]
void computeMain()
{
    int kind = outputBuffer[14];
    outputBuffer[0] = 0;
    outputBuffer[17] = 0;

    dyn IBase concrete = First(11);
    outputBuffer[1] = concrete.getValue();
    outputBuffer[2] = pack(First(12)).getValue();

    dyn IDerived derived = First(21);
    dyn IBase directBase = derived;
    outputBuffer[3] = directBase.getValue();

    dyn IGrandchild grandchild = First(31);
    dyn IBase transitiveBase = grandchild;
    outputBuffer[4] = transitiveBase.getValue();

    dyn IGenericDerived<int> genericDerived = GenericValue<int>(41);
    dyn IGenericBase<int> genericBase = genericDerived;
    outputBuffer[5] = genericBase.getValue();

    // Each initializer opens one call result, including its witness, before repacking it.
    // The runtime input retains both possible payloads; the counter must finish at two.
    dyn IBase firstCall = makeEffectful(kind);
    dyn IBase secondCall = makeEffectful(kind + 1);
    outputBuffer[6] = firstCall.getValue();
    outputBuffer[7] = secondCall.getValue();

    dyn IBase noDiffBox = makeNoDiffBox(51);
    dyn IBase noDiffValue = makeNoDiffValue(52);
    outputBuffer[8] = noDiffBox.getValue();
    outputBuffer[9] = noDiffValue.getValue();

    BaseBox aliasedBase = makePure(22);
    outputBuffer[10] = aliasedBase.getValue();
    FieldHolder holder = FieldHolder();
    outputBuffer[11] = holder.value.getValue();

    // Replacing the upcast package must not replace the original derived box.
    directBase = First(99);
    outputBuffer[12] = derived.getValue();
    outputBuffer[13] = directBase.getValue();
    outputBuffer[14] = kind;

    // The boxed argument is implicitly opened before the other argument is evaluated.
    // Both calls must run once, leaving the trace 21 and the result 30.
    outputBuffer[15] = 0;
    outputBuffer[16] = combine(firstEffect(), secondEffect());

    // The box upcast in each `value` initializer uses an expression temporary, never another field.
    // Each constructor evaluates that declared initializer once.
    GenericFieldHolder<int> genericHolder = GenericFieldHolder<int>();
    outputBuffer[18] = genericHolder.value.getValue();

    // The upcast snapshots a property value once, even when the property also has a setter.
    outputBuffer[19] = 0;
    outputBuffer[20] = 0;
    AccessorStorage storage = AccessorStorage(0);
    dyn IBase accessorBox = storage.item;
    outputBuffer[21] = accessorBox.getValue();
}

// CHECK: type: int32_t
// CHECK-NEXT: {{^2$}}
// CHECK-NEXT: {{^11$}}
// CHECK-NEXT: {{^12$}}
// CHECK-NEXT: {{^21$}}
// CHECK-NEXT: {{^31$}}
// CHECK-NEXT: {{^41$}}
// CHECK-NEXT: {{^17$}}
// CHECK-NEXT: {{^34$}}
// CHECK-NEXT: {{^51$}}
// CHECK-NEXT: {{^52$}}
// CHECK-NEXT: {{^22$}}
// CHECK-NEXT: {{^71$}}
// CHECK-NEXT: {{^21$}}
// CHECK-NEXT: {{^99$}}
// CHECK-NEXT: {{^0$}}
// CHECK-NEXT: {{^21$}}
// CHECK-NEXT: {{^30$}}
// CHECK-NEXT: {{^2$}}
// CHECK-NEXT: {{^71$}}
// CHECK-NEXT: {{^1$}}
// CHECK-NEXT: {{^0$}}
// CHECK-NEXT: {{^37$}}
