//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-o tests/diagnostics/command-line/module-format-not-finalized.slang-module

// A library function gives the module something to serialize; no entry point is needed here.
public int add(int a, int b)
{
    return a + b;
}

//CHECK: note[E00088]
//CHECK-SAME: not versioned across compiler releases
//CHECK-SAME: exact same Slang compiler version
