using System;
using System.Collections.Generic;
using System.Text;

namespace KukuDotNet
{
    public class KukuTableDotNet<TItem> where TItem : IItem, new()
    {
        private readonly IntPtr _unmanagedkukuTable;
        private readonly int _dataLen;

        public KukuTableDotNetTable<TItem> Table { get; }

        public KukuTableDotNetStash<TItem> Stash { get; }

        public KukuTableDotNet(KukuTableParameters parameters)
        {
            this._unmanagedkukuTable = KukuDotNet.kukuTable_Create(parameters.LogTableSize, parameters.StashSize,
                parameters.LocFuncCount, parameters.LocFuncSeed, parameters.MaxProbe, parameters.EmptyItem);
            this._dataLen = new TItem().DataLen;
            this.Table = new KukuTableDotNetTable<TItem>(this);
            this.Stash = new KukuTableDotNetStash<TItem>(this);
        }

        public bool Insert(TItem item)
        {
            return KukuDotNet.kukuTable_Insert(this._unmanagedkukuTable, item.Data);
        }

        public QueryResultDotNet Query(TItem item)
        {
            var queryResult = new QueryResult();
            KukuDotNet.kukuTable_Query(this._unmanagedkukuTable, item.Data, ref queryResult);
            return new QueryResultDotNet(queryResult);
        }

        public bool IsEmptyItem(TItem item)
        {
            return KukuDotNet.kukuTable_IsEmptyItem(this._unmanagedkukuTable, item.Data);
        }

        public TItem LastInsertFailItem()
        {
            var numArray = new ulong[this._dataLen];
            KukuDotNet.kukuTable_LastInsertFailItem(this._unmanagedkukuTable, numArray);
            return new TItem {Data = numArray};
        }

        public double FillRate()
        {
            return KukuDotNet.kukuTable_FillRate(this._unmanagedkukuTable);
        }

        public uint GetLocation(TItem item, uint locFuncIndex)
        {
            return KukuDotNet.kukuTable_Location(this._unmanagedkukuTable, item.Data, locFuncIndex);
        }

        private TItem GetTableItem(uint index)
        {
            var numArray = new ulong[this._dataLen];
            KukuDotNet.kukuTable_Table(this._unmanagedkukuTable, index, numArray);
            return new TItem {Data = numArray};
        }

        private uint KukuTableSize()
        {
            return KukuDotNet.kukuTable_TableSize(this._unmanagedkukuTable);
        }

        private TItem GetStashItem(uint index)
        {
            var numArray = new ulong[this._dataLen];
            KukuDotNet.kukuTable_Stash(this._unmanagedkukuTable, index, numArray);
            return new TItem {Data = numArray};
        }

        private uint StashSize()
        {
            return KukuDotNet.kukuTable_StashSize(this._unmanagedkukuTable);
        }

        public class KukuTableDotNetTable<TTableItem> where TTableItem : IItem, new()
        {
            private readonly KukuTableDotNet<TTableItem> _kukuTable;

            public KukuTableDotNetTable(KukuTableDotNet<TTableItem> kukuTable)
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

        public class KukuTableDotNetStash<TStashItem> where TStashItem : IItem, new()
        {
            private readonly KukuTableDotNet<TStashItem> _kukuTable;

            public KukuTableDotNetStash(KukuTableDotNet<TStashItem> kukuTable)
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
