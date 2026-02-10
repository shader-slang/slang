// logical-operators-constant-fold.slang
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHK):-slang -compute -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHK):-vk -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHK):-cpu -compute -shaderobj

// Test that logical operators (!, &&, ||) are correctly constant-folded.
// This test verifies fixes for bugs where !false was folded to false (should be true)
// and !true was folded to true (should be false), and ensures compound expressions
// with && and || also work correctly.

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

// Test case 3: Using logical operators in generic type arguments
struct ValueHolder<bool condition>
{
    static const int value = condition ? 1 : 0;
};

// With regular static const
typealias TestNotFalse = ValueHolder<!FALSE_VAL>;              // ValueHolder<true>
typealias TestNotTrue = ValueHolder<!TRUE_VAL>;                // ValueHolder<false>
typealias TestAnd = ValueHolder<TRUE_VAL && !FALSE_VAL>;       // ValueHolder<true>
typealias TestOr = ValueHolder<FALSE_VAL || !FALSE_VAL>;       // ValueHolder<true>

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
        ;

    outputBuffer[0] = allCorrect ? 1 : 0;
}
