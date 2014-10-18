#ifndef IGEM_SRM_PIXELREGION_H
#define IGEM_SRM_PIXELREGION_H

class iGEM_Region;

class iGEM_SRM_PixelRegion
{
public:
    iGEM_SRM_PixelRegion(int iIndex, iGEM_Region *pRegion);

    int getIndex();

    iGEM_Region *getRegion();

    void updateRegion(iGEM_Region *pRegion);

private:
    int m_iIndex;
    iGEM_Region *m_pRegion;
};

#endif // IGEM_SRM_PIXELREGION_H
