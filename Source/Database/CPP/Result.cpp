#include "Result.hpp"
#include "../../../../LangShared/Source/InternalString/CPP/InternalString.hpp"
#include "../../../../LangShared/Source/Blob/CPP/Blob.hpp"
#include <arpa/inet.h>
#include <cstdint>
#include <cstring>
#include <limits>

namespace NumbatLogic
{
	namespace Database
	{
		static const Oid BOOLOID = 16;
		static const Oid INT2OID = 21;
		static const Oid INT4OID = 23;
		static const Oid INT8OID = 20;
		static const Oid FLOAT4OID = 700;
		static const Oid FLOAT8OID = 701;

		static uint64_t NetworkToHost64(uint64_t nValue)
		{
			#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
				return ((nValue & 0x00000000000000FFULL) << 56) |
					((nValue & 0x000000000000FF00ULL) << 40) |
					((nValue & 0x0000000000FF0000ULL) << 24) |
					((nValue & 0x00000000FF000000ULL) << 8) |
					((nValue & 0x000000FF00000000ULL) >> 8) |
					((nValue & 0x0000FF0000000000ULL) >> 24) |
					((nValue & 0x00FF000000000000ULL) >> 40) |
					((nValue & 0xFF00000000000000ULL) >> 56);
			#else
				return nValue;
			#endif
		}

		static bool IsValidCell(PGresult* pResult, int nRow, int nCol)
		{
			return pResult && nRow >= 0 && nCol >= 0 &&
				nRow < PQntuples(pResult) && nCol < PQnfields(pResult);
		}

		static bool GetCellData(PGresult* pResult, int nRow, int nCol, const unsigned char*& pData, int& nLen)
		{
			pData = nullptr;
			nLen = 0;
			if (!IsValidCell(pResult, nRow, nCol) || PQgetisnull(pResult, nRow, nCol))
				return false;
			pData = reinterpret_cast<const unsigned char*>(PQgetvalue(pResult, nRow, nCol));
			nLen = PQgetlength(pResult, nRow, nCol);
			return pData != nullptr && nLen >= 0;
		}

		static bool TryGetBinaryInt64(PGresult* pResult, int nRow, int nCol, int64_t& outVal)
		{
			const unsigned char* pData = nullptr;
			int nLen = 0;
			if (!GetCellData(pResult, nRow, nCol, pData, nLen) || PQfformat(pResult, nCol) != 1)
				return false;

			Oid nType = PQftype(pResult, nCol);
			if (nType == INT2OID && nLen == 2)
			{
				uint16_t v;
				memcpy(&v, pData, sizeof(v));
				outVal = static_cast<int16_t>(ntohs(v));
				return true;
			}
			if (nType == INT4OID && nLen == 4)
			{
				uint32_t v;
				memcpy(&v, pData, sizeof(v));
				outVal = static_cast<int32_t>(ntohl(v));
				return true;
			}
			if (nType == INT8OID && nLen == 8)
			{
				uint64_t v;
				memcpy(&v, pData, sizeof(v));
				outVal = static_cast<int64_t>(NetworkToHost64(v));
				return true;
			}
			return false;
		}

		static bool TryGetBinaryDouble(PGresult* pResult, int nRow, int nCol, double& outVal)
		{
			const unsigned char* pData = nullptr;
			int nLen = 0;
			if (!GetCellData(pResult, nRow, nCol, pData, nLen) || PQfformat(pResult, nCol) != 1)
				return false;

			Oid nType = PQftype(pResult, nCol);
			if (nType == FLOAT4OID && nLen == 4)
			{
				uint32_t raw = 0;
				memcpy(&raw, pData, sizeof(raw));
				raw = ntohl(raw);
				float f = 0.0f;
				memcpy(&f, &raw, sizeof(f));
				outVal = static_cast<double>(f);
				return true;
			}
			if (nType == FLOAT8OID && nLen == 8)
			{
				uint64_t raw = 0;
				memcpy(&raw, pData, sizeof(raw));
				raw = NetworkToHost64(raw);
				memcpy(&outVal, &raw, sizeof(outVal));
				return true;
			}
			return false;
		}

		Result::Result(PGresult* pResult)
			: m_pResult(pResult)
		{
		}

		Result::~Result()
		{
			if (m_pResult)
			{
				PQclear(m_pResult);
				m_pResult = nullptr;
			}
		}

		int Result::GetRowCount()
		{
			return m_pResult ? PQntuples(m_pResult) : 0;
		}

		int Result::GetColumnCount()
		{
			return m_pResult ? PQnfields(m_pResult) : 0;
		}

		bool Result::GetString(int nRow, int nCol, InternalString* pOut)
		{
			if (!pOut || !IsValidCell(m_pResult, nRow, nCol))
				return false;

		if (PQfformat(m_pResult, nCol) != 1)
				return false;

		const unsigned char* pData = nullptr;
		int nLen = 0;
		if (!GetCellData(m_pResult, nRow, nCol, pData, nLen))
			return false;
		pOut->Set("");
		pOut->AppendStringData(const_cast<unsigned char*>(pData), nLen);
			return true;
		}

		bool Result::GetBool(int nRow, int nCol, bool& outVal)
		{
			const unsigned char* pData = nullptr;
			int nLen = 0;
		if (!GetCellData(m_pResult, nRow, nCol, pData, nLen) || PQfformat(m_pResult, nCol) != 1 ||
			PQftype(m_pResult, nCol) != BOOLOID || nLen != 1)
			return false;
		outVal = (pData[0] != 0);
		return true;
		}

		bool Result::GetInt16(int nRow, int nCol, short& outVal)
		{
			int64_t nBinary = 0;
			if (!TryGetBinaryInt64(m_pResult, nRow, nCol, nBinary))
				return false;
			if (nBinary < std::numeric_limits<short>::min() || nBinary > std::numeric_limits<short>::max())
				return false;
			outVal = static_cast<short>(nBinary);
			return true;
		}

		bool Result::GetInt32(int nRow, int nCol, int& outVal)
		{
			int64_t nBinary = 0;
			if (!TryGetBinaryInt64(m_pResult, nRow, nCol, nBinary))
				return false;
			if (nBinary < std::numeric_limits<int>::min() || nBinary > std::numeric_limits<int>::max())
				return false;
			outVal = static_cast<int>(nBinary);
			return true;
		}

		bool Result::GetInt64(int nRow, int nCol, long long& outVal)
		{
			int64_t nBinary = 0;
			if (!TryGetBinaryInt64(m_pResult, nRow, nCol, nBinary))
				return false;
			outVal = static_cast<long long>(nBinary);
			return true;
		}

		bool Result::GetFloat(int nRow, int nCol, float& outVal)
		{
			double dBinary = 0.0;
			if (!TryGetBinaryDouble(m_pResult, nRow, nCol, dBinary))
				return false;
			outVal = static_cast<float>(dBinary);
			return true;
		}

		bool Result::GetDouble(int nRow, int nCol, double& outVal)
		{
			return TryGetBinaryDouble(m_pResult, nRow, nCol, outVal);
		}

		bool Result::GetBlob(int nRow, int nCol, gsBlob* pOut)
		{
			if (!m_pResult || !pOut || nRow < 0 || nCol < 0 ||
				nRow >= PQntuples(m_pResult) || nCol >= PQnfields(m_pResult))
				return false;

			if (PQgetisnull(m_pResult, nRow, nCol))
				return false;

			const char* pVal = PQgetvalue(m_pResult, nRow, nCol);
			int nLen = PQgetlength(m_pResult, nRow, nCol);
			if (!pVal || nLen < 0)
				return false;

			pOut->Reset();

		if (PQfformat(m_pResult, nCol) != 1)
			return false;

			for (int i = 0; i < nLen; i++)
				pOut->PackUint8(static_cast<unsigned char>(pVal[i]));
			pOut->SetOffset(0);
			return true;
		}
	}
}
