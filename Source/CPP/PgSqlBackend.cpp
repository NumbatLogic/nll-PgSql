#include "PgSqlBackend.hpp"
#include "PgSqlResult.hpp"
#include "../../../LangShared/InternalString/CPP/InternalString.hpp"
#include <string>
#include <vector>

namespace NumbatLogic
{
	static void SplitParamString(const char* sxParamString, int nParams, std::vector<std::string>& out)
	{
		out.clear();
		if (!sxParamString || nParams <= 0)
			return;
		out.reserve(static_cast<size_t>(nParams));
		const char* p = sxParamString;
		for (int i = 0; i < nParams; i++)
		{
			const char* start = p;
			while (*p != '\0' && *p != '\x01')
				p++;
			out.push_back(std::string(start, static_cast<size_t>(p - start)));
			if (*p == '\x01')
				p++;
		}
	}
	static void SetLastError(InternalString*& sErr, const char* szMsg)
	{
		if (sErr)
			delete sErr;
		sErr = new InternalString(szMsg ? szMsg : "");
	}

	static void ClearLastError(InternalString*& sErr)
	{
		if (sErr)
		{
			delete sErr;
			sErr = nullptr;
		}
	}

	PgSqlBackend::PgSqlBackend()
		: m_pConn(nullptr)
		, m_sLastError(nullptr)
	{
	}

	PgSqlBackend::~PgSqlBackend()
	{
		if (m_pConn)
		{
			PQfinish(m_pConn);
			m_pConn = nullptr;
		}
		delete m_sLastError;
		m_sLastError = nullptr;
	}

	bool PgSqlBackend::Connect(const char* sxHost, const char* sxUsername, const char* sxPassword, const char* sxDatabase)
	{
		if (m_pConn)
		{
			PQfinish(m_pConn);
			m_pConn = nullptr;
		}

		char connInfo[1024];
		snprintf(connInfo, sizeof(connInfo),
			"host=%s user=%s password=%s dbname=%s",
			sxHost ? sxHost : "",
			sxUsername ? sxUsername : "",
			sxPassword ? sxPassword : "",
			sxDatabase ? sxDatabase : "");

		m_pConn = PQconnectdb(connInfo);

		if (PQstatus(m_pConn) != CONNECTION_OK)
		{
			SetLastError(m_sLastError, PQerrorMessage(m_pConn));
			PQfinish(m_pConn);
			m_pConn = nullptr;
			return false;
		}
		ClearLastError(m_sLastError);
		return true;
	}

	bool PgSqlBackend::Exec(const char* sxSql)
	{
		if (!m_pConn || !sxSql)
		{
			ClearLastError(m_sLastError);
			return false;
		}

		PGresult* pResult = PQexec(m_pConn, sxSql);

		if (!pResult)
		{
			SetLastError(m_sLastError, PQerrorMessage(m_pConn));
			return false;
		}

		ExecStatusType status = PQresultStatus(pResult);
		if (status != PGRES_COMMAND_OK && status != PGRES_TUPLES_OK && status != PGRES_EMPTY_QUERY)
		{
			SetLastError(m_sLastError, PQresultErrorMessage(pResult));
			PQclear(pResult);
			return false;
		}
		PQclear(pResult);
		ClearLastError(m_sLastError);
		return true;
	}

	bool PgSqlBackend::QueryExists(const char* sxSql)
	{
		if (!m_pConn || !sxSql)
		{
			ClearLastError(m_sLastError);
			return false;
		}

		PGresult* pResult = PQexec(m_pConn, sxSql);

		if (!pResult)
		{
			SetLastError(m_sLastError, PQerrorMessage(m_pConn));
			return false;
		}

		ExecStatusType status = PQresultStatus(pResult);
		bool bHasRows = (status == PGRES_TUPLES_OK && PQntuples(pResult) > 0);
		if (!bHasRows && status != PGRES_TUPLES_OK && status != PGRES_EMPTY_QUERY)
			SetLastError(m_sLastError, PQresultErrorMessage(pResult));
		else
			ClearLastError(m_sLastError);
		PQclear(pResult);
		return bHasRows;
	}

	const char* PgSqlBackend::GetLastError()
	{
		return m_sLastError ? m_sLastError->GetExternalString() : "";
	}

	InternalString* PgSqlBackend::QueryFirstRowSingleColumn(const char* sxSql)
	{
		if (!m_pConn || !sxSql)
		{
			ClearLastError(m_sLastError);
			return nullptr;
		}
		PGresult* pResult = PQexec(m_pConn, sxSql);
		if (!pResult)
		{
			SetLastError(m_sLastError, PQerrorMessage(m_pConn));
			return nullptr;
		}
		if (PQresultStatus(pResult) != PGRES_TUPLES_OK || PQntuples(pResult) < 1 || PQnfields(pResult) < 1)
		{
			PQclear(pResult);
			return nullptr;
		}
		const char* v = PQgetvalue(pResult, 0, 0);
		InternalString* pOut = new InternalString(v ? v : "");
		PQclear(pResult);
		ClearLastError(m_sLastError);
		return pOut;
	}

	PgSqlResult* PgSqlBackend::ExecuteQuery(const char* sxSql, int nParams, const char* sxParamString)
	{
		if (!m_pConn || !sxSql)
		{
			ClearLastError(m_sLastError);
			return nullptr;
		}
		std::vector<std::string> paramStorage;
		SplitParamString(sxParamString, nParams, paramStorage);
		std::vector<const char*> paramPtrs;
		for (size_t i = 0; i < paramStorage.size(); i++)
			paramPtrs.push_back(paramStorage[i].c_str());
		const char* const* paramValues = paramPtrs.empty() ? nullptr : paramPtrs.data();
		PGresult* pResult = PQexecParams(m_pConn, sxSql, nParams, nullptr, paramValues, nullptr, nullptr, 0);
		if (!pResult)
		{
			SetLastError(m_sLastError, PQerrorMessage(m_pConn));
			return nullptr;
		}

		ExecStatusType eStatus = PQresultStatus(pResult);
		if (eStatus != PGRES_TUPLES_OK && eStatus != PGRES_COMMAND_OK)
		{
			SetLastError(m_sLastError, PQresultErrorMessage(pResult));
			PQclear(pResult);
			return nullptr;
		}

		ClearLastError(m_sLastError);
		return new PgSqlResult(pResult);
	}
}
