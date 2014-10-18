using Microsoft.Research.DynamicDataDisplay.Common;
using System;
using System.Collections.Generic;
using System.Linq;
using TCD;
using TCD.Controls;

namespace SerialClient
{
    public class SensorData : PropertyChangedBase
    {
        private double _Value;
        public double Value { get { return _Value; } set { _Value = value; OnPropertyChanged("Value"); } }

        private string _Raw;
        public string Raw { get { return _Raw; } set { _Raw = value; OnPropertyChanged("Raw"); } }
        
        private double _Time;
        public double Time { get { return _Time; } set { _Time = value; OnPropertyChanged("Time"); } }

        private double _StandardDeviation;
        public double StandardDeviation { get { return _StandardDeviation; } set { _StandardDeviation = value; OnPropertyChanged("StandardDeviation"); } }
        
			

        public SensorData(double time, double value, string raw)
        {
            this.Time = time;
            this.Value = value;
            this.Raw = raw;
        }
        public SensorData(List<double[]> data)
        {
            this.Time = data.Average(d => d[0]);
            this.Value = data.Average(d => d[1]);
            this.Raw = "does not apply";
            double sumOfSquaresOfDifferences = data.Select(val => (val[1] - this.Value) * (val[1] - this.Value)).Sum();
            this.StandardDeviation = Math.Sqrt(sumOfSquaresOfDifferences / (data.Count - 1));
        }
    }
    public class SensorDataPointCollection : RingArray<SensorData>
    {
        private const int TOTAL_POINTS = ViewModel.SecondsToShow * 1000 / ViewModel.UpdateInterval;

        public SensorDataPointCollection()
            : base(TOTAL_POINTS) // here i set how much values to show 
        {
        }
    }
    public enum CopyOption
    {
        [DisplayAttribute(Name = "Average of 5")]
        _AverageOfFive = 5,
        [DisplayAttribute(Name = "10")]
        _10 = 10,
        [DisplayAttribute(Name = "20")]
        _20 = 20,
        [DisplayAttribute(Name = "50")]
        _50 = 50,
        [DisplayAttribute(Name = "100")]
        _100 = 100,
        [DisplayAttribute(Name = "200")]
        _200 = 200,
        [DisplayAttribute(Name = "500")]
        _500 = 500,
        [DisplayAttribute(Name = "1000")]
        _1000 = 1000,
        [DisplayAttribute(Name = "All available")]
        All,
        [DisplayAttribute(Name = "Histogram")]
        Histogram
    }
    public enum BaudRate
    {
        [DisplayAttribute(Name = "300")]
        _300 = 300,
        [DisplayAttribute(Name = "600")]
        _600 = 600,
        [DisplayAttribute(Name = "1200")]
        _1200 = 1200,
        [DisplayAttribute(Name = "2400")]
        _2400 = 2400,
        [DisplayAttribute(Name = "4800")]
        _4800 = 4800,
        [DisplayAttribute(Name = "9600 (Default)")]
        _9600 = 9600,
        [DisplayAttribute(Name = "1440")]
        _14400 = 14400,
        [DisplayAttribute(Name = "19200")]
        _19200 = 19200,
        [DisplayAttribute(Name = "28800")]
        _28800 = 28800,
        [DisplayAttribute(Name = "31250")]
        _31250 = 31250,
        [DisplayAttribute(Name = "38400")]
        _38400 = 38400,
        [DisplayAttribute(Name = "57600")]
        _57600 = 57600,
        [DisplayAttribute(Name = "115200")]
        _115200 = 115200
    }
}
