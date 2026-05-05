#pragma once

#include <libpq-fe.h>

namespace NumbatLogic
{
	class gsBlob;
	class InternalString;
	namespace Database
	{
		class PgSqlResult
		{
			public:
				explicit PgSqlResult(PGresult* pResult);
				~PgSqlResult();

				int GetRowCount();
				int GetColumnCount();
				bool GetString(int nRow, int nCol, InternalString* pOut);
				bool GetBool(int nRow, int nCol, bool& outVal);
				bool GetInt8(int nRow, int nCol, signed char& outVal);
				bool GetInt16(int nRow, int nCol, short& outVal);
				bool GetInt32(int nRow, int nCol, int& outVal);
				bool GetUint8(int nRow, int nCol, unsigned char& outVal);
				bool GetUint16(int nRow, int nCol, unsigned short& outVal);
				bool GetUint32(int nRow, int nCol, unsigned int& outVal);
				bool GetFloat(int nRow, int nCol, float& outVal);
				bool GetDouble(int nRow, int nCol, double& outVal);
				bool GetBlob(int nRow, int nCol, gsBlob* pOut);

			private:
				PGresult* m_pResult;
		};
	}
}
