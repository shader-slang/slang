// nonmutating-ref-this-rooted.slang
//
// A `[nonmutating] ref` accessor on a value type (`struct`) receives `this` by value, so
// returning a reference rooted in `this` — a member of `this`, an element of an array
// member, or a swizzle/matrix-element of a vector/matrix member — hands out the address
// of a temporary copy and the caller's write is silently lost. The compiler rejects it
// (error 30073). This is the fail-loudly counterpart to the sound `[nonmutating] ref`
// that returns storage *outside* `this` — a global, storage reached through a pointer
// member, or a member *property* that forwards through its own accessor — which still
// compiles (see nonmutating-ref-accessor.slang). A `class` is a reference type (its `this`
// is always a reference), so a class accessor returning a field is sound and not rejected.
// https://github.com/shader-slang/slang/issues/9636, #12487

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

static int gBacking = 0;
static int gArr[4];

// Rejected: a property `[nonmutating] ref` returning a member of `this`.
struct PropThisRooted
{
    int _v;
    property int value
    {
        get { return _v; }
        [nonmutating] ref { return _v; }
//CHECK:                           ^^ a `[nonmutating] ref` accessor cannot return storage rooted in `this`
    }
}

// Rejected: a `__subscript` `[nonmutating] ref` returning an element of an array member
// of `this`. The index is a by-value `in`, but the storage is still in `this`.
struct SubscriptThisRooted
{
    int _arr[4];
    __subscript(int i)->int
    {
        get { return _arr[i]; }
        [nonmutating] ref { return _arr[i]; }
//CHECK:                               ^ a `[nonmutating] ref` accessor cannot return storage rooted in `this`
    }
}

// Rejected: a swizzle of a vector member of `this`.
struct SwizzleThisRooted
{
    float3 _vec;
    property float value
    {
        get { return _vec.x; }
        [nonmutating] ref { return _vec.x; }
//CHECK:                                ^ a `[nonmutating] ref` accessor cannot return storage rooted in `this`
    }
}

// Rejected: a matrix-element swizzle of a matrix member of `this`.
struct MatrixSwizzleThisRooted
{
    float2x2 _mat;
    property float value
    {
        get { return _mat._11; }
        [nonmutating] ref { return _mat._11; }
//CHECK:                                ^^^ a `[nonmutating] ref` accessor cannot return storage rooted in `this`
    }
}

// Allowed: a property `[nonmutating] ref` returning external global storage.
struct PropExternal
{
    property int value
    {
        get { return gBacking; }
        [nonmutating] ref { return gBacking; }
    }
}

// Allowed: a `__subscript` `[nonmutating] ref` returning an element of an external global
// array (the index is only read to compute an offset into external storage).
struct SubscriptExternal
{
    __subscript(int i)->int
    {
        get { return gArr[i]; }
        [nonmutating] ref { return gArr[i]; }
    }
}

// Allowed: the outer accessor returns a member *property*, which forwards through its own
// accessor to external storage — not directly rooted in `this`.
struct PropForward
{
    property int inner
    {
        get { return gBacking; }
        [nonmutating] ref { return gBacking; }
    }
    property int outer
    {
        get { return inner; }
        [nonmutating] ref { return inner; }
    }
}

// Allowed: a `class` is a reference type, so its `this` is a reference even for a
// `[nonmutating]` accessor; returning a field denotes the caller's object, not a copy.
class ClassThisField
{
    int _v;
    property int value
    {
        get { return _v; }
        [nonmutating] ref { return _v; }
    }
}

// Allowed: the same reasoning through an `extension` of a class. The `this` type is the
// extended class, so it stays a reference type and the accessor is not rejected.
class ClassForExtension
{
    int _v;
}
extension ClassForExtension
{
    property int value
    {
        get { return _v; }
        [nonmutating] ref { return _v; }
    }
}

// Allowed: storage reached *through* a pointer member is outside `this` (the pointer value
// lives in `this`, but the pointee does not), so the `DerefExpr` stops the root walk and no
// 30073 is reported.
struct PtrMember
{
    Ptr<int> p;
    property int value
    {
        get { return *p; }
        [nonmutating] ref { return *p; }
    }
}

// Rejected: a `[nonmutating]` accessor keeps `this` immutable in its own body, so writing a
// `this` field is rejected as a non-l-value (E30011) — the counterpart to the implicitly-
// mutating unannotated `ref`, which may write `this` (see ref-accessor-body-mutation.slang).
struct NonmutatingBodyWrite
{
    int _v;
    property int value
    {
        get { return _v; }
        [nonmutating] ref { _v = _v + 1; return gBacking; }
//CHECK:                    ^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification
//CHECK:                       ^ left of '=' is not an l-value
//CHECK:                       ^ left of '=' is not an l-value.
    }
}

// Rejected: a `[constref]` accessor receives `this` by const reference, so it too keeps
// `this` immutable in its own body. Both an unqualified member write (governed by the
// `_lookUpInScopes` mutability ladder) and an explicit `this.` write (governed by
// `visitThisExpr`) are rejected, confirming both l-value paths honor `[constref]`.
struct ConstRefBodyWrite
{
    int _v;
    property int value
    {
        get { return _v; }
        [constref] ref { _v = 1; this._v = 2; return gBacking; }
//CHECK:                 ^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification
//CHECK:                    ^ left of '=' is not an l-value
//CHECK:                    ^ left of '=' is not an l-value.
//CHECK:                              ^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification
//CHECK:                                 ^ left of '=' is not an l-value
//CHECK:                                 ^ left of '=' is not an l-value.
    }
}
