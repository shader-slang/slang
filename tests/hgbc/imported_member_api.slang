public struct ImportedFilm
{
    public uint2 resolution;

    [host]
    public __init(uint2 value)
    {
        resolution = value;
    }
}
