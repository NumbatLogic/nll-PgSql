#pragma once

#include <libpq-fe.h>
#include "PgSqlResult.hpp"

namespace NumbatLogic
{
	class InternalString;
	class PgSqlBackend
	{
		public:
			PgSqlBackend();
			~PgSqlBackend();

			bool Connect(const char* sxHost, const char* sxUsername, const char* sxPassword, const char* sxDatabase);
			bool Exec(const char* sxSql);
			bool QueryExists(const char* sxSql);
			const char* GetLastError();
			InternalString* QueryFirstRowSingleColumn(const char* sxSql);
			PgSqlResult* ExecuteQuery(const char* sxSql, int nParams, const char* sxParamString);

		private:
			PGconn* m_pConn;
			InternalString* m_sLastError;
	};
}
