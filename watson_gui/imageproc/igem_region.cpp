#include "igem_region.h"

iGEM_Region::iGEM_Region()
{
    this->init();
}

iGEM_Region::iGEM_Region(iGEM_Region *pRegion1, iGEM_Region *pRegion2)
{
    this->init();

    m_pPixels->insert(m_pPixels->end(), pRegion1->m_pPixels->begin(), pRegion1->m_pPixels->end());
    m_pPixels->insert(m_pPixels->end(), pRegion2->m_pPixels->begin(), pRegion2->m_pPixels->end());

    m_pPixelColors->insert(m_pPixelColors->end(), pRegion1->m_pPixelColors->begin(), pRegion1->m_pPixelColors->end());
    m_pPixelColors->insert(m_pPixelColors->end(), pRegion2->m_pPixelColors->begin(), pRegion2->m_pPixelColors->end());

    std::vector< iGEM_SRM_PixelRegion* >::iterator oIt;

    for (oIt = m_pPixels->begin(); oIt != m_pPixels->end(); ++oIt)
    {
        (*(oIt))->updateRegion(this);
    }

   // qDebug() << m_pIndices->size();
}

iGEM_Region::~iGEM_Region()
{
    m_pPixels->clear();
    m_pPixelColors->clear();

    delete(m_pPixels);
    delete(m_pPixelColors);
}

void iGEM_Region::init()
{
    m_pPixels = new std::vector< iGEM_SRM_PixelRegion* >();
    m_pPixelColors = new std::vector<QRgb*>();

	oAvgColor = qRgb(0, 0, 0);
}

bool iGEM_Region::addPixel(iGEM_SRM_PixelRegion *pRegion, unsigned int *puintColor)
{
	QColor oAvgCol = QColor(oAvgColor);
	QColor oColor = QColor((QRgb)*puintColor);

	
	
	size_t iTotal = m_pPixelColors->size() + 1;

	float fFac1 = (float)m_pPixelColors->size() / (float)iTotal;
	float fFac2 = 1.0f / (float)iTotal;

	float fR = fFac1 * oAvgCol.redF() + fFac2 * oColor.redF();
	float fG = fFac1 * oAvgCol.greenF() + fFac2 * oColor.greenF();
	float fB = fFac1 * oAvgCol.blueF() + fFac2 * oColor.blueF();
	float fA = fFac1 * oAvgCol.alphaF() + fFac2 * oColor.alphaF();

	oAvgColor = QColor::fromRgbF(fR, fG, fB).rgba();

	//qDebug() << oAvgCol << oColor << QColor::fromRgba(oAvgColor);

	m_pPixels->insert(m_pPixels->end(), pRegion);
	m_pPixelColors->insert(m_pPixelColors->end(), puintColor);

    return true;
}

iGEM_Region* iGEM_Region::addRegion(iGEM_Region *pRegion)
{
	size_t iTotal = m_pPixelColors->size() + pRegion->m_pPixelColors->size();

	QColor oAvgCol = QColor(oAvgColor);
    QColor oColor = QColor(pRegion->getAvgColor());

	float fFac1 = (float)m_pPixelColors->size() / (float)iTotal;
	float fFac2 = (float)pRegion->m_pPixelColors->size() / (float)iTotal;

	float fR = fFac1 * oAvgCol.redF() + fFac2 * oColor.redF();
	float fG = fFac1 * oAvgCol.greenF() + fFac2 * oColor.greenF();
	float fB = fFac1 * oAvgCol.blueF() + fFac2 * oColor.blueF();
	float fA = fFac1 * oAvgCol.alphaF() + fFac2 * oColor.alphaF();

	oAvgColor = QColor::fromRgbF(fR, fG, fB).rgba();

	m_pPixels->insert(m_pPixels->end(), pRegion->m_pPixels->begin(), pRegion->m_pPixels->end());
	m_pPixelColors->insert(m_pPixelColors->end(), pRegion->m_pPixelColors->begin(), pRegion->m_pPixelColors->end());
	
    std::vector< iGEM_SRM_PixelRegion* >::iterator oIt;
    for (oIt = pRegion->m_pPixels->begin(); oIt != pRegion->m_pPixels->end(); ++oIt)
    {
        (*(oIt))->updateRegion(this);
    }

    return this;
}

int iGEM_Region::size()
{
    return m_pPixels->size();
}

QRgb iGEM_Region::getAvgColor()
{
	return oAvgColor;

	float fR = 0.0f;
	float fG = 0.0f;
	float fB = 0.0f;
	float fA = 0.0f;

	for (std::vector<QRgb*>::iterator oIt = m_pPixelColors->begin(); oIt != m_pPixelColors->end(); ++oIt)
	{
		QColor oColor(**oIt);
		qDebug() << oColor;
		fR += oColor.redF();
		fG += oColor.greenF();
		fB += oColor.blueF();
		fA += oColor.alphaF();
	}

	float fScale = 1.0f / float(m_pPixelColors->size());
	fR = fR * fScale;
	fG = fG * fScale;
	fB = fB * fScale;
	fA = fA * fScale;

	//qDebug() << QColor::fromRgb(oAvgColor) << oAvgCol << oColor;
	//qDebug() << QColor::fromRgbF(fR, fG, fB, fA);
}

void iGEM_Region::setPixels(unsigned int *pImage)
{

    unsigned int iVecLen = m_pPixels->size();

    if (iVecLen != m_pPixelColors->size())
        exit(1);

    int iIndex = 0;

    //srand(time(NULL) + clock());

    //QRgb oRandomColor = QColor::fromRgbF((float)(rand() % 255) / 255.0f,(float)(rand() % 255) / 255.0f,(float)(rand() % 255) / 255.0f, 1.0f).rgb();
    QRgb oAvgColor = this->getAvgColor();
    for (int i = 0; i < iVecLen; ++i)
    {
        iIndex = m_pPixels->at(i)->getIndex();
        pImage[iIndex] = oAvgColor;//oAvgColor;//oRandomColor;
        //pImage[iIndex] = (unsigned int) *(m_pPixelColors->at(i));
    }

}

void iGEM_Region::setPixelsDebug(int *pImage, int iRegion)
{

    unsigned int iVecLen = m_pPixels->size();

    if (iVecLen != m_pPixelColors->size())
        exit(1);

    int iIndex = 0;

    for (int i = 0; i < iVecLen; ++i)
    {
        iIndex = m_pPixels->at(i)->getIndex();
        pImage[iIndex] = iRegion;//oRandomColor;
    }

}

