//TEST:INTERPRET(filecheck=CHECK):
//TEST:COMPILE: tests/language-feature/interfaces/conditional-associated-identity-module.slang -o tests/language-feature/interfaces/conditional-associated-identity-module.slang-module
//TEST:COMPILE: tests/language-feature/interfaces/conditional-associated-identity-caller.slang -r tests/language-feature/interfaces/conditional-associated-identity-module.slang-module -o tests/language-feature/interfaces/conditional-associated-identity-caller.slang-module
//TEST:SIMPLE: -r tests/language-feature/interfaces/conditional-associated-identity-module.slang-module -r tests/language-feature/interfaces/conditional-associated-identity-caller.slang-module -target hlsl -entry computeMain -o conditional-associated-identity.hlsl

import "conditional-associated-identity-module";
import "conditional-associated-identity-caller";

T.Value identity<T : IMixedValue>(T.Value value) { return value; }

int evaluate()
{
    Wrapper<Box<int>> box;
    box.value.payload = 42;
    // MixedBox<float>'s conformance-local alias wins over its nominal identity alias.
    float other = identity<MixedBox<float>>(1.5f);
    return read(box) + readBound(box) + int(other);
}

void main()
{
    // CHECK: 85
    printf("%d\n", evaluate());
}

RWStructuredBuffer<int> output;
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain() { output[0] = evaluate(); }
