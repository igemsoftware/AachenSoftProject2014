#include "igem_srm_pixelregion.h"

iGEM_SRM_PixelRegion::iGEM_SRM_PixelRegion(int iIndex, iGEM_Region *pRegion)
    : m_pRegion(pRegion)
{
    m_iIndex = iIndex;
}

int iGEM_SRM_PixelRegion::getIndex()
{
    return m_iIndex;
}

iGEM_Region *iGEM_SRM_PixelRegion::getRegion()
{
    return m_pRegion;
}

void iGEM_SRM_PixelRegion::updateRegion(iGEM_Region *pRegion)
{
    m_pRegion = pRegion;
}
