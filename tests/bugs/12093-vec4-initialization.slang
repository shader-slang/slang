//TEST:INTERPRET(filecheck=CHECK):  -warnings-as-errors all

void printInt4(int4 v)
{
    printf("(%d,%d,%d,%d)\n", v.x, v.y, v.z, v.w);
}

void main()
{
    // population by elements
    printInt4(1);
//CHECK: (1,1,1,1)

    printInt4({ 1 });
//CHECK-NEXT: (1,1,1,1)

    printInt4({ 1, 2 });
//CHECK-NEXT: (1,2,0,0)

    printInt4({ 1, 2, 3 });
//CHECK-NEXT: (1,2,3,0)

    printInt4({ 1, 2, 3, 4 });
//CHECK-NEXT: (1,2,3,4)

    // population by int2 and elements
    printInt4({ int2(1, 2), int2(3, 4) });
//CHECK-NEXT: (1,2,3,4)

    printInt4({ int2(1, 2), 3, 4 });
//CHECK-NEXT: (1,2,3,4)

    printInt4({ 1, int2(2, 3), 4 });
//CHECK-NEXT: (1,2,3,4)

    printInt4({ 1, 2, int2(3, 4) });
//CHECK-NEXT: (1,2,3,4)

    // population by int3 and elements
    printInt4({ int3(1, 2, 3), 4 });
//CHECK-NEXT: (1,2,3,4)

    printInt4({ 1, int3(2, 3, 4) });
//CHECK-NEXT: (1,2,3,4)

    // population by int4
    printInt4({ int4(1, 2, 3, 4) });
//CHECK-NEXT: (1,2,3,4)
}

