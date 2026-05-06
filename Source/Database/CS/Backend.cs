using System;
using Npgsql;
using NpgsqlTypes;

namespace NumbatLogic.Database
{
	public class Backend
	{
		private NpgsqlConnection m_pConn;
		private string m_sLastError;

		public Backend()
		{
			m_pConn = null;
			m_sLastError = null;
		}

		private void SetLastError(string szMsg)
		{
			m_sLastError = szMsg ?? string.Empty;
		}

		private void ClearLastError()
		{
			m_sLastError = null;
		}

		public bool Connect(string sxHost, string sxUsername, string sxPassword, string sxDatabase)
		{
			try
			{
				if (m_pConn != null)
				{
					m_pConn.Dispose();
					m_pConn = null;
				}

				var builder = new NpgsqlConnectionStringBuilder
				{
					Host = sxHost ?? string.Empty,
					Username = sxUsername ?? string.Empty,
					Password = sxPassword ?? string.Empty,
					Database = sxDatabase ?? string.Empty
				};

				m_pConn = new NpgsqlConnection(builder.ConnectionString);
				m_pConn.Open();
				ClearLastError();
				return true;
			}
			catch (Exception ex)
			{
				SetLastError(ex.Message);
				if (m_pConn != null)
				{
					m_pConn.Dispose();
					m_pConn = null;
				}
				return false;
			}
		}

		public bool Exec(string sxSql)
		{
			if (m_pConn == null || sxSql == null)
			{
				ClearLastError();
				return false;
			}

			try
			{
				using (var cmd = new NpgsqlCommand(sxSql, m_pConn))
				{
					cmd.ExecuteNonQuery();
				}
				ClearLastError();
				return true;
			}
			catch (Exception ex)
			{
				SetLastError(ex.Message);
				return false;
			}
		}

		public bool QueryExists(string sxSql)
		{
			if (m_pConn == null || sxSql == null)
			{
				ClearLastError();
				return false;
			}

			try
			{
				using (var cmd = new NpgsqlCommand(sxSql, m_pConn))
				using (var reader = cmd.ExecuteReader())
				{
					bool hasRows = reader.HasRows;
					ClearLastError();
					return hasRows;
				}
			}
			catch (Exception ex)
			{
				SetLastError(ex.Message);
				return false;
			}
		}

		public string GetLastError()
		{
			return m_sLastError ?? string.Empty;
		}

		public string GetParameterPrefix()
		{
			return "@p";
		}

		public InternalString QueryFirstRowSingleColumn(string sxSql)
		{
			if (m_pConn == null || sxSql == null)
			{
				ClearLastError();
				return null;
			}

			try
			{
				using (var cmd = new NpgsqlCommand(sxSql, m_pConn))
				using (var reader = cmd.ExecuteReader())
				{
					if (!reader.Read() || reader.FieldCount < 1)
						return null;

					object value = reader.GetValue(0);
					string text = value == null ? string.Empty : Convert.ToString(value);
					ClearLastError();
					return new InternalString(text);
				}
			}
			catch (Exception ex)
			{
				SetLastError(ex.Message);
				return null;
			}
		}

		public Result ExecuteQuery(string sxSql, Query pQuery)
		{
			if (m_pConn == null || sxSql == null || pQuery == null)
			{
				ClearLastError();
				return null;
			}

			try
			{
				int nParameterCount = pQuery.GetParameterCount();

				using (var cmd = new NpgsqlCommand(sxSql, m_pConn))
				{
					for (int i = 0; i < nParameterCount; i++)
					{
						string paramName = "p" + (i + 1).ToString();
						switch (pQuery.GetParameterType(i))
						{
							case Value.Type.UINT32:
								cmd.Parameters.AddWithValue(paramName, pQuery.GetParameterUint32(i));
								break;
							case Value.Type.INT32:
								cmd.Parameters.AddWithValue(paramName, pQuery.GetParameterInt32(i));
								break;
							case Value.Type.INT64:
								cmd.Parameters.AddWithValue(paramName, pQuery.GetParameterInt64(i));
								break;
							case Value.Type.INT16:
								cmd.Parameters.AddWithValue(paramName, pQuery.GetParameterInt16(i));
								break;
							case Value.Type.BOOL:
								cmd.Parameters.AddWithValue(paramName, pQuery.GetParameterBool(i));
								break;
							case Value.Type.DOUBLE:
								cmd.Parameters.AddWithValue(paramName, pQuery.GetParameterDouble(i));
								break;
							case Value.Type.BLOB:
							{
								gsBlob pBlob = pQuery.GetParameterBlob(i);
								byte[] blob = pBlob == null || pBlob.__nSize <= 0
									? Array.Empty<byte>()
									: new byte[pBlob.__nSize];
								if (pBlob != null && pBlob.__nSize > 0)
									Buffer.BlockCopy(pBlob.__pBuffer, 0, blob, 0, pBlob.__nSize);
								var dbParam = cmd.Parameters.Add(paramName, NpgsqlDbType.Bytea);
								dbParam.Value = blob;
								break;
							}
							case Value.Type.STRING:
							{
								string sxText = pQuery.GetParameterTextExternal(i) ?? string.Empty;
								cmd.Parameters.AddWithValue(paramName, sxText);
								break;
							}
							default:
							{
								string sxFallback = pQuery.GetParameterString(i) ?? string.Empty;
								cmd.Parameters.AddWithValue(paramName, (object)sxFallback);
								break;
							}
						}
					}

					using (var reader = cmd.ExecuteReader())
					{
						var result = new Result(reader);
						ClearLastError();
						return result;
					}
				}
			}
			catch (Exception ex)
			{
				SetLastError(ex.Message);
				return null;
			}
		}
	}
}

