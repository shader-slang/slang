implementing pragma_pushpop_test;

// An unrelated push/pop of a *different* warning id. This must not disturb the
// root module's disable of 30856 that governs the earlier body file.
#pragma warning(push)
#pragma warning(disable: 31000)
#pragma warning(pop)
