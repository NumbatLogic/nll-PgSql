using System;
using System.Collections.Generic;
using Npgsql;

namespace NumbatLogic
{
	public class PgSqlBackend
	{
		private NpgsqlConnection m_pConn;
		private string m_sLastError;

		public PgSqlBackend()
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

		private static List<string> SplitParamString(string sxParamString, int nParams)
		{
			var result = new List<string>(nParams > 0 ? nParams : 0);
			if (sxParamString == null || nParams <= 0)
				return result;

			int start = 0;
			for (int i = 0; i < nParams; i++)
			{
				if (start >= sxParamString.Length)
				{
					result.Add(string.Empty);
					continue;
				}

				int idx = sxParamString.IndexOf('\x01', start);
				if (idx < 0)
				{
					result.Add(sxParamString.Substring(start));
					start = sxParamString.Length;
				}
				else
				{
					result.Add(sxParamString.Substring(start, idx - start));
					start = idx + 1;
				}
			}

			return result;
		}

		private static string RewriteSqlForNpgsql(string sxSql, int nParams)
		{
			// Original SQL uses $1, $2 ... style. Npgsql by default expects @p1, @p2 ...
			// To avoid touching the NLL-side replacement logic, just adapt it here.
			if (sxSql == null || nParams <= 0)
				return sxSql;

			string sqlOut = sxSql;
			for (int i = 1; i <= nParams; i++)
			{
				string from = "$" + i.ToString();
				string to = "@p" + i.ToString();
				sqlOut = sqlOut.Replace(from, to);
			}
			return sqlOut;
		}

		public PgSqlResult ExecuteQuery(string sxSql, int nParams, string sxParamString)
		{
			if (m_pConn == null || sxSql == null)
			{
				ClearLastError();
				return null;
			}

			try
			{
				List<string> paramValues = SplitParamString(sxParamString, nParams);
				string finalSql = RewriteSqlForNpgsql(sxSql, nParams);

				using (var cmd = new NpgsqlCommand(finalSql, m_pConn))
				{
					for (int i = 0; i < paramValues.Count; i++)
					{
						string paramName = "p" + (i + 1).ToString();
						cmd.Parameters.AddWithValue(paramName, (object)paramValues[i] ?? DBNull.Value);
					}

					using (var reader = cmd.ExecuteReader())
					{
						var result = new PgSqlResult(reader);
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

