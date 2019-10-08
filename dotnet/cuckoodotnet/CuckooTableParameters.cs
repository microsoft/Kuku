using System.Runtime.Serialization;

namespace KukuDotNet
{
    [DataContract]
    public class KukuTableParameters
    {
        [DataMember(Order = 1)]
        public int LogTableSize { get; set; }

        [DataMember(Order = 2)]
        public uint StashSize { get; set; }

        [DataMember(Order = 3)]
        public uint LocFuncCount { get; set; }

        [DataMember(Order = 4)]
        public ulong[] LocFuncSeed { get; set; }

        [DataMember(Order = 5)]
        public ulong MaxProbe { get; set; }

        [DataMember(Order = 6)]
        public ulong[] EmptyItem { get; set; }
    }
}
