// syntax-error-op-line-2.slang

// NOTE! That although this is a 'diagnostic' like test, it tests using downstream compiler
// the downstream compiler being present is a requirement, so we mark as a 'TEST' so that
// those tests are made.

//TEST:SIMPLE(filecheck=CHK):-entry computeMain -target spirv -emit-spirv-via-glsl
//TEST:SIMPLE(filecheck=CHK):-entry computeMain -target dxil -profile cs_6_0
//TEST:SIMPLE(filecheck=CHK):-entry computeMain -target dxbc
//TEST:SIMPLE(filecheck=CHK):-entry computeMain -target shader-dll
//TEST:SIMPLE(filecheck=CHK):-entry computeMain -target ptx

// Here the thing being checked is error reporting around return, and +=

int doSomething(int a)
{
    a += a;

    return a
    +=
    __SyntaxError();
    //CHK:([[#@LINE-1]]): error
}

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int id = int(dispatchThreadID.x);

    int v = int(dispatchThreadID.y);

    v += doSomething(id);
}
