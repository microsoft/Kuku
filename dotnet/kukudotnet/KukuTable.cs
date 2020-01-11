using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;

namespace KukuDotNet
{
    public class KukuTable<TItem> where TItem : IItem, new()
    {
        private readonly IntPtr _unmanagedkukuTable;
        private readonly int _dataLen;

        public KukuTableTable<TItem> Table { get; }

        public KukuTableStash<TItem> Stash { get; }

        public KukuTable(KukuTableParameters parameters)
        {
            this._unmanagedkukuTable = KukuNativeMethods.kukuTable_Create(parameters.TableSize, parameters.StashSize,
                parameters.LocFuncCount, parameters.LocFuncSeed, parameters.MaxProbe, parameters.EmptyItem);

            this._dataLen = new TItem().DataLen;
            this.Table = new KukuTableTable<TItem>(this);
            this.Stash = new KukuTableStash<TItem>(this);
        }

        public bool Insert(TItem item)
        {
            return KukuNativeMethods.kukuTable_Insert(this._unmanagedkukuTable, item.Data);
        }

        public QueryResultWrapper Query(TItem item)
        {
            var queryResult = new QueryResult();
            KukuNativeMethods.kukuTable_Query(this._unmanagedkukuTable, item.Data, ref queryResult);
            return new QueryResultWrapper(queryResult);
        }

        public bool IsEmptyItem(TItem item)
        {
            return KukuNativeMethods.kukuTable_IsEmptyItem(this._unmanagedkukuTable, item.Data);
        }

        public TItem LastInsertFailItem()
        {
            var numArray = new ulong[this._dataLen];
            KukuNativeMethods.kukuTable_LastInsertFailItem(this._unmanagedkukuTable, numArray);
            return new TItem {Data = numArray};
        }

        public double FillRate()
        {
            return KukuNativeMethods.kukuTable_FillRate(this._unmanagedkukuTable);
        }

        public uint GetLocation(TItem item, uint locFuncIndex)
        {
            return KukuNativeMethods.kukuTable_Location(this._unmanagedkukuTable, item.Data, locFuncIndex);
        }

        private TItem GetTableItem(uint index)
        {
            var numArray = new ulong[this._dataLen];
            KukuNativeMethods.kukuTable_Table(this._unmanagedkukuTable, index, numArray);
            return new TItem {Data = numArray};
        }

        private uint KukuTableSize()
        {
            return KukuNativeMethods.kukuTable_TableSize(this._unmanagedkukuTable);
        }

        private TItem GetStashItem(uint index)
        {
            var numArray = new ulong[this._dataLen];
            KukuNativeMethods.kukuTable_Stash(this._unmanagedkukuTable, index, numArray);
            return new TItem {Data = numArray};
        }

        private uint StashSize()
        {
            return KukuNativeMethods.kukuTable_StashSize(this._unmanagedkukuTable);
        }

        public class KukuTableTable<TTableItem> where TTableItem : IItem, new()
        {
            private readonly KukuTable<TTableItem> _kukuTable;

            public KukuTableTable(KukuTable<TTableItem> kukuTable)
            {
                this._kukuTable = kukuTable;
            }

            public TTableItem this[uint index] => this._kukuTable.GetTableItem(index);

            public uint Size => this._kukuTable.KukuTableSize();

            public override string ToString()
            {
                var sb = new StringBuilder();
                var columns = 8;

                for (var i = 0; i < this.Size / columns; i++)
                {
                    for (var j = 0; j < columns; ++j)
                    {
                        var index = (uint)(i * columns + j);
                        sb.Append(string.Format(string.Join(',', this[index]) + "\t"));
                    }
                    sb.Append("\n");
                }
                return sb.ToString();
            }
        }

        public class KukuTableStash<TStashItem> where TStashItem : IItem, new()
        {
            private readonly KukuTable<TStashItem> _kukuTable;

            public KukuTableStash(KukuTable<TStashItem> kukuTable)
            {
                this._kukuTable = kukuTable;
            }

            public TStashItem this[uint index] => this._kukuTable.GetStashItem(index);

            public uint Size => this._kukuTable.StashSize();

            public override string ToString()
            {
                var sb = new StringBuilder();
                var size = this.Size;

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
