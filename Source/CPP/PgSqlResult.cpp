#include "PgSqlResult.hpp"
#include "../../../LangShared/InternalString/CPP/InternalString.hpp"
#include <cstdlib>
#include <cstring>

namespace NumbatLogic
{
	static const char* GetCell(PGresult* pResult, int nRow, int nCol)
	{
		if (!pResult || nRow < 0 || nCol < 0 ||
			nRow >= PQntuples(pResult) || nCol >= PQnfields(pResult))
			return nullptr;
		const char* v = PQgetvalue(pResult, nRow, nCol);
		return v ? v : "";
	}
	PgSqlResult::PgSqlResult(PGresult* pResult)
		: m_pResult(pResult)
	{
	}

	PgSqlResult::~PgSqlResult()
	{
		if (m_pResult)
		{
			PQclear(m_pResult);
			m_pResult = nullptr;
		}
	}

	int PgSqlResult::GetRowCount()
	{
		return m_pResult ? PQntuples(m_pResult) : 0;
	}

	int PgSqlResult::GetColumnCount()
	{
		return m_pResult ? PQnfields(m_pResult) : 0;
	}

	bool PgSqlResult::GetString(int nRow, int nCol, InternalString* pOut)
	{
		const char* v = GetCell(m_pResult, nRow, nCol);
		if (!pOut || !v)
			return false;
		pOut->Set(v);
		return true;
	}

	bool PgSqlResult::GetBool(int nRow, int nCol, bool& outVal)
	{
		const char* v = GetCell(m_pResult, nRow, nCol);
		if (!v)
			return false;
		if (v[0] == 't' || v[0] == 'T' || (v[0] == '1' && v[1] == '\0'))
		{ outVal = true; return true; }
		if (v[0] == 'f' || v[0] == 'F' || v[0] == '0')
		{ outVal = false; return true; }
		if (strcmp(v, "true") == 0 || strcmp(v, "yes") == 0)
		{ outVal = true; return true; }
		if (strcmp(v, "false") == 0 || strcmp(v, "no") == 0)
		{ outVal = false; return true; }
		return false;
	}

	bool PgSqlResult::GetInt8(int nRow, int nCol, signed char& outVal)
	{
		const char* v = GetCell(m_pResult, nRow, nCol);
		if (!v || v[0] == '\0')
			return false;
		char* end = nullptr;
		long n = strtol(v, &end, 10);
		if (end == v || *end != '\0' || n < -128 || n > 127)
			return false;
		outVal = static_cast<signed char>(n);
		return true;
	}

	bool PgSqlResult::GetInt16(int nRow, int nCol, short& outVal)
	{
		const char* v = GetCell(m_pResult, nRow, nCol);
		if (!v || v[0] == '\0')
			return false;
		char* end = nullptr;
		long n = strtol(v, &end, 10);
		if (end == v || *end != '\0' || n < -32768 || n > 32767)
			return false;
		outVal = static_cast<short>(n);
		return true;
	}

	bool PgSqlResult::GetInt32(int nRow, int nCol, int& outVal)
	{
		const char* v = GetCell(m_pResult, nRow, nCol);
		if (!v || v[0] == '\0')
			return false;
		char* end = nullptr;
		long n = strtol(v, &end, 10);
		if (end == v || *end != '\0' || n < -2147483648L || n > 2147483647L)
			return false;
		outVal = static_cast<int>(n);
		return true;
	}

	bool PgSqlResult::GetUint8(int nRow, int nCol, unsigned char& outVal)
	{
		const char* v = GetCell(m_pResult, nRow, nCol);
		if (!v || v[0] == '\0')
			return false;
		char* end = nullptr;
		unsigned long n = strtoul(v, &end, 10);
		if (end == v || *end != '\0' || n > 255)
			return false;
		outVal = static_cast<unsigned char>(n);
		return true;
	}

	bool PgSqlResult::GetUint16(int nRow, int nCol, unsigned short& outVal)
	{
		const char* v = GetCell(m_pResult, nRow, nCol);
		if (!v || v[0] == '\0')
			return false;
		char* end = nullptr;
		unsigned long n = strtoul(v, &end, 10);
		if (end == v || *end != '\0' || n > 65535)
			return false;
		outVal = static_cast<unsigned short>(n);
		return true;
	}

	bool PgSqlResult::GetUint32(int nRow, int nCol, unsigned int& outVal)
	{
		const char* v = GetCell(m_pResult, nRow, nCol);
		if (!v || v[0] == '\0')
			return false;
		char* end = nullptr;
		unsigned long n = strtoul(v, &end, 10);
		if (end == v || *end != '\0' || n > 4294967295UL)
			return false;
		outVal = static_cast<unsigned int>(n);
		return true;
	}

	bool PgSqlResult::GetFloat(int nRow, int nCol, float& outVal)
	{
		const char* v = GetCell(m_pResult, nRow, nCol);
		if (!v || v[0] == '\0')
			return false;
		char* end = nullptr;
		double d = strtod(v, &end);
		if (end == v)
			return false;
		outVal = static_cast<float>(d);
		return true;
	}

	bool PgSqlResult::GetDouble(int nRow, int nCol, double& outVal)
	{
		const char* v = GetCell(m_pResult, nRow, nCol);
		if (!v || v[0] == '\0')
			return false;
		char* end = nullptr;
		outVal = strtod(v, &end);
		if (end == v)
			return false;
		return true;
	}
}
