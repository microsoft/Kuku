// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

using Microsoft.Research.Kuku;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;

namespace KukuNetTest
{
    [TestClass]
    public class ItemTests
    {
        [TestMethod]
        public void SetItem()
        {
            Item bl = new Item();
            bl.SetItem((0, 0));
            Assert.AreEqual((0ul, 0ul), bl.Data);

            bl.SetItem((1, 0));
            Assert.AreEqual((1ul, 0ul), bl.Data);

            bl.SetItem((0, 1));
            Assert.AreEqual((0ul, 1ul), bl.Data);

            bl.SetItem((0xF00F, 0xBABA));
            Assert.AreEqual((0xF00Ful, 0xBABAul), bl.Data);

            bl.SetItem((0xF00FF00FF00FF00Ful, 0xBABABABABABABABAul));
            Assert.AreEqual((0xF00FF00FF00FF00Ful, 0xBABABABABABABABAul), bl.Data);
        }

        [TestMethod]
        public void SetZeroItem()
        {
            Item bl = new Item();
            bl.SetZeroItem();
            Assert.AreEqual((0ul, 0ul), bl.Data);

            bl.SetItem((0xF00FF00FF00FF00F, 0xBABABABABABABABA));
            bl.SetZeroItem();
            Assert.AreEqual((0ul, 0ul), bl.Data);
        }

        [TestMethod]
        public void SetAllOnesItem()
        {
            Item bl = new Item();
            bl.SetZeroItem();
            bl.SetAllOnesItem();
            Assert.AreEqual((0xFFFFFFFFFFFFFFFFul, 0xFFFFFFFFFFFFFFFFul), bl.Data);

            bl.SetItem((0xF00FF00FF00FF00F, 0xBABABABABABABABA));
            bl.SetAllOnesItem();
            Assert.AreEqual((0xFFFFFFFFFFFFFFFFul, 0xFFFFFFFFFFFFFFFFul), bl.Data);
        }

        [TestMethod]
        public void IsZeroItem()
        {
            Item bl = new Item();
            bl.SetItem((0, 0));
            Assert.IsTrue(bl.IsZeroItem());

            bl.SetItem((1, 0));
            Assert.IsFalse(bl.IsZeroItem());

            bl.SetItem((0, 1));
            Assert.IsFalse(bl.IsZeroItem());

            bl.SetItem((0xF00FF00FF00FF00F, 0xBABABABABABABABA));
            Assert.IsFalse(bl.IsZeroItem());
        }

        [TestMethod]
        public void IsAllOnesItem()
        {
            Item bl = new Item();
            bl.SetAllOnesItem();
            Assert.IsTrue(bl.IsAllOnesItem());

            bl.SetZeroItem();
            Assert.IsFalse(bl.IsAllOnesItem());

            bl.SetItem((1, 0));
            Assert.IsFalse(bl.IsAllOnesItem());

            bl.SetItem((0, 1));
            Assert.IsFalse(bl.IsAllOnesItem());

            bl.SetItem((0xF00FF00FF00FF00F, 0xBABABABABABABABA));
            Assert.IsFalse(bl.IsAllOnesItem());
        }

        [TestMethod]
        public void SetRandomItem()
        {
            Item bl = new Item();
            bl.SetRandomItem();
            Assert.IsFalse(bl.IsZeroItem());
            Item bl2 = new Item(bl);
            bl.SetRandomItem();
            Assert.AreNotEqual(bl, bl2);
        }

        [TestMethod]
        public void ZeroItem()
        {
            Item bl = Item.MakeRandomItem();
            Assert.IsFalse(bl.IsZeroItem());
            bl.SetZeroItem();
            Assert.IsTrue(bl.IsZeroItem());
        }

        [TestMethod]
        public void IncrementItem()
        {
            Item bl = Item.MakeItem((0, 0));
            bl.IncrementItem();
            Assert.AreEqual((1ul, 0ul), bl.Data);

            bl.SetItem((0xF00F, 0xBAAB));
            bl.IncrementItem();
            Assert.AreEqual((0xF010ul, 0xBAABul), bl.Data);

            bl.SetItem((0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFE));
            bl.IncrementItem();
            Assert.AreEqual((0ul, 0xFFFFFFFFFFFFFFFFul), bl.Data);

            bl.SetItem((0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF));
            bl.IncrementItem();
            Assert.AreEqual((0ul, 0ul), bl.Data);
        }
    }
}