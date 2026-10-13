//TEST:SIMPLE(filecheck=CHECK): -no-codegen -dump-ir

// A property can satisfy an interface requirement after an implicit conversion changes its result
// type. `trySynthesizePropertyRequirementWitness` asks `synthesizeAccessorRequirements` to build a
// private wrapper getter whose body returns the satisfying property and coerces that expression to
// the required result type. When checking the wrapper body, semantic checking must interpret the
// selected initializer's parameter mode to choose how the wrapper getter accesses the original
// property.

struct WitnessConversionSource
{
    int value;
}

struct WitnessConversionResult
{
    int value;

    __implicit_conversion(0)
    __init(__constref WitnessConversionSource source)
    {
        value = source.value;
    }
}

interface IConvertedWitnessProperty
{
    property WitnessConversionResult value
    {
        [nonmutating]
        get;
    }
}

// The declared property has type `WitnessConversionSource`, while the interface requires
// `WitnessConversionResult`. The synthesized wrapper getter therefore calls the implicit
// initializer above. That conversion first materializes the declared property's value through
// `get`; the initializer then borrows the materialized value. It must not switch to the distinct
// storage exposed by `ref` merely because the selected initializer takes `__constref`.
struct ConvertedWitnessProperty : IConvertedWitnessProperty
{
    WitnessConversionSource storedValue;
    static WitnessConversionSource referencedValue = {};

    property WitnessConversionSource value
    {
        [nonmutating]
        get { return storedValue; }

        [mutating]
        ref { return referencedValue; }
    }
}

// CHECK-LABEL: [nameHint("ConvertedWitnessProperty")]
// CHECK-LABEL: [nameHint("ConvertedWitnessProperty.value.get")]
// CHECK: func [[WITNESS_GET:%[A-Za-z0-9_]+]] :
// CHECK: func {{%[A-Za-z0-9_]+}} : Func(%WitnessConversionResult, %ConvertedWitnessProperty)
// CHECK-NOT: call %ConvertedWitnessPropertyx5Fvaluex5Fref
// CHECK: call [[WITNESS_GET]]
// CHECK-NOT: call %ConvertedWitnessPropertyx5Fvaluex5Fref
// CHECK-LABEL: [nameHint("ConvertedWitnessProperty.value.ref")]
