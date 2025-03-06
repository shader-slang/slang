//TEST:COMPILE: tests/library/precompiled-spirv-global.slang -o tests/library/precompiled-spirv-global.slang-module -target spirv -embed-downstream-ir

//import "export-library-global.slang";  //This would make the test pass

extern int myGlobalVar; //This makes the test fail

layout(location = 0) out float4 fragColor;

[shader("fragment")]
void main() {
    fragColor = float4(myGlobalVar);
}
