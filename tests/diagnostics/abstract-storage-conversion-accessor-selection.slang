//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

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
    // its mutating getter instead of borrowing from the non-mutating `ref` accessor.
    ConvertedOptionalAggregate result = {holder.value};
//CHECK:                                        ^ mutating storage accessor cannot be called on an immutable value
//CHECK:                                        ^ mutating 'get' accessor cannot be called on an immutable value
}
