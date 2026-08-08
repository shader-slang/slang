// Regression test for shader-slang/slang#12386.
//
// Comparing a pointer whose pointee type transitively contains an empty struct used to abort
// slangc with `InternalError unexpected: non-simple operand(s)!` on the C-family targets
// (CUDA/CPP), producing no output at all.
//
// Empty-struct legalization lowers `Empty` to nothing, so `Ptr<Empty>` had its pointee legalized
// away. `createLegalPtrType` already answers that case for a physical pointer by producing an
// untyped `Ptr<void>` -- an address is still an address even when its pointee has no fields -- but
// it only did so for the `UserPointer` and `Global` address spaces. The pointer here comes from a
// compiler-generated `var` whose `IRPtrType` carries no address-space operand, so it reported the
// default `AddressSpace::Generic` and fell through to "legalize the pointer away entirely". The
// comparison consuming it then reached the unhandled `default:` arm of `legalizeInst` with a
// non-simple operand and a simple `bool` result, which is the abort.
//
// The trigger is wider than the reported case in two ways: `nullptr` is not required (comparing two
// such pointers to each other aborts just the same), and the pointee need not be spelled as an
// empty struct -- any struct whose fields all legalize away reaches the same state. What matters is
// that the pointee legalizes to nothing *entirely*: a struct with an empty field alongside a real
// one keeps its storage and always compiled, so it is included below as a control.

//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute -line-directive-mode none

struct Empty
{
}

// Every field legalizes away, so `AllEmpty` itself legalizes to nothing just as `Empty` does. This
// is the shape that shows the pointee does not have to be written as an empty struct.
struct AllEmpty
{
    Empty a;
    Empty b;
}

// A control: this keeps a real field, so it retains storage and its address is a genuine pointer.
// It compiled before this fix and must still emit an ordinary comparison after it.
struct Mixed
{
    Empty e;
    int v;
}

RWStructuredBuffer<uint> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // Shape 1 -- the reported case: pointer to an empty struct compared against null.
    Empty value;
    Ptr<Empty> pointer = __getAddress(value);
    uint a = pointer == nullptr ? 1 : 0;

    // Shape 2 -- two pointers compared to each other, with no `nullptr` anywhere, so the abort
    // was never null-specific.
    Empty other;
    Ptr<Empty> p = __getAddress(value);
    Ptr<Empty> q = __getAddress(other);
    uint b = p == q ? 1 : 0;

    // Shape 3 -- the pointee is a struct whose fields all legalize away, so it is not spelled as
    // an empty struct but reaches the same state.
    AllEmpty allEmpty;
    Ptr<AllEmpty> allEmptyPtr = __getAddress(allEmpty);
    uint c = allEmptyPtr == nullptr ? 1 : 0;

    // Shape 4 -- inequality, to cover `cmpNE` as well as `cmpEQ`.
    uint d = p != q ? 1 : 0;

    // Control -- a pointee that keeps storage. This always compiled, and must still emit a real
    // comparison rather than being folded.
    Mixed mixed;
    mixed.v = 1;
    Ptr<Mixed> mixedPtr = __getAddress(mixed);
    uint e = mixedPtr == nullptr ? 1 : 0;

    output[0] = a + b + c + d + e;
}

// The comparisons on pointers that legalized away are answered at compile time: a local's address
// is never null, so shapes 1 and 3 are false; `p` and `q` are distinct locals, so shape 2 is false
// and shape 4 (its inequality) is true. Only the control's comparison survives into the output,
// because that pointee kept its storage.
//CUDA: computeMain
//CUDA: (&mixed{{.*}}) == nullptr
//CUDA-NOT: (&value{{.*}}) == nullptr
//CUDA-NOT: (&allEmpty{{.*}}) == nullptr

//CPP: computeMain
//CPP: (&mixed{{.*}}) == nullptr
//CPP-NOT: (&value{{.*}}) == nullptr
//CPP-NOT: (&allEmpty{{.*}}) == nullptr
