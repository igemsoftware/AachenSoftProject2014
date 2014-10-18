using System;
using System.Globalization;
using System.Windows.Data;
using System;

namespace TimeseriesLabeller
{
    public class UIConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            switch (parameter as string)
            {
                case "Size":
                    try
                    {
                        return Math.Max(System.Convert.ToDouble((string)value), 50);
                    }
                    catch
                    {
                        return 50;
                    }
                default:
                    return null;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            switch (parameter as string)
            {
                case "Size":
                    try
                    {
                        return ((double)value).ToString();
                    }
                    catch
                    {
                        return "NaN";
                    }
                default:
                    return null;
            }
        }
    }
}
