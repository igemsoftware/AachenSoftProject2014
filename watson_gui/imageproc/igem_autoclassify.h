#ifndef IGEM_AUTOCLASSIFY_H
#define IGEM_AUTOCLASSIFY_H

#include <QImage>
#include <QColor>
#include <cmath>

class iGEM_AutoClassify
{
public:
    iGEM_AutoClassify();

    void calculateGradients(float* pXGrad, float *pYGrad, float* pImage, size_t iX, size_t iY);
    float* getGaussianBlur(float* pImage, size_t iX, size_t iY);
    bool threshold(float fSim, unsigned int iGV);
    float arrMul(float *pInImage, size_t x, size_t y, float fValue);
    unsigned int arrMul(unsigned int *pInImage, size_t x, size_t y, float fValue);
    float getMax(float *pInImage, size_t x, size_t y);
    unsigned int getMax(unsigned int *pInImage, size_t x, size_t y);
    float *toFloat(unsigned int *pInImage, size_t x, size_t y);
    unsigned int *toGray(unsigned int *pInImage, size_t x, size_t y);
    float *similarity(float *pInImage, size_t x, size_t y, unsigned int k);
    float getSimScore(float *pInImage, size_t x, size_t y, size_t i, size_t j, unsigned int k);
    QImage *classify(QImage *pInImage, bool *pMask);
};

#endif // IGEM_AUTOCLASSIFY_H
