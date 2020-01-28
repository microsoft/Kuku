// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

using System;

namespace Microsoft.Research.Kuku
{
    public class Item128 : IItem
    {
        private ulong[] _data;

        public int DataLen => 2;

        public ulong[] Data
        {
            get => this._data;
            set
            {
                if (value.Length != 2)
                {
                    throw new ArgumentException(
                        $"Invalid input array. Expected array of length 2. Got: {value.Length}.");
                }

                this._data = new[] { value[0], value[1] };
            }
        }

        public override string ToString()
        {
            return this._data == null ?
                string.Empty : string.Format($"{this._data[0]},{this._data[1]}");
        }
    }
}
