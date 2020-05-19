// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

using System;
using System.Collections.Generic;
using System.Text;

namespace Microsoft.Research.Kuku
{
    public class KukuTable
    {
        private readonly IntPtr _unmanagedkukuTable;

        public KukuTableTable Table { get; }

        public KukuTableStash Stash { get; }

        public KukuTable(KukuTableParameters parameters)
        {
            if (parameters.LocFuncCount < Common.MinLocFuncCount ||
                parameters.LocFuncCount > Common.MaxLocFuncCount)
            {
                throw new ArgumentException("LocFuncCount is out of range");
            }
            if (parameters.TableSize < Common.MinTableSize ||
                parameters.TableSize > Common.MaxTableSize)
            {
                throw new ArgumentException("TableSize is out of range");
            }
            if (0 == parameters.MaxProbe)
            {
                throw new ArgumentException("MaxProbe cannot be zero");
            }

            ulong[] locFuncSeedData = new ulong[2] {
                parameters.LocFuncSeed.Data.Item1, parameters.LocFuncSeed.Data.Item2 };
            ulong[] emptyItemData = new ulong[2] {
                parameters.EmptyItem.Data.Item1, parameters.EmptyItem.Data.Item2 };

            _unmanagedkukuTable = NativeMethods.KukuTable_Create(
                parameters.TableSize,
                parameters.StashSize,
                parameters.LocFuncCount,
                locFuncSeedData,
                parameters.MaxProbe,
                emptyItemData);

            Table = new KukuTableTable(this);
            Stash = new KukuTableStash(this);
        }

        public bool Insert(Item item)
        {
            if (IsEmptyItem(item))
            {
                throw new ArgumentException("item cannot be the empty item");
            }

            ulong[] data = new ulong[2] { item.Data.Item1, item.Data.Item2 };
            Console.WriteLine($"Inserting {data[0]} - {data[1]}");
            return NativeMethods.KukuTable_Insert(_unmanagedkukuTable, data);
        }

        public QueryResult Query(Item item)
        {
            if (IsEmptyItem(item))
            {
                throw new ArgumentException("item cannot be the empty item");
            }

            var queryResult = new QueryResultData();
            ulong[] data = new ulong[2] { item.Data.Item1, item.Data.Item2 };
            NativeMethods.KukuTable_Query(_unmanagedkukuTable, data, ref queryResult);
            return new QueryResult(queryResult);
        }

        public bool IsEmptyItem(Item item)
        {
            ulong[] data = new ulong[2] { item.Data.Item1, item.Data.Item2 };
            return NativeMethods.KukuTable_IsEmptyItem(_unmanagedkukuTable, data);
        }

        public bool IsEmpty(uint index)
        {
            return IsEmptyItem(this[index]);
        }

        public Item EmptyItem
        {
            get
            {
                ulong[] data = new ulong[2];
                NativeMethods.KukuTable_EmptyItem(_unmanagedkukuTable, data);
                return Item.MakeItem((data[0], data[1]));
            }
        }

        public void ClearTable()
        {
            NativeMethods.KukuTable_ClearTable(_unmanagedkukuTable);
        }

        public Item LeftoverItem
        {
            get
            {
                var data = new ulong[2];
                NativeMethods.KukuTable_LeftoverItem(_unmanagedkukuTable, data);
                return Item.MakeItem((data[0], data[1]));
            }
        }

        public double FillRate => NativeMethods.KukuTable_FillRate(_unmanagedkukuTable);

        public uint LocFuncCount => NativeMethods.KukuTable_LocFuncCount(_unmanagedkukuTable);

        public uint Location(Item item, uint locFuncIndex)
        {
            if (locFuncIndex > LocFuncCount)
            {
                throw new ArgumentOutOfRangeException("index is out of range");
            }

            ulong[] data = new ulong[2] { item.Data.Item1, item.Data.Item2 };
            return NativeMethods.KukuTable_Location(_unmanagedkukuTable, data, locFuncIndex);
        }

        public HashSet<uint> AllLocations(Item item)
        {
            ulong[] data = new ulong[2] { item.Data.Item1, item.Data.Item2 };
            uint[] locations = new uint[LocFuncCount];
            NativeMethods.KukuTable_AllLocations(_unmanagedkukuTable, data, locations, out uint count);

            HashSet<uint> result = new HashSet<uint>();
            for (uint i = 0; i < count; i++)
            {
                result.Add(locations[i]);
            }
            return result;
        }

        public Item this[uint index]
        {
            get
            {
                if (index > TableSize)
                {
                    throw new ArgumentOutOfRangeException("index is out of range");
                }

                var data = new ulong[2];
                NativeMethods.KukuTable_Table(_unmanagedkukuTable, index, data);
                return Item.MakeItem((data[0], data[1]));
            }
        }

        public uint TableSize => NativeMethods.KukuTable_TableSize(_unmanagedkukuTable);

        public Item StashItem(uint index)
        {
            if (index > TableSize)
            {
                throw new ArgumentOutOfRangeException("index is out of range");
            }

            var data = new ulong[2];
            NativeMethods.KukuTable_Stash(_unmanagedkukuTable, index, data);
            return Item.MakeItem((data[0], data[1]));
        }

        public uint StashSize => NativeMethods.KukuTable_StashSize(_unmanagedkukuTable);

        public class KukuTableTable
        {
            private readonly KukuTable _kukuTable;

            public KukuTableTable(KukuTable kukuTable)
            {
                _kukuTable = kukuTable;
            }

            public Item this[uint index] => _kukuTable[index];

            public uint Size => _kukuTable.TableSize;

            public override string ToString()
            {
                var sb = new StringBuilder();
                uint columns = 8;
                uint size = Size;

                for (uint i = 0; i < size; ++i)
                {
                    sb.Append(string.Format(
                        string.Join(",", this[i]) + ((i % columns == columns - 1) ? "\n" : "\t")));
                }
                return sb.ToString();
            }
        }

        public class KukuTableStash
        {
            private readonly KukuTable _kukuTable;

            public KukuTableStash(KukuTable kukuTable)
            {
                _kukuTable = kukuTable;
            }

            public Item this[uint index] => _kukuTable.StashItem(index);

            public uint Size => _kukuTable.StashSize;

            public override string ToString()
            {
                var sb = new StringBuilder();
                var size = Size;

                for (uint i = 0; i < size; ++i)
                {
                    sb.Append($"{i}: {string.Join(",", this[i])}\t");
                }

                sb.Append("\n");
                return sb.ToString();
            }
        }
    }
}