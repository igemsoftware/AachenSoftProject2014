using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Forms;
using System.IO;
using TCD.Controls;
using System.Diagnostics;

namespace TimeseriesLabeller
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private string[] LoadedFiles;



        public MainWindow()
        {
            InitializeComponent();
        }



        private async Task FrameAndSave(BitmapSource bmp, string title, string time)
        {
            primaryImage.Source = bmp;
            timeText.Text = string.Format("t = {0} {1}", time, unitTB.Text);
            await Task.Delay(100);
            //RenderTargetBitmap frame = new RenderTargetBitmap((int)frameBorder.ActualWidth, (int)frameBorder.ActualHeight, 96, 96, PixelFormats.Pbgra32);
            //frame.Render(frameBorder);
            CreateBitmapFromVisual(frameBorder, Path.Combine(Directory.GetCurrentDirectory(), string.Format("{0}_{1}.png", title, time)));
            await Task.Delay(100);
        }
        void CreateBitmapFromVisual(Visual target, string filename)
        {
            if (target == null)
                return;

            Rect bounds = VisualTreeHelper.GetDescendantBounds(target);

            RenderTargetBitmap rtb = new RenderTargetBitmap((Int32)bounds.Width, (Int32)bounds.Height, 96, 96, PixelFormats.Pbgra32);

            DrawingVisual dv = new DrawingVisual();

            using (DrawingContext dc = dv.RenderOpen())
            {
                VisualBrush vb = new VisualBrush(target);
                dc.DrawRectangle(vb, null, new Rect(new Point(), bounds.Size));
            }

            rtb.Render(dv);

            PngBitmapEncoder png = new PngBitmapEncoder();

            png.Frames.Add(BitmapFrame.Create(rtb));

            using (Stream stm = File.Create(filename))
            {
                png.Save(stm);
            }
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            OpenFileDialog ofd = new OpenFileDialog();
            ofd.Multiselect = true;
            ofd.ShowDialog();

            if (ofd.FileNames == null || ofd.FileNames.Length == 0)
                return;
            //extract information from the files
            LoadedFiles = ofd.FileNames;
            string times = string.Empty;

            foreach (string file in LoadedFiles)
            {
                string ending = file.Substring(file.LastIndexOf('_') + 1);
                string timestring = string.Empty;
                for (int c = 0; c < ending.Length; c++)
                {
                    if (char.IsDigit(ending[c]))
                        timestring += ending[c];
                    else
                        break;
                }
                //now timestring is just the numbers
                times += timestring + ",";
            }
            //now times is a comma-separated enumeration of all timestamps
            timesTB.Text = times;
        }

        private async void RenderButton_Click(object sender, RoutedEventArgs e)
        {
            string[] times = timesTB.Text.Split(new char[]{','}, StringSplitOptions.RemoveEmptyEntries);
            if (LoadedFiles.Length != times.Length)
                await CustomMessageBox.ShowAsync("Error", string.Format("The number of times {0} does not match the number of loaded files ({1}).", times.Length, LoadedFiles.Length), MessageBoxImage.Error, 0, "I'll correct that - thanks!");
            for (int f = 0; f < LoadedFiles.Length; f++)
            {
                BitmapSource bmp = new BitmapImage(new Uri(LoadedFiles[f]));
                await FrameAndSave(bmp, titleText.Text, times[f]);
            }
        }

        private void FolderButton_Click(object sender, RoutedEventArgs e)
        {
            Process.Start(Directory.GetCurrentDirectory());
        }
    }
    public static class Extensions
    {
        public static string HoursToString(this double hours)
        {
            TimeSpan ts = new DateTime() - new DateTime().AddHours(hours);
            return ts.TotalHours + ":" + ts.TotalMinutes.ToString("00");
        }

    }
}
