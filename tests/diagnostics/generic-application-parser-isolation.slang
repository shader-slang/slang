//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Generic-application disambiguation must not run semantic expression checking from the parser.
// Direct declaration lookup and an isolated grammar probe provide the evidence needed below.

void checkLambdaBase()
{
    ((int value) => value).missing<int>();
//CHECK:                   ^^^^^^^ member not found
//CHECK:                   ^^^^^^^ 'missing' is not a member of 'checkLambdaBase._slang_Lambda_checkLambdaBase_0'.
}

interface IValue
{
    associatedtype Element;
    Element get();
}

void requireSameType<T>(T first, T second) {}

void checkAssignmentBase(IValue value, IValue replacement)
{
    IValue local = value;

    // Opening the same existential twice before the assignment must produce the same type. The
    // parser probe for the later generic-looking member access must not check the assignment and
    // mark `local` as reassigned early.
    requireSameType(local.get(), local.get());

    (local = replacement).missing<int>();
//CHECK:                  ^^^^^^^ member not found
//CHECK:                  ^^^^^^^ 'missing' is not a member of 'checkAssignmentBase..This'.
}

bool checkInlineSpirvComparison(int value)
{
    // The spaces surrounding `< value >` preserve comparison parsing when neither declaration
    // lookup nor the grammar probe provides evidence that this is a generic application.
    return (spirv_asm
    {
        OpMemoryBarrier Device AcquireRelease | MemorySemanticsAtomicCounterMemory;
        result : $$int = OpConstant 1;
    }) < value > 0;
}

struct ComparisonOperand
{
    int value;
}

ComparisonOperand getComparisonOperand();

bool checkLiteralComparison(bool chooseFirst, int upperBound)
{
    // With a value argument and a nonempty parenthesized expression after `>`, this token sequence
    // is inherently ambiguous. On an unclassified base, the spaced spelling remains two
    // comparisons for compatibility.
    return (chooseFirst ? getComparisonOperand() : getComparisonOperand()).value < 4 > (upperBound);
}

bool checkNamedValueComparison(
    bool chooseFirst,
    int lowerBound,
    int upperBound)
{
    return (chooseFirst ? getComparisonOperand() : getComparisonOperand()).value < lowerBound >
           (upperBound);
}

typealias UnaryIntFunction = functype(int) -> int;

T identity<T>(T value)
{
    return value;
}

ComparisonOperand operator<(int left, UnaryIntFunction middle)
{
    return { left };
}

bool operator>(ComparisonOperand left, int right)
{
    return left.value > right;
}

bool checkNestedGenericValueComparison(bool chooseFirst, int upperBound)
{
    // Type syntax inside a nested generic function specialization does not prove that the outer
    // `<` starts another generic application. The specialization is the middle operand of two
    // comparisons here, and the surrounding spaces preserve that interpretation.
    return (chooseFirst ? getComparisonOperand() : getComparisonOperand()).value < identity<int> >
           (upperBound);
}

bool checkNestedProbeValueComparison(bool chooseFirst, int upperBound)
{
    // The middle operand requires its own generic-application probe while the outer probe is still
    // parsing. Both probes use the same isolated AST builder, and the outer spaced comparison must
    // retain its comparison interpretation.
    return (chooseFirst ? getComparisonOperand() : getComparisonOperand()).value <
           makeGenericMethodFactory().identity<int> > (upperBound);
}

struct GenericMethodFactory
{
    T identity<T>(T value) { return value; }
    int constant<let N : int>() { return N; }
}

GenericMethodFactory makeGenericMethodFactory();

namespace GenericApplicationArgumentTypes
{
    struct Container
    {
        struct Value
        {
            int payload;
        }
    }
}

int checkQualifiedTypeArgument(GenericApplicationArgumentTypes::Container::Value value)
{
    // The method call has no declaration that parser lookup can classify. Its qualified type
    // argument nevertheless makes the spaced `<...>` sequence a generic application rather than
    // two comparisons.
    return makeGenericMethodFactory().identity <
        GenericApplicationArgumentTypes::Container::Value > (value).payload;
}

int directValueApplication<let N : int>(int value)
{
    return N + value;
}

int operator+(functype(int) -> int operation, int value)
{
    return operation(value);
}

__postfix
functype(int) -> int operator++(functype(int) -> int operation)
{
    return operation;
}

void checkGenericApplications()
{
    // A direct generic declaration is conclusive even when a value argument, `>`, and the call's
    // opening parenthesis are all separated by whitespace.
    let directResult = directValueApplication <4> (1);

    // The receiver expressions below have no declaration that direct parser lookup can classify.
    // A directly named type argument makes the first call conclusive. An array type is likewise
    // conclusive because its indexed base directly names a type. Before Slang 2026, the empty
    // argument list makes the value-generic call conclusive because `()` cannot be the right
    // operand of `>`.
    let typeArgumentResult = makeGenericMethodFactory().identity <int> (2);
    int[4] arrayValue;
    let arrayTypeArgumentResult =
        makeGenericMethodFactory().identity <int[4]> (arrayValue);
    let valueArgumentResult = makeGenericMethodFactory().constant <4> ();

    // `expand` and `each` are contextual keywords. The definitive parser lookup recognizes these
    // local declarations as shadows, so the isolated probe must recognize them as value arguments
    // too.
    static const int expand = 5;
    static const int each = 6;
    let expandResult = makeGenericMethodFactory().constant<expand>();
    let eachResult = makeGenericMethodFactory().constant<each>();

    // The isolated grammar probe accepts the expression continuations that can legally follow a
    // generic application. These overloads make the expressions valid only when `<int>` is parsed
    // as a generic application.
    let invoked = makeGenericMethodFactory().identity <int>(3);
    let sum = makeGenericMethodFactory().identity <int> + 4;
    makeGenericMethodFactory().identity <int>++;

    // A semicolon cannot continue the comparison interpretation, so a successful grammar probe
    // also preserves a standalone generic application.
    let operation = makeGenericMethodFactory().identity <int> ;
}

// Type aliases derive from `SimpleTypeDecl` rather than `AggTypeDeclBase`. Direct parser lookup
// must still classify the alias as a type, so semantic checking sees a generic application and
// diagnoses the attempt to specialize a non-generic declaration.
typealias IntAlias = int;

IntAlias<float> invalidTypeAliasApplication;
/*CHECK:
^^^^^^^^ expected a generic when using '<...>'
^^^^^^^^ expected a generic when using '<...>' (found: 'typeof(IntAlias)')
*/
