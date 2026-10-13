//TEST:SIMPLE(filecheck=CHECK): -no-codegen -dump-ir

// A conversion from an abstract-storage expression may read the property's value through `get`
// or borrow its storage through `ref`. These cases exercise conversions that consume a value
// without writing a converted value back to the property. Each property gives `get` and `ref`
// opposite receiver modes so that semantic checking and IR expose which accessor was selected.

[__NonCopyableType]
struct ConversionPayload
{
    int value;
}

static ConversionPayload externalPayload = {0};

struct ConversionSource
{
    property ConversionPayload item
    {
        [nonmutating]
        get
        {
            ConversionPayload result;
            result.value = 1;
            return result;
        }

        [mutating]
        ref { return externalPayload; }
    }
}

struct ConversionResult
{
    int value;

    __implicit_conversion(100)
    __init(ConversionPayload payload)
    {
        value = payload.value;
    }
}

// A plain parameter of the noncopyable `ConversionPayload` type borrows its argument. For a
// whole-property borrow, semantic checking selects `get` rather than switching to the distinct
// storage exposed by `ref`. Lowering materializes the getter result through its implicit output
// parameter in IR, so these cases also exercise that accessor ABI.

ConversionResult convertAbstractStorage()
{
    let source = ConversionSource();
    return source.item;
}

void consumeConversion(ConversionResult value) {}

void convertAbstractStorageAsArgument()
{
    let source = ConversionSource();
    consumeConversion(source.item);
}

ConversionResult convertAbstractStorageByAssignment()
{
    let source = ConversionSource();
    ConversionResult result;
    result = source.item;
    return result;
}

ConversionResult convertAbstractStorageExplicitly()
{
    let source = ConversionSource();
    return ConversionResult(source.item);
}

// CHECK-LABEL: [nameHint("ConversionSource.item.get")]
// CHECK: func [[GET:%[A-Za-z0-9_]+]] :
// CHECK-LABEL: [nameHint("ConversionSource.item.ref")]
// CHECK: func [[REF:%[A-Za-z0-9_]+]] :
// CHECK-LABEL: [nameHint("convertAbstractStorage")]
// CHECK-NOT: call [[REF]]
// CHECK: call [[GET]]
// CHECK-NOT: call [[REF]]
// CHECK-LABEL: [nameHint("convertAbstractStorageAsArgument")]
// CHECK-NOT: call [[REF]]
// CHECK: call [[GET]]
// CHECK-NOT: call [[REF]]
// CHECK-LABEL: [nameHint("convertAbstractStorageByAssignment")]
// CHECK-NOT: call [[REF]]
// CHECK: call [[GET]]
// CHECK-NOT: call [[REF]]
// CHECK-LABEL: [nameHint("convertAbstractStorageExplicitly")]
// CHECK-NOT: call [[REF]]
// CHECK: call [[GET]]
// CHECK-NOT: call [[REF]]

struct ParameterGroupPayload
{
    int value;
}

uniform ConstantBuffer<ParameterGroupPayload> parameterGroupPayload;

struct Holder
{
    property ConstantBuffer<ParameterGroupPayload> constantBufferProperty
    {
        [nonmutating]
        get { return parameterGroupPayload; }

        [mutating]
        ref { return parameterGroupPayload; }
    }
}

struct Target
{
    int value;

    __init(__constref ParameterGroupPayload payload)
    {
        value = payload.value;
    }
}

Target convertParameterGroupPropertyExplicitly()
{
    let holder = Holder();

    // Conversion checking implicitly dereferences the constant buffer returned by the property,
    // so lowering reads the property through `get`. Rechecking the original property as the
    // initializer's `__constref` argument would incorrectly select the mutating `ref` accessor.
    return Target(holder.constantBufferProperty);
}

// CHECK-LABEL: [nameHint("Holder.constantBufferProperty.get")]
// CHECK: func [[PARAMETER_GROUP_GET:%[A-Za-z0-9_]+]] :
// CHECK-LABEL: [nameHint("Holder.constantBufferProperty.ref")]
// CHECK: func [[PARAMETER_GROUP_REF:%[A-Za-z0-9_]+]] :
// CHECK-LABEL: [nameHint("convertParameterGroupPropertyExplicitly")]
// CHECK-NOT: call [[PARAMETER_GROUP_REF]]
// CHECK: call [[PARAMETER_GROUP_GET]]
// CHECK-NOT: call [[PARAMETER_GROUP_REF]]

static Optional<int> optionalValue;

struct OptionalConversionHolder
{
    property Optional<int> value
    {
        [nonmutating]
        get { return optionalValue; }

        [mutating]
        ref { return optionalValue; }
    }
}

void consumeConvertedOptional(__constref Optional<float> value) {}

void convertOptionalProperty()
{
    let holder = OptionalConversionHolder();

    // Converting `Optional<int>` to `Optional<float>` requires a new optional value. Semantic
    // checking therefore selects `get`, and lowering materializes the converted value before
    // passing it to the `__constref` parameter.
    consumeConvertedOptional(holder.value);
}

// CHECK-LABEL: [nameHint("OptionalConversionHolder.value.get")]
// CHECK: func [[OPTIONAL_GET:%[A-Za-z0-9_]+]] :
// CHECK-LABEL: [nameHint("OptionalConversionHolder.value.ref")]
// CHECK: func [[OPTIONAL_REF:%[A-Za-z0-9_]+]] :
// CHECK-LABEL: [nameHint("convertOptionalProperty")]
// CHECK-NOT: call [[OPTIONAL_REF]]
// CHECK: call [[OPTIONAL_GET]]
// CHECK-NOT: call [[OPTIONAL_REF]]

enum EnumPayload
{
    Zero,
}

static EnumPayload enumValue = EnumPayload.Zero;

struct EnumConversionHolder
{
    property EnumPayload value
    {
        [nonmutating]
        get { return enumValue; }

        [mutating]
        ref { return enumValue; }
    }
}

void consumeEnumTag(__constref int value) {}

void convertEnumPropertyToTag()
{
    let holder = EnumConversionHolder();

    // Converting an enum to its integer tag requires a value. Semantic checking therefore selects
    // `get`; it must not select the property's `ref` accessor merely because the parameter uses
    // `__constref` mode.
    consumeEnumTag(holder.value);
}

// CHECK-LABEL: [nameHint("EnumConversionHolder.value.get")]
// CHECK: func [[ENUM_GET:%[A-Za-z0-9_]+]] :
// CHECK-LABEL: [nameHint("EnumConversionHolder.value.ref")]
// CHECK: func [[ENUM_REF:%[A-Za-z0-9_]+]] :
// CHECK-LABEL: [nameHint("convertEnumPropertyToTag")]
// CHECK-NOT: call [[ENUM_REF]]
// CHECK: call [[ENUM_GET]]
// CHECK-NOT: call [[ENUM_REF]]

static int2 externalBuiltinValue = int2(0);

struct BuiltinConversionSource
{
    property int2 item
    {
        [mutating]
        get { return int2(1); }

        [nonmutating]
        ref { return externalBuiltinValue; }
    }
}

extension float2
{
    __implicit_conversion(0)
    __init(inout int2 value)
    {
        this = float2(42.0);
    }
}

float2 convertBuiltinArithmeticOperand()
{
    let source = BuiltinConversionSource();

    // The selected `float2` initializer takes the `int2` operand through `inout`, so semantic
    // checking selects the address supplied by `ref`. Checking the enclosing addition must not
    // replace that address with a value read through `get`.
    return source.item + float2(1.0);
}

// CHECK-LABEL: [nameHint("BuiltinConversionSource.item.get")]
// CHECK: func [[BUILTIN_GET:%[A-Za-z0-9_]+]] :
// CHECK-LABEL: [nameHint("BuiltinConversionSource.item.ref")]
// CHECK: func [[BUILTIN_REF:%[A-Za-z0-9_]+]] :
// CHECK-LABEL: [nameHint("convertBuiltinArithmeticOperand")]
// CHECK-NOT: call [[BUILTIN_GET]]
// CHECK: call [[BUILTIN_REF]]
// CHECK-NOT: call [[BUILTIN_GET]]
