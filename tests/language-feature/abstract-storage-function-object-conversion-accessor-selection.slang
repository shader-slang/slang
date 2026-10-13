//TEST:SIMPLE(filecheck=CHECK): -no-codegen -dump-ir

// Converting a function object to its synthesized static function depends only on the object's
// type. Semantic checking replaces the abstract-storage expression without scheduling property
// evaluation, so neither `get` nor `ref` may appear in the function that consumes the result.

struct FunctionObject
{
    int operator()() { return 1; }
}

static FunctionObject functionObject;

struct FunctionObjectConversionHolder
{
    property FunctionObject value
    {
        [nonmutating]
        get { return functionObject; }

        [mutating]
        ref { return functionObject; }
    }
}

func consumeFunctionObject(__constref value: functype()->int) {}

void convertFunctionObjectWithoutReadingProperty()
{
    let holder = FunctionObjectConversionHolder();
    consumeFunctionObject(holder.value);
}

// CHECK-LABEL: [nameHint("FunctionObjectConversionHolder.value.get")]
// CHECK: func [[FUNCTION_OBJECT_GET:%[A-Za-z0-9_]+]] :
// CHECK-LABEL: [nameHint("FunctionObjectConversionHolder.value.ref")]
// CHECK: func [[FUNCTION_OBJECT_REF:%[A-Za-z0-9_]+]] :
// CHECK-LABEL: [nameHint("consumeFunctionObject")]
// CHECK: func [[CONSUME_FUNCTION_OBJECT:%[A-Za-z0-9_]+]] :
// CHECK-LABEL: [nameHint("convertFunctionObjectWithoutReadingProperty")]
// CHECK-NOT: call [[FUNCTION_OBJECT_GET]]
// CHECK-NOT: call [[FUNCTION_OBJECT_REF]]
// CHECK: call [[CONSUME_FUNCTION_OBJECT]]
// CHECK-NOT: call [[FUNCTION_OBJECT_GET]]
// CHECK-NOT: call [[FUNCTION_OBJECT_REF]]
// CHECK: return_val
