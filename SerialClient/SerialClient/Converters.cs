using System;
using System.Globalization;
using System.Windows.Data;

namespace SerialClient
{
    public class UIConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            switch (parameter as string)
            {
                case "BaudRate":
                    if (value is int)
                        return (BaudRate)value;
                    else
                        return (int)value;
                case "SensorDataSetCount": return string.Format("{0} points", value);
                case "VerticalAxisTitle":
                    return ((bool)value == true) ? App.ViewModel.Unit : "value   [-]";
                case "StringVsInt": return ((int)value).ToString();
                default:
                    return null;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            switch (parameter as string)
            {
                case "BaudRate":
                    if (value is int)
                        return (BaudRate)value;
                    else
                        return (int)value;
                case "StringVsInt":
                    try { return System.Convert.ToInt32(value as string); }
                    catch { return -1; }
                default:
                    return null;
            }
        }
    }
}
