//DIAGNOSTIC_TEST:SIMPLE(diag=diag): -stage compute -entry computeMain -target cpp
//
// Regression test for https://github.com/shader-slang/slang/issues/12266

RWStructuredBuffer<uint> output;

[numthreads(1,1,1)]
void computeMain()
{
    // variable declarations
    defer uint v0 = 1;
//diag:        ^^ declaration not allowed here
//diag:        ^^ variable definition is not allowed here.
    defer let v1 = 1U;
//diag:       ^^ declaration not allowed here
//diag:       ^^ immutable value definition is not allowed here.
    defer var v2 : uint = 1;
//diag:       ^^ declaration not allowed here
//diag:       ^^ variable definition is not allowed here.

    defer if (true) uint v3 = 1;
//diag:                  ^^ declaration not allowed here
//diag:                  ^^ variable definition is not allowed here.

    // check that statement behind labels are checked, too
    defer test: rest: var v4 : uint = 10;
//diag:                   ^^ declaration not allowed here
//diag:                   ^^ variable definition is not allowed here.

    defer uint v5 = 1, v6 = 1;
//diag:        ^^ declaration not allowed here
//diag:        ^^ variable definition is not allowed here.

    // these used to crash the compiler before 12266 was fixed
    output[0] = v0;
    output[1] = v1;
    output[2] = v2;
    output[3] = v3;
    output[4] = v4;
    output[5] = v5;
    output[6] = v6;

    // type declarations
    defer typealias T1 = uint;
//diag:             ^^ declaration not allowed here
//diag:             ^^ typealias is not allowed here.
    defer typedef uint T2;
//diag:                ^^ declaration not allowed here
//diag:                ^^ typedef is not allowed here.

    // deferred declarations in block statements are still ok
    defer {
        let v3 = 2;
    }
}
