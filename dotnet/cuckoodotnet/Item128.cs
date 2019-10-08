using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;

namespace CuckooDotNet
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
            return this._data == null ? string.Empty : string.Format($"{this._data[0]},{this._data[1]}");
        }
    }
}
