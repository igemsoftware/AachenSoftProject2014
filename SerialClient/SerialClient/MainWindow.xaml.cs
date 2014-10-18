using Microsoft.Research.DynamicDataDisplay;
using System.Windows;
using System.Windows.Media;
using TCD.Controls;

namespace SerialClient
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
            copyOptionsBox.SetUpItems(CopyOption._AverageOfFive);
            baudrateBox.SetUpItems(BaudRate._9600);
            //plotting
            plotter.AddLineGraph(App.ViewModel.DataSource, Colors.Blue, 2, "Values");
            plotter.LegendVisible = false;            
        }
    }
}