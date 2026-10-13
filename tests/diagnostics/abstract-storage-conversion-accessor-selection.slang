//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

// An initializer-list element conversion consumes a property's value. Semantic checking must
// therefore validate the selected getter rather than borrow from an available `ref` accessor;
// lowering later materializes the getter result for the conversion.

static Optional<int> optionalValue;

struct OptionalConversionHolder
{
    property Optional<int> value
    {
        [mutating]
        get { return optionalValue; }

        [nonmutating]
        ref { return optionalValue; }
    }
}

struct ConvertedOptionalAggregate
{
    Optional<float> value;
}

void rejectMaterializingInitializerListConversion()
{
    let holder = OptionalConversionHolder();

    // The element conversion constructs a new Optional<float>, so it must read the property through
    // its mutating getter instead of borrowing from the nonmutating `ref` accessor.
    ConvertedOptionalAggregate result = {holder.value};
//CHECK:                                        ^^^^^ mutating method cannot be called on immutable value
//CHECK:                                        ^^^^^ mutating method 'get' cannot be called on an immutable value
}

void rejectMaterializingExplicitConversion()
{
    let holder = OptionalConversionHolder();

    // The explicit-conversion fast path in `_coerce` returns a larger expression. `_coerce` must
    // leave getter validation to the enclosing initializer's `coerceAndFinalizeForStorageUse`
    // call.
    Optional<float> result = Optional<float>(holder.value);
//CHECK:                                            ^^^^^ mutating method cannot be called on immutable value
//CHECK:                                            ^^^^^ mutating method 'get' cannot be called on an immutable value
}

