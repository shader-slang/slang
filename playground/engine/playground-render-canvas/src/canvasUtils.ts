import { HashedStringData } from "slang-playground-shared";

export class NotReadyError extends Error {
    constructor(message: string) {
        super(message);
    }
}

export function isWebGPUSupported() {
    return 'gpu' in navigator && navigator.gpu !== null;
}

export function sizeFromFormat(format: GPUTextureFormat) {
    switch (format) {
        case "r8unorm":
        case "r8snorm":
        case "r8uint":
        case "r8sint":
            return 1;
        case "r16uint":
        case "r16sint":
        case "r16float":
        case "rg8unorm":
        case "rg8snorm":
        case "rg8uint":
        case "rg8sint":
            return 2;
        case "r32uint":
        case "r32sint":
        case "r32float":
        case "rg16uint":
        case "rg16sint":
        case "rg16float":
        case "rgba8unorm":
        case "rgba8unorm-srgb":
        case "rgba8snorm":
        case "rgba8uint":
        case "rgba8sint":
        case "bgra8unorm":
        case "bgra8unorm-srgb":
        case "rgb10a2unorm":
        case "rg11b10ufloat":
        case "rgb9e5ufloat":
            return 4;
        case "rg32uint":
        case "rg32sint":
        case "rg32float":
        case "rgba16uint":
        case "rgba16sint":
        case "rgba16float":
            return 8;
        case "rgba32uint":
        case "rgba32sint":
        case "rgba32float":
            return 16;
        default:
            throw new Error(`Could not get size of unrecognized format "${format}"`)
    }
}

function reinterpretUint32AsFloat(uint32: number) {
    const buffer = new ArrayBuffer(4);
    const uint32View = new Uint32Array(buffer);
    const float32View = new Float32Array(buffer);

    uint32View[0] = uint32;
    return float32View[0];
}

type FormatSpecifier = {
    type: "text",
    value: string,
} | {
    type: "specifier",
    flags: string,
    width: number | null,
    precision: number | null,
    specifierType: string,
};

function parsePrintfFormat(formatString: string): FormatSpecifier[] {
    const formatSpecifiers: FormatSpecifier[] = [];
    const regex = /%([-+ #0]*)(\d*)(\.\d+)?([diufFeEgGxXosc])/g;
    let lastIndex = 0;

    let match;
    while ((match = regex.exec(formatString)) !== null) {
        const [fullMatch, flags, width, precision, type] = match;
        const literalText = formatString.slice(lastIndex, match.index);

        // Add literal text before the match as a token, if any
        if (literalText) {
            formatSpecifiers.push({ type: 'text', value: literalText });
        }

        let precision_text = precision ? precision.slice(1) : null; // remove leading '.'
        let precision_number = precision_text ? parseInt(precision_text) : null;

        let width_number = width ? parseInt(width) : null;

        // Add the format specifier as a token
        formatSpecifiers.push({
            type: 'specifier',
            flags: flags || '',
            width: width_number,
            precision: precision_number,
            specifierType: type
        });

        lastIndex = regex.lastIndex;
    }

    // Add any remaining literal text after the last match
    if (lastIndex < formatString.length) {
        formatSpecifiers.push({ type: 'text', value: formatString.slice(lastIndex) });
    }

    return formatSpecifiers;
}

// // Example usage
// const formatString = "Int: %d, Unsigned: %u, Hex: %X, Float: %8.2f, Sci: %e, Char: %c, Str: %.5s!";
// const parsedTokens = parsePrintfFormat(formatString);
// const data = [42, -42, 255, 3.14159, 0.00001234, 65, "Hello, world"];
// const output = formatPrintfString(parsedTokens, data);
// console.log(output);
function formatPrintfString(parsedTokens: FormatSpecifier[], data: any[]) {
    let result = '';
    let dataIndex = 0;

    parsedTokens.forEach((token) => {
        if (token.type === 'text') {
            result += token.value;
        }
        else if (token.type === 'specifier') {
            const value = data[dataIndex++];
            result += formatSpecifier(value, token);
        }
    });

    return result;
}

// Helper function to format each specifier
function formatSpecifier(value: string, { flags, width, precision, specifierType }: FormatSpecifier & { type: 'specifier' }) {
    let formattedValue;
    let specifiedPrecision = precision != null;
    if (precision == null)
        precision = 6; //eww magic number
    switch (specifierType) {
        case 'd':
        case 'i': // Integer (decimal)
            if (typeof value !== "number") throw new Error("Invalid state");

            let valueAsSignedInteger = value | 0;
            formattedValue = valueAsSignedInteger.toString();
            break;
        case 'u': // Unsigned integer
            formattedValue = Math.abs(parseInt(value)).toString();
            break;
        case 'o': // Octal
            formattedValue = Math.abs(parseInt(value)).toString(8);
            break;
        case 'x': // Hexadecimal (lowercase)
            formattedValue = Math.abs(parseInt(value)).toString(16);
            break;
        case 'X': // Hexadecimal (uppercase)
            formattedValue = Math.abs(parseInt(value)).toString(16).toUpperCase();
            break;
        case 'f':
        case 'F': // Floating-point
            formattedValue = parseFloat(value).toFixed(precision);
            break;
        case 'e': // Scientific notation (lowercase)
            formattedValue = parseFloat(value).toExponential(precision);
            break;
        case 'E': // Scientific notation (uppercase)
            formattedValue = parseFloat(value).toExponential(precision).toUpperCase();
            break;
        case 'g':
        case 'G': // Shortest representation of floating-point
            formattedValue = parseFloat(value).toPrecision(precision);
            break;
        case 'c': // Character
            formattedValue = String.fromCharCode(parseInt(value));
            break;
        case 's': // String
            formattedValue = String(value);
            if (specifiedPrecision) {
                formattedValue = formattedValue.slice(0, precision);
            }
            break;
        case '%': // Literal '%'
            return '%';
        default:
            throw new Error(`Unsupported specifier: ${specifierType}`);
    }

    // Handle width and flags (like zero-padding, space, left alignment, sign)
    if (width) {
        const paddingChar = flags.includes('0') && !flags.includes('-') ? '0' : ' ';
        const isLeftAligned = flags.includes('-');
        const needsSign = flags.includes('+') && parseFloat(value) >= 0;
        const needsSpace = flags.includes(' ') && !needsSign && parseFloat(value) >= 0;

        if (needsSign) {
            formattedValue = '+' + formattedValue;
        }
        else if (needsSpace) {
            formattedValue = ' ' + formattedValue;
        }

        if (formattedValue.length < width) {
            const padding = paddingChar.repeat(width - formattedValue.length);
            formattedValue = isLeftAligned ? formattedValue + padding : padding + formattedValue;
        }
    }

    return formattedValue;
}

// This is the definition of the printf buffer.
// struct FormattedStruct
// {
//     uint32_t type = 0xFFFFFFFF;
//     uint32_t low = 0;
//     uint32_t high = 0;
// };
//

export function parsePrintfBuffer(hashedStrings: HashedStringData, printfValueResource: GPUBuffer, bufferElementSize: number) {
    // Read the printf buffer
    const printfBufferArray = new Uint32Array(printfValueResource.getMappedRange())

    let elementIndex = 0;
    let numberElements = printfBufferArray.byteLength / bufferElementSize;

    // TODO: We currently doesn't support 64-bit data type (e.g. uint64_t, int64_t, double, etc.)
    // so 32-bit array should be able to contain everything we need.
    let dataArray = [];
    const elementSizeInWords = bufferElementSize / 4;
    let outStrArry = [];
    let formatString = "";
    for (elementIndex = 0; elementIndex < numberElements; elementIndex++) {
        let offset = elementIndex * elementSizeInWords;
        const type = printfBufferArray[offset];
        switch (type) {
            case 1: // format string
                formatString = hashedStrings[(printfBufferArray[offset + 1] << 0)]!;  // low field
                break;
            case 2: // normal string
                dataArray.push(hashedStrings[(printfBufferArray[offset + 1] << 0)]);  // low field
                break;
            case 3: // integer
                dataArray.push(printfBufferArray[offset + 1]);  // low field
                break;
            case 4: // float
                const floatData = reinterpretUint32AsFloat(printfBufferArray[offset + 1]);
                dataArray.push(floatData);                      // low field
                break;
            case 5: // TODO: We can't handle 64-bit data type yet.
                dataArray.push(0);                              // low field
                break;
            case 0xFFFFFFFF:
                {
                    const parsedTokens = parsePrintfFormat(formatString);
                    const output = formatPrintfString(parsedTokens, dataArray);
                    outStrArry.push(output);
                    formatString = "";
                    dataArray = [];
                    if (elementIndex < numberElements - 1) {
                        const nextOffset = offset + elementSizeInWords;
                        // advance to the next element to see if it's a format string, if it's not we just early return
                        // the results, otherwise just continue processing.
                        if (printfBufferArray[nextOffset] != 1)          // type field
                        {
                            return outStrArry;
                        }
                    }
                    break;
                }
        }
    }

    if (formatString != "") {
        // If we are here, it means that the printf buffer is used up, and we are in the middle of processing
        // one printf string, so we are still going to format it, even though there could be some data missing, which
        // will be shown as 'undef'.
        const parsedTokens = parsePrintfFormat(formatString);
        const output = formatPrintfString(parsedTokens, dataArray);
        outStrArry.push(output);
        outStrArry.push("Print buffer is out of boundary, some data is missing!!!");
    }

    return outStrArry;
}