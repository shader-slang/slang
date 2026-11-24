//TEST:INTERPRET(hgvm,filecheck=CHECK): -api default

// A runtime failure ends with the HGVM call stack, innermost call first, so the failing code can be
// found without a debugger. Each frame with line information names its call; the standard module's
// `fail`, which `assert` calls, is precompiled without line information and names only itself.

import slang.runtime;

[host]
void check(int value)
{
    assert(value > 0);
}

[host]
void validate(int count)
{
    for (int i = 0; i < count; ++i)
        check(2 - i);
}

[host]
void main()
{
    validate(1);
    validate(3);
}

// CHECK: result code = 1
// CHECK: assertion failed
// CHECK-NEXT: at fail{{$}}
// CHECK-NEXT: at check ({{.*}}runtime-failure-stack-trace.slang:12:5)
// CHECK-NEXT: at validate ({{.*}}runtime-failure-stack-trace.slang:19:9)
// CHECK-NEXT: at main ({{.*}}runtime-failure-stack-trace.slang:26:5)
