using Microsoft.Research.DynamicDataDisplay.DataSources;
using System;
using System.IO;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Threading.Tasks;
using System.Windows;
using TCD;
using TCD.Controls;

namespace SerialClient
{
    public class ViewModel : PropertyChangedBase
    {
        #region Commands
        public RelayCommand CommandReset { get; set; }
        public RelayCommand CommandCopy { get; set; }
        #endregion

        #region Data
        private DateTime _LastReset;
        public DateTime LastReset { get { return _LastReset; } set { _LastReset = value; OnPropertyChanged("LastReset"); } }
        
        private SensorDataPointCollection _SensorDataCollection = new SensorDataPointCollection();//contains only recent datapoints
        public SensorDataPointCollection SensorDataCollection { get { return _SensorDataCollection; } set { _SensorDataCollection = value; OnPropertyChanged("SensorDataCollection"); } }

        private ObservableCollection<SensorData> _SensorDataSet = new ObservableCollection<SensorData>();//contains all datapoints
        public ObservableCollection<SensorData>  SensorDataSet { get { return _SensorDataSet; } set { _SensorDataSet = value; OnPropertyChanged("SensorDataSet"); } }

        private List<double[]> _SensorDataCache = new List<double[]>();//time and value only
        public List<double[]> SensorDataCache { get { return _SensorDataCache; } set { _SensorDataCache = value; OnPropertyChanged("SensorDataCache"); } }

        private int _SensorDataCacheSize = 1;
        public int SensorDataCacheSize { get { return _SensorDataCacheSize; } set { _SensorDataCacheSize = Math.Max(value, 1); OnPropertyChanged("SensorDataCacheSize"); } }
        
        private EnumerableDataSource<SensorData> _DataSource;
        public EnumerableDataSource<SensorData> DataSource { get { return _DataSource; } set { _DataSource = value; OnPropertyChanged("DataSource"); } }

        private string _SampleID;
        public string SampleID { get { return _SampleID; } set { _SampleID = value; OnPropertyChanged("SampleID"); } }	
			
        #endregion

        #region Calibration
        private bool _UseMappings;
        public bool UseMappings { 
            get { return _UseMappings; }
            set
            {
                _UseMappings = value;
                DataSource.SetYMapping(y => (value) ? (y.Value - _MinimumValue) * ReadoutPerValue + _MinimumReadout : y.Value);
                OnPropertyChanged("UseMappings");
            }
        }

        private string _Unit;
        public string Unit { get { return _Unit; } set { _Unit = value; OnPropertyChanged("Unit"); } }

        private int _MinimumValue = 0;
        public int MinimumValue { get { return _MinimumValue; } set { _MinimumValue = value; CalculateReadoutPerValue(); OnPropertyChanged("MinimumValue"); } }

        private int _MinimumReadout = 0;
        public int MinimumReadout { get { return _MinimumReadout; } set { _MinimumReadout = value; CalculateReadoutPerValue(); OnPropertyChanged("MinimumReadout"); } }

        private int _MaximumValue = 1;
        public int MaximumValue { get { return _MaximumValue; } set { _MaximumValue = value; CalculateReadoutPerValue(); OnPropertyChanged("MaximumValue"); } }

        private int _MaximumReadout = 100;
        public int MaximumReadout { get { return _MaximumReadout; } set { _MaximumReadout = value; CalculateReadoutPerValue(); OnPropertyChanged("MaximumReadout"); } }

        private double ReadoutPerValue;
        #endregion

        #region Export
        private CopyOption _SelectedCopyOption = CopyOption._100;
        public CopyOption SelectedCopyOption { get { return _SelectedCopyOption; } set { _SelectedCopyOption = value; OnPropertyChanged("SelectedCopyOption"); } }
        
			
        #endregion

        #region Chart
        public const int UpdateInterval = 100;
        public const int SecondsToShow = 10;

        public double MinY
        {
            get
            {
                double min = 0;
                foreach (SensorData p in SensorDataCollection)
                    if (p.Value < min)
                        min = p.Value;
                return Math.Floor(min / 10) * 10;
            }
        }
        public double MaxY
        {
            get
            {
                double max = 0;
                foreach (SensorData p in SensorDataCollection)
                    if (p.Value > max)
                        max = p.Value;
                return Math.Ceiling(max / 10) * 10;
            }
        }
        
        	
        #endregion

        public ViewModel()
        {
            App.ViewModel = this;
            ReadoutPerValue = (double)(MaximumReadout - MinimumReadout) / (double)(MaximumValue - MinimumValue);
            DataSource = new EnumerableDataSource<SensorData>(SensorDataCollection);
            DataSource.SetXMapping(x => x.Time);
            DataSource.SetYMapping(y => y.Value);
            CommandReset = new RelayCommand(delegate
                {
                    SensorDataCollection.Clear();
                    SensorDataSet.Clear();
                    LastReset = DateTime.Now;
                });
            CommandCopy = new RelayCommand(CopyDataForExcel);
            //charting
            LastReset = DateTime.Now;
        }
        
        public void LogData(double value, string raw)
        {
            if (SensorDataCacheSize <= 1)
            {
                SensorData data = new SensorData((DateTime.Now - LastReset).TotalSeconds, value, raw);
                SensorDataSet.Add(data);
                SensorDataCollection.Add(data);
            }
            else
            {
                SensorDataCache.Add(new double[] { (DateTime.Now - LastReset).TotalSeconds, value });
                if (SensorDataCache.Count >= SensorDataCacheSize)
                {
                    SensorData averaged = new SensorData(SensorDataCache);
                    SensorDataSet.Add(averaged);
                    SensorDataCollection.Add(averaged);
                    SensorDataCache.Clear();
                }
            }
        }
        private void CalculateReadoutPerValue()
        {
            ReadoutPerValue = (MaximumReadout - MinimumReadout) / (MaximumValue - MinimumValue);
        }
        private async void CopyDataForExcel()
        {
            if (SensorDataSet.Count <= 1)
                return;
            string outputTabbed = string.Empty;
            int start = (SelectedCopyOption == CopyOption.All) ? 0 : Math.Max(SensorDataSet.Count - (int)SelectedCopyOption, 0);
            switch (SelectedCopyOption)
            {
                case CopyOption._AverageOfFive://calculate average and standard deviation of the last five, export to clipboard -- also exports the last five raw strings to a file
                    List<double[]> lastfive = new List<double[]>();
                    StreamWriter writer = null;//create a writer for writing results to a file
                    string filename = (string.IsNullOrWhiteSpace(SampleID) ? DateTime.Now.ToString("yy-MM-dd hh-mm-ss") : SampleID);
                    try
                    {
                        writer = File.CreateText(Path.Combine(Directory.GetCurrentDirectory(), filename + ".txt"));//create the textfile
                    }
                    catch { }    
                    for (int i = start; i < _SensorDataSet.Count; i++)
                    {
                        lastfive.Add(new double[] { _SensorDataSet[i].Time, MapValue(_SensorDataSet[i].Value) });//make a list of the last five
                        if (writer != null)
                            await writer.WriteLineAsync(SensorDataSet[i].Raw);
                    }
                    if (writer != null)//a textfile was successfully created
                    {
                        await writer.FlushAsync();//finish writing
                        writer.Dispose();
                    }
                    SensorData lastfiveresult = new SensorData(lastfive);//calculate average and deviation
                    outputTabbed = string.Format("{0}\t{1}\t{2}\t{3}\r\n", SampleID, lastfiveresult.Time, lastfiveresult.Value, lastfiveresult.StandardDeviation);//export a line with sample id, time, value and standard deviation
                    break;
                case CopyOption._10:
                case CopyOption._20:
                case CopyOption._50:
                case CopyOption._100:
                case CopyOption._200:
                case CopyOption._500:
                case CopyOption._1000:
                case CopyOption.All:
                    outputTabbed = string.Format("{0}\t{1}\t{2}\r\n", "Time [s]", Unit, "Standard Deviation");
                    for (int i = start; i < _SensorDataSet.Count; i++)
                        outputTabbed += string.Format("{0}\t{1}\t{2}\r\n", _SensorDataSet[i].Time, MapValue(_SensorDataSet[i].Value), _SensorDataSet[i].StandardDeviation);
                    break;
                case CopyOption.Histogram:
                    //get min and max
                    double min = double.PositiveInfinity;
                    double max = double.NegativeInfinity;
                    foreach (var dp in SensorDataSet)
                    {
                        if (MapValue(dp.Value) > max)
                            max = MapValue(dp.Value);
                        else if (MapValue(dp.Value) < min)
                            min = MapValue(dp.Value);
                    }
                    //calculate number of bins
                    int k = (int)Math.Ceiling(Math.Sqrt(SensorDataSet.Count));
                    //calculate width of bins
                    double binWidth = (max - min) / k;
                    //count
                    int[] bins = new int[k];
                    foreach (var kvp in SensorDataSet)
                    {
                        for (int b = 0; b < k; b++)//find the bin this is in
                            if (MapValue(kvp.Value) >= min + b * binWidth && MapValue(kvp.Value) < min + (b + 1) * binWidth)
                            {
                                bins[b]++;
                                break;
                            }
                    }
                    //make into table
                    outputTabbed = "Interval\tFrequency\r\n";
                    for (int i = 0; i < k; i++)
                        outputTabbed += string.Format("{0}\t{1}\r\n", min + i * binWidth, bins[i]);
                    break;
            }
            outputTabbed = outputTabbed.TrimEnd('\n', '\r');
            try
            {
                Clipboard.SetDataObject(outputTabbed, true);
            }
            catch (Exception ex)
            {
                Task t = CustomMessageBox.ShowAsync("Error", "Export to Clipboard failed.\n\nPlease try again.", MessageBoxImage.Warning, 0, "Ok");
            }
        }
        
        private double MapValue(double unmapped)
        {
            return (_UseMappings) ? (unmapped - _MinimumValue) * ReadoutPerValue + _MinimumReadout : unmapped;
        }
    }
}
