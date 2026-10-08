//TEST:SIMPLE: -no-codegen
//TEST:SIMPLE(filecheck=CHECK): -no-codegen -dump-ir

// A property can satisfy an interface requirement after an implicit conversion changes its result
// type. `trySynthesizePropertyRequirementWitness` asks `synthesizeAccessorRequirements` to build a
// private wrapper getter whose body returns the satisfying property and coerces that expression to
// the required result type. This test verifies that the selected initializer's parameter mode
// determines how the wrapper getter accesses the original property.

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
// initializer above. Because the initializer borrows its argument through `__constref`, the
// wrapper must access the original property through `ref` instead of rechecking the storage
// expression as a value read through the mutating `get` accessor.
struct ConvertedWitnessProperty : IConvertedWitnessProperty
{
    WitnessConversionSource storedValue;

    property WitnessConversionSource value
    {
        [mutating]
        get { return storedValue; }

        [nonmutating]
        ref { return storedValue; }
    }
}

// CHECK-LABEL: [nameHint("ConvertedWitnessProperty")]
// CHECK-LABEL: [nameHint("ConvertedWitnessProperty.value.ref")]
// CHECK: func [[WITNESS_REF:%[A-Za-z0-9_]+]] :
// CHECK: func {{%[A-Za-z0-9_]+}} : Func(%WitnessConversionResult, %ConvertedWitnessProperty)
// CHECK: call [[WITNESS_REF]]
// CHECK-LABEL: [nameHint("ConvertedWitnessProperty.value.get")]
// CHECK: func [[WITNESS_GET:%[A-Za-z0-9_]+]] :
