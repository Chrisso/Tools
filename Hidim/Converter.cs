using System;
using System.IO;
using System.Drawing;

namespace Hidim.Logic
{
    public static class Converter
    {
        private static string GetHidimHeader(string filename, int height)
        {
            System.Text.StringBuilder sb = new System.Text.StringBuilder();

            sb.Append("hidim is torrents!");
            sb.AppendFormat("i{0}e", height);
            sb.AppendFormat("{0}:{1}", Path.GetFileName(filename).Length, Path.GetFileName(filename));

            try
            {
                using (FileStream fs = File.OpenRead(filename))
                {
                    byte[] hash = new System.Security.Cryptography.SHA1Managed().ComputeHash(fs);
                    string sha1 = BitConverter.ToString(hash).Replace("-", "");
                    sb.AppendFormat("{0}:{1}", sha1.Length, sha1);
                }
            }
            catch (Exception)
            {
                sb.AppendFormat("{0}:{1}", 40, "deadbeefdeadbeefdeadbeefdeadbeebadcoffee");
            }

            sb.AppendFormat("i{0}e", new FileInfo(filename).Length);

            return sb.ToString();
        }

        public static Image ToImage(string filename)
        {
            long size = (new FileInfo(filename).Length / 3) + 1;
            return ToImage(filename, (int)Math.Ceiling(Math.Sqrt((9.0 * size) / 16.0)), true);
        }

        public static Image ToImage(string filename, int height)
        {
            return ToImage(filename, height, true);
        }

        public static Image ToImage(string filename, int height, bool with_header)
        {
            height = Math.Max(height, 16);  // needed for header

            string header = with_header ? GetHidimHeader(filename, height) : string.Empty;
            int num_pixels = (int)Math.Ceiling((System.Text.Encoding.ASCII.GetByteCount(header) + new FileInfo(filename).Length) / 3.0);
            int width = (int)Math.Ceiling((double)num_pixels / (double)height);

            System.Diagnostics.Debug.WriteLine(string.Format("Output-Image: {0} x {1}", width, height));
            Bitmap result = new Bitmap(width, height, System.Drawing.Imaging.PixelFormat.Format32bppArgb);

            using (Graphics g = Graphics.FromImage(result))
            {
                g.Clear(Color.Black);
            }

            using (PrefixedFileStream fs = new PrefixedFileStream(filename, System.Text.Encoding.ASCII.GetBytes(header)))
            {
                byte[] pixel = new byte[3];
                int x, y, offset = 0;
                while (fs.Read(pixel, 0, 3) > 0)
                {
                    x = offset / height;
                    y = (height - (offset % height)) - 1;  // offset % height

                    result.SetPixel(x, y, Color.FromArgb(pixel[0], pixel[1], pixel[2]));

                    offset++;
                    Array.Clear(pixel, 0, 3);
                }
            }

            return result;
        }

        public static HidemImageStream ToBinaryStream(Image image)
        {
            return new HidemImageStream(image as Bitmap, true);
        }
    }
}
