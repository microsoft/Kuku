using System;
using System.Runtime.InteropServices;

namespace CuckooDotNet
{
    internal class CuckooDotNet
    {
        private const string DllName = "CuckooLib.dll";

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr CuckooTable_Create(int log_table_size, uint stash_size, uint loc_func_count, ulong[] loc_func_seed, ulong max_probe, ulong[] empty_item);

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        internal static extern bool CuckooTable_Insert(IntPtr cuckoo_table, ulong[] item);

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        internal static extern bool CuckooTable_Query(IntPtr cuckoo_table, ulong[] item, ref QueryResult query_result);

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        internal static extern bool CuckooTable_IsEmptyItem(IntPtr cuckoo_table, ulong[] item);

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        internal static extern bool CuckooTable_LastInsertFailItem(IntPtr cuckoo_table, ulong[] item);

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        internal static extern double CuckooTable_FillRate(IntPtr cuckoo_table);

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        internal static extern bool CuckooTable_Table(IntPtr cuckoo_table, uint index, ulong[] item);

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        internal static extern uint CuckooTable_TableSize(IntPtr cuckoo_table);

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        internal static extern bool CuckooTable_Stash(IntPtr cuckoo_table, uint index, ulong[] item);

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        internal static extern uint CuckooTable_StashSize(IntPtr cuckoo_table);
    }

    internal struct QueryResult
    {
        internal long location;
        internal long loc_func_index;
        internal bool in_stash;
    }
}