//TEST:INTERPRET(hgvm):

import hgvm_relative_imported;

void fail(String message)
{
    hgbc_asm {
        OpFail $message;
    };
}

[host]
void main()
{
    if (getImportedValue() != 42)
        fail("HGVM did not resolve the module beside the test source");
}
