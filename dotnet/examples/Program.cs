using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using CuckooDotNet;

namespace CuckooDotNetExample
{
    public class Program
    {
        static void Main(string[] args)
        {
            var parameters = ParseParameters(args);
            RunExample(parameters);
        }

        static void RunExample(CuckooTableParameters parameters)
        {
            var cuckooTable = new CuckooTableDotNet128(parameters);

            ulong roundCounter = 0;
            while (true)
            {
                Console.WriteLine($"Inserted {roundCounter * 20} items.");
                Console.WriteLine($"Fill rate: {cuckooTable.FillRate()}.");

                for (ulong i = 0; i < 20; i++)
                {
                    var item = new Item128 { Data = new[] { i + 1, roundCounter + 1 } };
                    if (!cuckooTable.Insert(item))
                    {
                        Console.WriteLine($"Insertion failed: round_counter = {roundCounter}, i = {i}");
                        Console.WriteLine($"Inserted successfully {roundCounter * 20 + i} items.");
                        Console.WriteLine($"Fill rate: {cuckooTable.FillRate()}.");

                        var lastItem = cuckooTable.LastInsertFailItem();
                        Console.WriteLine($"Leftover item: [{string.Join(',', lastItem)}].");
                        break;
                    }
                }

                PrintTable(cuckooTable);

                if (!cuckooTable.IsEmptyItem(cuckooTable.LastInsertFailItem()))
                {
                    break;
                }

                roundCounter++;
            }

            while (true)
            {
                Console.WriteLine("Query item: ");
                var input = Console.ReadLine();
                Item128 item;
                try
                {
                    if (input == null) continue;

                    var data = input.Split(",").Select(ulong.Parse).ToArray();
                    item = new Item128 { Data = data};
                }
                catch
                {
                    Console.WriteLine($"Incorrect format. Expected: <ulong>,<ulong>");
                    continue;
                }

                var result = cuckooTable.Query(item);
                if (result.Location == 0 && !result.InStash)
                {
                    Console.WriteLine($"[{string.Join(',', item)}] was not found.");
                }
                else
                {
                    Console.WriteLine($"Found: [{string.Join(',', item)}]");
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

        private static CuckooTableParameters ParseParameters(string[] args)
        {
            if (args.Length != 4)
            {
                Console.WriteLine("Incorrect number of arguments. Expected: 4.");
                ShowHelp();
                Environment.Exit(1);
            }

            int logTableSize;
            if (!int.TryParse(args[0], out logTableSize))
            {
                Console.WriteLine($"Invalid value for parameter {nameof(logTableSize)}. Expected int.");
                ShowHelp();
                Environment.Exit(1);
            }

            uint stashSize;
            if (!uint.TryParse(args[1], out stashSize))
            {
                Console.WriteLine($"Invalid value for parameter {nameof(stashSize)}. Expected uint.");
                ShowHelp();
                Environment.Exit(1);
            }

            uint locFuncCount;
            if (!uint.TryParse(args[2], out locFuncCount))
            {
                Console.WriteLine($"Invalid value for parameter {nameof(locFuncCount)}. Expected uint.");
                ShowHelp();
                Environment.Exit(1);
            }

            ulong maxProbe;
            if (!ulong.TryParse(args[3], out maxProbe))
            {
                Console.WriteLine($"Invalid value for parameter {nameof(maxProbe)}. Expected uint.");
                ShowHelp();
                Environment.Exit(1);
            }

            var parameters = new CuckooTableParameters()
            {
                LogTableSize = logTableSize, StashSize = stashSize, LocFuncCount = locFuncCount, MaxProbe = maxProbe
            };

            var random = new Random();
            parameters.LocFuncSeed = new[] { (ulong)LongRandom(0, long.MaxValue, random), (ulong)LongRandom(0, long.MaxValue, random) };
            parameters.EmptyItem = new ulong[] { 0, 0 };

            return parameters;
        }

        private static void ShowHelp()
        {
            var name = typeof(Program).Assembly.GetName().Name;
            Console.WriteLine($"Usage: dotnet {name}.dll <logTableSize> <stashSize> <locFuncCount> <maxProbe>");
        }

        static void PrintTable(CuckooTableDotNet128 cuckooTable)
        {
            Console.WriteLine($"Table size: { cuckooTable.Table.Size }");
            Console.WriteLine($"{cuckooTable.Table}");

            Console.WriteLine($"Stash size: { cuckooTable.Stash.Size }");
            Console.WriteLine($"{cuckooTable.Stash}");
        }
    }
}
