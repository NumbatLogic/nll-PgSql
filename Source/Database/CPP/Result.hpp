#pragma once

#include <libpq-fe.h>

namespace NumbatLogic
{
	class gsBlob;
	class InternalString;
	namespace Database
	{
		class Result
		{
			public:
				explicit Result(PGresult* pResult);
				~Result();

				int GetRowCount();
				int GetColumnCount();
				bool GetString(int nRow, int nCol, InternalString* pOut);
				bool GetBool(int nRow, int nCol, bool& outVal);
				bool GetInt16(int nRow, int nCol, short& outVal);
				bool GetInt32(int nRow, int nCol, int& outVal);
				bool GetInt64(int nRow, int nCol, long long& outVal);
				bool GetFloat(int nRow, int nCol, float& outVal);
				bool GetDouble(int nRow, int nCol, double& outVal);
				bool GetBlob(int nRow, int nCol, gsBlob* pOut);

			private:
				PGresult* m_pResult;
		};
	}
}
