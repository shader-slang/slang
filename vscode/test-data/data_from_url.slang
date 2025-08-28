import playground;

// Load 1024 bytes of ascending hex values (0x00, 0x01, 0x02, ... 0xFF, repeating)
[playground::DATA("static/hex_demo.bin")]
StructuredBuffer<uint4> hexData;

// Helper function to get a byte from the packed uint4 data
uint getByte(int byteIndex)
{
    int elementIndex = byteIndex / 16;         // Which uint4 element (16 bytes each)
    int remainingBytes = byteIndex % 16;       // Byte offset within the uint4
    int componentIndex = remainingBytes / 4;   // Which component (x,y,z,w)
    int byteInComponent = remainingBytes % 4;  // Which byte within the uint32
    
    uint4 element = hexData[elementIndex];
    uint component;
    
    if (componentIndex == 0) component = element.x;
    else if (componentIndex == 1) component = element.y;
    else if (componentIndex == 2) component = element.z;
    else component = element.w;
    
    // Extract the specific byte from the uint32 component (little-endian)
    return (component >> (byteInComponent * 8)) & 0xFF;
}

void printMain()
{
    const int totalBytes = hexData.getCount() * 16;
    const int displayBytes = 256;  // Show first 256 bytes to stay within printf buffer
    
    print("=== playground::DATA Hexdump Demo ===\n");
    print("Loaded %d bytes of ascending hex data\n", totalBytes);
    print("Displaying first %d bytes in hexdump format:\n\n", displayBytes);
    
    // Display data in hexdump format (16 bytes per line)
    for (int offset = 0; offset < displayBytes; offset += 16)
    {
        // Print offset
        print("%08x  ", offset);
        
        // Print hex values (16 bytes per line)
        for (int i = 0; i < 16; ++i)
        {
            if (offset + i < totalBytes && offset + i < displayBytes)
            {
                uint byte_val = getByte(offset + i);
                print("%02x ", byte_val);
            }
            else
            {
                print("   ");  // Padding for incomplete lines
            }
            
            // Add extra space after 8 bytes for readability
            if (i == 7) print(" ");
        }
        
        print(" |");
        
        // Print ASCII representation
        for (int i = 0; i < 16; ++i)
        {
            if (offset + i < totalBytes && offset + i < displayBytes)
            {
                uint byte_val = getByte(offset + i);
                if (byte_val >= 32 && byte_val <= 126)
                    print("%c", byte_val);
                else
                    print(".");
            }
            else
            {
                print(" ");
            }
        }
        print("|\n");
    }
    
    print("\n=== Data Validation ===\n");
    print("Expected pattern: 0x00, 0x01, 0x02, ... 0xFF, 0x00, 0x01, ...\n");
    print("First few bytes: ");
    for (int i = 0; i < 8; ++i)
    {
        print("0x%02x ", getByte(i));
    }
    print("\n");
    
    print("Bytes 252-259: ");
    for (int i = 252; i < 260 && i < totalBytes; ++i)
    {
        print("0x%02x ", getByte(i));
    }
    print("\n");
}