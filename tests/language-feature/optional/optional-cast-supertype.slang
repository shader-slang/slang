//TEST:INTERPRET(filecheck=CHECK):
//TEST:SIMPLE(filecheck=IR): -target hlsl -entry upcastToOptional -stage compute -dump-ir

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

// The repro lives in its own function so the IR check below anchors on it alone.
// `b` must be a local variable (an l-value): the base-subobject upcast then lowers
// to a `get_field_addr`, whose address is loaded to a value before the payload
// field read — the read uses that loaded value, not the address.
[shader("compute")]
void upcastToOptional(uniform int seed, out int result)
{
    B b;
    b.x = seed;
    b.y = seed + 1;
    Optional<A> o = b as A;
    // IR: func %upcastToOptional
    // IR: get_field_addr({{.*}}, %base)
    // IR: [[BASE:%[0-9]+]] : %A = load
    // IR: get_field([[BASE]], %x)
    result = o.hasValue ? o.value.x : -1;
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
    // extraction, so its payload is already a plain value.
    A a;
    a.x = 7;
    Optional<A> o2 = a as A;
    // CHECK: 7
    if (o2.hasValue)
        printf("%d\n", o2.value.x);

    // Control: the implicit upcast path (no `as`).
    B b3;
    b3.x = 9;
    b3.y = 10;
    A a3 = b3;
    // CHECK: 9
    printf("%d\n", a3.x);
}
