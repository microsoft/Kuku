// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

using System;

namespace Microsoft.Research.Kuku
{
    public class Item : IEquatable<Item>
    {
        public Item()
        {
        }

        public Item(Item copy)
        {
            Data = copy.Data;
        }

        public void Set(Item assign) => Data = assign.Data;

        public (ulong, ulong) Data { get; set; } = (0, 0);

        public static Item MakeItem((ulong, ulong) data) => new Item(data);

        public static Item MakeItem(ulong lw, ulong hw) => new Item((lw, hw));

        public void SetItem((ulong, ulong) data) => Data = data;

        public void SetItem(ulong lw, ulong hw) => SetItem((lw, hw));

        public static Item MakeZeroItem() => new Item();

        public void SetZeroItem() => Data = (0, 0);

        public static Item MakeAllOnesItem() => new Item((~(ulong)0, ~(ulong)0));

        public void SetAllOnesItem() => Data = (~(ulong)0, ~(ulong)0);

        public bool IsZeroItem() => Data == (0, 0);

        public bool IsAllOnesItem() => Data == (~(ulong)0, ~(ulong)0);

        public override bool Equals(object obj) => Equals(obj as Item);

        public bool Equals(Item other) => Data == other.Data;

        public override int GetHashCode()
        {
            return Data.GetHashCode();
        }

        public static Item MakeRandomItem()
        {
            ulong[] itemArray = new ulong[2];
            NativeMethods.Common_SetRandomItem(itemArray);
            return new Item((itemArray[0], itemArray[1]));
        }

        public void SetRandomItem()
        {
            ulong[] itemArray = new ulong[2];
            NativeMethods.Common_SetRandomItem(itemArray);
            Data = (itemArray[0], itemArray[1]);
        }

        public void IncrementItem()
        {
            ulong[] itemArray = new ulong[2] { Data.Item1, Data.Item2 };
            NativeMethods.Common_IncrementItem(itemArray);
            Data = (itemArray[0], itemArray[1]);
        }

        public override string ToString()
        {
            return string.Format($"{Data.Item1},{Data.Item2}");
        }

        private Item((ulong, ulong) data) => Data = data;
    }
}