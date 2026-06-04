implementing pragma_emptypushpop_test;

// An empty push/pop. This must not disturb the root module's disable of 30856.
#pragma warning(push)
#pragma warning(pop)
