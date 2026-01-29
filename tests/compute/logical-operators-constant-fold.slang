// logical-operators-constant-fold.slang
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHK):-slang -compute -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHK):-vk -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHK):-cpu -compute -shaderobj

// Test that logical operators (!, &&, ||) are correctly constant-folded.
// This test verifies fixes for bugs where !false was folded to false (should be true)
// and !true was folded to true (should be false), and ensures compound expressions
// with && and || also work correctly.
//
// Tests both regular static const values and extern static const values with initializers.

// Expected output: all tests pass (output = 1)
//CHK:1

// Test case 1: Direct constant folding with regular static const
static const bool FALSE_VAL = false;
static const bool TRUE_VAL = true;

static const bool NOT_FALSE = !FALSE_VAL;  // Should be true
static const bool NOT_TRUE = !TRUE_VAL;    // Should be false

// Test case 2: Compound expressions with && and ||
static const bool AND_TEST = TRUE_VAL && !FALSE_VAL;   // true && true = true
static const bool OR_TEST = FALSE_VAL || !FALSE_VAL;   // false || true = true
static const bool COMPLEX = !FALSE_VAL && !TRUE_VAL;   // true && false = false

// Test case 3: Extern const with initializers
// This tests the fix where extern const initializers are now properly folded
// instead of being left as symbolic references, enabling their use in compile-time expressions.
public static extern const bool EXTERN_FALSE = false;
public static extern const bool EXTERN_TRUE = true;

static const bool EXTERN_AND = EXTERN_TRUE && !EXTERN_FALSE;  // true && true = true
static const bool EXTERN_OR = EXTERN_FALSE || !EXTERN_FALSE;  // false || true = true

// Test case 4: Using logical operators in generic type arguments
struct ValueHolder<bool condition>
{
    static const int value = condition ? 1 : 0;
};

// With regular static const
typealias TestNotFalse = ValueHolder<!FALSE_VAL>;              // ValueHolder<true>
typealias TestNotTrue = ValueHolder<!TRUE_VAL>;                // ValueHolder<false>
typealias TestAnd = ValueHolder<TRUE_VAL && !FALSE_VAL>;       // ValueHolder<true>
typealias TestOr = ValueHolder<FALSE_VAL || !FALSE_VAL>;       // ValueHolder<true>

// With extern const (requires compile-time constant evaluation)
typealias TestExternNot = ValueHolder<!EXTERN_FALSE>;                    // ValueHolder<true>
typealias TestExternNot2 = ValueHolder<!EXTERN_TRUE>;                    // ValueHolder<false>
typealias TestExternAnd = ValueHolder<EXTERN_TRUE && !EXTERN_FALSE>;     // ValueHolder<true>
typealias TestExternOr = ValueHolder<EXTERN_FALSE || !EXTERN_FALSE>;     // ValueHolder<true>

// Test case 5: Using Conditional.get() at runtime
//TEST_INPUT: set conditionalValue = ubuffer(data=[36], stride=4)
uniform Conditional<int, !FALSE_VAL> conditionalValue;           // With static const
//TEST_INPUT: set externConditionalValue = ubuffer(data=[99], stride=4)
uniform Conditional<int, EXTERN_TRUE && !EXTERN_FALSE> externConditionalValue;  // With extern const

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
{
    // All conditions should be true if logical operators work correctly
    bool allCorrect = true
        // Regular static const tests
        && (NOT_FALSE == true)                // !false should be true
        && (NOT_TRUE == false)                // !true should be false
        && (AND_TEST == true)                 // true && true should be true
        && (OR_TEST == true)                  // false || true should be true
        && (COMPLEX == false)                 // true && false should be false
        && (TestNotFalse.value == 1)          // ValueHolder<true>.value should be 1
        && (TestNotTrue.value == 0)           // ValueHolder<false>.value should be 0
        && (TestAnd.value == 1)               // ValueHolder<true>.value should be 1
        && (TestOr.value == 1)                // ValueHolder<true>.value should be 1
        // Extern const tests
        && ((!EXTERN_FALSE) == true)          // !false should be true
        && ((!EXTERN_TRUE) == false)          // !true should be false
        && (EXTERN_AND == true)               // true && true should be true
        && (EXTERN_OR == true)                // false || true should be true
        && (TestExternNot.value == 1)         // ValueHolder<true>.value should be 1
        && (TestExternNot2.value == 0)        // ValueHolder<false>.value should be 0
        && (TestExternAnd.value == 1)         // ValueHolder<true>.value should be 1
        && (TestExternOr.value == 1)          // ValueHolder<true>.value should be 1
        ;

    // Runtime .get() tests
    if (let v = conditionalValue.get())
    {
        allCorrect = allCorrect && (v == 36);  // Should get the value we set
    }
    else
    {
        allCorrect = false;  // Should have a value
    }

    if (let v = externConditionalValue.get())
    {
        allCorrect = allCorrect && (v == 99);  // Should get the value we set
    }
    else
    {
        allCorrect = false;  // Should have a value
    }

    outputBuffer[0] = allCorrect ? 1 : 0;
}
