// Zero-initializing an array of Optional<T>, (Optional<T>[N])0, must yield
// `none` in every element. This shares the getDefaultVal fix from issue #12556:
// Optional carries __intrinsic_type, so it now routes through emitDefaultConstruct
// (-> makeOptionalNone) instead of a fieldless makeStruct. The lowering is
// target-independent, so the interpreter exercises the same path.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    Optional<int> a[2] = (Optional<int>[2])0;

    // CHECK: none none
    printf("%s %s\n", a[0].hasValue ? "some" : "none", a[1].hasValue ? "some" : "none");
}
