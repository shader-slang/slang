//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):
enum MyError
{
    Fail
}

void f()
{
    throw MyError.Fail;
    catch(err: MyError)
    {
        printf("Catch!\n");
    }
    printf("Invalid command after catch!\n");
}

// CHECK: error 20103
