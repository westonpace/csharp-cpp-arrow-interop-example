using Apache.Arrow.Ipc;
using System;
using System.IO;
using System.Runtime.InteropServices;

namespace ArrowDotNetCInterop
{

    class Program
    {
        const int BUF_SIZE = 8 * 1024 * 1024;

        [DllImport("arrow-dotnet-c-interop.so", CharSet = CharSet.Ansi)]
        private static extern void StartReading(string path, IntPtr buf, long bufLen);
        [DllImport("arrow-dotnet-c-interop.so", CharSet = CharSet.Ansi)]
        private static extern bool ReadNextBatch();

        static unsafe void Main(string[] args)
        {
            IntPtr buffer = Marshal.AllocHGlobal(BUF_SIZE);
            StartReading("/home/pace/dev/data/dataset/csv/5", buffer, BUF_SIZE);
            var stream = new UnmanagedMemoryStream((byte*)buffer, BUF_SIZE);

            ArrowStreamReader reader = new ArrowStreamReader(stream);
            var rowCount = 0;
            while (true)
            {
                var batchRead = ReadNextBatch();
                if (!batchRead)
                {
                    break;
                }
                stream.Seek(0, SeekOrigin.Begin);
                var nextBatch = reader.ReadNextRecordBatch();
                rowCount += nextBatch.Length;
            }
            Console.WriteLine("Read in a batch with " + rowCount + " rows");
        }
    }
}
