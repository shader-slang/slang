//TEST:SIMPLE(filecheck=CHECK): -target hgbc

import imported_member_api;

public struct RootValue
{
    public uint value;

    [host]
    public __init(ImportedFilm film)
    {
        value = film.resolution.x;
    }
}

[host]
public uint useImported(uint2 resolution)
{
    let film = ImportedFilm(resolution);
    return RootValue(film).value;
}

// The root constructor has both an entry-point wrapper and a callable implementation, but it is
// one API overload. The imported constructor only has its callable implementation in the linked
// IR and must still retain its public graph API.
// CHECK-COUNT-2: OpExport Constructor
