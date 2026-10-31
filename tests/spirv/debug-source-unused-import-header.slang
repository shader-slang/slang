// Non-test helper header for debug-source-unused-import.slang (no //TEST: directive), #include'd by
// debug-source-unused-import-module.slang. Only the uncompiled entry point of that test calls into
// this header, so none of its debug records belong in the output.

public float unusedHeaderMarker13982(float x)
{
    return x + 4.0;
}
