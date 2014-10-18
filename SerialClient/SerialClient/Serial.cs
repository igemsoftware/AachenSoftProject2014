using System;
using System.IO.Ports;
using System.Linq;
using System.Windows.Threading;
using TCD;
using TCD.Controls;

namespace SerialClient
{
    public class Serial : PropertyChangedBase
    {
        #region Commands
        public RelayCommand CommandRefreshPorts { get; set; }
        #endregion

        private string[] _PortNames;
        public string[] PortNames { get { return _PortNames; } set { _PortNames = value; OnPropertyChanged("PortNames"); } }

        private int _SelectedPort;
        public int SelectedPort { get { return _SelectedPort; } set { _SelectedPort = value; OnPropertyChanged("SelectedPort"); ChangePort(); } }

        private SerialPort _ActivePort;
        public SerialPort ActivePort { get { return _ActivePort; } set { _ActivePort = value; OnPropertyChanged("ActivePort"); } }

        #region Fake Data
        private Random rnd = new Random();
        private DispatcherTimer FakeSensorTimer = new DispatcherTimer() { Interval = TimeSpan.FromMilliseconds(ViewModel.UpdateInterval) };
        #endregion

        public Serial()
        {
            App.Serial = this;
            //fake data
            FakeSensorTimer.Tick += FakeSensorTimer_Tick;
            FakeSensorTimer.Start();
            //ports
            CommandRefreshPorts = new RelayCommand(delegate
            {
                var ports = SerialPort.GetPortNames().ToList();
#if DEBUG
            ports.Insert(0, "Fake");
#endif
                PortNames = ports.ToArray();
                SelectedPort = 0;
            });
            CommandRefreshPorts.Execute(null);
        }
        private void ChangePort()
        {
            try
            {
                if (ActivePort != null)
                    ActivePort.Close();
                App.ViewModel.SensorDataCollection.Clear();
                App.ViewModel.SensorDataSet.Clear();
                if (PortNames[SelectedPort] == "Fake")
                {
                    ActivePort = null;
                    FakeSensorTimer.Start();
                }
                else
                {
                    FakeSensorTimer.Stop();
                    ActivePort = new SerialPort(PortNames[SelectedPort], (int)BaudRate._9600);
                    ActivePort.DataReceived += ActivePort_DataReceived;
                    ActivePort.Open();
                }
            }
            catch(Exception ex)
            {
                CustomMessageBox.ShowAsync("Exception occured", ex.Message, System.Windows.MessageBoxImage.Error, 0, "Close");
            }
            
        }

        private void ActivePort_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            try
            {
                string line = ActivePort.ReadLine().TrimEnd('\r');
                string[] data = line.Split(new char[] { ';' }, StringSplitOptions.RemoveEmptyEntries);
                double value = Convert.ToDouble(data[0]);
                //us ethe Dispatcher to get to the UI thread
                App.Current.Dispatcher.BeginInvoke((Action)(() =>
                {
                    App.ViewModel.LogData(value, line);
                }));
            }
            catch { }
        }
        private void FakeSensorTimer_Tick(object sender, EventArgs e)
        {
            App.ViewModel.LogData(0.5 + (rnd.NextDouble() - 0.5) * (rnd.NextDouble() - 0.5), "bla");
        }
    }
}
