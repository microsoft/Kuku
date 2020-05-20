// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

using System;
using System.Collections.Generic;
using System.Text;

namespace Microsoft.Research.Kuku
{
    /// <summary>
    /// The KukuTable class represents a cockoo hash table. It includes information about the location functions (hash
    /// functions) and holds the items inserted into the table.
    /// </summary>
    public class KukuTable
    {
        private readonly IntPtr _unmanagedkukuTable;

        /// <summary>Returns a <see cref="KukuTableTable"/> instance referring to the hash table.</summary>
        public KukuTableTable Table { get; }

        /// <summary>Returns a <see cref="KukuTableStash"/> instance referring to the stash.</summary>
        public KukuTableStash Stash { get; }

        /// <summary>Creates a new empty hash table with given <see cref="KukuTableParameters"/>.</summary>
        /// <param name="parameters">The hash table parameters to use</param>
        /// <exception cref="ArgumentException">if <see cref="KukuTableParameters.LocFuncCount"/> (in parameters) is too
        /// large or too small</exception>
        /// <exception cref="ArgumentException">if <see cref="KukuTableParameters.TableSize"/> (in parameters) is too
        /// large or too small</exception>
        /// <exception cref="ArgumentException">if <see cref="KukuTableParameters.MaxProbe"/> (in parameters) is
        /// zero</exception>
        public KukuTable(KukuTableParameters parameters)
        {
            if (parameters.LocFuncCount < Common.MinLocFuncCount ||
                parameters.LocFuncCount > Common.MaxLocFuncCount)
            {
                throw new ArgumentException($"{nameof(parameters.LocFuncCount)} is out of range");
            }
            if (parameters.TableSize < Common.MinTableSize ||
                parameters.TableSize > Common.MaxTableSize)
            {
                throw new ArgumentException($"{nameof(parameters.TableSize)} is out of range");
            }
            if (0 == parameters.MaxProbe)
            {
                throw new ArgumentException($"{nameof(parameters.MaxProbe)} cannot be zero");
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

        /// <summary>
        /// Adds a single item to the hash table using random walk cuckoo hashing. The return value indicates whether
        /// the item was successfully inserted (possibly into the stash) or not.
        /// </summary>
        /// <param name="item">The hash table item to insert</param>
        /// <exception cref="ArgumentNullException">if item is null</exception>
        /// <exception cref="ArgumentException">if the given item is the empty item for this hash table</exception>
        public bool Insert(Item item)
        {
            if (null == item)
            {
                throw new ArgumentNullException($"{nameof(item)} cannot be null");
            }
            if (IsEmptyItem(item))
            {
                throw new ArgumentException($"{nameof(item)} cannot be the empty item");
            }

            ulong[] data = new ulong[2] { item.Data.Item1, item.Data.Item2 };
            return NativeMethods.KukuTable_Insert(_unmanagedkukuTable, data);
        }

        /// <summary>Queries for the presence of a given item in the hash table and stash.</summary>
        /// <param name="item">The hash table item to query</param>
        /// <exception cref="ArgumentNullException">if item is null</exception>
        /// <exception cref="ArgumentException">if the given item is the empty item for this hash table</exception>
        public QueryResult Query(Item item)
        {
            if (null == item)
            {
                throw new ArgumentNullException($"{nameof(item)} cannot be null");
            }
            if (IsEmptyItem(item))
            {
                throw new ArgumentException($"{nameof(item)} cannot be the empty item");
            }

            var queryResult = new QueryResultData();
            ulong[] data = new ulong[2] { item.Data.Item1, item.Data.Item2 };
            NativeMethods.KukuTable_Query(_unmanagedkukuTable, data, ref queryResult);
            return new QueryResult(queryResult);
        }

        /// <summary>Returns whether a given item is the empty item for this hash table.</summary>
        /// <exception cref="ArgumentNullException">if item is null</exception>
        public bool IsEmptyItem(Item item)
        {
            if (null == item)
            {
                throw new ArgumentNullException($"{nameof(item)} cannot be null");
            }

            ulong[] data = new ulong[2] { item.Data.Item1, item.Data.Item2 };
            return NativeMethods.KukuTable_IsEmptyItem(_unmanagedkukuTable, data);
        }

        /// <summary>Returns whether a given location in the table is empty.</summary>
        /// <param name="index">The index in the hash table</param>
        /// <exception cref="ArgumentOutOfRangeException">if index is out of range</exception>
        public bool IsEmpty(uint index)
        {
            return IsEmptyItem(this[index]);
        }

        /// <summary>Returns the hash table item that represents an empty location in the table.</summary>
        public Item EmptyItem
        {
            get
            {
                ulong[] data = new ulong[2];
                NativeMethods.KukuTable_EmptyItem(_unmanagedkukuTable, data);
                return Item.MakeItem((data[0], data[1]));
            }
        }

        /// <summary>Clears the hash table by filling every location with the empty item.</summary>
        public void ClearTable()
        {
            NativeMethods.KukuTable_ClearTable(_unmanagedkukuTable);
        }

        /// <summary>Returns the leftover item.</summary>
        /// <remarks>
        /// When the insert function fails to insert a hash table item, there is a leftover item that could not be
        /// inserted into the table. This function will return the empty item if insertion never failed, and otherwise
        /// it will return the latest leftover item. Note that due to how the random walk insertion process works, the
        /// leftover item is usually not the same one that insert was called with.  
        /// </remarks>
        public Item LeftoverItem
        {
            get
            {
                var data = new ulong[2];
                NativeMethods.KukuTable_LeftoverItem(_unmanagedkukuTable, data);
                return Item.MakeItem((data[0], data[1]));
            }
        }

        /// <summary>Returns the current fill rate of the hash table and stash.</summary>
        public double FillRate => NativeMethods.KukuTable_FillRate(_unmanagedkukuTable);

        /// <summary>Returns the number of location functions used by the hash table.</summary>
        public uint LocFuncCount => NativeMethods.KukuTable_LocFuncCount(_unmanagedkukuTable);

        /// <summary>Returns a location that a given hash table item may be placed at.</summary>
        /// <param name="item">The hash table item for which the location is to be obtained</param>
        /// <param name="locFuncIndex">The index of the location function which to use to compute the location</param>
        /// <exception cref="ArgumentNullException">if item is null</exception>
        /// <exception cref="ArgumentOutOfRangeException">if locFuncIndex is out of range</exception>
        /// <exception cref="ArgumentException">if the given item is the empty item for this hash table</exception>
        public uint Location(Item item, uint locFuncIndex)
        {
            if (null == item)
            {
                throw new ArgumentNullException($"{nameof(item)} cannot be null");
            }
            if (locFuncIndex >= LocFuncCount)
            {
                throw new ArgumentOutOfRangeException($"{nameof(locFuncIndex)} is out of range");
            }
            if (IsEmptyItem(item))
            {
                throw new ArgumentException($"{nameof(item)} cannot be the empty item");
            }

            ulong[] data = new ulong[2] { item.Data.Item1, item.Data.Item2 };
            return NativeMethods.KukuTable_Location(_unmanagedkukuTable, data, locFuncIndex);
        }

        /// <summary>Returns all hash table locations that this item may be placed at.</summary>
        /// <param name="item">The hash table item for which the location is to be obtained</param>
        /// <exception cref="ArgumentNullException">if item is null</exception>
        /// <exception cref="ArgumentException">if the given item is the empty item for this hash table</exception>
        public HashSet<uint> AllLocations(Item item)
        {
            if (null == item)
            {
                throw new ArgumentNullException($"{nameof(item)} cannot be null");
            }
            if (IsEmptyItem(item))
            {
                throw new ArgumentException($"{nameof(item)} cannot be the empty item");
            }

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

        /// <summary>Returns a reference to a specific location in the hash table.</summary>
        /// <param name="index">The index in the hash table</param>
        /// <exception cref="ArgumentOutOfRangeException">if index is out of range</exception>
        public Item this[uint index]
        {
            get
            {
                if (index >= TableSize)
                {
                    throw new ArgumentOutOfRangeException($"{nameof(index)} is out of range");
                }

                var data = new ulong[2];
                NativeMethods.KukuTable_Table(_unmanagedkukuTable, index, data);
                return Item.MakeItem((data[0], data[1]));
            }
        }

        /// <summary>Returns the size of the hash table.</summary>
        public uint TableSize => NativeMethods.KukuTable_TableSize(_unmanagedkukuTable);

        /// <summary>Returns a reference to a specific location in the stash.</summary>
        /// <param name="index">The index in the stash</param>
        /// <exception cref="ArgumentOutOfRangeException">if index is out of range</exception>
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

        /// <summary>Returns the size of the stash for the hash table.</summary>
        public uint StashSize => NativeMethods.KukuTable_StashSize(_unmanagedkukuTable);

        /// <summary>Gives access to the hash table associated with a <see cref="KukuTable"/> instance.</summary>
        /// <remarks>
        /// Gives access to the hash table associated with a <see cref="KukuTable"/> instance. The hash table is
        /// automatically created when a <see cref="KukuTable"/> instance is created, and can be accessed through the
        /// <see cref="KukuTable"/> class as well. This class merely provides another convenient handle to view the hash
        /// table data.
        /// </remarks>
        public class KukuTableTable
        {
            private readonly KukuTable _kukuTable;

            /// <summary>
            /// Creates a new KukuTableTable object referring to the hash table of a given <see cref="KukuTable"/>.
            /// </summary>
            /// <param name="kukuTable">The <see cref="KukuTable"/> instance this hash table belongs to.</param>
            /// <exception cref="ArgumentNullException">if kukuTable is null</exception>
            public KukuTableTable(KukuTable kukuTable)
            {
                _kukuTable = kukuTable ?? throw new ArgumentNullException($"{nameof(kukuTable)} cannot be null");
            }

            /// <summary>Returns a reference to a specific location in the hash table.</summary>
            /// <param name="index">The index in the hash table</param>
            /// <exception cref="ArgumentOutOfRangeException">if index is out of range</exception>
            public Item this[uint index] => _kukuTable[index];

            /// <summary>Returns the size of the hash table.</summary>
            public uint Size => _kukuTable.TableSize;

            /// <summary>Returns the hash table data as a formatted string.</summary>
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

        /// <summary>Gives access to the stash associated with a <see cref="KukuTable"/> instance.</summary>
        /// <remarks>
        /// Gives access to the stash associated with a <see cref="KukuTable"/> instance. The stash is automatically used
        /// for items that could otherwise not fit into the hash table. The stash can have any size, including zero.
        /// True insertion failure occurs only when a hash table item cannot be inserted into the table, and the stash
        /// is full so it cannot be inserted into the stash either. A stash is automatically created when a hash table
        /// is created, and can be accessed through the <see cref="KukuTable"/> class as well. This class merely provides
        /// another convenient handle to view the stash data.
        /// </remarks>
        public class KukuTableStash
        {
            private readonly KukuTable _kukuTable;

            /// <summary>
            /// Creates a new KukuTableStash object referring to the stash of a given <see cref="KukuTable"/>.
            /// </summary>
            /// <param name="kukuTable">The <see cref="KukuTable"/> instance this stash belongs to.</param>
            /// <exception cref="ArgumentNullException">if kukuTable is null</exception>
            public KukuTableStash(KukuTable kukuTable)
            {
                _kukuTable = kukuTable ?? throw new ArgumentNullException($"{nameof(kukuTable)} cannot be null");
            }

            /// <summary>Returns a reference to a specific location in the stash.</summary>
            /// <param name="index">The index in the stash</param>
            /// <exception cref="ArgumentOutOfRangeException">if index is out of range</exception>
            public Item this[uint index] => _kukuTable.StashItem(index);

            /// <summary>Returns the size of the stash.</summary>
            public uint Size => _kukuTable.StashSize;

            /// <summary>Returns the stash data as a formatted string.</summary>
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