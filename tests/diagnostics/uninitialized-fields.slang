//TEST:SIMPLE(filecheck=CHK): -target spirv

extern static const int flag;

//struct None
//{
//    int a;
//    int b;

//    __init()
//    {
//        a = 0;
//        b = 0;
//    }
//}

//struct Simple
//{
//    int value;
//    Simple *next;

//    __init(int v, Simple* ptr)
//    {
//        value = v;
//        //CHK-DAG: warning 41020: exiting constructor without initializing field 'next'
//    }
//}

//struct Conditional
//{
//    int field;
//    int another_field;

//    __init()
//    {
//        field = 0;
//        if (flag == 0)
//            //CHK-DAG: warning 41020: exiting constructor without initializing field 'another_field'
//            return;

//        another_field = 0;
//    }
//};

//struct Partial
//{
//    int inline = 0;
//    int body = 0;

//    __init(int x)
//    {
//        body = x;
//    }
//}

// TODO: warning should be a bit different for here?
struct Test
{
	int a;
	int b = 1;
	int c;
	int d = 1 + 1;
}

struct Normal
{
    int x;
    __init() { x = 0; }
}

//CHK-NOT: warning 41020
