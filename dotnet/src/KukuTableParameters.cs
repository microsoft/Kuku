// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

using System.Runtime.Serialization;

namespace Microsoft.Research.Kuku
{
    [DataContract]
    public class KukuTableParameters
    {
        [DataMember(Order = 1)]
        public uint TableSize { get; set; }

        [DataMember(Order = 2)]
        public uint StashSize { get; set; }

        [DataMember(Order = 3)]
        public uint LocFuncCount { get; set; }

        [DataMember(Order = 4)]
        public Item LocFuncSeed { get; set; }

        [DataMember(Order = 5)]
        public ulong MaxProbe { get; set; }

        [DataMember(Order = 6)]
        public Item EmptyItem { get; set; }
    }
}
