//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct S
{
    private int structPrivate()
    {
        return 1;
    }

    int useExtensionPrivate(S s)
    {
        return s.extensionPrivate();
    }
}

extension S
{
    private int extensionPrivate()
    {
        return 2;
    }

    int useStructPrivate(S s)
    {
        return s.structPrivate();
    }
}

extension S
{
    int useSiblingExtensionPrivate(S s)
    {
        return s.extensionPrivate();
    }
}

void test(S s)
{
    s.extensionPrivate();
/*CHECK:
      ^^^^^^^^^^^^^^^^ declaration not accessible
      ^^^^^^^^^^^^^^^^ 'extensionPrivate' is not accessible from the current context.
*/
}
