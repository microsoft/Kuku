// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

using System;
using System.Text;

namespace Microsoft.Research.Kuku
{
    public class KukuTable<TItem> where TItem : IItem, new()
    {
        private readonly IntPtr _unmanagedkukuTable;
        private readonly int _dataLen;

        public KukuTableTable<TItem> Table { get; }

        public KukuTableStash<TItem> Stash { get; }

        public KukuTable(KukuTableParameters parameters)
        {
            _unmanagedkukuTable = NativeMethods.KukuTable_Create(
                parameters.TableSize,
                parameters.StashSize,
                parameters.LocFuncCount,
                parameters.LocFuncSeed,
                parameters.MaxProbe,
                parameters.EmptyItem);

            _dataLen = new TItem().DataLen;
            Table = new KukuTableTable<TItem>(this);
            Stash = new KukuTableStash<TItem>(this);
        }

        public bool Insert(TItem item) => NativeMethods.KukuTable_Insert(_unmanagedkukuTable, item.Data);

        public QueryResultWrapper Query(TItem item)
        {
            var queryResult = new QueryResult();
            NativeMethods.KukuTable_Query(_unmanagedkukuTable, item.Data, ref queryResult);
            return new QueryResultWrapper(queryResult);
        }

        public bool IsEmptyItem(TItem item) => NativeMethods.KukuTable_IsEmptyItem(_unmanagedkukuTable, item.Data);

        public TItem LeftoverItem
        {
            get
            {
                var numArray = new ulong[this._dataLen];
                NativeMethods.KukuTable_LeftoverItem(_unmanagedkukuTable, numArray);
                return new TItem { Data = numArray };
            }
        }

        public double FillRate => NativeMethods.KukuTable_FillRate(_unmanagedkukuTable);

        public uint GetLocation(TItem item, uint locFuncIndex) =>
            NativeMethods.KukuTable_Location(_unmanagedkukuTable, item.Data, locFuncIndex);

        private TItem GetTableItem(uint index)
        {
            var numArray = new ulong[_dataLen];
            NativeMethods.KukuTable_Table(_unmanagedkukuTable, index, numArray);
            return new TItem {Data = numArray};
        }

        private uint KukuTableSize => NativeMethods.KukuTable_TableSize(_unmanagedkukuTable);

        private TItem GetStashItem(uint index)
        {
            var numArray = new ulong[_dataLen];
            NativeMethods.KukuTable_Stash(_unmanagedkukuTable, index, numArray);
            return new TItem {Data = numArray};
        }

        private uint StashSize => NativeMethods.KukuTable_StashSize(_unmanagedkukuTable);

        public class KukuTableTable<TTableItem> where TTableItem : IItem, new()
        {
            private readonly KukuTable<TTableItem> _kukuTable;

            public KukuTableTable(KukuTable<TTableItem> kukuTable)
            {
                _kukuTable = kukuTable;
            }

            public TTableItem this[uint index] => _kukuTable.GetTableItem(index);

            public uint Size => _kukuTable.KukuTableSize;

            public override string ToString()
            {
                var sb = new StringBuilder();
                uint columns = 8;
                uint size = Size;

                for (uint i = 0; i < size; ++i)
                {
                    sb.Append(string.Format(
                        string.Join(',', this[i]) + ((i % columns == columns - 1) ? "\n" : "\t")));
                }
                return sb.ToString();
            }
        }

        public class KukuTableStash<TStashItem> where TStashItem : IItem, new()
        {
            private readonly KukuTable<TStashItem> _kukuTable;

            public KukuTableStash(KukuTable<TStashItem> kukuTable)
            {
                _kukuTable = kukuTable;
            }

            public TStashItem this[uint index] => _kukuTable.GetStashItem(index);

            public uint Size => _kukuTable.StashSize;

            public override string ToString()
            {
                var sb = new StringBuilder();
                var size = Size;

                for (uint i = 0; i < size; ++i)
                {
                    sb.Append($"{i}: {string.Join(',', this[i])}\t");
                }

                sb.Append("\n");
                return sb.ToString();
            }
        }
    }
}