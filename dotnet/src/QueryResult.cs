// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

namespace Microsoft.Research.Kuku
{
    /// <summary>The QueryResult class represents the result of a hash table query.</summary>
    /// <remarks>
    /// The QueryResult class represents the result of a hash table query. It includes information about whether a
    /// queried item was found in the hash table, its location in the hash table or stash (if found), and the index of
    /// the location function (hash function) that was used to insert it. QueryResult objects are returned by the
    /// <see cref="KukuTable.Query" /> function.
    /// </remarks>
    public class QueryResult
    {
        private readonly QueryResultData _resultStruct;

        internal QueryResult(QueryResultData resultStruct)
        {
             _resultStruct = resultStruct;
        }

        /// <summary>Returns whether the queried item was found in the hash table or in the stash.</summary>
        public bool Found => _resultStruct.found;

        /// <summary>Returns whether the queried item was found in the stash.</summary>
        public bool InStash => _resultStruct.in_stash;

        /// <summary>Returns the hash table or stash location represented by this QueryResult.</summary>
        public uint Location => _resultStruct.location;

        /// <summary>Returns the index of the location function that was used to insert the queried item.</summary>
        /// <remarks>
        /// Returns the index of the location function that was used to insert the queried item. This value is
        /// meaningless when <see cref="InStash" /> is true. A value equal to <see cref="Common.MaxLocFuncCount" /> indicates
        /// the item was not found in the table or stash.
        /// </remarks>
        public uint LocFuncIndex => _resultStruct.loc_func_index;
    }
}