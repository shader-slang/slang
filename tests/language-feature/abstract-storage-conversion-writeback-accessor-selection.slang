//TEST:SIMPLE: -no-codegen
//TEST:SIMPLE(filecheck=CHECK): -no-codegen -dump-ir

// Reversible l-value conversions can read a converted temporary, write it back, or do both.
// Accessor selection must follow those actual operations. The `out` case below performs only a
// setter writeback. For the projection cases, semantic checking preserves an address from `ref`
// through the conversion, so the read and writeback do not restart from the property expression.

struct OutConversionHolder
{
    property int value
    {
        [nonmutating]
        set {}
    }
}

void initializeConvertedValue(out uint value)
{
    value = 1;
}

void convertOutArgumentWithoutReadingProperty()
{
    OutConversionHolder holder = {};

    // An `out` l-value conversion leaves its temporary uninitialized and only writes the converted
    // result back through `set`. It must not invent a read from this setter-only property.
    initializeConvertedValue(holder.value);
}

// CHECK-LABEL: [nameHint("OutConversionHolder.value.set")]
// CHECK: func [[OUT_CONVERSION_SET:%[A-Za-z0-9_]+]] :
// CHECK-LABEL: [nameHint("convertOutArgumentWithoutReadingProperty")]
// CHECK: call [[OUT_CONVERSION_SET]]

static int4 externalWritableCastValue = int4(0);

struct WritableSwizzleConversionSource
{
    property int4 item
    {
        [mutating]
        get { return int4(1); }

        [mutating]
        set { externalWritableCastValue = newValue; }

        [nonmutating]
        ref { return externalWritableCastValue; }
    }
}

void mutateWritableSwizzle(inout uint2 value)
{
    value += uint2(1);
}

void convertWritableSwizzle()
{
    let source = WritableSwizzleConversionSource();

    // The reversible int-to-uint cast wraps a temporary around the two-component swizzle, then
    // writes the converted result back through the same partially addressed value. The property's
    // `ref` accessor remains the source of that address; decomposing the swizzle into a getter read
    // and setter write would select two mutating accessors on the immutable receiver.
    mutateWritableSwizzle(source.item.xy);
}

// CHECK-LABEL: [nameHint("WritableSwizzleConversionSource.item.get")]
// CHECK: func [[WRITABLE_SWIZZLE_GET:%[A-Za-z0-9_]+]] :
// CHECK-LABEL: [nameHint("WritableSwizzleConversionSource.item.set")]
// CHECK: func [[WRITABLE_SWIZZLE_SET:%[A-Za-z0-9_]+]] :
// CHECK-LABEL: [nameHint("WritableSwizzleConversionSource.item.ref")]
// CHECK: func [[WRITABLE_SWIZZLE_REF:%[A-Za-z0-9_]+]] :
// CHECK-LABEL: [nameHint("convertWritableSwizzle")]
// CHECK-NOT: call [[WRITABLE_SWIZZLE_GET]]
// CHECK-NOT: call [[WRITABLE_SWIZZLE_SET]]
// CHECK-NOT: call [[WRITABLE_SWIZZLE_REF]]
// CHECK: let [[WRITABLE_SWIZZLE_ADDRESS:%[A-Za-z0-9_]+]] : Ptr({{.*}}) =
// CHECK-SAME: call [[WRITABLE_SWIZZLE_REF]]
// CHECK-NOT: call [[WRITABLE_SWIZZLE_GET]]
// CHECK-NOT: call [[WRITABLE_SWIZZLE_SET]]
// CHECK-NOT: call [[WRITABLE_SWIZZLE_REF]]
// CHECK: swizzledStore([[WRITABLE_SWIZZLE_ADDRESS]], {{.*}}, 0 : Int, 1 : Int)
// CHECK-NOT: call [[WRITABLE_SWIZZLE_GET]]
// CHECK-NOT: call [[WRITABLE_SWIZZLE_SET]]
// CHECK-NOT: call [[WRITABLE_SWIZZLE_REF]]

static int2x2 externalWritableMatrixCastValue = int2x2(0, 0, 0, 0);

struct WritableMatrixSwizzleConversionSource
{
    property int2x2 item
    {
        [mutating]
        get { return int2x2(1, 1, 1, 1); }

        [mutating]
        set { externalWritableMatrixCastValue = newValue; }

        [nonmutating]
        ref { return externalWritableMatrixCastValue; }
    }
}

void convertWritableMatrixSwizzle()
{
    let source = WritableMatrixSwizzleConversionSource();

    // Semantic checking also wraps this writable matrix swizzle in a temporary for the reversible
    // cast, but retains the address returned by `ref` as the base of the partial l-value. Both the
    // initial read and final writeback use that representation instead of restarting from the
    // property expression.
    mutateWritableSwizzle(source.item._m00_m11);
}

// CHECK-LABEL: [nameHint("WritableMatrixSwizzleConversionSource.item.get")]
// CHECK: func [[WRITABLE_MATRIX_SWIZZLE_GET:%[A-Za-z0-9_]+]] :
// CHECK-LABEL: [nameHint("WritableMatrixSwizzleConversionSource.item.set")]
// CHECK: func [[WRITABLE_MATRIX_SWIZZLE_SET:%[A-Za-z0-9_]+]] :
// CHECK-LABEL: [nameHint("WritableMatrixSwizzleConversionSource.item.ref")]
// CHECK: func [[WRITABLE_MATRIX_SWIZZLE_REF:%[A-Za-z0-9_]+]] :
// CHECK-LABEL: [nameHint("convertWritableMatrixSwizzle")]
// CHECK-NOT: call [[WRITABLE_MATRIX_SWIZZLE_GET]]
// CHECK-NOT: call [[WRITABLE_MATRIX_SWIZZLE_SET]]
// CHECK-NOT: call [[WRITABLE_MATRIX_SWIZZLE_REF]]
// CHECK: let [[WRITABLE_MATRIX_SWIZZLE_ADDRESS:%[A-Za-z0-9_]+]] : Ptr({{.*}}) =
// CHECK-SAME: call [[WRITABLE_MATRIX_SWIZZLE_REF]]
// CHECK-NOT: call [[WRITABLE_MATRIX_SWIZZLE_GET]]
// CHECK-NOT: call [[WRITABLE_MATRIX_SWIZZLE_SET]]
// CHECK-NOT: call [[WRITABLE_MATRIX_SWIZZLE_REF]]
// CHECK: matrixSwizzleStore([[WRITABLE_MATRIX_SWIZZLE_ADDRESS]], {{%[A-Za-z0-9_]+}},
// CHECK-SAME: 0 : Int, 0 : Int, 1 : Int, 1 : Int)
// CHECK-NOT: call [[WRITABLE_MATRIX_SWIZZLE_GET]]
// CHECK-NOT: call [[WRITABLE_MATRIX_SWIZZLE_SET]]
// CHECK-NOT: call [[WRITABLE_MATRIX_SWIZZLE_REF]]

static int4 externalFoldedSwizzleCastValue = int4(0);

struct FoldedSwizzleConversionSource
{
    property int4 item
    {
        [mutating]
        get { return int4(1); }

        [mutating]
        set { externalFoldedSwizzleCastValue = newValue; }

        [nonmutating]
        ref { return externalFoldedSwizzleCastValue; }
    }
}

void mutateWritableScalar(inout uint value)
{
    value += 1;
}

void convertFoldedWritableSwizzle()
{
    let source = FoldedSwizzleConversionSource();

    // The constant index lets lowering preserve the selected component in its swizzled-l-value
    // representation. Semantic checking therefore keeps the address from `ref`, without falling
    // back to a whole-property getter/setter pair around the reversible scalar cast.
    mutateWritableScalar(source.item.xy[0]);
}

// CHECK-LABEL: [nameHint("FoldedSwizzleConversionSource.item.get")]
// CHECK: func [[FOLDED_SWIZZLE_GET:%[A-Za-z0-9_]+]] :
// CHECK-LABEL: [nameHint("FoldedSwizzleConversionSource.item.set")]
// CHECK: func [[FOLDED_SWIZZLE_SET:%[A-Za-z0-9_]+]] :
// CHECK-LABEL: [nameHint("FoldedSwizzleConversionSource.item.ref")]
// CHECK: func [[FOLDED_SWIZZLE_REF:%[A-Za-z0-9_]+]] :
// CHECK-LABEL: [nameHint("convertFoldedWritableSwizzle")]
// CHECK-NOT: call [[FOLDED_SWIZZLE_GET]]
// CHECK-NOT: call [[FOLDED_SWIZZLE_SET]]
// CHECK: call [[FOLDED_SWIZZLE_REF]]
// CHECK-NOT: call [[FOLDED_SWIZZLE_GET]]
// CHECK-NOT: call [[FOLDED_SWIZZLE_SET]]
