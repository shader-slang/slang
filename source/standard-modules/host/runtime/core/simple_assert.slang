// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

public void fail(String message)
{
    hgbc_asm
    {
        OpFail $message;
    };
}

public void fail()
{
    fail("program aborted");
}

[ForceInline]
public void assert(bool condition)
{
    if (!condition)
        fail("assertion failed");
}
