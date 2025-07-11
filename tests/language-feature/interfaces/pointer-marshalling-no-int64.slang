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
        return m_ptr[0];
    }
};

struct IndexFetcherU16 : IIndexFetcher
{
    uint16_t *m_ptr;

    __init(uint *ptr) 
    {
        m_ptr = Ptr<uint16_t>(ptr);
    }

    uint get_index()
    {
        return uint(m_ptr[0]);
    }
};

[shader("compute")]
void main()
{
    IIndexFetcher pf;
    if (global_data.type == 0) {
        pf = IndexFetcherU32(global_data.index_buffer);
    } else {
        pf = IndexFetcherU16(global_data.index_buffer);
    } 
  
    result[0] = pf.get_index();
}