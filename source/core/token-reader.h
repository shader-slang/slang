#ifndef CORE_TOKEN_READER_H
#define CORE_TOKEN_READER_H

#include "basic.h"

namespace Slang
{
    enum class TokenType
    {
        EndOfFile = -1,
        // illegal
        Unknown,
        // identifier
        Identifier,
        // constant
        IntLiteral, DoubleLiteral, StringLiteral, CharLiteral,
        // operators
        Semicolon, Comma, Dot, LBrace, RBrace, LBracket, RBracket, LParent, RParent,
        OpAssign, OpAdd, OpSub, OpMul, OpDiv, OpMod, OpNot, OpBitNot, OpLsh, OpRsh,
        OpEql, OpNeq, OpGreater, OpLess, OpGeq, OpLeq,
        OpAnd, OpOr, OpBitXor, OpBitAnd, OpBitOr,
        OpInc, OpDec, OpAddAssign, OpSubAssign, OpMulAssign, OpDivAssign, OpModAssign,
        OpShlAssign, OpShrAssign, OpOrAssign, OpAndAssign, OpXorAssign,

        QuestionMark, Colon, RightArrow, At, Pound, PoundPound, Scope,
    };

    class CodePosition
    {
    public:
        int Line = -1, Col = -1, Pos = -1;
        String FileName;
        String ToString()
        {
            StringBuilder sb(100);
            sb << FileName;
            if (Line != -1)
                sb << "(" << Line << ")";
            return sb.ProduceString();
        }
        CodePosition() = default;
        CodePosition(int line, int col, int pos, String fileName)
        {
            Line = line;
            Col = col;
            Pos = pos;
            this->FileName = fileName;
        }
        bool operator < (const CodePosition & pos) const
        {
            return FileName < pos.FileName || (FileName == pos.FileName && Line < pos.Line) ||
                (FileName == pos.FileName && Line == pos.Line && Col < pos.Col);
        }
        bool operator == (const CodePosition & pos) const
        {
            return FileName == pos.FileName && Line == pos.Line && Col == pos.Col;
        }
    };

    enum TokenFlag : unsigned int
    {
        AtStartOfLine = 1 << 0,
        AfterWhitespace = 1 << 1,
    };
    typedef unsigned int TokenFlags;

    class Token
    {
    public:
        TokenType Type = TokenType::Unknown;
        String Content;
        CodePosition Position;
        TokenFlags flags;
        Token() = default;
        Token(TokenType type, const String & content, int line, int col, int pos, String fileName, TokenFlags flags = 0)
            : flags(flags)
        {
            Type = type;
            Content = content;
            Position = CodePosition(line, col, pos, fileName);
        }
    };

    class TextFormatException : public Exception
    {
    public:
        TextFormatException(String message)
            : Exception(message)
        {}
    };

    class TokenReader
    {
    private:
        bool legal;
        List<Token> tokens;
        int tokenPtr;
    public:
        TokenReader(String text);
        int ReadInt()
        {
            auto token = ReadToken();
            bool neg = false;
            if (token.Content == '-')
            {
                neg = true;
                token = ReadToken();
            }
            if (token.Type == TokenType::IntLiteral)
            {
                if (neg)
                    return -StringToInt(token.Content);
                else
                    return StringToInt(token.Content);
            }
            throw TextFormatException("Text parsing error: int expected.");
        }
        unsigned int ReadUInt()
        {
            auto token = ReadToken();
            if (token.Type == TokenType::IntLiteral)
            {
                return StringToUInt(token.Content);
            }
            throw TextFormatException("Text parsing error: int expected.");
        }
        double ReadDouble()
        {
            auto token = ReadToken();
            bool neg = false;
            if (token.Content == '-')
            {
                neg = true;
                token = ReadToken();
            }
            if (token.Type == TokenType::DoubleLiteral || token.Type == TokenType::IntLiteral)
            {
                if (neg)
                    return -StringToDouble(token.Content);
                else
                    return StringToDouble(token.Content);
            }
            throw TextFormatException("Text parsing error: floating point value expected.");
        }
        float ReadFloat()
        {
            return (float)ReadDouble();
        }
        String ReadWord()
        {
            auto token = ReadToken();
            if (token.Type == TokenType::Identifier)
            {
                return token.Content;
            }
            throw TextFormatException("Text parsing error: identifier expected.");
        }
        String Read(const char * expectedStr)
        {
            auto token = ReadToken();
            if (token.Content == expectedStr)
            {
                return token.Content;
            }
            throw TextFormatException("Text parsing error: \'" + String(expectedStr) + "\' expected.");
        }
        String Read(String expectedStr)
        {
            auto token = ReadToken();
            if (token.Content == expectedStr)
            {
                return token.Content;
            }
            throw TextFormatException("Text parsing error: \'" + expectedStr + "\' expected.");
        }

        String ReadStringLiteral()
        {
            auto token = ReadToken();
            if (token.Type == TokenType::StringLiteral)
            {
                return token.Content;
            }
            throw TextFormatException("Text parsing error: string literal expected.");
        }
        void Back(int count)
        {
            tokenPtr -= count;
        }
        Token ReadToken()
        {
            if (tokenPtr < (int)tokens.Count())
            {
                auto &rs = tokens[tokenPtr];
                tokenPtr++;
                return rs;
            }
            throw TextFormatException("Unexpected ending.");
        }
        Token NextToken(int offset = 0)
        {
            if (tokenPtr + offset < (int)tokens.Count())
                return tokens[tokenPtr + offset];
            else
            {
                Token rs;
                rs.Type = TokenType::Unknown;
                return rs;
            }
        }
        bool LookAhead(String token)
        {
            if (tokenPtr < (int)tokens.Count())
            {
                auto next = NextToken();
                return next.Content == token;
            }
            else
            {
                return false;
            }
        }
        bool IsEnd()
        {
            return tokenPtr == (int)tokens.Count();
        }
    public:
        bool IsLegalText()
        {
            return legal;
        }
    };

    inline List<String> Split(String text, char c)
    {
        List<String> result;
        StringBuilder sb;
        for (int i = 0; i < (int)text.Length(); i++)
        {
            if (text[i] == c)
            {
                auto str = sb.ToString();
                if (str.Length() != 0)
                    result.Add(str);
                sb.Clear();
            }
            else
                sb << text[i];
        }
        auto lastStr = sb.ToString();
        if (lastStr.Length())
            result.Add(lastStr);
        return result;
    }
}


#endif