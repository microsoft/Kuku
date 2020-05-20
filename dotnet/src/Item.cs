// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

using System;

namespace Microsoft.Research.Kuku
{
    /// <summary>Represents a 128-bit item that can be added to the hash table.</summary>
    public class Item : IEquatable<Item>
    {
        /// <summary>Creates a new hash table item with all zero bits.</summary>
        public Item()
        {
        }

        /// <summary>Creates a copy of a given hash table item.</summary>
        /// <param name="copy">The hash table item to copy from</param>
        public Item(Item copy)
        {
            Data = copy.Data;
        }

        /// <summary>Assigns this hash table item to be equal to a given item.</summary>
        /// <param name="assign">The hash table item to assign from</param>
        public void Set(Item assign) => Data = assign.Data;

        /// <summary>The hash table item value represented as a pair of 64-bit words.</summary>
        public (ulong, ulong) Data { get; set; } = (0, 0);

        /// <summary>Creates a hash table item and sets its value from a pair of 64-bit words.</summary>
        /// <param name="data">Pair of high and low words representing the value of the hash table item</param>
        public static Item MakeItem((ulong, ulong) data) => new Item(data);

        /// <summary>Creates a hash table item and sets its value from a pair of 64-bit words.</summary>
        /// <param name="lw">The low 64 bits of the value of the item</param>
        /// <param name="hw">The high 64 bits of the value of the item</param>
        public static Item MakeItem(ulong lw, ulong hw) => new Item((lw, hw));

        /// <summary>Sets the value of the hash table item from a pair of 64-bit words.</summary>
        /// <param name="data">Pair of high and low words representing the value of the hash table item</param>
        public void SetItem((ulong, ulong) data) => Data = data;

        /// <summary>Sets the value of the hash table item from a pair of 64-bit words.</summary>
        /// <param name="lw">The low 64 bits of the value of the item</param>
        /// <param name="hw">The high 64 bits of the value of the item</param>
        public void SetItem(ulong lw, ulong hw) => SetItem((lw, hw));

        /// <summary>Creates a zero hash table item.</summary>
        public static Item MakeZeroItem() => new Item();

        /// <summary>Sets the value of this hash table item to zero.</summary>
        public void SetZeroItem() => Data = (0, 0);

        /// <summary>Creates a hash table item and sets its value to all one-bits.</summary>
        public static Item MakeAllOnesItem() => new Item((~(ulong)0, ~(ulong)0));

        /// <summary>Sets the value of a given hash table item to all one-bits.</summary>
        public void SetAllOnesItem() => Data = (~(ulong)0, ~(ulong)0);

        /// <summary>Returns whether a given hash table item is zero.</summary>
        public bool IsZeroItem() => Data == (0, 0);

        /// <summary>Returns whether a given has table item has all one-bits.</summary>
        public bool IsAllOnesItem() => Data == (~(ulong)0, ~(ulong)0);

        /// <summary>Returns whether two hash table items are equal.</summary>
        /// <param name="obj">An Item object to be compared with this item</param>
        public override bool Equals(object obj) => Equals(obj as Item);

        /// <summary>Returns whether two hash table items are equal.</summary>
        /// <param name="other">An item to be compared with this item</param>
        public bool Equals(Item other) => Data == other.Data;

        /// <summary>Returns the hash code for this item.</summary>
        public override int GetHashCode()
        {
            return Data.GetHashCode();
        }

        /// <summary>Creates a random hash table item.</summary>
        public static Item MakeRandomItem()
        {
            ulong[] itemArray = new ulong[2];
            NativeMethods.Common_SetRandomItem(itemArray);
            return new Item((itemArray[0], itemArray[1]));
        }

        /// <summary>Sets this hash table item to a random value.</summary>
        public void SetRandomItem()
        {
            ulong[] itemArray = new ulong[2];
            NativeMethods.Common_SetRandomItem(itemArray);
            Data = (itemArray[0], itemArray[1]);
        }

        /// <summary>Interprets a hash table item as a 128-bit integer and increments its value by one.</summary>
        public void IncrementItem()
        {
            ulong[] itemArray = new ulong[2] { Data.Item1, Data.Item2 };
            NativeMethods.Common_IncrementItem(itemArray);
            Data = (itemArray[0], itemArray[1]);
        }

        /// <summary>Returns the item as a formatted string with high and low words separated by a comma.</summary>
        public override string ToString()
        {
            return string.Format($"{Data.Item1},{Data.Item2}");
        }

        private Item((ulong, ulong) data) => Data = data;
    }
}