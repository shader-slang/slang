//TEST:SIMPLE(filecheck=CHECK): -no-codegen -dump-ir
//TEST:SIMPLE(filecheck=OUTPUT_ONLY): -no-codegen -dump-ir
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=RUNTIME): -cpu -output-using-type

// Reversible l-value conversions can read a converted temporary, write it back, or do both.
// Accessor selection must follow those actual operations. Each section below gives the conversion
// a different abstract-storage path and checks the accessors required by that path.

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

    // For an `out` l-value conversion, lowering leaves the temporary uninitialized and writes only
    // the converted result back through `set`. It must not invent a read from this setter-only
    // property.
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

    // Lowering wraps the reversible int-to-uint cast in a temporary around the two-component
    // swizzle, then writes the converted result back through the same partially addressed value.
    // The property's `ref` accessor remains the source of that address; decomposing the swizzle
    // into a getter read and setter write would select two mutating accessors on the immutable
    // receiver.
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

    // Semantic checking records the reversible cast and writable matrix-swizzle projection.
    // Lowering derives the `ref` plan, obtains its address, and uses it for the temporary's initial
    // read and final writeback instead of restarting from the property expression.
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

    // Semantic checking records the constant component projection. Lowering derives the `ref` plan
    // and keeps the returned address instead of using a whole-property getter/setter pair around
    // the reversible scalar cast.
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

// A setter-only subscript beneath an output conversion needs no old value. Lowering must still
// evaluate its receiver and index before the call, then invoke the setter with the converted
// result.

static int outputOnlyEvaluationTrace;
static int outputOnlyStoredValue;

void appendOutputOnlyTrace(int digit)
{
    outputOnlyEvaluationTrace = outputOnlyEvaluationTrace * 10 + digit;
}

struct SetterOnlySubscriptStorage
{
    __subscript(int index) -> int
    {
        [nonmutating]
        set
        {
            appendOutputOnlyTrace(3);
            outputOnlyStoredValue = newValue + index;
        }
    }
}

SetterOnlySubscriptStorage evaluateOutputOnlyReceiver()
{
    appendOutputOnlyTrace(1);
    return {};
}

int evaluateOutputOnlyIndex()
{
    appendOutputOnlyTrace(2);
    return 2;
}

void initializeTrackedConvertedValue(out uint value)
{
    appendOutputOnlyTrace(4);
    value = 20;
}

void convertSetterOnlySubscriptWithoutReading()
{
    outputOnlyEvaluationTrace = 0;
    outputOnlyStoredValue = 0;
    initializeTrackedConvertedValue(evaluateOutputOnlyReceiver()[evaluateOutputOnlyIndex()]);
}

// An assignment expression denotes its destination after completing the inner write. A projected
// `out` conversion must read that newly written whole value once, update its field, and write the
// complete value back through the same stabilized existential receiver.
struct AssignmentProjectionValue
{
    int field;
    int sibling;
}

interface IAssignmentProjectionStorage
{
    property AssignmentProjectionValue item
    {
        [mutating] get;
        [mutating] set;
    }

    [nonmutating] AssignmentProjectionValue getStoredValue();
    [nonmutating] int getGetterCount();
    [nonmutating] int getSetterCount();
}

interface IDerivedAssignmentProjectionStorage : IAssignmentProjectionStorage
{}

struct AssignmentProjectionStorage : IDerivedAssignmentProjectionStorage
{
    AssignmentProjectionValue storedValue;
    int getterCount;
    int setterCount;

    property AssignmentProjectionValue item
    {
        [mutating]
        get
        {
            getterCount++;
            return storedValue;
        }

        [mutating]
        set
        {
            setterCount++;
            storedValue = newValue;
        }
    }

    [nonmutating] AssignmentProjectionValue getStoredValue() { return storedValue; }
    [nonmutating] int getGetterCount() { return getterCount; }
    [nonmutating] int getSetterCount() { return setterCount; }
}

void writeConvertedUint(out uint value)
{
    value = 13;
}

void convertProjectedAssignmentResult(inout IDerivedAssignmentProjectionStorage storage)
{
    writeConvertedUint((storage.item = AssignmentProjectionValue(1, 2)).field);
}

// A whole-value `out` conversion needs no getter. The first setter commits the inner assignment;
// after the call, the second setter commits the converted result.
interface ISetterOnlyAssignmentStorage
{
    property int item
    {
        [mutating] set;
    }

    [nonmutating] int getStoredValue();
    [nonmutating] int getSetterCount();
}

interface IDerivedSetterOnlyAssignmentStorage : ISetterOnlyAssignmentStorage
{}

struct SetterOnlyAssignmentStorage : IDerivedSetterOnlyAssignmentStorage
{
    int storedValue;
    int setterCount;

    property int item
    {
        [mutating]
        set
        {
            setterCount++;
            storedValue = newValue;
        }
    }

    [nonmutating] int getStoredValue() { return storedValue; }
    [nonmutating] int getSetterCount() { return setterCount; }
}

void convertWholeSetterOnlyAssignmentResult(inout IDerivedSetterOnlyAssignmentStorage storage)
{
    writeConvertedUint(storage.item = 7);
}

// Slots 0-1: setter-only subscript [evaluation trace, stored value].
// Slots 2-5: projected assignment result [field, sibling, getter count, setter count].
// Slots 6-7: whole setter-only assignment result [stored value, setter count].
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

static const uint kSetterOnlySubscriptBase = 0;
static const uint kProjectedAssignmentResultBase = 2;
static const uint kWholeAssignmentResultBase = 6;

[numthreads(1, 1, 1)]
void computeMain()
{
    convertSetterOnlySubscriptWithoutReading();
    outputBuffer[kSetterOnlySubscriptBase + 0] = outputOnlyEvaluationTrace;
    outputBuffer[kSetterOnlySubscriptBase + 1] = outputOnlyStoredValue;

    IDerivedAssignmentProjectionStorage projectionStorage =
        AssignmentProjectionStorage(AssignmentProjectionValue(0, 0), 0, 0);
    convertProjectedAssignmentResult(projectionStorage);
    outputBuffer[kProjectedAssignmentResultBase + 0] = projectionStorage.getStoredValue().field;
    outputBuffer[kProjectedAssignmentResultBase + 1] = projectionStorage.getStoredValue().sibling;
    outputBuffer[kProjectedAssignmentResultBase + 2] = projectionStorage.getGetterCount();
    outputBuffer[kProjectedAssignmentResultBase + 3] = projectionStorage.getSetterCount();

    IDerivedSetterOnlyAssignmentStorage setterOnlyStorage = SetterOnlyAssignmentStorage(0, 0);
    convertWholeSetterOnlyAssignmentResult(setterOnlyStorage);
    outputBuffer[kWholeAssignmentResultBase + 0] = setterOnlyStorage.getStoredValue();
    outputBuffer[kWholeAssignmentResultBase + 1] = setterOnlyStorage.getSetterCount();
}

// The receiver and index run before the callee (`1`, `2`, `4`), and the setter runs last (`3`).
// RUNTIME: type: int32_t
// RUNTIME-NEXT: 1243
// RUNTIME-NEXT: 22
// The projected conversion preserves the sibling and reads/writes the property exactly once.
// RUNTIME-NEXT: 13
// RUNTIME-NEXT: 2
// RUNTIME-NEXT: 1
// RUNTIME-NEXT: 2
// The whole-value conversion performs the inner and outer setter calls without a getter.
// RUNTIME-NEXT: 13
// RUNTIME-NEXT: 2

// OUTPUT_ONLY-LABEL: [nameHint("evaluateOutputOnlyReceiver")]
// OUTPUT_ONLY: func [[EVALUATE_OUTPUT_RECEIVER:%[A-Za-z0-9_]+]] :
// OUTPUT_ONLY-LABEL: [nameHint("evaluateOutputOnlyIndex")]
// OUTPUT_ONLY: func [[EVALUATE_OUTPUT_INDEX:%[A-Za-z0-9_]+]] :
// OUTPUT_ONLY-LABEL: [nameHint("initializeTrackedConvertedValue")]
// OUTPUT_ONLY: func [[INITIALIZE_TRACKED_CONVERSION:%[A-Za-z0-9_]+]] :
// OUTPUT_ONLY-LABEL: [nameHint("SetterOnlySubscriptStorage.operator[].set")]
// OUTPUT_ONLY: func [[SETTER_ONLY_SUBSCRIPT:%[A-Za-z0-9_]+]] :
// OUTPUT_ONLY-LABEL: [nameHint("convertSetterOnlySubscriptWithoutReading")]
// OUTPUT_ONLY: call [[EVALUATE_OUTPUT_RECEIVER]]
// OUTPUT_ONLY: call [[EVALUATE_OUTPUT_INDEX]]
// OUTPUT_ONLY: call [[INITIALIZE_TRACKED_CONVERSION]]
// OUTPUT_ONLY: call [[SETTER_ONLY_SUBSCRIPT]]
