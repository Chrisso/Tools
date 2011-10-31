using System;
using System.IO;

namespace Hidim.Logic
{
    internal class PrefixedFileStream : FileStream
    {
        private string FileName;
        private byte[] Prefix;
        private int Offset;

        public PrefixedFileStream(string filename, byte[] prefix)
            : base(filename, FileMode.Open)
        {
            Offset   = 0;
            FileName = filename;
            Prefix   = new byte[prefix.Length];
            Array.Copy(prefix, Prefix, prefix.Length);
        }

        public override int ReadByte()
        {
            if (Offset < Prefix.Length)
                return Prefix[Offset++];

            return base.ReadByte();
        }

        public override int Read(byte[] array, int offset, int count)
        {
            int idx = 0;

            for (int i = 0; i < count; i++)
            {
                int b = ReadByte();

                if (b == -1)
                    return idx;

                array[idx + offset] = (byte)b;
                idx++;
            }

            return idx;
        }
    }
}
