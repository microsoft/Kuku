using System;
using System.Collections.Generic;
using System.Text;

namespace CuckooDotNet
{
    public class CuckooTableDotNet<TItem> where TItem : IItem, new()
    {
        private readonly IntPtr _unmanagedCuckooTable;
        private readonly int _dataLen;

        public CuckooTableDotNetTable<TItem> Table { get; }

        public CuckooTableDotNetStash<TItem> Stash { get; }

        public CuckooTableDotNet(CuckooTableParameters parameters)
        {
            this._unmanagedCuckooTable = CuckooDotNet.CuckooTable_Create(parameters.LogTableSize, parameters.StashSize,
                parameters.LocFuncCount, parameters.LocFuncSeed, parameters.MaxProbe, parameters.EmptyItem);
            this._dataLen = new TItem().DataLen;
            this.Table = new CuckooTableDotNetTable<TItem>(this);
            this.Stash = new CuckooTableDotNetStash<TItem>(this);
        }

        public bool Insert(TItem item)
        {
            return CuckooDotNet.CuckooTable_Insert(this._unmanagedCuckooTable, item.Data);
        }

        public QueryResultDotNet Query(TItem item)
        {
            var queryResult = new QueryResult();
            CuckooDotNet.CuckooTable_Query(this._unmanagedCuckooTable, item.Data, ref queryResult);
            return new QueryResultDotNet(queryResult);
        }

        public bool IsEmptyItem(TItem item)
        {
            return CuckooDotNet.CuckooTable_IsEmptyItem(this._unmanagedCuckooTable, item.Data);
        }

        public TItem LastInsertFailItem()
        {
            var numArray = new ulong[this._dataLen];
            CuckooDotNet.CuckooTable_LastInsertFailItem(this._unmanagedCuckooTable, numArray);
            return new TItem {Data = numArray};
        }

        public double FillRate()
        {
            return CuckooDotNet.CuckooTable_FillRate(this._unmanagedCuckooTable);
        }

        private TItem GetTableItem(uint index)
        {
            var numArray = new ulong[this._dataLen];
            CuckooDotNet.CuckooTable_Table(this._unmanagedCuckooTable, index, numArray);
            return new TItem {Data = numArray};
        }

        private uint TableSize()
        {
            return CuckooDotNet.CuckooTable_TableSize(this._unmanagedCuckooTable);
        }

        private TItem GetStashItem(uint index)
        {
            var numArray = new ulong[this._dataLen];
            CuckooDotNet.CuckooTable_Stash(this._unmanagedCuckooTable, index, numArray);
            return new TItem {Data = numArray};
        }

        private uint StashSize()
        {
            return CuckooDotNet.CuckooTable_StashSize(this._unmanagedCuckooTable);
        }

        public class CuckooTableDotNetTable<TTableItem> where TTableItem : IItem, new()
        {
            private readonly CuckooTableDotNet<TTableItem> _cuckooTable;

            public CuckooTableDotNetTable(CuckooTableDotNet<TTableItem> cuckooTable)
            {
                this._cuckooTable = cuckooTable;
            }

            public TTableItem this[uint index] => this._cuckooTable.GetTableItem(index);

            public uint Size => this._cuckooTable.TableSize();

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

        public class CuckooTableDotNetStash<TStashItem> where TStashItem : IItem, new()
        {
            private readonly CuckooTableDotNet<TStashItem> _cuckooTable;

            public CuckooTableDotNetStash(CuckooTableDotNet<TStashItem> cuckooTable)
            {
                this._cuckooTable = cuckooTable;
            }

            public TStashItem this[uint index] => this._cuckooTable.GetStashItem(index);

            public uint Size => this._cuckooTable.StashSize();

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
