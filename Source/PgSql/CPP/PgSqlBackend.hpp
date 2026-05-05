#pragma once

#include <libpq-fe.h>
#include "PgSqlResult.hpp"

namespace NumbatLogic
{
	class InternalString;
	namespace Database
	{
		class DatabaseQuery;
		class PgSqlBackend
		{
			public:
				PgSqlBackend();
				~PgSqlBackend();

				bool Connect(const char* sxHost, const char* sxUsername, const char* sxPassword, const char* sxDatabase);
				bool Exec(const char* sxSql);
				bool QueryExists(const char* sxSql);
				const char* GetLastError();
				const char* GetParameterPrefix();
				InternalString* QueryFirstRowSingleColumn(const char* sxSql);
				PgSqlResult* ExecuteQuery(const char* sxSql, DatabaseQuery* pQuery);

			private:
				PGconn* m_pConn;
				InternalString* m_sLastError;
		};
	}
}
