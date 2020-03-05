// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

namespace Microsoft.Research.Kuku
{
    public static class Common
    {
        public static uint MinTableSize => NativeMethods.Common_MinTableSize();

        public static uint MaxTableSize => NativeMethods.Common_MaxTableSize();

        public static uint MinLocFuncCount => NativeMethods.Common_MinLocFuncCount();

        public static uint MaxLocFuncCount => NativeMethods.Common_MaxLocFuncCount();
    }
}