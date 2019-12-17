using System;
using System.Reflection;
using System.Runtime.InteropServices;

namespace KukuDotNet
{
    internal class KukuDotNet
    {
	private const string DllName = "KukuLib";
        static KukuDotNet()
        {
                NativeLibrary.SetDllImportResolver(typeof(KukuDotNet).Assembly, ImportResolver);
        }

        private static IntPtr ImportResolver(string libraryName, Assembly assembly, DllImportSearchPath? searchPath)
        {
                IntPtr libHandle = IntPtr.Zero;
                if (libraryName == DllName)
                {
                        libHandle = NativeLibrary.Load("libKukuLib.so", assembly, DllImportSearchPath.AssemblyDirectory);
                        Console.WriteLine(libHandle == IntPtr.Zero);
                }
                
                return libHandle;
        }

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

    [StructLayout(LayoutKind.Sequential)]
    public struct QueryResult
    {
        public ulong location;
        public ulong loc_func_index;
        public bool in_stash;
    }
}
