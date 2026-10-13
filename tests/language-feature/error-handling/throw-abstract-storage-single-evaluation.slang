//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj
//TEST:SIMPLE(filecheck=IR): -no-codegen -dump-ir

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

// A `throw` consumes its expression once whether a matching `catch` is local or the error escapes
// to the caller. The getter count detects a second materialization of the same deferred property;
// the setter count confirms that the complete-value read does not invent a write-back.

struct ErrorStorage
{
    uint storedError;
    int getterCount;
    int setterCount;

    property uint error
    {
        [mutating]
        get
        {
            getterCount++;
            return storedError;
        }

        [mutating]
        set
        {
            setterCount++;
            storedError = newValue;
        }
    }
}

int throwToLocalCatch(inout ErrorStorage storage)
{
    do
    {
        throw storage.error;
    }
    catch (error : uint)
    {
        return 7;
    }
}

void throwToCaller(inout ErrorStorage storage) throws uint
{
    throw storage.error;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    ErrorStorage localStorage = {7, 0, 0};
    outputBuffer[0] = throwToLocalCatch(localStorage);
    outputBuffer[1] = localStorage.getterCount;
    outputBuffer[2] = localStorage.setterCount;

    ErrorStorage callerStorage = {9, 0, 0};
    do
    {
        try throwToCaller(callerStorage);
    }
    catch (error : uint)
    {
        outputBuffer[3] = 9;
    }
    outputBuffer[4] = callerStorage.getterCount;
    outputBuffer[5] = callerStorage.setterCount;
}

// CHECK: 7
// CHECK-NEXT: 1
// CHECK-NEXT: 0
// CHECK-NEXT: 9
// CHECK-NEXT: 1
// CHECK-NEXT: 0

// IR-LABEL: [nameHint("ErrorStorage.error.get")]
// IR: func [[GET_ERROR:%[A-Za-z0-9_]+]] :
// IR-LABEL: [nameHint("throwToLocalCatch")]
// IR: call [[GET_ERROR]]
// IR-NOT: call [[GET_ERROR]]
// IR-NOT: call {{.*ErrorStoragex5Ferrorx5Fset}}
// IR-LABEL: [nameHint("throwToCaller")]
// IR: call [[GET_ERROR]]
// IR-NOT: call [[GET_ERROR]]
// IR-NOT: call {{.*ErrorStoragex5Ferrorx5Fset}}
// IR: return_val
