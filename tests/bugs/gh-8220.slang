//TEST:SIMPLE(filecheck=WGSL): -target wgsl -stage compute -entry main

[shader("compute")]
[numthreads(4, 4, 4)]
void main()
{
    static groupshared Array<float, 4096> array;

    let lambda = () => { return array[1]; };
    array[0] = lambda();
}

// WGSL-NOT: array<f32, i32(4096)>,
