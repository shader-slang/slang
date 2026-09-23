//TEST:INTERPRET(filecheck=CHECK): -std legacy -Wno-31200 -warnings-as-errors all

// Exercise the functionality of the deprecated constructors.
//
// Note: The diagnostic testing is done by:
// tests/diagnostics/vec4-init-with-3-elements.slang

void printInt4(int4 v)
{
    printf("(%d,%d,%d,%d)\n", v.x, v.y, v.z, v.w);
}

void main()
{
    // population by elements
    printInt4({ 1, int2(2, 3) });
//CHECK: (1,1,2,3)

    printInt4({ int2(1, 2), 3 });
//CHECK-NEXT: (1,2,3,3)
}

