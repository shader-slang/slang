// Which `spirv_asm` constants share one id, beyond the `OpConstant` case of
// spirv-asm-constant-memoize-decoration.slang:
// - `OpConstantFalse` and `OpConstantNull` written in two blocks share one id.
// - Two `OpConstantComposite`s share one id when their constituents do. A block that decorates
//   a constituent keeps its own constituent, so its composite is a separate one too.
// - A constant given a name with `OpName` keeps its own id, as a decorated one does.
// - A specialization constant defined in a block is never shared: each one can be overridden
//   on its own.
// - A constant written as the block's result (`result:$$T = OpConstant ...`) is shared too.

//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute -O0
//TEST:SIMPLE(filecheck=FALSE): -target spirv -entry computeMain -stage compute -O0
//TEST:SIMPLE(filecheck=NULL): -target spirv -entry computeMain -stage compute -O0
//TEST:SIMPLE(filecheck=COMP): -target spirv -entry computeMain -stage compute -O0
//TEST:SIMPLE(filecheck=COMPV): -target spirv -entry computeMain -stage compute -O0
//TEST:SIMPLE(filecheck=NAMED): -target spirv -entry computeMain -stage compute -O0
//TEST:SIMPLE(filecheck=SPEC): -target spirv -entry computeMain -stage compute -O0
//TEST:SIMPLE(filecheck=RES): -target spirv -entry computeMain -stage compute -O0

// FALSE-COUNT-1: OpConstantFalse %bool
// FALSE-NOT: OpConstantFalse %bool

// NULL-COUNT-1: OpConstantNull %uint
// NULL-NOT: OpConstantNull %uint

// The two plain composites share one composite of one constituent; the decorated block has its
// own constituent and its own composite.
// CHECK-DAG: OpDecorate %[[RELAXED:[A-Za-z0-9_]+]] RelaxedPrecision
// CHECK-DAG: %[[RELAXED]] = OpConstant %int 7001
// CHECK-DAG: = OpConstantComposite %v2int %[[RELAXED]] %[[RELAXED]]
// COMP-COUNT-2: OpConstant %int 7001
// COMP-NOT: OpConstant %int 7001
// COMPV-COUNT-2: OpConstantComposite %v2int
// COMPV-NOT: OpConstantComposite %v2int

// NAMED-COUNT-2: OpConstant %int 7002
// NAMED-NOT: OpConstant %int 7002

// SPEC-COUNT-2: OpSpecConstantTrue %bool
// SPEC-NOT: OpSpecConstantTrue %bool

// RES-COUNT-1: OpConstant %int 7003
// RES-NOT: OpConstant %int 7003

RWStructuredBuffer<int> outputBuffer;

bool falseValue()
{
    return spirv_asm
    {
        %c = OpConstantFalse $$bool;
        result:$$bool = OpCopyObject %c
    };
}

bool falseValueAgain()
{
    return spirv_asm
    {
        %c = OpConstantFalse $$bool;
        result:$$bool = OpCopyObject %c
    };
}

uint nullValue()
{
    return spirv_asm
    {
        %c = OpConstantNull $$uint;
        result:$$uint = OpCopyObject %c
    };
}

uint nullValueAgain()
{
    return spirv_asm
    {
        %c = OpConstantNull $$uint;
        result:$$uint = OpCopyObject %c
    };
}

int2 composite()
{
    return spirv_asm
    {
        %a = OpConstant $$int 7001;
        %v = OpConstantComposite $$int2 %a %a;
        result:$$int2 = OpCopyObject %v
    };
}

int2 compositeAgain()
{
    return spirv_asm
    {
        %a = OpConstant $$int 7001;
        %v = OpConstantComposite $$int2 %a %a;
        result:$$int2 = OpCopyObject %v
    };
}

int2 compositeWithDecoratedConstituent()
{
    return spirv_asm
    {
        %a = OpConstant $$int 7001;
        OpDecorate %a RelaxedPrecision;
        %v = OpConstantComposite $$int2 %a %a;
        result:$$int2 = OpCopyObject %v
    };
}

int namedValue()
{
    return spirv_asm
    {
        %c = OpConstant $$int 7002;
        OpName %c "namedConstant";
        result:$$int = OpCopyObject %c
    };
}

int plainValue()
{
    return spirv_asm
    {
        %c = OpConstant $$int 7002;
        result:$$int = OpCopyObject %c
    };
}

bool specValue()
{
    return spirv_asm
    {
        %s = OpSpecConstantTrue $$bool;
        result:$$bool = OpCopyObject %s
    };
}

bool specValueAgain()
{
    return spirv_asm
    {
        %s = OpSpecConstantTrue $$bool;
        result:$$bool = OpCopyObject %s
    };
}

int resultConstant()
{
    return spirv_asm
    {
        result:$$int = OpConstant 7003
    };
}

int resultConstantAgain()
{
    return spirv_asm
    {
        result:$$int = OpConstant 7003
    };
}

// Each value is stored on its own, so that no arithmetic folds the constants away.
[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = falseValue() ? 1 : 0;
    outputBuffer[1] = falseValueAgain() ? 1 : 0;
    outputBuffer[2] = int(nullValue());
    outputBuffer[3] = int(nullValueAgain());
    outputBuffer[4] = composite().x;
    outputBuffer[5] = compositeAgain().y;
    outputBuffer[6] = compositeWithDecoratedConstituent().x;
    outputBuffer[7] = namedValue();
    outputBuffer[8] = plainValue();
    outputBuffer[9] = specValue() ? 1 : 0;
    outputBuffer[10] = specValueAgain() ? 1 : 0;
    outputBuffer[11] = resultConstant();
    outputBuffer[12] = resultConstantAgain();
}
