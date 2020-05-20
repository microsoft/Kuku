// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

namespace Microsoft.Research.Kuku
{
    /// <summary>
    /// Set of common properties
    /// </summary>
    public static class Common
    {
        /// <summary>The smallest allowed table size.</summary>
        public static uint MinTableSize => NativeMethods.Common_MinTableSize();

        /// <summary>The largest allowed table size.</summary>
        public static uint MaxTableSize => NativeMethods.Common_MaxTableSize();

        /// <summary>The smallest allowed number of hash functions.</summary>
        public static uint MinLocFuncCount => NativeMethods.Common_MinLocFuncCount();

        /// <summary>The largest allowed number of hash functions. This must be a power of two for correct behavior.</summary>
        public static uint MaxLocFuncCount => NativeMethods.Common_MaxLocFuncCount();
    }
}