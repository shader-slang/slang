# Diagnostic Line Number Test

This test verifies that error diagnostics in `.slang.md` files
report the correct line number from the original Markdown file.

```slang
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void foo()
{
    int x = undefinedVar;
//CHECK:    ^ undefined identifier
//CHECK:    ^ undefined identifier 'undefinedVar'.
}
```
