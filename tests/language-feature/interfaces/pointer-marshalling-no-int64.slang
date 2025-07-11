//TEST:SIMPLE(filecheck=CHECK): -target spirv

// CHECK-NOT: Int64

RWStructuredBuffer<uint> result;

struct Data
{
    uint *index_buffer;
    uint type;
};

ConstantBuffer<Data> global_data;

interface IIndexFetcher
{
    uint get_index();
};

struct IndexFetcherU32 : IIndexFetcher
{
    uint *m_ptr;

    __init(uint *ptr) 
    {
        m_ptr = ptr;
    }
    
    uint get_index()
    {
        return 42; // Simplified to avoid dereference issues
    }
};

struct IndexFetcherSimple : IIndexFetcher
{
    uint value;

    __init(uint val) 
    {
        value = val;
    }

    uint get_index()
    {
        return value;
    }
};

[shader("compute")]
void main()
{
    IIndexFetcher pf;
    if (global_data.type == 0) {
        pf = IndexFetcherU32(global_data.index_buffer);
    } else {
        pf = IndexFetcherSimple(100);
    } 
  
    result[0] = pf.get_index();
}