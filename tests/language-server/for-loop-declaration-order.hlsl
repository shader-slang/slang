//TEST:LANG_SERVER(filecheck=CHECK):

// Hover must reach references inside implicit unbraced bodies and use the declaration bound
// at each reference, even after a later loop initializer replaces the same name.

void bindLoopInitializers(bool condition)
{
    int result = 0;
    for (int index = 0; index < 1; ++index)
//HOVER:11,19
        result += index;
    for (float index = 0; index < 1; ++index)
//HOVER:14,23
        result += int(index);
    if (condition)
        for (uint index = 0; index < 1; ++index)
//HOVER:18,27
            result += int(index);
//HOVER:20,19
    result += int(index);
}

// CHECK: (local variable) int index
// CHECK: (local variable) float index
// CHECK: (local variable) uint index
// CHECK: (local variable) float index
