#ifndef IGEM_HSVMASK_H
#define IGEM_HSVMASK_H

#include <QImage>
#include <QSize>

class iGEM_HSVMask
{
public:
    iGEM_HSVMask();

    bool* maskImage(QImage* pInImage)
    {
        // how many elements?
        QSize oSize = pInImage->size();
        size_t x = oSize.width();
        size_t y = oSize.height();

        // step 1: get QRgb array
        unsigned int* pImage = (unsigned int *) malloc( sizeof(unsigned int) * x * y );
        size_t iIndex = 0;
        for (size_t i = 0; i < x; ++i)
        {
            for (size_t j = 0; j < y; ++j)
            {
                QRgb oRgb = pInImage->pixel(i,j);
                pImage[iIndex] = oRgb;
                iIndex += 1;
            }
        }

        bool* bMask = (bool*) calloc( oSize.width() * oSize.height(), sizeof(bool));

        sHSV* pHSVImage = this->RGBtoHSV(pImage, x, y);


        for (size_t i = 0; i < x; ++i)
        {
            for (size_t j = 0; j < y; ++j)
            {
                iIndex = i * y + j;

                bMask[iIndex] = this->threshold(&(pHSVImage[iIndex]));
            }
        }


        return bMask;
    }

private:

    struct sHSV {
        float fH;
        float fS;
        float fV;
    };

    // as by http://math.stackexchange.com/questions/556341/rgb-to-hsv-color-conversion-algorithm
    sHSV toHSV(QRgb oRgb)
    {
        sHSV oReturn;

        double mini, maxi, delta;

        float fR = qRed(oRgb) / 255.0f;
        float fG = qGreen(oRgb) / 255.0f;
        float fB = qBlue(oRgb) / 255.0f;

        mini = fR < fG ? fR : fG;
        mini = mini < fB ? mini : fB;

        maxi = fR > fG ? fR : fG;
        maxi = maxi > fB ? maxi : fB;

        oReturn.fV = maxi;
        delta = maxi - mini;
        if(maxi > 0.0){
            oReturn.fS = (delta/maxi);
        }
        else
        {
            oReturn.fS = 0.0f;
            oReturn.fH = 0.0f;
            return oReturn;
        }

        if(qRed(oRgb) >= maxi){
            oReturn.fH = (fG -fB) / delta;
        }
        else
        {
            if(qGreen(oRgb) >= maxi){
                oReturn.fH = 2.0 + (fB - fR) / delta;
            }else{
                oReturn.fH = 4.0 + (fR - fG) / delta;
            }
        }

        oReturn.fH *= 60.0;

        if(oReturn.fH < 0.0){
            oReturn.fH += 360.0f;
        }        

        oReturn.fH/=360.0f;

        return oReturn;
    }

    sHSV* RGBtoHSV(unsigned int* pImage, size_t x, size_t y)
    {

        sHSV* pReturn = (sHSV*) malloc(sizeof(sHSV) * x * y);
        size_t iIndex = 0;
        for (size_t i = 0; i < x; ++i)
        {
            for (size_t j = 0; j < y; ++j)
            {
                iIndex = i * y + j;

                pReturn[iIndex] = this->toHSV( pImage[iIndex] );
            }
        }


        return pReturn;
    }

    bool threshold(sHSV* pHSV)
    {
        /*
         *
         * channel1Min = 0.462;
channel1Max = 0.520;

% Define thresholds for channel 2 based on histogram settings
channel2Min = 0.99;
channel2Max = 1.000;

% Define thresholds for channel 3 based on histogram settings
channel3Min = 0.25;
channel3Max = 0.32;
         *
         *
         * */

        bool bFulfilled = (pHSV->fH >= 0.49f) && (pHSV->fH <= 0.55f);
        bFulfilled &= (pHSV->fS >= 0.99f) && (pHSV->fS <= 1.0f);
        bFulfilled &= (pHSV->fV >= 0.26f) && (pHSV->fV <= 0.32f);

        return bFulfilled;

    }
};

#endif // IGEM_HSVMASK_H
