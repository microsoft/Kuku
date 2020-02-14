// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

namespace Microsoft.Research.Kuku
{
    public interface IItem
    {
        ulong[] Data { get; set; }

        int DataLen { get; }
    }
}