#ifndef IGEM_SRM_REGION_H
#define IGEM_SRM_REGION_H

#include <vector>
#include <time.h>
#include <QDebug>
#include <stdlib.h>
#include <QColor>
#include <igem_srm_pixelregion.h>

class iGEM_Region
{
public:
    iGEM_Region();
    iGEM_Region(iGEM_Region *pRegion1, iGEM_Region *pRegion2);

    ~iGEM_Region();

    bool addPixel(iGEM_SRM_PixelRegion *pRegion, unsigned int *puintColor);
    iGEM_Region *addRegion(iGEM_Region *pRegion);

    int size();
    QRgb getAvgColor();

    void setPixels(unsigned int *pImage);
    void setPixelsDebug(int *pImage, int iRegion);

    int regionId; //test
private:
    void init();

	QRgb oAvgColor;

protected:
    std::vector< iGEM_SRM_PixelRegion* > *m_pPixels;
    std::vector<QRgb*> *m_pPixelColors;
};

#endif // IGEM_SRM_REGION_H
