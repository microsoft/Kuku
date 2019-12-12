using System;
using System.Reflection;
using System.Runtime.InteropServices;

namespace KukuDotNet
{
    internal class KukuDotNet
    {

        internal static class Library
        {
            internal const string MyLibrary = "libKukuLib";

            static Library()
            {
                NativeLibrary.SetDllImportResolver(typeof(Library).Assembly, ImportResolver);
            }

            private static IntPtr ImportResolver(string libraryName, Assembly assembly, DllImportSearchPath? searchPath)
            {
                IntPtr libHandle = IntPtr.Zero;
                if (libraryName == MyLibrary)
                {
                    // Try using the system library 'libmylibrary.so.5'
                    NativeLibrary.TryLoad("libKukuLib.so", assembly, DllImportSearchPath.System32, out libHandle);
                }

                return libHandle;
            }
        }

        private const string DllName = Library.MyLibrary;

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr kukuTable_Create(int log_table_size, uint stash_size, uint loc_func_count, ulong[] loc_func_seed, ulong max_probe, ulong[] empty_item);

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        internal static extern bool kukuTable_Insert(IntPtr kuku_table, ulong[] item);

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        internal static extern bool kukuTable_Query(IntPtr kuku_table, ulong[] item, ref QueryResult query_result);

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        internal static extern bool kukuTable_IsEmptyItem(IntPtr kuku_table, ulong[] item);

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        internal static extern bool kukuTable_LastInsertFailItem(IntPtr kuku_table, ulong[] item);

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        internal static extern double kukuTable_FillRate(IntPtr kuku_table);

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        internal static extern bool kukuTable_Table(IntPtr kuku_table, uint index, ulong[] item);

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        internal static extern uint kukuTable_TableSize(IntPtr kuku_table);

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        internal static extern bool kukuTable_Stash(IntPtr kuku_table, uint index, ulong[] item);

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        internal static extern uint kukuTable_StashSize(IntPtr kuku_table);

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        internal static extern uint kukuTable_Location(IntPtr kuku_table, ulong[] item, uint index);
    }

    internal struct QueryResult
    {
        internal long location;
        internal long loc_func_index;
        internal bool in_stash;
    }
}