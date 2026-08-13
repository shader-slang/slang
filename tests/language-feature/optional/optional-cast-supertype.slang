//TEST:INTERPRET(filecheck=CHECK):
//TEST:SIMPLE(filecheck=IR): -target hlsl -entry main -stage compute -dump-ir

// An Optional initialized from a `derived as Base` upcast must hold the base value,
// not the address of the base sub-object, so `.value` reads fields of a struct.
//
// The IR check pins that property directly: the base sub-object address from
// `get_field_addr` is `load`ed to a value, and the payload field read uses that
// loaded value — not the pointer the buggy lowering (#12495) left in place.

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
    // The upcast payload is the loaded base value, and `.value.x` reads a field
    // of that value — not of the `get_field_addr` pointer (the #12495 bug).
    // IR: func %main
    // IR: [[BASE:%[0-9]+]] : %A = load
    // IR: get_field([[BASE]], %x)
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
