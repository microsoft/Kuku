// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

using Microsoft.Research.Kuku;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;

namespace KukuNetTest
{
    [TestClass]
    public class KukuTableTests
    {
        [TestMethod]
        public void CreateTest()
        {
            KukuTable table = null;

            Utilities.AssertThrows<ArgumentException>(() => table = new KukuTable(
                new KukuTableParameters
                {
                    TableSize = 0,
                    StashSize = 0,
                    LocFuncCount = 2,
                    LocFuncSeed = Item.MakeZeroItem(),
                    MaxProbe = 1,
                    EmptyItem = Item.MakeZeroItem()
                })
            );

            Utilities.AssertThrows<ArgumentException>(() => table = new KukuTable(
                new KukuTableParameters
                {
                    TableSize = 1,
                    StashSize = 0,
                    LocFuncCount = 0,
                    LocFuncSeed = Item.MakeZeroItem(),
                    MaxProbe = 1,
                    EmptyItem = Item.MakeZeroItem()
                })
            );

            Utilities.AssertThrows<ArgumentException>(() => table = new KukuTable(
                new KukuTableParameters
                {
                    TableSize = 1,
                    StashSize = 0,
                    LocFuncCount = 2,
                    LocFuncSeed = Item.MakeZeroItem(),
                    MaxProbe = 0,
                    EmptyItem = Item.MakeZeroItem()
                })
            );

            table = new KukuTable(
                new KukuTableParameters
                {
                    TableSize = Common.MinTableSize,
                    StashSize = 0,
                    LocFuncCount = 2,
                    LocFuncSeed = Item.MakeZeroItem(),
                    MaxProbe = 1,
                    EmptyItem = Item.MakeZeroItem()
                }
            );

            table = new KukuTable(
                new KukuTableParameters
                {
                    TableSize = 1,
                    StashSize = 0,
                    LocFuncCount = 1,
                    LocFuncSeed = Item.MakeZeroItem(),
                    MaxProbe = 1,
                    EmptyItem = Item.MakeZeroItem()
                }
            );
        }

        [TestMethod]
        public void Populate1()
        {
            KukuTable ct = new KukuTable(
                new KukuTableParameters
                {
                    TableSize = 1,
                    StashSize = 0,
                    LocFuncCount = 1,
                    LocFuncSeed = Item.MakeZeroItem(),
                    MaxProbe = 10,
                    EmptyItem = Item.MakeZeroItem()
                }
            );

            Assert.IsTrue(ct.IsEmptyItem(ct[0]));
            Assert.IsTrue(ct.Insert(Item.MakeItem(1, 0)));
            Assert.IsFalse(ct.Insert(Item.MakeItem(0, 1)));
            Utilities.AssertThrows<ArgumentException>(() => ct.Insert(ct.EmptyItem));
            Utilities.AssertThrows<ArgumentException>(() => ct.Insert(Item.MakeZeroItem()));
            Assert.IsFalse(ct.IsEmpty(0));

            ct = new KukuTable(
                new KukuTableParameters
                {
                    TableSize = 1,
                    StashSize = 0,
                    LocFuncCount = 2,
                    LocFuncSeed = Item.MakeZeroItem(),
                    MaxProbe = 10,
                    EmptyItem = Item.MakeZeroItem()
                }
            );

            Assert.IsTrue(ct.IsEmptyItem(ct[0]));
            Assert.IsTrue(ct.Insert(Item.MakeItem(1, 0)));
            Assert.IsFalse(ct.Insert(Item.MakeItem(0, 1)));
            Utilities.AssertThrows<ArgumentException>(() => ct.Insert(ct.EmptyItem));
            Utilities.AssertThrows<ArgumentException>(() => ct.Insert(Item.MakeZeroItem()));
            Assert.IsFalse(ct.IsEmpty(0));

            ct = new KukuTable(
                new KukuTableParameters
                {
                    TableSize = 2,
                    StashSize = 0,
                    LocFuncCount = 1,
                    LocFuncSeed = Item.MakeZeroItem(),
                    MaxProbe = 10,
                    EmptyItem = Item.MakeZeroItem()
                }
            );

            Assert.IsTrue(ct.IsEmptyItem(ct[0]));
            Assert.IsTrue(ct.Insert(Item.MakeItem(1, 0)));

            // Collision
            Assert.IsFalse(ct.Insert(Item.MakeItem(0, 1)));

            // No collision
            Assert.IsTrue(ct.Insert(Item.MakeItem(0, 2)));

            Assert.IsFalse(ct.IsEmpty(0));
            Assert.IsFalse(ct.IsEmpty(1));
        }

        [TestMethod]
        public void Populate2()
        {
            KukuTable ct = new KukuTable(
                new KukuTableParameters
                {
                    TableSize = 1 << 10,
                    StashSize = 0,
                    LocFuncCount = 2,
                    LocFuncSeed = Item.MakeZeroItem(),
                    MaxProbe = 10,
                    EmptyItem = Item.MakeZeroItem()
                }
            );

            for (uint i = 0; i < ct.TableSize; i++)
            {
                Assert.IsTrue(ct.IsEmpty(i));
            }

            Assert.IsTrue(ct.Insert(Item.MakeItem(1, 0)));
            Assert.IsTrue(ct.Insert(Item.MakeItem(0, 1)));
            Assert.IsTrue(ct.Insert(Item.MakeItem(1, 1)));
            Assert.IsTrue(ct.Insert(Item.MakeItem(2, 2)));
            Utilities.AssertThrows<ArgumentException>(() => ct.Insert(ct.EmptyItem));
            Utilities.AssertThrows<ArgumentException>(() => ct.Insert(Item.MakeZeroItem()));

            int nonEmpties = 0;
            for (uint i = 0; i < ct.TableSize; i++)
            {
                nonEmpties += ct.IsEmpty(i) ? 0 : 1;
            }
            Assert.AreEqual(4, nonEmpties);

            Assert.IsTrue(ct.Query(Item.MakeItem(1, 0)).Found);
            Assert.IsTrue(ct.Query(Item.MakeItem(0, 1)).Found);
            Assert.IsTrue(ct.Query(Item.MakeItem(1, 1)).Found);
            Assert.IsTrue(ct.Query(Item.MakeItem(2, 2)).Found);
            Assert.IsFalse(ct.Query(Item.MakeItem(3, 3)).Found);
        }

        [TestMethod]
        public void Populate3()
        {
            KukuTable ct = new KukuTable(
                new KukuTableParameters
                {
                    TableSize = 1 << 10,
                    StashSize = 0,
                    LocFuncCount = 2,
                    LocFuncSeed = Item.MakeZeroItem(),
                    MaxProbe = 10,
                    EmptyItem = Item.MakeRandomItem()
                }
            );

            for (uint i = 0; i < ct.TableSize; i++)
            {
                Assert.IsTrue(ct.IsEmpty(i));
            }

            Assert.IsTrue(ct.Insert(Item.MakeItem(0, 0)));
            Assert.IsTrue(ct.Insert(Item.MakeItem(1, 0)));
            Assert.IsTrue(ct.Insert(Item.MakeItem(0, 1)));
            Assert.IsTrue(ct.Insert(Item.MakeItem(1, 1)));
            Assert.IsTrue(ct.Insert(Item.MakeItem(2, 2)));
            Utilities.AssertThrows<ArgumentException>(() => ct.Insert(ct.EmptyItem));

            // Fails
            Assert.IsFalse(ct.Insert(Item.MakeItem(2, 2)));

            int nonEmpties = 0;
            for (uint i = 0; i < ct.TableSize; i++)
            {
                nonEmpties += ct.IsEmpty(i) ? 0 : 1;
            }
            Assert.AreEqual(5, nonEmpties);

            Assert.IsTrue(ct.Query(Item.MakeItem(0, 0)).Found);
            Assert.IsTrue(ct.Query(Item.MakeItem(1, 0)).Found);
            Assert.IsTrue(ct.Query(Item.MakeItem(0, 1)).Found);
            Assert.IsTrue(ct.Query(Item.MakeItem(1, 1)).Found);
            Assert.IsTrue(ct.Query(Item.MakeItem(2, 2)).Found);
            Assert.IsFalse(ct.Query(Item.MakeItem(3, 3)).Found);
        }

        [TestMethod]
        public void Fill1()
        {
            KukuTable ct = new KukuTable(
                new KukuTableParameters
                {
                    TableSize = 1 << 10,
                    StashSize = 0,
                    LocFuncCount = 2,
                    LocFuncSeed = Item.MakeZeroItem(),
                    MaxProbe = 10,
                    EmptyItem = Item.MakeRandomItem()
                }
            );

            List<Item> insertedItems = new List<Item>();
            for (uint i = 0; i < 100; i++)
            {
                insertedItems.Add(Item.MakeRandomItem());
                Assert.IsTrue(ct.Insert(insertedItems[insertedItems.Count - 1]));
            }
            foreach (Item it in insertedItems)
            {
                Assert.IsTrue(ct.Query(it).Found);
            }
            Assert.IsFalse(ct.Query(Item.MakeRandomItem()).Found);
        }

        [TestMethod]
        public void Fill2()
        {
            KukuTable ct = new KukuTable(
                new KukuTableParameters
                {
                    TableSize = (1 << 10) - 1,
                    StashSize = 0,
                    LocFuncCount = 4,
                    LocFuncSeed = Item.MakeZeroItem(),
                    MaxProbe = 100,
                    EmptyItem = Item.MakeRandomItem()
                }
            );

            List<Item> insertedItems = new List<Item>();
            for (uint i = 0; i < 600; i++)
            {
                insertedItems.Add(Item.MakeRandomItem());
                Assert.IsTrue(ct.Insert(insertedItems[insertedItems.Count - 1]));
            }
            foreach (Item it in insertedItems)
            {
                Assert.IsTrue(ct.Query(it).Found);
            }
            Assert.IsFalse(ct.Query(Item.MakeRandomItem()).Found);
        }

        [TestMethod]
        public void Fill3()
        {
            KukuTable ct = new KukuTable(
                new KukuTableParameters
                {
                    TableSize = (1 << 10) + 1,
                    StashSize = 4,
                    LocFuncCount = 2,
                    LocFuncSeed = Item.MakeZeroItem(),
                    MaxProbe = 100,
                    EmptyItem = Item.MakeRandomItem()
                }
            );

            List<Item> insertedItems = new List<Item>();
            for (uint i = 0; i < 950; i++)
            {
                insertedItems.Add(Item.MakeRandomItem());
                if (!ct.Insert(insertedItems[insertedItems.Count - 1]))
                {
                    Item it = insertedItems.Find((Item it) => it.Equals(ct.LeftoverItem));
                    Assert.IsNotNull(it);
                    Assert.IsFalse(ct.Query(ct.LeftoverItem).Found);
                    insertedItems.Remove(it);
                }
            }

            foreach (Item it in insertedItems)
            {
                Assert.IsTrue(ct.Query(it).Found);
            }
            Assert.IsFalse(ct.Query(Item.MakeRandomItem()).Found);

        }

        [TestMethod]
        public void Locations()
        {
            uint lfc = 2;
            KukuTable ct = new KukuTable(
                new KukuTableParameters
                {
                    TableSize = (1 << 10) + 1,
                    StashSize = 4,
                    LocFuncCount = lfc,
                    LocFuncSeed = Item.MakeZeroItem(),
                    MaxProbe = 100,
                    EmptyItem = Item.MakeRandomItem()
                }
            );

            for (ulong k = 0; k < 20; k++)
            {
                Item it = Item.MakeRandomItem();
                HashSet<uint> allLocs = ct.AllLocations(it);

                bool collisionFound = false;
                for (uint i = 0; i < lfc; i++)
                {
                    for (uint j = 0; j < i; j++)
                    {
                        collisionFound = collisionFound || (ct.Location(it, i) == ct.Location(it, j));
                    }
                }

                Assert.AreEqual(allLocs.Count < lfc, collisionFound);
            }
        }

        [TestMethod]
        public void RepeatedInsert()
        {
            KukuTable ct = new KukuTable(
                new KukuTableParameters
                {
                    TableSize = 1 << 10,
                    StashSize = 0,
                    LocFuncCount = 4,
                    LocFuncSeed = Item.MakeZeroItem(),
                    MaxProbe = 10,
                    EmptyItem = Item.MakeZeroItem()
                }
            );

            Assert.IsTrue(ct.Insert(Item.MakeItem(1, 0)));
            Assert.IsTrue(ct.Insert(Item.MakeItem(0, 1)));
            Assert.IsTrue(ct.Insert(Item.MakeItem(1, 1)));
            Assert.IsTrue(ct.Insert(Item.MakeItem(2, 2)));

            Assert.IsFalse(ct.Insert(Item.MakeItem(1, 0)));
            Assert.IsFalse(ct.Insert(Item.MakeItem(0, 1)));
            Assert.IsFalse(ct.Insert(Item.MakeItem(1, 1)));
            Assert.IsFalse(ct.Insert(Item.MakeItem(2, 2)));
        }
    }
}
