using System;
using System.Collections.Generic;
using System.Text;

namespace KukuDotNet
{
    public interface IItem
    {
        ulong[] Data { get; set; }

        int DataLen { get; }
    }
}
