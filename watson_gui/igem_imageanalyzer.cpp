#include "igem_imageanalyzer.h"

igem_imageAnalyzer::igem_imageAnalyzer(QObject *parent) :
    QThread(parent)
{
}

void igem_imageAnalyzer::run()
{
  iGEM_SRM* pSRM = new iGEM_SRM();
  segmented = pSRM->getSegmentedImage(img);

  iGEM_HSVMask* pHSV = new iGEM_HSVMask();
  bool* pHSVMask = pHSV->maskImage(segmented);

  iGEM_AutoClassify* pClass = new iGEM_AutoClassify();

  masked = pClass->classify( segmented, pHSVMask );
}
