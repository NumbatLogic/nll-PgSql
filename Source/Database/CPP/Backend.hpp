#pragma once

#include <libpq-fe.h>
#include "Result.hpp"

namespace NumbatLogic
{
	class InternalString;
	namespace Database
	{
		class Query;
		class Backend
		{
			public:
				Backend();
				~Backend();

				bool Connect(const char* sxHost, const char* sxUsername, const char* sxPassword, const char* sxDatabase);
				bool Exec(const char* sxSql);
				bool QueryExists(const char* sxSql);
				const char* GetLastError();
				const char* GetParameterPrefix();
				InternalString* QueryFirstRowSingleColumn(const char* sxSql);
				Result* ExecuteQuery(const char* sxSql, Query* pQuery);

			private:
				PGconn* m_pConn;
				InternalString* m_sLastError;
		};
	}
}
