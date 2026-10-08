//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

int externalValue;

struct Holder
{
    property int refPreferred
    {
        [nonmutating] get { return 0; }
        [nonmutating] set {}
        [mutating] ref { return externalValue; }
    }

    property int accessorWriteback
    {
        [nonmutating] get { return 0; }
        [nonmutating] set {}
    }

    property int nonmutatingRefPreferred
    {
        [mutating] get { return 0; }
        [mutating] set {}
        [nonmutating] ref { return externalValue; }
    }
}

void mutate(inout uint value)
{
    value = 1;
}

void testImplicitLValueCast()
{
    Holder holder = {};

    // Lowering aggressively requests an address for the value under an implicit l-value cast. A
    // present `ref` accessor wins and must be valid for the captured receiver.
    let rejectMutatingRef = () => { mutate(holder.refPreferred); };
//CHECK:                                          ^^^^^^^^^^^^ mutating storage accessor cannot be called on an immutable value
//CHECK:                                          ^^^^^^^^^^^^ mutating 'ref' accessor cannot be called on an immutable value
//CHECK:                                   ^^^^^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification

    // Without a `ref` accessor, lowering copies through the getter and setter instead.
    let acceptAccessorWriteback = () => { mutate(holder.accessorWriteback); };

    // A present nonmutating `ref` accessor bypasses the mutating getter and setter, so only the
    // `ref` accessor participates in receiver validation.
    let acceptNonmutatingRef = () => { mutate(holder.nonmutatingRefPreferred); };

    rejectMutatingRef();
    acceptAccessorWriteback();
    acceptNonmutatingRef();
}
