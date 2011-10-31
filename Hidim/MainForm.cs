using System;
using System.IO;
using System.Drawing;
using System.Windows.Forms;

namespace Hidim.Gui
{
    public partial class MainForm : Form
    {
        public MainForm()
        {
            InitializeComponent();
            Text = Properties.Resources.AppTitle;
        }

        private void OnFileOpen(object sender, EventArgs e)
        {
            OpenFileDialog ofd = new OpenFileDialog();
            ofd.Filter = Properties.Resources.FileFilter_All;
            ofd.RestoreDirectory = true;
            if (ofd.ShowDialog() == DialogResult.OK)
            {
                if (string.Compare(Path.GetExtension(ofd.FileName), ".png", true) != 0)
                {
                    if (new FileInfo(ofd.FileName).Length > 1024 * 512)
                        pictureBox.Image = Hidim.Logic.Converter.ToImage(ofd.FileName);
                    else
                        pictureBox.Image = Hidim.Logic.Converter.ToImage(ofd.FileName, 30);
                }
                else pictureBox.Image = new Bitmap(ofd.FileName);

                Text = Properties.Resources.AppTitle + " - " + Path.GetFileName(ofd.FileName);
            }            
        }

        private void FileSaveAsImage(object sender, EventArgs e)
        {
            if (pictureBox.Image == null)
            {
                MessageBox.Show(Properties.Resources.Err_NoFile, Properties.Resources.Error, MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            SaveFileDialog sfd = new SaveFileDialog();
            sfd.Filter = Properties.Resources.FileFilter_Png;
            sfd.RestoreDirectory = true;

            if (sfd.ShowDialog() == DialogResult.OK)
            {
                pictureBox.Image.Save(sfd.FileName, System.Drawing.Imaging.ImageFormat.Png);
            }
        }

        private void FileSaveAsBinary(object sender, EventArgs e)
        {
            if (pictureBox.Image == null)
            {
                MessageBox.Show(Properties.Resources.Err_NoFile, Properties.Resources.Error, MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            try
            {
                Hidim.Logic.HidemImageStream his = Hidim.Logic.Converter.ToBinaryStream(pictureBox.Image);
                
                SaveFileDialog sfd = new SaveFileDialog();
                sfd.FileName = his.FileName;
                sfd.Filter = Properties.Resources.FileFilter_All;
                sfd.RestoreDirectory = true;

                if (sfd.ShowDialog() == DialogResult.OK)
                {
                    using (FileStream fs = File.Create(sfd.FileName))
                    {
                        for (int i = 0; i < his.Length; i++)
                            fs.WriteByte((byte)his.ReadByte());
                    }
                }
            }
            catch (Exception exc)
            {
                MessageBox.Show(exc.Message, Properties.Resources.Error, MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }
        
        private void OnDragOver(object sender, DragEventArgs e)
        {
            if (e.Data.GetDataPresent(DataFormats.FileDrop))
                e.Effect = DragDropEffects.Move;
        }

        private void OnDragDrop(object sender, DragEventArgs e)
        {
            if (e.Data.GetDataPresent(DataFormats.FileDrop))
            {
                string[] filenames = e.Data.GetData(DataFormats.FileDrop) as string[];
                
                if (string.Compare(Path.GetExtension(filenames[0]), ".png", true) != 0)
                {
                    if (new FileInfo(filenames[0]).Length > 1024 * 512)
                        pictureBox.Image = Hidim.Logic.Converter.ToImage(filenames[0]);
                    else
                        pictureBox.Image = Hidim.Logic.Converter.ToImage(filenames[0], 30);
                }
                else pictureBox.Image = new Bitmap(filenames[0]);

                Text = Properties.Resources.AppTitle + " - " + Path.GetFileName(filenames[0]);
            }
        }

        private void OnFileExit(object sender, EventArgs e)
        {
            Close();
        }

        private void OnHelpAbout(object sender, EventArgs e)
        {
            MessageBox.Show(Properties.Resources.AppInfo, Properties.Resources.Information, MessageBoxButtons.OK, MessageBoxIcon.Information);
        }
    }
}
