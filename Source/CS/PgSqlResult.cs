using System;
using System.Collections.Generic;
using Npgsql;

namespace NumbatLogic
{
	class PgSqlResult
	{
		private readonly List<object[]> m_pRows;
		private readonly int m_nColumnCount;

		public PgSqlResult(NpgsqlDataReader reader)
		{
			if (reader == null)
			{
				m_pRows = new List<object[]>();
				m_nColumnCount = 0;
				return;
			}

			m_nColumnCount = reader.FieldCount;
			m_pRows = new List<object[]>();

			while (reader.Read())
			{
				object[] values = new object[m_nColumnCount];
				reader.GetValues(values);
				m_pRows.Add(values);
			}
		}

		private bool TryGetCell(int nRow, int nCol, out object value)
		{
			value = null;
			if (nRow < 0 || nCol < 0)
				return false;
			if (nRow >= m_pRows.Count || (m_pRows.Count > 0 && nCol >= m_nColumnCount))
				return false;
			value = m_pRows[nRow][nCol];
			return true;
		}

		public int GetRowCount()
		{
			return m_pRows.Count;
		}

		public int GetColumnCount()
		{
			return m_nColumnCount;
		}

		public bool GetString(int nRow, int nCol, InternalString pOut)
		{
			if (pOut == null)
				return false;

			if (!TryGetCell(nRow, nCol, out object value))
				return false;

			string text = value == null ? string.Empty : Convert.ToString(value);
			pOut.Set(text ?? string.Empty);
			return true;
		}

		public bool GetBool(int nRow, int nCol, ref bool pOut)
		{
			if (!TryGetCell(nRow, nCol, out object value))
				return false;

			if (value == null)
				return false;

			if (value is bool b)
			{
				pOut = b;
				return true;
			}

			string s = Convert.ToString(value);
			if (string.IsNullOrEmpty(s))
				return false;

			if (s == "t" || s == "T" || s == "1")
			{
				pOut = true;
				return true;
			}
			if (s == "f" || s == "F" || s == "0")
			{
				pOut = false;
				return true;
			}
			if (string.Equals(s, "true", StringComparison.OrdinalIgnoreCase) ||
				string.Equals(s, "yes", StringComparison.OrdinalIgnoreCase))
			{
				pOut = true;
				return true;
			}
			if (string.Equals(s, "false", StringComparison.OrdinalIgnoreCase) ||
				string.Equals(s, "no", StringComparison.OrdinalIgnoreCase))
			{
				pOut = false;
				return true;
			}
			return false;
		}

		private static bool TryParseInt(string s, long min, long max, out long value)
		{
			value = 0;
			if (string.IsNullOrEmpty(s))
				return false;
			if (!long.TryParse(s, out long v))
				return false;
			if (v < min || v > max)
				return false;
			value = v;
			return true;
		}

		private static bool TryParseUInt(string s, ulong max, out ulong value)
		{
			value = 0;
			if (string.IsNullOrEmpty(s))
				return false;
			if (!ulong.TryParse(s, out ulong v))
				return false;
			if (v > max)
				return false;
			value = v;
			return true;
		}

		public bool GetInt8(int nRow, int nCol, ref sbyte pOut)
		{
			if (!TryGetCell(nRow, nCol, out object value))
				return false;
			if (value == null)
				return false;

			if (value is sbyte sb)
			{
				pOut = sb;
				return true;
			}

			string s = Convert.ToString(value);
			if (!TryParseInt(s, -128, 127, out long v))
				return false;
			pOut = (sbyte)v;
			return true;
		}

		public bool GetInt16(int nRow, int nCol, ref short pOut)
		{
			if (!TryGetCell(nRow, nCol, out object value))
				return false;
			if (value == null)
				return false;

			if (value is short sh)
			{
				pOut = sh;
				return true;
			}

			string s = Convert.ToString(value);
			if (!TryParseInt(s, -32768, 32767, out long v))
				return false;
			pOut = (short)v;
			return true;
		}

		public bool GetInt32(int nRow, int nCol, ref int pOut)
		{
			if (!TryGetCell(nRow, nCol, out object value))
				return false;
			if (value == null)
				return false;

			if (value is int i)
			{
				pOut = i;
				return true;
			}

			string s = Convert.ToString(value);
			if (!TryParseInt(s, int.MinValue, int.MaxValue, out long v))
				return false;
			pOut = (int)v;
			return true;
		}

		public bool GetUint8(int nRow, int nCol, ref byte pOut)
		{
			if (!TryGetCell(nRow, nCol, out object value))
				return false;
			if (value == null)
				return false;

			if (value is byte b)
			{
				pOut = b;
				return true;
			}

			string s = Convert.ToString(value);
			if (!TryParseUInt(s, 255, out ulong v))
				return false;
			pOut = (byte)v;
			return true;
		}

		public bool GetUint16(int nRow, int nCol, ref ushort pOut)
		{
			if (!TryGetCell(nRow, nCol, out object value))
				return false;
			if (value == null)
				return false;

			if (value is ushort us)
			{
				pOut = us;
				return true;
			}

			string s = Convert.ToString(value);
			if (!TryParseUInt(s, 65535, out ulong v))
				return false;
			pOut = (ushort)v;
			return true;
		}

		public bool GetUint32(int nRow, int nCol, ref uint pOut)
		{
			if (!TryGetCell(nRow, nCol, out object value))
				return false;
			if (value == null)
				return false;

			if (value is uint ui)
			{
				pOut = ui;
				return true;
			}

			string s = Convert.ToString(value);
			if (!TryParseUInt(s, 4294967295UL, out ulong v))
				return false;
			pOut = (uint)v;
			return true;
		}

		public bool GetFloat(int nRow, int nCol, ref float pOut)
		{
			if (!TryGetCell(nRow, nCol, out object value))
				return false;
			if (value == null)
				return false;

			if (value is float f)
			{
				pOut = f;
				return true;
			}

			string s = Convert.ToString(value);
			if (string.IsNullOrEmpty(s))
				return false;

			if (!double.TryParse(s, out double d))
				return false;
			pOut = (float)d;
			return true;
		}

		public bool GetDouble(int nRow, int nCol, ref double pOut)
		{
			if (!TryGetCell(nRow, nCol, out object value))
				return false;
			if (value == null)
				return false;

			if (value is double d)
			{
				pOut = d;
				return true;
			}

			string s = Convert.ToString(value);
			if (string.IsNullOrEmpty(s))
				return false;

			if (!double.TryParse(s, out double dv))
				return false;
			pOut = dv;
			return true;
		}
	}
}

