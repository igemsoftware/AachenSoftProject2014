#ifndef IGEM_MAINWINDOW_H
#define IGEM_MAINWINDOW_H

#include <QStatusBar>
#include <QMainWindow>
#include <QtNetwork/QNetworkReply>
#include <QPushButton>
#include <QInputDialog>
#include <QPixmap>
#include <QGroupBox>
#include <QGraphicsScene>
#include <QTextEdit>
#include <QProgressBar>
#include <QProcess>
#include <QDebug>
#include <QtNetwork/QNetworkAccessManager>
#include <QUrl>
#include <QtGui>
#include <QBoxLayout>
#include <QSpinBox>
#include <QSpacerItem>
#include <QLineEdit>
#include <QComboBox>
#include <QFileDialog>
#include <QLabel>
#include <QTimer>
#include <QImage>
#include <QGraphicsView>
#include <QCheckBox>
#include <QTest>
#include <sstream>
#include <string>
#include <QDesktopWidget>
#include <QMenuBar>
#include <QHostAddress>
#include <QThread>
#include "igem_imageanalyzer.h"

struct cameraOption {
  QString name, desc, ex, awb;
  int t, ev, ss, iso;
  std::pair<double, double> awbg;
};

class igem_MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit igem_MainWindow(QWidget *parent = 0);
  ~igem_MainWindow(){};
 public slots:
  void closing();

  void imageAnalyzerFinished();

 protected:
  void resizeEvent(QResizeEvent *event);

 private slots:
  void startNetworkRequest(QUrl url, bool onClose);
  void networkReplyHandler(QNetworkReply *);
  void networkReplyError(QNetworkReply::NetworkError);

  void serverRunning();

  void takeImage();
  void takeImageFinished(QString);

  void startTimelapse();
  void stopTimelapse();

  void saveImage();
  void saveTimeseriesImage();

  void loadImage(QString fileName);
  void loadImageFinished(QNetworkReply *);

  void loadTimeseriesImage();
  void loadTimeseriesImageFinished(QNetworkReply *);

  void setCameraOption(int option);
  void setTimelapseInterval(int interval);
  void getSettingsAll();
  void addSetting();
  void updateSetting();
  void deleteSetting();
  void settingChanged(QString);
  void settingChanged() {
    settingChanged("");
  };

  void prevImage();
  void nextImage();
  void displayImage(QImage *pxMap);

  void checkSaveTimelapse(bool check);
  void saveDirChanged();

  void cancelNetworkRequest();

  void analyzeImage();

  void showOriginal(){
    if( no_images>0 )
      displayImage(image_list.at(current_image));
  };
  void showAnalyzed(){
    if( no_images>0 )
      displayImage(image_list.at(current_image));
  };
 private:
  void setupUi();
  void showImageInfo();


  void readSetting(QString s);
  void updateSettingsComboBox();
  void printSettings();

  void activateGUI();
  void deactivateGuiForTimelapse();
  void activateGuiAfterTimelapse();

  bool isInit;
  bool isClosing;

  // Network
  QNetworkAccessManager *networkManager;
  QNetworkRequest *networkRequest;
  QNetworkReply *networkReply;
  bool networkRequestPending;
  bool serverIsRunning;
  bool receivedSettings;
  int serverConnectAttempts;
  QTimer *timer_request;
  QString local_ip;

  QUrl url_server, url_takeImage, url_serverRunning, url_fileExists,
      url_downloadImage, url_timelapseStart, url_timelapseStop,
      url_getTimelapseImages, url_serverNotConnected, url_settingsAll,
      url_addSetting, url_deleteSetting, url_updateSetting;

  // GUI elements
  QMenuBar *menu;
  QPushButton *bt_takeImage, *bt_startTimelapse, *bt_analyze, *bt_save,
      *bt_next, *bt_prev, *bt_stopTimelapse, *bt_quit, *bt_original, *bt_analyzed;
  QComboBox *combo_cameraOptions, *combo_leds;
  QCheckBox *check_saveTimelapse;
  QLabel *lb_interval, *lb_camera;
  QSpinBox *spin_interval;
  QImage *curr_image;
  QTextEdit *ed_info;
  QLineEdit *ed_timelapseSaveName;
  QGraphicsView *gv_view;
  QGraphicsScene *gs_scene;

  QLineEdit* ed_name, *ed_date, *ed_iso, *ed_ss;

  QLineEdit *option_name;
  QTextEdit *option_desc;
  QSpinBox *option_ss, *option_iso;
  QPushButton *option_add, *option_update, *option_delete;

  QStatusBar *statusBar;
  QProgressBar *progressBar;

  QFileDialog *fileDialog;

  // camera options
  QList<QAction *> cameraOptions;
  int no_cameraOptions;
  int activeCameraOption;

  int timelapseInterval;
  bool timelapseRunning;
  int currentTimelapseImage;
  int no_timelapseImages;
  QTimer *timer_timelapse, *timer_serverRunning;

  QString saveDirTimelapse;

  QList<cameraOption> cameraOptionsList;

  QString newImageName;
  cameraOption newImageOptions;
  QList<QImage *> image_list;
  QList<QString> image_info_list;
  int no_images, current_image;

  QList<QImage *> analyzed_image_list;
  int analyzeImage_index;
  QThread* analyzeThread;
  igem_imageAnalyzer* analyzeImageWorker;
  QImage* lowRes;
};

#endif  // IGEM_MAINWINDOW_H
