#include "PgSqlBackend.hpp"
#include "PgSqlResult.hpp"
#include "../../../Transpiled/PgSql/DatabaseQuery.hpp"
#include "../../../Transpiled/PgSql/DatabaseValue.hpp"
#include "../../../../LangShared/Source/Assert/CPP/Assert.hpp"
#include "../../../../LangShared/Source/InternalString/CPP/InternalString.hpp"
#include "../../../../LangShared/Source/Blob/CPP/Blob.hpp"

#include <arpa/inet.h>
#include <cstring>
#include <vector>
#include <libpq-fe.h>

namespace NumbatLogic
{
	namespace Database
	{
		static const Oid BOOLOID = 16;
		static const Oid INT2OID = 21;
		static const Oid INT4OID = 23;
		static const Oid FLOAT4OID = 700;
		static const Oid FLOAT8OID = 701;
		static const Oid BYTEAOID = 17;

		static uint64_t HostToNetwork64(uint64_t nValue)
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

		const char* PgSqlBackend::GetParameterPrefix()
		{
			return "$";
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

		PgSqlResult* PgSqlBackend::ExecuteQuery(const char* sxSql, DatabaseQuery* pQuery)
		{
			if (!m_pConn || !sxSql || !pQuery)
			{
				ClearLastError(m_sLastError);
				return nullptr;
			}

			PGresult* pResult;
			
			const char** pParamValues = nullptr;
			int* pParamLengths = nullptr;
			int* pParamFormats = nullptr;
			Oid* pParamTypes = nullptr;

			int nParameterCount = pQuery->GetParameterCount();
			std::vector<std::vector<unsigned char>> paramBinary;
			if (nParameterCount > 0)
			{
				pParamValues = new const char*[nParameterCount];
				pParamLengths = new int[nParameterCount];
				pParamFormats = new int[nParameterCount];
				pParamTypes = new Oid[nParameterCount];
				paramBinary.resize(static_cast<size_t>(nParameterCount));

				for (int i = 0; i < nParameterCount; i++)
				{
					pParamValues[i] = nullptr;
					pParamLengths[i] = 0;
					pParamFormats[i] = 0;
					pParamTypes[i] = 0;

					DatabaseValue::Type eType = pQuery->GetParameterType(i);
					switch (eType)
					{
						case DatabaseValue::Type::STRING:
						{
							pParamValues[i] = pQuery->GetParameterTextExternal(i);
							pParamFormats[i] = 0;
							pParamTypes[i] = 0;
							break;
						}
						case DatabaseValue::Type::BOOL:
						{
							paramBinary[static_cast<size_t>(i)].resize(1);
							paramBinary[static_cast<size_t>(i)][0] = pQuery->GetParameterBool(i) ? 1 : 0;
							pParamValues[i] = reinterpret_cast<const char*>(paramBinary[static_cast<size_t>(i)].data());
							pParamLengths[i] = 1;
							pParamFormats[i] = 1;
							pParamTypes[i] = BOOLOID;
							break;
						}
						case DatabaseValue::Type::INT16:
						{
							short v = pQuery->GetParameterInt16(i);
							uint16_t rawBits = 0;
							memcpy(&rawBits, &v, sizeof(rawBits));
							uint16_t net = htons(rawBits);
							paramBinary[static_cast<size_t>(i)].resize(sizeof(net));
							memcpy(paramBinary[static_cast<size_t>(i)].data(), &net, sizeof(net));
							pParamValues[i] = reinterpret_cast<const char*>(paramBinary[static_cast<size_t>(i)].data());
							pParamLengths[i] = static_cast<int>(sizeof(net));
							pParamFormats[i] = 1;
							pParamTypes[i] = INT2OID;
							break;
						}
						case DatabaseValue::Type::INT32:
						{
							int v = pQuery->GetParameterInt32(i);
							uint32_t net = htonl(static_cast<uint32_t>(v));
							paramBinary[static_cast<size_t>(i)].resize(sizeof(net));
							memcpy(paramBinary[static_cast<size_t>(i)].data(), &net, sizeof(net));
							pParamValues[i] = reinterpret_cast<const char*>(paramBinary[static_cast<size_t>(i)].data());
							pParamLengths[i] = static_cast<int>(sizeof(net));
							pParamFormats[i] = 1;
							pParamTypes[i] = INT4OID;
							break;
						}
						case DatabaseValue::Type::UINT32:
						{
							unsigned int v = pQuery->GetParameterUint32(i);
							uint32_t net = htonl(static_cast<uint32_t>(v));
							paramBinary[static_cast<size_t>(i)].resize(sizeof(net));
							memcpy(paramBinary[static_cast<size_t>(i)].data(), &net, sizeof(net));
							pParamValues[i] = reinterpret_cast<const char*>(paramBinary[static_cast<size_t>(i)].data());
							pParamLengths[i] = static_cast<int>(sizeof(net));
							pParamFormats[i] = 1;
							pParamTypes[i] = INT4OID;
							break;
						}
						case DatabaseValue::Type::DOUBLE:
						{
							double v = pQuery->GetParameterDouble(i);
							uint64_t raw = 0;
							memcpy(&raw, &v, sizeof(raw));
							raw = HostToNetwork64(raw);
							paramBinary[static_cast<size_t>(i)].resize(sizeof(raw));
							memcpy(paramBinary[static_cast<size_t>(i)].data(), &raw, sizeof(raw));
							pParamValues[i] = reinterpret_cast<const char*>(paramBinary[static_cast<size_t>(i)].data());
							pParamLengths[i] = static_cast<int>(sizeof(raw));
							pParamFormats[i] = 1;
							pParamTypes[i] = FLOAT8OID;
							break;
						}
						case DatabaseValue::Type::BLOB:
						{
							gsBlob* pBlob = pQuery->GetParameterBlob(i);
							Assert::Plz(pBlob != nullptr);
							pParamValues[i] = (const char*)pBlob->GetData();
							pParamLengths[i] = pBlob->GetSize();
							pParamFormats[i] = 1;
							pParamTypes[i] = BYTEAOID;
							break;
						}
						default:
						{
							Assert::Plz(false);
							break;
						}
					}
				}
			}

			pResult = PQexecParams(m_pConn, sxSql, nParameterCount, pParamTypes, pParamValues, pParamLengths, pParamFormats, 1);
			
			delete[] pParamValues;
			delete[] pParamLengths;
			delete[] pParamFormats;
			delete[] pParamTypes;

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
}
