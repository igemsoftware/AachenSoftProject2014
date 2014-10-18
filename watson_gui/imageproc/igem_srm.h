#ifndef IGEM_SRM_H
#define IGEM_SRM_H

#include <QImage>
#include <QColor>

#include <igem_region.h>
#include <igem_srm_pixelregion.h>
#include <vector>
#include <math.h>
#include <limits>
#include <algorithm>

class iGEM_SRM
{
public:
    iGEM_SRM();
    ~iGEM_SRM();
    void setParameters();
    QImage *getSegmentedImage(QImage* pInImage);

private:

    bool testMergeRegions(iGEM_SRM_PixelRegion *pR1, iGEM_SRM_PixelRegion *pR2);

    static int distanceFunction(iGEM_Region *pR1, iGEM_Region *pR2);
    static bool sortGradients(std::pair<iGEM_SRM_PixelRegion *, iGEM_SRM_PixelRegion *> *pP1, std::pair<iGEM_SRM_PixelRegion *, iGEM_SRM_PixelRegion *> *pP2);

    iGEM_Region *mergeRegions(iGEM_Region *pR1, iGEM_Region *pR2);

    float getB(iGEM_Region *pRegion);
    int getRegionsOfCardinality(int iCardinality);

    void printRegions(int x, int y);

    unsigned int *m_pSImage;

    std::vector< iGEM_SRM_PixelRegion* > *m_pPixelRegions;
    std::vector< std::pair<iGEM_SRM_PixelRegion*, iGEM_SRM_PixelRegion*> *> *m_pEdges;
    //std::vector< iGEM_Region* > *m_pRegions;
    std::map< int, iGEM_Region* > *m_pRegions;

    std::map<int,int> regionsOfCardinality;

    int m_iQ;
    int m_iImageSize;

};

#endif // IGEM_SRM_H
