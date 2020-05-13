// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

using System;
using System.Linq;
using Microsoft.Research.Kuku;

namespace KukuNetExample
{
    public class Program
    {
        static void Main(string[] args)
        {
            var parameters = ParseParameters(args);
            RunExample(parameters);
        }

        static void RunExample(KukuTableParameters parameters)
        {
            var kukuTable = new KukuTable(parameters);

            ulong roundCounter = 0;
            while (true)
            {
                Console.WriteLine($"Inserted {roundCounter * 20} items.");
                Console.WriteLine($"Fill rate: {kukuTable.FillRate}.");

                Console.ReadKey();

                for (ulong i = 0; i < 20; i++)
                {
                    var item = Item.MakeItem(i + 1, roundCounter + 1);
                    if (!kukuTable.Insert(item))
                    {
                        Console.WriteLine($"Insertion failed: round_counter = {roundCounter}, i = {i}");
                        Console.WriteLine($"Inserted successfully {roundCounter * 20 + i} items.");
                        Console.WriteLine($"Fill rate: {kukuTable.FillRate}.");

                        var lastItem = kukuTable.LeftoverItem;
                        Console.WriteLine($"Leftover item: [{string.Join(',', lastItem)}].");
                        break;
                    }
                }

                PrintTable(kukuTable);

                if (!kukuTable.IsEmptyItem(kukuTable.LeftoverItem))
                {
                    break;
                }

                roundCounter++;
            }

            while (true)
            {
                Console.WriteLine("Query item: ");
                var input = Console.ReadLine();
                Item item;
                try
                {
                    if (input == null) continue;

                    var data = input.Split(",").Select(ulong.Parse).ToArray();
                    item = Item.MakeItem(data[0], data[1]);
                }
                catch
                {
                    Console.WriteLine($"Incorrect format. Expected: <ulong>,<ulong>");
                    continue;
                }

                var result = kukuTable.Query(item);
                Console.WriteLine($"Found: {result.Found}");
                if (result.Found)
                {
                    Console.WriteLine($"Location: {result.Location}");
                    Console.WriteLine($"In stash: {result.InStash}");
                    Console.WriteLine($"Hash function index: {result.LocFuncIndex}");
                }
            }
        }

        static long LongRandom(long min, long max, Random rand)
        {
            byte[] buf = new byte[8];
            rand.NextBytes(buf);
            long longRand = BitConverter.ToInt64(buf, 0);

            return (Math.Abs(longRand % (max - min)) + min);
        }

        private static KukuTableParameters ParseParameters(string[] args)
        {
            if (args.Length != 4)
            {
                Console.WriteLine("Incorrect number of arguments. Expected: 4.");
                ShowHelp();
                Environment.Exit(1);
            }

            if (!uint.TryParse(args[0], out uint tableSize))
            {
                Console.WriteLine($"Invalid value for parameter {nameof(tableSize)}. Expected uint.");
                ShowHelp();
                Environment.Exit(1);
            }

            if (!uint.TryParse(args[1], out uint stashSize))
            {
                Console.WriteLine($"Invalid value for parameter {nameof(stashSize)}. Expected uint.");
                ShowHelp();
                Environment.Exit(1);
            }

            if (!uint.TryParse(args[2], out uint locFuncCount))
            {
                Console.WriteLine($"Invalid value for parameter {nameof(locFuncCount)}. Expected uint.");
                ShowHelp();
                Environment.Exit(1);
            }

            if (!ulong.TryParse(args[3], out ulong maxProbe))
            {
                Console.WriteLine($"Invalid value for parameter {nameof(maxProbe)}. Expected uint.");
                ShowHelp();
                Environment.Exit(1);
            }

            var parameters = new KukuTableParameters()
            {
                TableSize = tableSize, StashSize = stashSize, LocFuncCount = locFuncCount, MaxProbe = maxProbe
            };

            var random = new Random();
            parameters.LocFuncSeed = Item.MakeRandomItem();
            parameters.EmptyItem = Item.MakeZeroItem();

            return parameters;
        }

        private static void ShowHelp()
        {
            var name = typeof(Program).Assembly.GetName().Name;
            Console.WriteLine($"Usage: dotnet {name}.dll <tableSize> <stashSize> <locFuncCount> <maxProbe>");
        }

        static void PrintTable(KukuTable kukuTable)
        {
            Console.WriteLine($"Table size: { kukuTable.Table.Size }");
            Console.WriteLine($"{kukuTable.Table}");

            Console.WriteLine($"Stash size: { kukuTable.Stash.Size }");
            Console.WriteLine($"{kukuTable.Stash}");
        }
    }
}
