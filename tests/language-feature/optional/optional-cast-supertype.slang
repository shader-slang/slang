//TEST:INTERPRET(filecheck=CHECK):

// An Optional initialized from a `derived as Base` upcast must hold the base value,
// not the address of the base sub-object, so `.value` reads fields of a struct.

struct A
{
    int x;
}

struct B : A
{
    int y;
}

void main()
{
    // Repro: `b as A` on a value whose base is a stored sub-object.
    B b;
    b.x = 5;
    b.y = 6;
    Optional<A> o = b as A;
    // CHECK: 5
    if (o.hasValue)
        printf("%d\n", o.value.x);

    // Control: a same-type `as` is an identity cast, not a base-subobject
    // extraction, so its payload is already a plain value; must keep working.
    A a;
    a.x = 7;
    Optional<A> o2 = a as A;
    // CHECK: 7
    if (o2.hasValue)
        printf("%d\n", o2.value.x);

    // Control: the implicit upcast path (no `as`) must keep working.
    B b3;
    b3.x = 9;
    b3.y = 10;
    A a3 = b3;
    // CHECK: 9
    printf("%d\n", a3.x);
}
