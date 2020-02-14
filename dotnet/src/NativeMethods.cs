// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

using System;
using System.Runtime.InteropServices;

namespace Microsoft.Research.Kuku
{
    internal class NativeMethods
    {
    	private const string DllName = "kukuc";

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl, SetLastError = true)]
        public static extern IntPtr KukuTable_Create(uint table_size, uint stash_size, uint loc_func_count, ulong[] loc_func_seed, ulong max_probe, ulong[] empty_item);

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl, SetLastError = true)]
        internal static extern bool KukuTable_Insert(IntPtr kuku_table, ulong[] item);

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl, SetLastError = true)]
        internal static extern bool KukuTable_Query(IntPtr kuku_table, ulong[] item, ref QueryResult query_result);

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl, SetLastError = true)]
        internal static extern bool KukuTable_IsEmptyItem(IntPtr kuku_table, ulong[] item);

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl, SetLastError = true)]
        internal static extern bool KukuTable_LastInsertFailItem(IntPtr kuku_table, ulong[] item);

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl, SetLastError = true)]
        internal static extern double KukuTable_FillRate(IntPtr kuku_table);

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl, SetLastError = true)]
        internal static extern bool KukuTable_Table(IntPtr kuku_table, uint index, ulong[] item);

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl, SetLastError = true)]
        internal static extern uint KukuTable_TableSize(IntPtr kuku_table);

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl, SetLastError = true)]
        internal static extern bool KukuTable_Stash(IntPtr kuku_table, uint index, ulong[] item);

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl, SetLastError = true)]
        internal static extern uint KukuTable_StashSize(IntPtr kuku_table);

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl, SetLastError = true)]
        internal static extern uint KukuTable_Location(IntPtr kuku_table, ulong[] item, uint loc_func_index);
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct QueryResult
    {
        public bool found;
        public bool in_stash;
        public uint location;
        public uint loc_func_index;
    }
}
