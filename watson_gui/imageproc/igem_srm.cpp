#include "igem_srm.h"
#include <QElapsedTimer>
#include <QDebug>
#include <iostream>

iGEM_SRM::iGEM_SRM()
      : m_pSImage(NULL)
{
    m_pPixelRegions = new std::vector< iGEM_SRM_PixelRegion* >();
    m_pEdges = new std::vector< std::pair<iGEM_SRM_PixelRegion*, iGEM_SRM_PixelRegion*> *>();
    //m_pRegions = new std::vector< iGEM_Region* >();
    m_pRegions = new std::map< int, iGEM_Region* >();
}

iGEM_SRM::~iGEM_SRM()
{

    //delete(m_pSImage);
    //delete(m_pImage);

}

void iGEM_SRM::setParameters()
{

}

void iGEM_SRM::printRegions(int x, int y)
{
    int rCounter = 0;
    int *pChart = (int*) malloc(sizeof(int) * x * y);
    //for (  std::vector<iGEM_Region*>::iterator oIt = m_pRegions->begin(); oIt != m_pRegions->end(); ++oIt)
    for (  std::map<int,iGEM_Region*>::iterator oIt = m_pRegions->begin(); oIt != m_pRegions->end(); ++oIt)
    {
        iGEM_Region *pRegion = (*oIt).second;

        pRegion->setPixelsDebug(pChart,rCounter++);
    }

    for (int i = 0 ; i < x; ++i)
    {

        for (int j = 0; j < y; ++j)
        {
            qDebug() << pChart[i*y+j] << " ; ";
        }
        qDebug() << "";
    }
}

long long int tTMR=0, tTMR2=0, tmpTMR;

QImage *iGEM_SRM::getSegmentedImage(QImage *pInImage)
{
    qDebug() << "Preparing SRM segmented Image";

    QSize oSize = pInImage->size();
    m_iImageSize = oSize.width() * oSize.height();

    qDebug() << "Allocating space for segmentation";
    m_pSImage = (unsigned int *) malloc( sizeof(unsigned int) * m_iImageSize );

    int x = oSize.width();
    int y = oSize.height();
    size_t iIndex = 0;
    for (size_t i = 0; i < x; ++i)
    {
        for (size_t j = 0; j < y; ++j)
        {
            QRgb oRgb = pInImage->pixel(i,j);
            m_pSImage[iIndex] = oRgb;
            //m_pSImage[iIndex + 0] = qRed(oRgb);
            //m_pSImage[iIndex + 1] = qGreen(oRgb);
            //m_pSImage[iIndex + 2] = qBlue(oRgb);

            iIndex += 1;
        }
    }

    // step 1: create regions for each pixel.
    unsigned int* pImage = (unsigned int *) malloc( sizeof(unsigned int) * m_iImageSize );
    iIndex = 0;
    for (size_t i = 0; i < x; ++i)
    {
        for (size_t j = 0; j < y; ++j)
        {
            QRgb oRgb = pInImage->pixel(i,j);
            pImage[iIndex] = oRgb;
            //m_pSImage[iIndex + 0] = qRed(oRgb);
            //m_pSImage[iIndex + 1] = qGreen(oRgb);
            //m_pSImage[iIndex + 2] = qBlue(oRgb);

            iIndex += 1;
        }
    }
    qDebug() << "Creating PixelRegions";


    iGEM_SRM_PixelRegion **pRegions = (iGEM_SRM_PixelRegion**) malloc(sizeof(iGEM_SRM_PixelRegion*) * x * y);

    iIndex = 0;
    iGEM_Region *pRegion;
    for (int i = 0; i < (x); ++i){
        for (int j = 0; j < (y); ++j)
        {
            iIndex = i*y+j;
            pRegion = new iGEM_Region();
            pRegions[iIndex] = new iGEM_SRM_PixelRegion(iIndex, pRegion);
            pRegion->addPixel(pRegions[iIndex], &pImage[iIndex]);
            //m_pRegions->insert(m_pRegions->end(), pRegion);
            pRegion->regionId = iIndex;
            m_pRegions->insert( std::pair<int,iGEM_Region*>( iIndex, pRegion ) );
        }
    }
    regionsOfCardinality[1] = x*y;


    qDebug() << "Creating links between PixelRegions";

    for (int i = 0; i < (x-1); ++i)
        for (int j = 0; j < (y-1); ++j)
        {
            iIndex = i*y + j;

            m_pEdges->insert(m_pEdges->end(), new std::pair<iGEM_SRM_PixelRegion*, iGEM_SRM_PixelRegion*>(pRegions[iIndex], pRegions[iIndex + 1]));
            m_pEdges->insert(m_pEdges->end(), new std::pair<iGEM_SRM_PixelRegion*, iGEM_SRM_PixelRegion*>(pRegions[iIndex], pRegions[iIndex + y]));
        }

    for (int i = 0; i < y-1; ++i)
    {
        iIndex = (x-1) * y + i;
        m_pEdges->insert(m_pEdges->end(), new std::pair<iGEM_SRM_PixelRegion*, iGEM_SRM_PixelRegion*>(pRegions[iIndex], pRegions[iIndex + 1]));
    }

    for (int i = 0; i < x-1; ++i)
    {
        iIndex = i * y;
        m_pEdges->insert(m_pEdges->end(), new std::pair<iGEM_SRM_PixelRegion*, iGEM_SRM_PixelRegion*>(pRegions[iIndex], pRegions[iIndex + y]));
    }

    // step 2: sort regions
    qDebug() << "Sort Links";
    std::sort(m_pEdges->begin(), m_pEdges->end(), &(iGEM_SRM::sortGradients));

    // step 3: perform merging
    m_iQ = 256;

    iGEM_Region *pDeleteRegion, *pMergedRegion;
    std::vector< iGEM_Region* >::iterator oPRIt;

    QElapsedTimer timer;
    timer.start();

    long long int t1=0, t2=0, t3=0, tmp;
    bool test;

    while (m_iQ == 256)
    {
        qDebug() << "Starting run with Q=" << m_iQ;

        std::vector< std::pair<iGEM_SRM_PixelRegion*, iGEM_SRM_PixelRegion*>* >::iterator oIt = m_pEdges->begin();
        std::pair<iGEM_SRM_PixelRegion*, iGEM_SRM_PixelRegion*> *pCurrentEdge = NULL;

        int i = 0;
        int iStartRegionCount = m_pRegions->size();

        for ( ; oIt != m_pEdges->end(); ++oIt)
        {

            pCurrentEdge = *(oIt);

            tmp=timer.elapsed();
            test = this->testMergeRegions(pCurrentEdge->first, pCurrentEdge->second);
            t1+=(timer.elapsed()-tmp);

            if (test)
            {

                tmp=timer.elapsed();
                pDeleteRegion = pCurrentEdge->second->getRegion();
                pMergedRegion = this->mergeRegions(pCurrentEdge->first->getRegion(), pCurrentEdge->second->getRegion());
                t2+=(timer.elapsed()-tmp);

                tmp=timer.elapsed();

                m_pRegions->erase( pDeleteRegion->regionId );
                delete pDeleteRegion;
/*
                oPRIt = m_pRegions->begin();
                for ( ; oPRIt != m_pRegions->end(); ++oPRIt)
                {
                    if (*(oPRIt) == pDeleteRegion)
                    {
                        m_pRegions->erase(oPRIt);
                        break;
                    }
                }
*/
                t3+=(timer.elapsed()-tmp);
            }

            ++i;

            if (i % 10000 == 0){
                std::cerr << "Finished " << i << " of " << m_pEdges->size() << " time " << timer.elapsed() << std::endl;
                std::cerr << t1 << "\t" << t2 << "\t" << t3 << "\t" << tTMR << "\t" << tTMR2  << std::endl;
                std::cerr << regionsOfCardinality.at(1) << std::endl;
            }
        }

        m_iQ = m_iQ / 2;
        int iEndRegionCount = m_pRegions->size();

        qDebug() << "Reduced Regions from " << iStartRegionCount << " to " << iEndRegionCount;
    }

//    for( std::map<int,int>::iterator it=regionsOfCardinality.begin(); it!=regionsOfCardinality.end(); it++ ){
//        qDebug() << (*it).first << " " << (*it).second;
//    }

    //for (  std::vector<iGEM_Region*>::iterator oIt = m_pRegions->begin(); oIt != m_pRegions->end(); ++oIt)
    for (  std::map<int,iGEM_Region*>::iterator oIt = m_pRegions->begin(); oIt != m_pRegions->end(); ++oIt)
    {
        iGEM_Region *pRegion = (*oIt).second;

        pRegion->setPixels(m_pSImage);

    }

    //this->printRegions(x,y);
    qDebug() << "total time " << timer.elapsed();

    QImage* pReturnImage = new QImage(*pInImage);

    iIndex = 0;
    for (size_t i = 0; i < x; ++i)
    {
        for (size_t j = 0; j < y; ++j)
        {
            QRgb oRgb = m_pSImage[iIndex];

            pReturnImage->setPixel(i,j, oRgb);

            iIndex += 1;
        }
    }

    return pReturnImage;
}


bool iGEM_SRM::testMergeRegions(iGEM_SRM_PixelRegion *pPR1, iGEM_SRM_PixelRegion *pPR2)
{
    static QElapsedTimer timerTMR;
    timerTMR.start();

    iGEM_Region *pR1 =pPR1->getRegion();
    iGEM_Region *pR2 =pPR2->getRegion();

    if (pR1 == pR2)
        return false; //already belong to the same group -> no merge

    tmpTMR = timerTMR.elapsed();
    float fB1 = getB(pR1);
    float fB2 = getB(pR2);
    tTMR += (timerTMR.elapsed()-tmpTMR);

    fB1 = fB1 * fB1;
    fB2 = fB2 * fB2;

    tmpTMR = timerTMR.elapsed();
    QRgb oRGBAvg1 = pR1->getAvgColor();
    QRgb oRGBAvg2 = pR2->getAvgColor();
    tTMR2 += (timerTMR.elapsed()-tmpTMR);

        bool bComparison = true;
        bool bTestR = false;
        bool bTestG = false;
        bool bTestB = false;

        int iColorR1 = 0;
        int iColorG1 = 0;
        int iColorB1 = 0;

        int iColorR2 = 0;
        int iColorG2 = 0;
        int iColorB2 = 0;

        iColorR1 = qRed(oRGBAvg1);
        iColorR2 = qRed(oRGBAvg2);
        bTestR = (float) abs(iColorR2 - iColorR1) <= sqrt(fB1 + fB2);
        bComparison &= bTestR;

        iColorG1 = qGreen(oRGBAvg1);
        iColorG2 = qGreen(oRGBAvg2);
        bTestG = (float) abs(iColorG2 - iColorG1) <= sqrt(fB1 + fB2);
        bComparison &= bTestG;

        iColorB1 = qBlue(oRGBAvg1);
        iColorB2 = qBlue(oRGBAvg2);
        bTestB = (float) abs(iColorB2 - iColorB1) <= sqrt(fB1 + fB2);
        bComparison &= bTestB;

        return bComparison;
}

int iGEM_SRM::distanceFunction(iGEM_Region *pR1, iGEM_Region *pR2)
{
    QRgb oAvg1 = pR1->getAvgColor();
    QRgb oAvg2 = pR2->getAvgColor();

    int iMax = INT_MIN ;


        iMax = std::max(iMax, abs(qRed(oAvg1) - qRed(oAvg2)));
        iMax = std::max(iMax, abs(qGreen(oAvg1) - qGreen(oAvg2)));
        iMax = std::max(iMax, abs(qBlue(oAvg1) - qBlue(oAvg2)));

    return iMax;
}

bool iGEM_SRM::sortGradients(std::pair<iGEM_SRM_PixelRegion*, iGEM_SRM_PixelRegion*> *pP1, std::pair<iGEM_SRM_PixelRegion*, iGEM_SRM_PixelRegion*> *pP2)
{
    float fDist1 = iGEM_SRM::distanceFunction(pP1->first->getRegion(), pP1->second->getRegion());
    float fDist2 = iGEM_SRM::distanceFunction(pP2->first->getRegion(), pP2->second->getRegion());

    return fDist1 < fDist2;
}

iGEM_Region *iGEM_SRM::mergeRegions(iGEM_Region *pR1, iGEM_Region *pR2)
{  
    int size1 = pR1->size();
    int size2 = pR2->size();
    int rSize = regionsOfCardinality[size1]-1;
    if( rSize == 0 ){
        regionsOfCardinality.erase(size1);
    }
    else{
        regionsOfCardinality[size1] = rSize;
    }
    rSize = regionsOfCardinality[size2]-1;
    if( rSize == 0 ){
        regionsOfCardinality.erase(size2);
    }
    else{
        regionsOfCardinality[size2] = rSize;
    }
    regionsOfCardinality[size1+size2] = (regionsOfCardinality[size1+size2] + 1);

    return pR1->addRegion(pR2);
}

float iGEM_SRM::getB(iGEM_Region *pRegion)
{
    float fG = 256.0f;

    float fFac = 1.0f / (2.0f * m_iQ * pRegion->size());

    float fDelta = regionsOfCardinality[pRegion->size()] * (6.0f * m_iImageSize * m_iImageSize) ;// 1 / (6 |I|^2)
    //float fDelta = this->getRegionsOfCardinality(pRegion->size()) * (6.0f * m_iImageSize * m_iImageSize) ;// 1 / (6 |I|^2)

    float fSqrt =  sqrtf( fFac * log( fDelta ));

    return fG * fSqrt;
}

int iGEM_SRM::getRegionsOfCardinality(int iCardinality)
{
    int iCount = 0;
/*
    for (std::vector<iGEM_Region*>::iterator oIt = m_pRegions->begin() ; oIt != m_pRegions->end(); ++oIt)
    {
        if ((*oIt)->size() == iCardinality)
            ++iCount;
    }
*/
    return iCount;
}


