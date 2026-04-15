// Helper for api-removed-since-2.slang for checking that imported
// symbols are checked correctly against the language version of the importer

//TEST:SIMPLE:

// Note: keep this version below what is in api-removed-since-2.slang
#language slang 2025

[deprecated("This struct is deprecated")]
[RemovedSince(2027, "Use something else")]
public struct RemovedStructSince2027
{
}

[deprecated("This struct is deprecated")]
[RemovedSince(2026, "Use something else")]
public struct RemovedStructSince2026
{
}

[RemovedSince(2026, "Use something else")]
public struct RemovedStructSince2026NoDeprecation
{
}

[deprecated("This struct is deprecated")]
[RemovedSince(2024, "Use something else")]
public struct RemovedStructSince2024
{
}

void checkAccess()
{
    var s1 : RemovedStructSince2026NoDeprecation;
}
