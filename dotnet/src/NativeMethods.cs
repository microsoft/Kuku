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
        [return: MarshalAs(UnmanagedType.I1)]
        internal static extern bool KukuTable_Insert(IntPtr kuku_table, ulong[] item);

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl, SetLastError = true)]
        [return: MarshalAs(UnmanagedType.I1)]
        internal static extern bool KukuTable_Query(IntPtr kuku_table, ulong[] item, ref QueryResultData query_result);

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl, SetLastError = true)]
        [return: MarshalAs(UnmanagedType.I1)]
        internal static extern bool KukuTable_IsEmptyItem(IntPtr kuku_table, ulong[] item);

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl, SetLastError = true)]
        internal static extern void KukuTable_EmptyItem(IntPtr kuku_table, ulong[] item);

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl, SetLastError = true)]
        internal static extern void KukuTable_LeftoverItem(IntPtr kuku_table, ulong[] item);

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl, SetLastError = true)]
        internal static extern double KukuTable_FillRate(IntPtr kuku_table);

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl, SetLastError = true)]
        internal static extern uint KukuTable_LocFuncCount(IntPtr kuku_table);

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl, SetLastError = true)]
        internal static extern void KukuTable_Table(IntPtr kuku_table, uint index, ulong[] item);

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl, SetLastError = true)]
        internal static extern uint KukuTable_TableSize(IntPtr kuku_table);

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl, SetLastError = true)]
        internal static extern void KukuTable_Stash(IntPtr kuku_table, uint index, ulong[] item);

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl, SetLastError = true)]
        internal static extern uint KukuTable_StashSize(IntPtr kuku_table);

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl, SetLastError = true)]
        internal static extern uint KukuTable_Location(IntPtr kuku_table, ulong[] item, uint loc_func_index);

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl, SetLastError = true)]
        internal static extern void KukuTable_AllLocations(IntPtr kuku_table, ulong[] item, uint[] locations, out uint count);

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl, SetLastError = true)]
        internal static extern void KukuTable_ClearTable(IntPtr kuku_table);

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl, SetLastError = true)]
        internal static extern void Common_SetRandomItem(ulong[] item);

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl, SetLastError = true)]
        internal static extern void Common_IncrementItem(ulong[] item);

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl, SetLastError = true)]
        internal static extern uint Common_MinTableSize();

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl, SetLastError = true)]
        internal static extern uint Common_MaxTableSize();

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl, SetLastError = true)]
        internal static extern uint Common_MinLocFuncCount();

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl, SetLastError = true)]
        internal static extern uint Common_MaxLocFuncCount();
    }

    [StructLayout(LayoutKind.Sequential)]
    internal struct QueryResultData
    {
        [MarshalAs(UnmanagedType.I1)]
        public bool found;
        [MarshalAs(UnmanagedType.I1)]
        public bool in_stash;
        public uint location;
        public uint loc_func_index;
    }
}
