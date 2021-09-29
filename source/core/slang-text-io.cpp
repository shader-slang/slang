#include "slang-text-io.h"

namespace Slang
{

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! StreamWriter !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

StreamWriter::StreamWriter(const String& path, CharEncoding* encoding)
{
    FileStream* fileStream = new FileStream;
    m_stream = fileStream;

    if (SLANG_FAILED(fileStream->init(path, FileMode::Create)))
    {
        StringBuilder buf;
        buf << "Unable to open '" << path << "'";
        throw IOException(buf.ProduceString());
    }

	m_encoding = encoding;
	if (encoding == CharEncoding::UTF16)
	{
		m_stream->write(&kUTF16Header, 2);
	}
	else if (encoding == CharEncoding::UTF16Reversed)
	{
		m_stream->write(&kUTF16ReversedHeader, 2);
	}
}

StreamWriter::StreamWriter(RefPtr<Stream> stream, CharEncoding* encoding)
{
	m_stream = stream;
	m_encoding = encoding;
	if (encoding == CharEncoding::UTF16)
	{
		m_stream->write(&kUTF16Header, 2);
	}
	else if (encoding == CharEncoding::UTF16Reversed)
	{
		m_stream->write(&kUTF16ReversedHeader, 2);
	}
}

void StreamWriter::writeSlice(const UnownedStringSlice& slice)
{
	m_encodingBuffer.clear();

    StringBuilder sb;
#ifdef _WIN32
	const char newLine[] = "\r\n";
#else
	const char newLine[] = "\n";
#endif
    const Index length = slice.getLength();
    
	for (Index i = 0; i < length; i++)
	{
		if (slice[i] == '\r')
			sb << newLine;
		else if (slice[i] == '\n')
		{
			if (i > 0 && slice[i - 1] != '\r')
				sb << newLine;
		}
		else
			sb << slice[i];
	}

	m_encoding->GetBytes(m_encodingBuffer, sb.ProduceString());
	m_stream->write(m_encodingBuffer.getBuffer(), m_encodingBuffer.getCount());
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! StreamReader !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

StreamReader::StreamReader(const String& path)
{
    FileStream* fileStream = new FileStream;
    m_stream = fileStream;

    if (SLANG_FAILED(fileStream->init(path, FileMode::Open)))
    {
        StringBuilder buf;
        buf << "Unable to open '" << path << "'";
        throw IOException(buf.ProduceString());
    }

	readBuffer();
	m_encoding = determineEncoding();

	if (m_encoding == nullptr)
		m_encoding = CharEncoding::UTF8;
}

StreamReader::StreamReader(RefPtr<Stream> stream, CharEncoding * encoding)
{
    m_stream = stream;
    m_encoding = encoding;
	readBuffer();
	auto determinedEncoding = determineEncoding();
	if (m_encoding == nullptr)
        m_encoding = determinedEncoding;
}

static bool _hasNullBytes(char * str, int len)
{
    bool hasSeenNull = false;
    for (int i = 0; i < len - 1; i++)
        if (str[i] == 0)
            hasSeenNull = true;
        else if (hasSeenNull)
            return true;
    return false;
}

CharEncoding* StreamReader::determineEncoding()
{
    // TODO(JS): Assumes the bytes are suitably aligned

	if (m_buffer.getCount() >= 3 && (unsigned char)(m_buffer[0]) == 0xEF && (unsigned char)(m_buffer[1]) == 0xBB && (unsigned char)(m_buffer[2]) == 0xBF)
	{
		m_index += 3;
		return CharEncoding::UTF8;
	}
	else if (*((unsigned short*)(m_buffer.getBuffer())) == 0xFEFF)
	{
        m_index += 2;
		return CharEncoding::UTF16;
	}
	else if (*((unsigned short*)(m_buffer.getBuffer())) == 0xFFFE)
	{
        m_index += 2;
		return CharEncoding::UTF16Reversed;
	}
	else
	{
        // find null bytes
        if (_hasNullBytes(m_buffer.getBuffer(), (int)m_buffer.getCount()))
        {
            return CharEncoding::UTF16;
        }
		return CharEncoding::UTF8;
	}
}
		
void StreamReader::readBuffer()
{
    if (m_stream->isEnd())
    {
        throw EndOfStreamException();
    }

	m_buffer.setCount(4096);
    memset(m_buffer.getBuffer(), 0, m_buffer.getCount() * sizeof(m_buffer[0]));

    size_t readBytes;
    if (SLANG_FAILED(m_stream->read(m_buffer.getBuffer(), m_buffer.getCount(), readBytes)))
    {
        throw IOException("Error reading from stream");
    }

	m_buffer.setCount(Index(readBytes));
	m_index = 0;
}

char StreamReader::readBufferChar()
{
	if (m_index < m_buffer.getCount())
	{
		return m_buffer[m_index++];
	}
	if (!m_stream->isEnd())
		readBuffer();
	if (m_index < m_buffer.getCount())
	{
		return m_buffer[m_index++];
	}
	return 0;
}
int TextReader::read(char * destBuffer, int length)
{
	int i = 0;
	for (i = 0; i < length; i++)
	{
		try
		{
			auto ch = read();
			if (isEnd())
				break;
			if (ch == '\r')
			{
				if (peek() == '\n')
					read();
				break;
			}
			else if (ch == '\n')
			{
				break;
			}
			destBuffer[i] = ch;
		}
		catch (const EndOfStreamException& )
		{
			break;
		}
	}
	return i;
}

String StreamReader::readLine()
{
	StringBuilder sb(256);
	while (!isEnd())
	{
		try
		{
			auto ch = read();
			if (isEnd())
				break;
			if (ch == '\r')
			{
				if (peek() == '\n')
					read();
				break;
			}
			else if (ch == '\n')
			{
				break;
			}
			sb.Append(ch);
		}
		catch (const EndOfStreamException&)
		{
			break;
		}
	}
	return sb.ProduceString();
}

String StreamReader::readToEnd()
{
	StringBuilder sb(16384);
	while (!isEnd())
	{
		try
		{
			auto ch = read();
			if (isEnd())
				break;
			if (ch == '\r')
			{
				sb.Append('\n');
				if (peek() == '\n')
					read();
			}
			else
				sb.Append(ch);
		}
		catch (const EndOfStreamException&)
		{
			break;
		}
	}
	return sb.ProduceString();
}

} // namespace Slang
