#include "igem_autoclassify.h"
#include <iostream>
#include <QDebug>

iGEM_AutoClassify::iGEM_AutoClassify()
{
}

    QImage* iGEM_AutoClassify::classify(QImage* pInImage, bool *pMask)
    {
        // how many elements?
        QSize oSize = pInImage->size();
        size_t x = oSize.width();
        size_t y = oSize.height();

        // step 1: get QRgb array
        unsigned int* pImage = (unsigned int *) malloc( sizeof(unsigned int) * x * y );
        size_t iIndex = 0;
        QRgb oBlack = qRgb(0,0,0);

        for (size_t i = 0; i < x; ++i)
        {
            for (size_t j = 0; j < y; ++j)
            {
                QRgb oRgb = pInImage->pixel(i,j);

                if (pMask[iIndex] == true)
                {
                    pImage[iIndex] = oRgb;
                } else {
                    pImage[iIndex] = oBlack;
                }

                iIndex += 1;
            }
        }

        QImage* pReturnImage = new QImage(*pInImage);

        iIndex = 0;
        for (size_t i = 0; i < x; ++i)
        {
            for (size_t j = 0; j < y; ++j)
            {
                QRgb oRgb = pImage[iIndex];

                pReturnImage->setPixel(i,j, oRgb);

                iIndex += 1;
            }
        }

        return pReturnImage;


        // step 2 RGB to gray
        unsigned int* pGrayImage = this->toGray(pImage, x, y);
        unsigned int iGrayMax = this->getMax(pGrayImage, x, y);
        float fFactor = 255.0f / (float) iGrayMax;
        this->arrMul(pGrayImage, x, y, fFactor);

        // step 3 cascade gaussian filters
        float* pFGrayImage = this->toFloat(pGrayImage, x, y);

        this->getGaussianBlur(pFGrayImage, x, y);

        float fBlurrMax = this->getMax(pFGrayImage, x, y);
        fFactor = 255.0f / fBlurrMax;
        this->arrMul(pFGrayImage, x, y, fFactor);

        // step 4 calculate similarity score :D

        unsigned int k = 2;

        float* pSimilarity = this->similarity(pFGrayImage, x, y, k);
        float fSimMax = this->getMax(pSimilarity, x, y);
        fFactor = 1.0f / fSimMax;
        this->arrMul(pSimilarity, x, y, fFactor);

        // mask target pixels
        bool* bMask = (bool*) calloc( oSize.width() * oSize.height(), sizeof(bool));
        QRgb oColor = qRgb(255,0,0);
        for (size_t i = 0; i < x; ++i)
        {
            for (size_t j = 0; j < y; ++j)
            {
                iIndex = i * y + j;

                bMask[iIndex] = this->threshold(pSimilarity[iIndex], pFGrayImage[iIndex]);
                if(bMask[iIndex])
                    pInImage->setPixel(i,j, oColor);
            }
        }

/*

        QImage* pReturnImage = new QImage(*pInImage);

        iIndex = 0;
        for (size_t i = 0; i < x; ++i)
        {
            for (size_t j = 0; j < y; ++j)
            {
                QRgb oRgb = qRgb(pSimilarity[iIndex]*255.0,pSimilarity[iIndex]*255.0,pSimilarity[iIndex]*255.0);

                pReturnImage->setPixel(i,j, oRgb);

                iIndex += 1;
            }
        }

        return pReturnImage;


*/

        // mark target pixels in image?

        free(pGrayImage);
        free(pFGrayImage);
        free(pImage);
        free(bMask);

        return pInImage;


    }

    float iGEM_AutoClassify::getSimScore(float* pInImage, size_t x, size_t y, size_t i, size_t j, unsigned int k)
    {

        size_t is = std::max( 0,(int) (i - k));
        size_t ie = std::min(x, i + k);

        size_t js = std::max(0, (int)(j -k));
        size_t je = std::min(y, j +k);

        float fSum = 0.0f;

        for ( size_t ii = is; ii < ie; ++ii)
        {
            for (size_t jj = js; jj < je; ++jj)
            {
                fSum += std::abs(pInImage[ii * y + jj]);
            }
        }

        return fSum;

    }

    float* iGEM_AutoClassify::similarity(float* pInImage, size_t x, size_t y, unsigned int k)
    {
        float* pSimilarity = (float*) malloc( sizeof(float) * x * y );

        float* pfX = (float*) malloc( sizeof(float) * x * y );
        float* pfY = (float*) malloc( sizeof(float) * x * y );

        this->calculateGradients(pfX, pfY, pInImage, x, y);

        size_t iIndex = 0;

        for (size_t i = 0; i < x; ++i)
        {
            for (size_t j = 0; j < y; ++j)
            {
                iIndex = i * y + j;
                pSimilarity[iIndex] = this->getSimScore(pfX, x, y, i ,j, k);
                pSimilarity[iIndex] += this->getSimScore(pfY, x, y, i ,j, k);

            }
        }

        free(pfX);
        free(pfY);

        float fSMax = this->getMax(pSimilarity, x, y);

        for (size_t i = 0; i < x; ++i)
        {
            for (size_t j = 0; j < y; ++j)
            {
                iIndex = i * y + j;
                pSimilarity[iIndex] = fSMax - pSimilarity[iIndex];

            }
        }

        return pSimilarity;
    }

    unsigned int* iGEM_AutoClassify::toGray(unsigned int* pInImage, size_t x, size_t y)
    {

        unsigned int* pImage = (unsigned int *) malloc( sizeof(unsigned int) * x * y );
        size_t iIndex = 0;

        for (size_t i = 0; i < x; ++i)
        {
            for (size_t j = 0; j < y; ++j)
            {
                iIndex = i * y + j;
                QRgb oRGB = (QRgb) pInImage[iIndex];
                pImage[iIndex] = qGray(oRGB);
            }
        }

        return pImage;

    }

    float* iGEM_AutoClassify::toFloat(unsigned int* pInImage, size_t x, size_t y)
    {

        float* pImage = (float*) malloc( sizeof(float) * x * y );
        size_t iIndex = 0;

        for (size_t i = 0; i < x; ++i)
        {
            for (size_t j = 0; j < y; ++j)
            {
                iIndex = i * y + j;
                unsigned int iValue = pInImage[iIndex];

                pImage[iIndex] = (float) pInImage[iIndex];
            }
        }

        return pImage;

    }

    unsigned int iGEM_AutoClassify::getMax(unsigned int* pInImage, size_t x, size_t y)
    {

        size_t iXMax;
        size_t iYMax;
        unsigned int iMax;

        size_t iIndex = 0;
        unsigned int iValue = 0;

        for (size_t i = 0; i < x; ++i)
        {
            for (size_t j = 0; j < y; ++j)
            {
                iIndex = i * y + j;

                iValue = pInImage[iIndex];
                if (iValue > iMax)
                {
                    iMax = iValue;
                    iXMax = i;
                    iYMax = j;
                }
            }
        }

        return iMax;

    }

    float iGEM_AutoClassify::getMax(float* pInImage, size_t x, size_t y)
    {

        size_t iXMax;
        size_t iYMax;
        float fMax = 0.0f;

        size_t iIndex = 0;
        float fValue = 0.0f;

        for (size_t i = 0; i < x; ++i)
        {
            for (size_t j = 0; j < y; ++j)
            {
                iIndex = i * y + j;

                fValue = pInImage[iIndex];
                if (fValue > fMax)
                {
                    fMax = fValue;
                    iXMax = i;
                    iYMax = j;
                }
            }
        }

        return fMax;

    }

    unsigned int iGEM_AutoClassify::arrMul(unsigned int* pInImage, size_t x, size_t y, float fValue)
    {
        size_t iIndex = 0;

        for (size_t i = 0; i < x; ++i)
        {
            for (size_t j = 0; j < y; ++j)
            {
                iIndex = i * y + j;
                pInImage[iIndex] = (unsigned int) fValue * pInImage[iIndex];
            }
        }
        return 1;
    }

    float iGEM_AutoClassify::arrMul(float* pInImage, size_t x, size_t y, float fValue)
    {
        size_t iIndex = 0;

        for (size_t i = 0; i < x; ++i)
        {
            for (size_t j = 0; j < y; ++j)
            {
                iIndex = i * y + j;
                pInImage[iIndex] = fValue * pInImage[iIndex];
            }
        }
        return 1;
    }

    bool iGEM_AutoClassify::threshold(float fSim, unsigned int iGV)
    {
//    cwseeds = ((sSI > 0.85) == 1) & ((Z > 235) == 1);
        bool bFulfilled = (fSim > 0.55f) &&(iGV > 210); //(fSim > 0.85f) &&

        return bFulfilled;

    }

    float* iGEM_AutoClassify::getGaussianBlur(float* pImage, size_t iX, size_t iY)
    {

        int iWidth = 5;
        int iLeftRight = iWidth / 2;
        float *pKernel = (float*)calloc(sizeof(float), iWidth * iWidth);
        /*

        first create matrix

        */

        // set standard deviation to 1.0
        float sigma = 1.0;
        float r, s = 2.0 * sigma * sigma;

        // sum is for normalization
        float sum = 0.0;

        // generate 5x5 kernel
        for (int x = -iLeftRight; x <= iLeftRight; x++)
        {
            for (int y = -iLeftRight; y <= iLeftRight; y++)
            {
                r = std::sqrt(x*x + y*y);
                pKernel[(x + iLeftRight) * iWidth + (y + iLeftRight)] = (std::exp(-(r*r) / s)) / (3.14f * s);

                sum += pKernel[(x + iLeftRight) * iWidth + (y + iLeftRight)];
            }
        }

        // normalize the Kernel
        for (int i = 0; i < iWidth; ++i)
            for (int j = 0; j < iWidth; ++j)
            {
                pKernel[i * iWidth + j] /= sum;
                qDebug() << pKernel[i * iWidth + j] << "\n";
            }



        float* pBuffer = (float*)calloc(sizeof(float) , iX * iY);

        for (int i = iLeftRight; i < iX - iLeftRight; ++i)
        {
            for (int j = iLeftRight; j < iY - iLeftRight; ++j)
            {

                pBuffer[i * iY + j] = 0.0f;

                for (int k = -iLeftRight; k < iLeftRight; ++k)
                {
                    for (int l = -iLeftRight; l < iLeftRight; ++l)
                    {
                        pBuffer[i * iY + j] += pKernel[(k + iLeftRight) * iWidth + (l + iLeftRight)] * pImage[(i - k) * iY + (j - l)];
                    }
                }
            }
        }

        memcpy(pImage, pBuffer, iX * iY * sizeof(float));

        free(pBuffer);
        free(pKernel);

        return pImage;

    }

    void iGEM_AutoClassify::calculateGradients(float *pXGrad, float* pYGrad, float* pImage, size_t iX, size_t iY)
    {

        size_t iIndex = 0;

        for (int j = 0, i = 0; j < iY; ++j)
        {
            pYGrad[ i * iY + j ] = 0.0f;
        }

        for (int i = 1; i < iX; ++i)
        {
            for (int j = 0; j < iY; ++j)
            {

                iIndex = i * iY + j;

                pYGrad[iIndex] = pImage[iIndex] - pImage[ (i - 1) * iY + j];
            }
        }

        for (int j = 0, i = 0; i < iX; ++i)
        {
            pXGrad[(i*iY + j)] = 0.0f;
        }

        for (int i = 0; i < iX; ++i)
        {
            for (int j = 1; j < iY; ++j)
            {
                iIndex = i * iY + j;

                pXGrad[iIndex] = pImage[iIndex] - pImage[i * iY + j - 1];
            }
        }
    }
