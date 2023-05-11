//TEST:SIMPLE:

func foo(f : functype (float) -> int) -> int{
    return f(0);
}

int bap<T>(float) {
    return 1;
}

int zap() {
    // We should be able to specify which foo we want
    return foo(bap<bool>);
}
