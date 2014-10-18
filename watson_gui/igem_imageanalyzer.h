#ifndef IGEM_IMAGEANALYZER_H
#define IGEM_IMAGEANALYZER_H

#include <QObject>
#include <QThread>
#include <igem_srm.h>
#include <igem_hsvmask.h>
#include "imageproc/igem_autoclassify.h"

class igem_imageAnalyzer : public QThread
{
    Q_OBJECT
public:
  explicit igem_imageAnalyzer(QObject *parent = 0);
  void run();
  QImage* getAnalysedImage(){ return masked; }
signals:
public slots:
  void setAnalyzeImage( QImage* i ){ img=i; }
private:
  QImage* img, *segmented, *masked;
};

#endif // IGEM_IMAGEANALYZER_H
