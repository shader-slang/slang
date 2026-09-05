//TEST:SIMPLE(filecheck=CHECK):

// Check that we allow buffers of existential types to be used in Slang 2025.
// This file contains many specializations of builtin generic types with existential types,
// which we still allow, despite disallowing specializing user-defined generics with
// existential types.

#language slang 2025

interface IFoo { void doSomething();}

struct Foo : IFoo{
    void doSomething() {}
}

StructuredBuffer<IFoo> fooBuffer;
RWStructuredBuffer<IFoo> fooBuffer2;
RWByteAddressBuffer byteBuffer;
ParameterBlock<IFoo> gFoo;
uniform IFoo* ptrFoo;

int genericFunc<T:IFoo>(T v)
{
    return 0;
}

void helper(IFoo g){g.doSomething();}

void caller(IFoo f)
{
    // CHECK-NOT: error 33180
    // Existential-specialization rejection now reports E38029 (was E33180); guard the new code too
    // so "builtin-generic-with-existential is allowed" stays protected under the migrated diagnostic.
    // CHECK-NOT: error 38029
    let a = fooBuffer.Load(0);
    let b = byteBuffer.Load<IFoo>(0);
    let c = createDynamicObject<IFoo>(0, 0);
    let d = fooBuffer[1];
    let e = fooBuffer2[2];
    gFoo.doSomething();
    helper(gFoo);
    byteBuffer.Store(0, e);
    *ptrFoo = c;
    let z = *ptrFoo;
    Optional<IFoo> optFoo = z;
    if (optFoo.hasValue)
    {
        optFoo.value.doSomething();
    }
}
