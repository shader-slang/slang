#include "slang-text-io.h"

namespace Slang
{
	
	StreamWriter::StreamWriter(const String & path, CharEncoding * encoding)
	{
        FileStream* fileStream = new FileStream;
        this->stream = fileStream;

        if (SLANG_FAILED(fileStream->init(path, FileMode::Create)))
        {
            StringBuilder buf;
            buf << "Unable to open '" << path << "'";
            throw IOException(buf.ProduceString());
        }
		this->encoding = encoding;
		if (encoding == CharEncoding::UTF16)
		{
			this->stream->write(&kUTF16Header, 2);
		}
		else if (encoding == CharEncoding::UTF16Reversed)
		{
			this->stream->write(&kUTF16ReversedHeader, 2);
		}
	}
	StreamWriter::StreamWriter(RefPtr<Stream> stream, CharEncoding * encoding)
	{
		this->stream = stream;
		this->encoding = encoding;
		if (encoding == CharEncoding::UTF16)
		{
			this->stream->write(&kUTF16Header, 2);
		}
		else if (encoding == CharEncoding::UTF16Reversed)
		{
			this->stream->write(&kUTF16ReversedHeader, 2);
		}
	}
	void StreamWriter::Write(const String & str)
	{
		encodingBuffer.clear();
        StringBuilder sb;
#ifdef _WIN32
		const char newLine[] = "\r\n";
#else
		const char newLine = "\n";
#endif
		for (Index i = 0; i < str.getLength(); i++)
		{
			if (str[i] == '\r')
				sb << newLine;
			else if (str[i] == '\n')
			{
				if (i > 0 && str[i - 1] != '\r')
					sb << newLine;
			}
			else
				sb << str[i];
		}
		encoding->GetBytes(encodingBuffer, sb.ProduceString());
		stream->write(encodingBuffer.getBuffer(), encodingBuffer.getCount());
	}
	void StreamWriter::Write(const char * str)
	{
		Write(String(str));
	}

	StreamReader::StreamReader(const String & path)
	{
        FileStream* fileStream = new FileStream;
        this->stream = fileStream;

        if (SLANG_FAILED(fileStream->init(path, FileMode::Open)))
        {
            StringBuilder buf;
            buf << "Unable to open '" << path << "'";
            throw IOException(buf.ProduceString());
        }

		ReadBuffer();
		encoding = DetermineEncoding();
		if (encoding == nullptr)
			encoding = CharEncoding::UTF8;
	}

	StreamReader::StreamReader(RefPtr<Stream> stream, CharEncoding * encoding)
	{
		this->stream = stream;
		this->encoding = encoding;
		ReadBuffer();
		auto determinedEncoding = DetermineEncoding();
		if (this->encoding == nullptr)
			this->encoding = determinedEncoding;
	}

    bool HasNullBytes(char * str, int len)
    {
        bool hasSeenNull = false;
        for (int i = 0; i < len - 1; i++)
            if (str[i] == 0)
                hasSeenNull = true;
            else if (hasSeenNull)
                return true;
        return false;
    }

	CharEncoding* StreamReader::DetermineEncoding()
	{
        // TODO(JS): Assumes the bytes are suitably aligned

		if (buffer.getCount() >= 3 && (unsigned char)(buffer[0]) == 0xEF && (unsigned char)(buffer[1]) == 0xBB && (unsigned char)(buffer[2]) == 0xBF)
		{
			ptr += 3;
			return CharEncoding::UTF8;
		}
		else if (*((unsigned short*)(buffer.getBuffer())) == 0xFEFF)
		{
			ptr += 2;
			return CharEncoding::UTF16;
		}
		else if (*((unsigned short*)(buffer.getBuffer())) == 0xFFFE)
		{
			ptr += 2;
			return CharEncoding::UTF16Reversed;
		}
		else
		{
            // find null bytes
            if (HasNullBytes(buffer.getBuffer(), (int)buffer.getCount()))
            {
                return CharEncoding::UTF16;
            }
			return CharEncoding::UTF8;
		}
	}
		
	void StreamReader::ReadBuffer()
	{
        if (stream->isEnd())
        {
            throw EndOfStreamException();
        }

		buffer.setCount(4096);
        memset(buffer.getBuffer(), 0, buffer.getCount() * sizeof(buffer[0]));

        size_t readBytes;
        if (SLANG_FAILED(stream->read(buffer.getBuffer(), buffer.getCount(), readBytes)))
        {
            throw IOException("Error reading from stream");
        }

		buffer.setCount(Index(readBytes));
		ptr = 0;
	}

	char StreamReader::ReadBufferChar()
	{
		if (ptr<buffer.getCount())
		{
			return buffer[ptr++];
		}
		if (!stream->isEnd())
			ReadBuffer();
		if (ptr<buffer.getCount())
		{
			return buffer[ptr++];
		}
		return 0;
	}
	int TextReader::Read(char * destBuffer, int length)
	{
		int i = 0;
		for (i = 0; i<length; i++)
		{
			try
			{
				auto ch = Read();
				if (IsEnd())
					break;
				if (ch == '\r')
				{
					if (peek() == '\n')
						Read();
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
	String StreamReader::ReadLine()
	{
		StringBuilder sb(256);
		while (!IsEnd())
		{
			try
			{
				auto ch = Read();
				if (IsEnd())
					break;
				if (ch == '\r')
				{
					if (peek() == '\n')
						Read();
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
	String StreamReader::ReadToEnd()
	{
		StringBuilder sb(16384);
		while (!IsEnd())
		{
			try
			{
				auto ch = Read();
				if (IsEnd())
					break;
				if (ch == '\r')
				{
					sb.Append('\n');
					if (peek() == '\n')
						Read();
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
}
