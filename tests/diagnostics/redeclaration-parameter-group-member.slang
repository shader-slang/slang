//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// The members of a `cbuffer` or `tbuffer` are visible in the enclosing scope, so they
// conflict with other declarations of the same name in that scope (shader-slang/slang#13876).
// The later declaration gets the diagnostic.

// The reproducer from the issue.

static const int overloadedThing = 1;
/*CHECK:
                 ^^^^^^^^^^^^^^^ see previous declaration of 'overloadedThing'
*/

[UnscopedEnum]
enum TestEnum
{
    overloadedThing = 2,
/*CHECK:
    ^^^^^^^^^^^^^^^ warning E30204
    ^^^^^^^^^^^^^^^ declaration of 'overloadedThing' conflicts with existing declaration
    ^^^^^^^^^^^^^^^ see previous declaration of 'overloadedThing'
*/
};

cbuffer CBUF
{
    int overloadedThing;
/*CHECK:
        ^^^^^^^^^^^^^^^ warning E30204
        ^^^^^^^^^^^^^^^ declaration of 'overloadedThing' conflicts with existing declaration
        ^^^^^^^^^^^^^^^ see previous declaration of 'overloadedThing'
*/
}

tbuffer TBUF
{
    int overloadedThing;
/*CHECK:
        ^^^^^^^^^^^^^^^ warning E30204
        ^^^^^^^^^^^^^^^ declaration of 'overloadedThing' conflicts with existing declaration
*/
}

// A declaration after the buffer gets the diagnostic.

cbuffer BufferFirst
{
    int memberThenConst;
/*CHECK:
        ^^^^^^^^^^^^^^^ see previous declaration of 'memberThenConst'
*/
}

static const int memberThenConst = 1;
/*CHECK:
                 ^^^^^^^^^^^^^^^ warning E30204
                 ^^^^^^^^^^^^^^^ declaration of 'memberThenConst' conflicts with existing declaration
*/

tbuffer TBufferFirst
{
    int tbufferMemberThenConst;
/*CHECK:
        ^^^^^^^^^^^^^^^^^^^^^^ see previous declaration of 'tbufferMemberThenConst'
*/
}

static const int tbufferMemberThenConst = 1;
/*CHECK:
                 ^^^^^^^^^^^^^^^^^^^^^^ warning E30204
                 ^^^^^^^^^^^^^^^^^^^^^^ declaration of 'tbufferMemberThenConst' conflicts with existing declaration
*/

// Members of two buffers.

cbuffer FirstBuffer
{
    int sharedMember;
/*CHECK:
        ^^^^^^^^^^^^ see previous declaration of 'sharedMember'
*/
    float otherMember;
/*CHECK:
          ^^^^^^^^^^^ see previous declaration of 'otherMember'
*/
}

cbuffer SecondBuffer
{
    float sharedMember;
/*CHECK:
          ^^^^^^^^^^^^ warning E30204
          ^^^^^^^^^^^^ declaration of 'sharedMember' conflicts with existing declaration
*/
}

tbuffer ThirdBuffer
{
    int otherMember;
/*CHECK:
        ^^^^^^^^^^^ warning E30204
        ^^^^^^^^^^^ declaration of 'otherMember' conflicts with existing declaration
*/
}

// A declaration that conflicts with several earlier ones gets one diagnostic. It is compared
// with the nearest earlier declaration, unless it has an earlier declaration of the same name
// in its own scope (`aroundBuffer`), which `checkForRedeclaration` compares it with.

cbuffer FirstOfTwo
{
    int afterTwoBuffers;
/*CHECK:
        ^^^^^^^^^^^^^^^ see previous declaration of 'afterTwoBuffers'
*/
}

cbuffer SecondOfTwo
{
    int afterTwoBuffers;
/*CHECK:
        ^^^^^^^^^^^^^^^ warning E30204
        ^^^^^^^^^^^^^^^ declaration of 'afterTwoBuffers' conflicts with existing declaration
        ^^^^^^^^^^^^^^^ see previous declaration of 'afterTwoBuffers'
*/
}

static const int afterTwoBuffers = 1;
/*CHECK:
                 ^^^^^^^^^^^^^^^ warning E30204
                 ^^^^^^^^^^^^^^^ declaration of 'afterTwoBuffers' conflicts with existing declaration
*/

static const int aroundBuffer = 1;
/*CHECK:
                 ^^^^^^^^^^^^ see previous declaration of 'aroundBuffer'
                 ^^^^^^^^^^^^ see previous declaration of 'aroundBuffer'
*/

cbuffer BufferBetweenConsts
{
    int aroundBuffer;
/*CHECK:
        ^^^^^^^^^^^^ warning E30204
        ^^^^^^^^^^^^ declaration of 'aroundBuffer' conflicts with existing declaration
*/
}

static const int aroundBuffer = 2;
/*CHECK:
                 ^^^^^^^^^^^^ error E30200
                 ^^^^^^^^^^^^ declaration of 'aroundBuffer' conflicts with existing declaration
*/

// Functions and types do not overload with buffer members.

cbuffer BufferWithOtherKinds
{
    int funcName;
/*CHECK:
        ^^^^^^^^ see previous declaration of 'funcName'
*/
    int typeName;
/*CHECK:
        ^^^^^^^^ see previous declaration of 'typeName'
*/
}

int funcName()
/*CHECK:
    ^^^^^^^^ warning E30204
    ^^^^^^^^ declaration of 'funcName' conflicts with existing declaration
*/
{
    return 0;
}

struct typeName
/*CHECK:
       ^^^^^^^^ warning E30204
       ^^^^^^^^ declaration of 'typeName' conflicts with existing declaration
*/
{
}

// A function or type declared before a buffer member also conflicts with it.

int funcFirst()
/*CHECK:
    ^^^^^^^^^ see previous declaration of 'funcFirst'
*/
{
    return 0;
}

struct TypeFirst
/*CHECK:
       ^^^^^^^^^ see previous declaration of 'TypeFirst'
*/
{
}

cbuffer BufferAfterOtherKinds
{
    int funcFirst;
/*CHECK:
        ^^^^^^^^^ warning E30204
        ^^^^^^^^^ declaration of 'funcFirst' conflicts with existing declaration
*/
    int TypeFirst;
/*CHECK:
        ^^^^^^^^^ warning E30204
        ^^^^^^^^^ declaration of 'TypeFirst' conflicts with existing declaration
*/
}

// An untyped `let`, whose own comparison runs in the replayed step, after a buffer member.

cbuffer BufferBeforeLet
{
    int letAfterBuffer;
/*CHECK:
        ^^^^^^^^^^^^^^ see previous declaration of 'letAfterBuffer'
*/
}

let letAfterBuffer = 1;
/*CHECK:
    ^^^^^^^^^^^^^^ warning E30204
    ^^^^^^^^^^^^^^ declaration of 'letAfterBuffer' conflicts with existing declaration
*/

// A generic function declared before a buffer member.

int genericFirst<T>(T value)
/*CHECK:
    ^^^^^^^^^^^^ see previous declaration of 'genericFirst'
*/
{
    return 0;
}

cbuffer BufferAfterGeneric
{
    int genericFirst;
/*CHECK:
        ^^^^^^^^^^^^ warning E30204
        ^^^^^^^^^^^^ declaration of 'genericFirst' conflicts with existing declaration
*/
}

// A buffer inside a struct conflicts with the other members of that struct.

struct BufferOwner
{
    static const int structScoped = 1;
/*CHECK:
                     ^^^^^^^^^^^^ see previous declaration of 'structScoped'
*/
    cbuffer BufferInStruct
    {
        int structScoped;
/*CHECK:
            ^^^^^^^^^^^^ warning E30204
            ^^^^^^^^^^^^ declaration of 'structScoped' conflicts with existing declaration
*/
    }
}

// A buffer in a namespace conflicts with the other declarations of that namespace.

namespace BufferNamespace
{
static const int namespaceName = 1;
/*CHECK:
                 ^^^^^^^^^^^^^ see previous declaration of 'namespaceName'
*/

cbuffer BufferInNamespace
{
    int namespaceName;
/*CHECK:
        ^^^^^^^^^^^^^ warning E30204
        ^^^^^^^^^^^^^ declaration of 'namespaceName' conflicts with existing declaration
*/
}
}

// A reopened namespace is one scope, so a buffer member in one body conflicts with a declaration
// in another body.

namespace ReopenedNamespace
{
cbuffer BufferInFirstBody
{
    int reopenedName;
/*CHECK:
        ^^^^^^^^^^^^ see previous declaration of 'reopenedName'
*/
}
}

namespace ReopenedNamespace
{
static const int reopenedName = 1;
/*CHECK:
                 ^^^^^^^^^^^^ warning E30204
                 ^^^^^^^^^^^^ declaration of 'reopenedName' conflicts with existing declaration
*/
}

// These declarations are in different scopes, so they do not conflict with the members.

cbuffer BufferForScopes
{
    int scopedName;
    int funcLocal;
}

struct StructWithField
{
    int scopedName;
}

namespace Namespace
{
static const int scopedName = 1;
}

int useFuncLocal()
{
    int funcLocal = 1;
    return funcLocal;
}

// Attributes are looked up separately from other declarations, so the attribute `Tag`
// that `TagAttribute` declares does not conflict with the member.

cbuffer BufferWithAttributeName
{
    int Tag;
}

[__AttributeUsage(_AttributeTargets.Function)]
struct TagAttribute
{
    int value;
}

[Tag(1)]
int useTag()
{
    return Tag;
}

// A buffer with an instance name is not transparent, so its members do not conflict.

cbuffer NamedBuffer
{
    int namedMember;
} namedBufferInstance;

static const int namedMember = 1;

// Function overloads are still allowed next to buffers.

int overloaded(int x)
{
    return x;
}

int overloaded(float x)
{
    return int(x);
}

cbuffer BufferNextToOverloads
{
    int notOverloaded;
}
