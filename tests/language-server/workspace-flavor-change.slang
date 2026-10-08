//TEST:LANG_SERVER:

// Changing `slang.workspaceFlavor` must take effect for already-open documents: the VFX flavor
// defines FRAGMENT_ANNOTATION, which selects the other branch below. The hover before and after
// the setting change must therefore describe different declarations of `marker`.

#ifdef FRAGMENT_ANNOTATION
int vfxMarker;
#define marker vfxMarker
#else
int standardMarker;
#define marker standardMarker
#endif

void test()
{
    int v = marker;
//HOVER:17,13
//SET_WORKSPACE_FLAVOR:vfx
//HOVER:17,13
// An unknown flavor value must keep the previous flavor (vfx), not reset it to standard.
//SET_WORKSPACE_FLAVOR:unknownFlavor
//HOVER:17,13
//SET_WORKSPACE_FLAVOR:standard
//HOVER:17,13
}
