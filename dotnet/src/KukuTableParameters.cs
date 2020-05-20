// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

using System.Runtime.Serialization;

namespace Microsoft.Research.Kuku
{
    /// <summary>
    /// Parameters for creating a Kuku table
    /// </summary>
    [DataContract]
    public class KukuTableParameters
    {
        /// <summary>The size of the hash table.</summary>
        [DataMember(Order = 1)]
        public uint TableSize { get; set; }

        /// <summary>The size of the stash (possibly zero).</summary>
        [DataMember(Order = 2)]
        public uint StashSize { get; set; }

        /// <summary>The number of location functions (hash functions) to use.</summary>
        [DataMember(Order = 3)]
        public uint LocFuncCount { get; set; }

        /// <summary>The 128-bit seed for the location functions, represented as a hash table item.</summary>
        [DataMember(Order = 4)]
        public Item LocFuncSeed { get; set; }

        /// <summary>The maximum number of random walk steps taken in attempting to insert an item.</summary>
        [DataMember(Order = 5)]
        public ulong MaxProbe { get; set; }

        /// <summary>A hash table item that represents an empty location in the table.</summary>
        [DataMember(Order = 6)]
        public Item EmptyItem { get; set; }
    }
}
