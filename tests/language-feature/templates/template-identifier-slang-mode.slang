//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage compute -entry computeMain

// `template` is a declaration keyword only in the HLSL-flavored dialect; in
// ordinary Slang code it is an ordinary identifier. Here it names a global
// function and a local variable, and the module compiles.

int template()
{
    return 3;
}

//CHECK: void computeMain
[numthreads(1, 1, 1)]
void computeMain()
{
    int template = 3;
    template += 1;
}
