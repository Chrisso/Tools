using System;
using System.IO;
using System.Drawing;

namespace Hidim.Logic
{
    public class HidemImageStream : Stream
    {
        private MemoryStream SourceData;
        private string FileName_ = "";
        private string SHA1_ = "";
        private long Length_ = 0;
        private long DataOffset_ = 0;
        private long CurrentOffset_ = 0;

        #region private helper methods
        private int ReadBecondedInt()
        {
            byte[] buffer = new byte[64];
            int current_pos = 0;
            do
            {
                buffer[current_pos] = (byte)SourceData.ReadByte();
            }
            while ((char)buffer[current_pos++] != 'e');

            String s = System.Text.Encoding.ASCII.GetString(buffer, 0, current_pos);
            return Convert.ToInt32(s.Substring(1, s.Length-2));
        }

        private string ReadBecondedString()
        {
            byte[] buffer = new byte[64];
            int current_pos = 0;
            do
            {
                buffer[current_pos] = (byte)SourceData.ReadByte();
            }
            while ((char)buffer[current_pos++] != ':');

            string s = System.Text.Encoding.ASCII.GetString(buffer, 0, current_pos);
            int len = Convert.ToInt32(s.Substring(0, s.Length - 1));

            buffer = new byte[len];
            for (int i = 0; i < len; i++)
                buffer[i] = (byte)SourceData.ReadByte();
            return System.Text.Encoding.ASCII.GetString(buffer, 0, len);
        }
        #endregion

        public HidemImageStream(Bitmap image)
            : this(image, true)
        {
        }

        public HidemImageStream(Bitmap image, bool with_header)
        {
            if (image == null || image.Height < 10)
                throw new InvalidDataException(Hidim.Gui.Properties.Resources.Err_NoImage);

            SourceData = new MemoryStream();

            if (with_header)
            {
                Point header_pos = new Point(-1, -1);
                int line_height = image.Height;

                #region detect header
                for (int x = 0; x < image.Width; x++)
                {
                    for (int y = image.Height - 1; y >= 0; y--)
                    {
                        Color rgb = image.GetPixel(x, y);
                        if (rgb.R == 104 && rgb.G == 105 && rgb.B == 100)
                        {
                            rgb = image.GetPixel(x, y-1);
                            if (rgb.R == 105 && rgb.G == 109 && rgb.B == 32)
                            {
                                header_pos.X = x;
                                header_pos.Y = y;
                            }
                        }
                    }
                }
                System.Diagnostics.Debug.WriteLine(string.Format("Header is at: ({0}, {1})", header_pos.X, header_pos.Y));
                if (header_pos.X == -1 || header_pos.Y == -1)
                    throw new InvalidDataException(Hidim.Gui.Properties.Resources.Err_NoHidimHeader);               
                #endregion

                #region extract lineheight
                for (int i = 0; i < 4; i++) // HACK: next 4 pixels on top of magic should contain line-height
                {
                    Color rgb = image.GetPixel(header_pos.X, header_pos.Y - (6 + i));
                    SourceData.WriteByte(rgb.R);
                    SourceData.WriteByte(rgb.G);
                    SourceData.WriteByte(rgb.B);
                }
                
                string tmp = System.Text.Encoding.ASCII.GetString(SourceData.ToArray());
                if (tmp.IndexOf('i') == -1 || tmp.IndexOf('e') == -1)
                    throw new InvalidDataException(Hidim.Gui.Properties.Resources.Err_InvalidHidimHeader);
                
                line_height = Convert.ToInt32(tmp.Substring(tmp.IndexOf('i') + 1, tmp.IndexOf('e') - tmp.IndexOf('i') - 1));
                System.Diagnostics.Debug.WriteLine(string.Format("Line-Height is: ({0})", line_height));

                SourceData.Dispose();
                SourceData = new MemoryStream();
                #endregion

                for (int x = header_pos.X; x < image.Width; x++)
                {
                    for (int y = header_pos.Y; y >= header_pos.Y - (line_height-1); y--)
                    {
                        Color rgb = image.GetPixel(x, y);
                        SourceData.WriteByte(rgb.R);
                        SourceData.WriteByte(rgb.G);
                        SourceData.WriteByte(rgb.B);
                    }
                }
                SourceData.Seek(0, SeekOrigin.Begin);   // ready for reading

                #region evaluate header
                SourceData.Seek(18, SeekOrigin.Begin);  // skip magic
                ReadBecondedInt();                      // skip line height
                FileName_ = ReadBecondedString();
                SHA1_     = ReadBecondedString();
                Length_   = ReadBecondedInt();
                #endregion
            }
            else
            {
                for (int x = 0; x < image.Width; x++)
                {
                    for (int y = image.Height-1; y >= 0 ; y--)
                    {
                        Color rgb = image.GetPixel(x, y);
                        SourceData.WriteByte(rgb.R);
                        SourceData.WriteByte(rgb.G);
                        SourceData.WriteByte(rgb.B);
                    }
                }
                SourceData.Seek(0, SeekOrigin.Begin);
            }
        }

        public string FileName
        {
            get { return FileName_; }
        }

        public string SHA1
        {
            get { return SHA1_; }
        }

        public override bool CanRead
        {
            get { return true; }
        }

        public override long Length
        {
            get { return Length_; }
        }

        public override int ReadByte()
        {
            if (CurrentOffset_++ < Length_)
                return SourceData.ReadByte();

            return -1;
        }

        public override int Read(byte[] buffer, int offset, int count)
        {
            int read = 0;
            int temp = 0;
            for (int i = 0; i < count; i++)
            {
                temp = ReadByte();
                if (temp == -1)
                    return read;
                buffer[offset + i] = (byte)temp;
                read++;
            }

            return read;
        }

        #region dummy/ default implementation
        public override bool CanSeek
        {
            get { return false; }
        }

        public override bool CanWrite
        {
            get { return false; }
        }

        public override void Flush()
        {
            throw new NotSupportedException();
        }

        public override long Position
        {
            get
            {
                throw new NotImplementedException();
            }
            set
            {
                throw new NotSupportedException();
            }
        }

        public override long Seek(long offset, SeekOrigin origin)
        {
            throw new NotSupportedException();
        }

        public override void SetLength(long value)
        {
            throw new NotSupportedException();
        }

        public override void Write(byte[] buffer, int offset, int count)
        {
            throw new NotSupportedException();
        }
        #endregion
    }
}
