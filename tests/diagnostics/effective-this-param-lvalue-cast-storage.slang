//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

// A reversible l-value cast can preserve an address returned by `ref` or write a converted value
// back through a getter/setter pair. Receiver validation must follow the selected representation.

// A module-scope variable without `static` is an immutable shader parameter. This backing value is
// explicitly static because these accessors need to return writable storage by reference.
static int externalValue;

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

void exerciseImplicitLValueCastAccessors()
{
    Holder holder = {};

    // The reversible int-to-uint cast needs an address for its `inout` temporary. When the property
    // provides a `ref` accessor, lowering retains the returned address in its extended-l-value
    // representation instead of materializing getter/setter writeback. The selected `ref` accessor
    // must accept the captured receiver.
    let rejectMutatingRef = () => { mutate(holder.refPreferred); };
//CHECK:                                          ^^^^^^^^^^^^ mutating method cannot be called on immutable value
//CHECK:                                          ^^^^^^^^^^^^ mutating method 'ref' cannot be called on an immutable value
//CHECK:                                   ^^^^^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification

    // Without a `ref` accessor, lowering copies through the getter and setter instead.
    let acceptAccessorWriteback = () => { mutate(holder.accessorWriteback); };

    // The nonmutating `ref` accessor similarly bypasses this property's mutating getter and setter,
    // so only `ref` participates in receiver validation.
    let acceptNonmutatingRef = () => { mutate(holder.nonmutatingRefPreferred); };

    rejectMutatingRef();
    acceptAccessorWriteback();
    acceptNonmutatingRef();
}

