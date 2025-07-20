

interface ILog
{
    static float log(float x);
}

struct SlowLog : ILog
{
    static float log(float x)
    {
        // implement slow log.
    }
}

struct FastLog : ILog
{
    static float log(float x)
    {
        // implement slow log.
    }
}

// prototype system
// declare ILog = SlowLog | FastLog;

// actual slang
extern struct MyLog : ILog; // slangApi->setLinkTimeConst("MyLog", "FastLog");


extern static const int TILE_SIZE; // slangApi->setLinkTimeConst("TILE_SIZE", 16);

void main()
{
    // Use 1
    MyLog.log(10.f);


    // Use 2
    MyLog.log(20.f);
}

matrix<T, N, M> matmul<T : __BuiltinFloatingPointType, int N, int M>(matrix<T, N, M>, matrix<T, N, M>)
{
    ....
}

void main()
{
    
    matmul<half, 4, 4>(cast<half>(matA), cast<half>(matB));
}

// slangApi->setLinkTimeConst("main.MyLog1", "FastLog");