// source-loc.h
#ifndef SLANG_SOURCE_LOC_H_INCLUDED
#define SLANG_SOURCE_LOC_H_INCLUDED

#include "../core/basic.h"

namespace Slang {

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


} // namespace Slang

#endif
