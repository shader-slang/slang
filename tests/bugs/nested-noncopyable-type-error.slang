//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute -DTYPE_0
//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute -DTYPE_1
//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute -DTYPE_2
//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute -DTYPE_3

// CHECK: OpEntryPoint
// CHECK-NOT: error

#ifdef TYPE_0
[__NonCopyableType]
struct NonCopyableType
{
    __init(){}
}

property data : NonCopyableType
{
    get
    {
        return NonCopyableType();
    }
}

void computeMain()
{
    NonCopyableType v = data;
}
#endif
#ifdef TYPE_1
[__NonCopyableType]
struct NonCopyableType
{
    __init(){}
}

struct Thing
{
    __init(){}
    __subscript(int a) -> NonCopyableType
    {
        get
        {
            return NonCopyableType();
        }
    }
}

void computeMain()
{
    Thing v = Thing();
    NonCopyableType nct = v[1];
}
#endif

#ifdef TYPE_2
[__NonCopyableType]
struct NonCopyableType
{
    __init() {}
}

NonCopyableType nestedReturnSomeNonCopyableType()
{
    return NonCopyableType();
}

NonCopyableType returnSomeNonCopyableType(int a)
{
    return nestedReturnSomeNonCopyableType();
}
void computeMain()
{
    NonCopyableType nct = returnSomeNonCopyableType(1);
}
#endif

#ifdef TYPE_3
[__NonCopyableType]
struct NonCopyableType
{
    __init() {}
}

struct MethodHolder
{
    int a = 0;
    
    __init() {}

    [mutating]
    NonCopyableType nestedReturnSomeNonCopyableType()
    {
        return NonCopyableType();
    }
}

void computeMain()
{
    MethodHolder m = MethodHolder();
    NonCopyableType nct = m.nestedReturnSomeNonCopyableType();
}
#endif