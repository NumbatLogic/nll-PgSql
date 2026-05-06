using System;
using System.Collections.Generic;
using Npgsql;

namespace NumbatLogic.Database
{
	public class Result
	{
		private readonly List<object[]> m_pRows;
		private readonly int m_nColumnCount;

		public Result(NpgsqlDataReader reader)
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

			if (!(value is string text))
				return false;
			pOut.Set(text);
			return true;
		}

		public bool GetBool(int nRow, int nCol, ref bool pOut)
		{
			if (!TryGetCell(nRow, nCol, out object value))
				return false;
			if (!(value is bool b))
				return false;
			pOut = b;
			return true;
		}

		private static bool TryGetSignedIntegral(object value, out long outVal)
		{
			outVal = 0;
			if (value is sbyte sb) { outVal = sb; return true; }
			if (value is short sh) { outVal = sh; return true; }
			if (value is int i) { outVal = i; return true; }
			if (value is long l) { outVal = l; return true; }
			if (value is byte b) { outVal = b; return true; }
			if (value is ushort us) { outVal = us; return true; }
			if (value is uint ui) { outVal = ui; return true; }
			if (value is ulong ul)
			{
				if (ul > (ulong)long.MaxValue)
					return false;
				outVal = (long)ul;
				return true;
			}
			return false;
		}

		public bool GetInt16(int nRow, int nCol, ref short pOut)
		{
			if (!TryGetCell(nRow, nCol, out object value))
				return false;
			if (!TryGetSignedIntegral(value, out long signedVal))
				return false;
			if (signedVal < short.MinValue || signedVal > short.MaxValue)
				return false;
			pOut = (short)signedVal;
			return true;
		}

		public bool GetInt32(int nRow, int nCol, ref int pOut)
		{
			if (!TryGetCell(nRow, nCol, out object value))
				return false;
			if (!TryGetSignedIntegral(value, out long signedVal))
				return false;
			if (signedVal < int.MinValue || signedVal > int.MaxValue)
				return false;
			pOut = (int)signedVal;
			return true;
		}

		public bool GetInt64(int nRow, int nCol, ref long pOut)
		{
			if (!TryGetCell(nRow, nCol, out object value))
				return false;
			if (!TryGetSignedIntegral(value, out long signedVal))
				return false;
			pOut = signedVal;
			return true;
		}

		public bool GetFloat(int nRow, int nCol, ref float pOut)
		{
			if (!TryGetCell(nRow, nCol, out object value))
				return false;
			if (!(value is float f))
				return false;
			pOut = f;
			return true;
		}

		public bool GetDouble(int nRow, int nCol, ref double pOut)
		{
			if (!TryGetCell(nRow, nCol, out object value))
				return false;
			if (!(value is double d))
				return false;
			pOut = d;
			return true;
		}

		public bool GetBlob(int nRow, int nCol, gsBlob pOut)
		{
			Assert.Plz(pOut != null);
			if (!TryGetCell(nRow, nCol, out object value))
				return false;
			if (value == null)
				return false;
			if (value is byte[] bytes)
			{
				pOut.Reset();
				for (int i = 0; i < bytes.Length; i++)
					pOut.PackUint8(bytes[i]);
				pOut.SetOffset(0);
				return true;
			}
			return false;
		}
	}
}

