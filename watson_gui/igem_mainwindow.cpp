#include "igem_mainwindow.h"

using namespace std;

//constructor
igem_MainWindow::igem_MainWindow(QWidget* parent) : QMainWindow(parent) {
  isInit = 1;
  isClosing = 0;
  serverConnectAttempts = 0;

  current_image = -1;
  no_images = 0;
  networkRequestPending = 0;
  timelapseInterval = 10;
  timelapseRunning = 0;
  currentTimelapseImage = -1;
  no_timelapseImages = 0;
  activeCameraOption = 0;
  no_cameraOptions = 0;
  timer_serverRunning = new QTimer();
  timer_request = new QTimer();
  timer_timelapse = new QTimer();
  timer_timelapse->setInterval(1000);
  networkManager = new QNetworkAccessManager();
  networkRequest = new QNetworkRequest();
  networkReply = 0;
  serverIsRunning = 0;
  url_serverNotConnected = QUrl("");
  receivedSettings = 0;

  analyzeImage_index = -1;
  analyzeImageWorker = new igem_imageAnalyzer();

  connect( analyzeImageWorker, SIGNAL(finished()), this, SLOT(imageAnalyzerFinished()) );

  setupUi();

  //pipe network reply to the network reply handler
  connect(networkManager, SIGNAL(finished(QNetworkReply*)), this,
          SLOT(networkReplyHandler(QNetworkReply*)));

  //frequently checks if new images are available in timelapse mode
  connect(timer_timelapse, SIGNAL(timeout()), this,
          SLOT(loadTimeseriesImage()));

  connect(timer_request, SIGNAL(timeout()), this, SLOT(cancelNetworkRequest()));

  //enter ip to establish connection
  bool ok = 0;
  local_ip = "127.0.0.1";
  QString rpiIP =
      QInputDialog::getText(this, "Enter IP Address of Raspberry Pi",
                            "IP Address:", QLineEdit::Normal, local_ip, &ok);
  if (!ok) {
    url_server = QUrl("http://" + local_ip + ":1233");
  } else {
    url_server = QUrl("http://" + rpiIP + ":1233");
  }

  //urls for different server requests
  url_serverRunning = QUrl("/status");
  url_takeImage = QUrl("/takeimage?name=");
  url_fileExists = QUrl("/fileexists?filepath=");
  url_downloadImage = QUrl("/downloadimage?filepath=");
  url_timelapseStart = QUrl("/timelapse/start");
  url_timelapseStop = QUrl("/timelapse/stop");
  url_getTimelapseImages = QUrl("/timelapse/images");
  url_settingsAll = QUrl("/settings/all");
  url_addSetting = QUrl("/settings/add");
  url_deleteSetting = QUrl("/settings/delete");
  url_updateSetting = QUrl("/settings/update");

  //send network requests to the server until we got an answer that it is running
  connect(timer_serverRunning, SIGNAL(timeout()), this, SLOT(serverRunning()));
  timer_serverRunning->start(1000);
}

//analyzing the image finished
void igem_MainWindow::imageAnalyzerFinished()
{
    qDebug() << "imageAnalyzerFinished" << endl;
    analyzed_image_list.replace(analyzeImage_index,analyzeImageWorker->getAnalysedImage());
    displayImage(analyzed_image_list.at(analyzeImage_index));

    delete lowRes;
}

//stop a running timelapse if the program is closed
void igem_MainWindow::closing() {
  isClosing = 1;
  if (timelapseRunning) {
    statusBar->showMessage("Exit: waiting for timelapse to stop");
    stopTimelapse();
    while (networkRequestPending) {
      QTest::qWait(50);
    }
  }
  exit(0);
}

//setup of graphical user interface
void igem_MainWindow::setupUi() {
  menu = new QMenuBar();
  QMenu* fileMenu = new QMenu("File");
  fileMenu->addAction("Quit", this, SLOT(closing()));
  menu->addMenu(fileMenu);
  this->setMenuBar(menu);

  bt_takeImage = new QPushButton(tr("Take image"));
  bt_startTimelapse = new QPushButton(tr("Start timelapse"));
  bt_stopTimelapse = new QPushButton(tr("Stop timelapse"));
  bt_analyze = new QPushButton(tr("Analyze"));
  bt_save = new QPushButton(tr("Save"));
  bt_next = new QPushButton(tr(">"));
  bt_prev = new QPushButton(tr("<"));
  lb_interval = new QLabel(tr("Interval (s)"));
  lb_camera = new QLabel(tr("Camera settings"));
  check_saveTimelapse = new QCheckBox("save automatic:");
  ed_timelapseSaveName = new QLineEdit();
  option_add = new QPushButton("Add");
  option_update = new QPushButton("Update");
  option_delete = new QPushButton("Delete");
  bt_original = new QPushButton("Original");
  bt_analyzed = new QPushButton("Analyzed");

  combo_leds = new QComboBox();
  combo_leds->addItem("480nm");
  combo_leds->addItem("450nm");

  spin_interval = new QSpinBox();
  spin_interval->setMinimum(5);
  spin_interval->setMaximum(3600);
  spin_interval->setValue(10);

  combo_cameraOptions = new QComboBox();
  combo_cameraOptions->setCurrentIndex(activeCameraOption);

  option_name = new QLineEdit();
  option_desc = new QTextEdit();
  option_desc->setFixedHeight(
      2 * (QFontMetrics(option_desc->font()).lineSpacing()));
  option_ss = new QSpinBox();
  option_ss->setMaximum(1000000);
  option_ss->setMinimum(10000);
  option_ss->setSingleStep(10000);
  option_iso = new QSpinBox();
  option_iso->setMaximum(800);
  option_iso->setMinimum(100);
  option_iso->setSingleStep(100);

  QVBoxLayout* main_vLayout = new QVBoxLayout();
  QHBoxLayout* main_hLayout = new QHBoxLayout();
  QHBoxLayout* bt_layout = new QHBoxLayout();
  QVBoxLayout* info_layout = new QVBoxLayout();
  QHBoxLayout* nav_layout = new QHBoxLayout();
  QVBoxLayout* saveT_layout = new QVBoxLayout();
  QVBoxLayout* options_layout = new QVBoxLayout();

  saveT_layout->addWidget(check_saveTimelapse);
  saveT_layout->addWidget(ed_timelapseSaveName);

  bt_layout->addWidget(bt_takeImage);
  bt_layout->addWidget(bt_startTimelapse);
  bt_layout->addWidget(bt_stopTimelapse);
  bt_layout->addWidget(lb_interval);
  bt_layout->addWidget(spin_interval);
  bt_layout->addWidget(combo_leds);
  bt_layout->addLayout(saveT_layout);

  gv_view = new QGraphicsView();
  gs_scene = new QGraphicsScene();
  gv_view->setScene(gs_scene);

  main_vLayout->addLayout(bt_layout);
  main_vLayout->addWidget(gv_view);

  //  camera options
  QHBoxLayout* hLay_option = new QHBoxLayout();
  hLay_option->addWidget(lb_camera);
  hLay_option->addWidget(combo_cameraOptions);
  options_layout->addLayout(hLay_option);

  QLabel* lb = new QLabel("Name:");
  hLay_option = new QHBoxLayout();
  hLay_option->addWidget(lb);
  hLay_option->addWidget(option_name);
  options_layout->addLayout(hLay_option);

  lb = new QLabel("Description:");
  hLay_option = new QHBoxLayout();
  hLay_option->addWidget(lb);
  hLay_option->addWidget(option_desc);
  options_layout->addLayout(hLay_option);

  lb = new QLabel("Iso:");
  hLay_option = new QHBoxLayout();
  hLay_option->addWidget(lb);
  hLay_option->addWidget(option_iso);
  options_layout->addLayout(hLay_option);

  lb = new QLabel("Shutterspeed:");
  hLay_option = new QHBoxLayout();
  hLay_option->addWidget(lb);
  hLay_option->addWidget(option_ss);
  options_layout->addLayout(hLay_option);

  hLay_option = new QHBoxLayout();
  hLay_option->addWidget(option_add);
  hLay_option->addWidget(option_delete);
  hLay_option->addWidget(option_update);
  options_layout->addLayout(hLay_option);
  //  camera options end

  nav_layout->addWidget(bt_prev);
  nav_layout->addWidget(bt_next);
  QHBoxLayout* nav2_lay = new QHBoxLayout();
  nav2_lay->addWidget(bt_original);
  nav2_lay->addWidget(bt_analyzed);

  QGroupBox* gb_info = new QGroupBox("Image information:");
  QVBoxLayout* gb_lay = new QVBoxLayout();
  ed_date = new QLineEdit();
  ed_iso = new QLineEdit();
  ed_ss = new QLineEdit();
  ed_date->setReadOnly(1);
  ed_ss->setReadOnly(1);
  ed_iso->setReadOnly(1);
  gb_lay->addWidget(ed_date);
  gb_lay->addWidget(ed_iso);
  gb_lay->addWidget(ed_ss);
  gb_info->setLayout(gb_lay);

  info_layout->addLayout(options_layout);

  QFrame* line = new QFrame();
  line->setFrameShape(QFrame::HLine);
  line->setFrameShadow(QFrame::Sunken);
  info_layout->addWidget(line);

  info_layout->addWidget(gb_info);

  line = new QFrame();
  line->setFrameShape(QFrame::HLine);
  line->setFrameShadow(QFrame::Sunken);
  info_layout->addWidget(line);

  info_layout->addWidget(bt_save);
  info_layout->addWidget(bt_analyze);
  info_layout->addLayout(nav_layout);
  info_layout->addLayout(nav2_lay);
  QSpacerItem* spacer = new QSpacerItem(0,0,QSizePolicy::Maximum,QSizePolicy::Expanding);
  info_layout->addSpacerItem(spacer);

  main_hLayout->addLayout(main_vLayout, 1);

  QWidget* sideW = new QWidget();
  sideW->setLayout(info_layout);
  sideW->setMaximumWidth(300);

  main_hLayout->addWidget(sideW);

  QWidget* centralW = new QWidget();
  centralW->setLayout(main_hLayout);
  setCentralWidget(centralW);

  statusBar = new QStatusBar();
  progressBar = new QProgressBar();
  progressBar->setMaximumWidth(150);
  statusBar->addPermanentWidget(progressBar);
  setStatusBar(statusBar);

  fileDialog = new QFileDialog();
  fileDialog->setDefaultSuffix("jpg");

  //connect signals to corresponding slots
  connect(bt_takeImage, SIGNAL(clicked()), this, SLOT(takeImage()));
  connect(bt_startTimelapse, SIGNAL(clicked()), this, SLOT(startTimelapse()));
  connect(bt_stopTimelapse, SIGNAL(clicked()), this, SLOT(stopTimelapse()));
  connect(combo_cameraOptions, SIGNAL(activated(int)), this,
          SLOT(setCameraOption(int)));
  connect(bt_next, SIGNAL(clicked()), this, SLOT(nextImage()));
  connect(bt_prev, SIGNAL(clicked()), this, SLOT(prevImage()));
  connect(bt_save, SIGNAL(clicked()), this, SLOT(saveImage()));
  connect(spin_interval, SIGNAL(valueChanged(int)), this,
          SLOT(setTimelapseInterval(int)));
  connect(check_saveTimelapse, SIGNAL(toggled(bool)), this,
          SLOT(checkSaveTimelapse(bool)));
  connect(ed_timelapseSaveName, SIGNAL(editingFinished()), this,
          SLOT(saveDirChanged()));
  connect(option_add, SIGNAL(clicked()), this, SLOT(addSetting()));
  connect(option_delete, SIGNAL(clicked()), this, SLOT(deleteSetting()));
  connect(option_update, SIGNAL(clicked()), this, SLOT(updateSetting()));
  connect(option_name, SIGNAL(textEdited(QString)), this,
          SLOT(settingChanged(QString)));
  connect(option_desc, SIGNAL(textChanged()), this, SLOT(settingChanged()));
  connect(option_ss, SIGNAL(valueChanged(QString)), this,
          SLOT(settingChanged()));
  connect(option_iso, SIGNAL(valueChanged(QString)), this,
          SLOT(settingChanged(QString)));
  connect(bt_original,SIGNAL(clicked()),this,SLOT(showOriginal()));
  connect(bt_analyzed,SIGNAL(clicked()),this,SLOT(showAnalyzed()));

  connect(bt_analyze,SIGNAL(clicked()),this,SLOT(analyzeImage()));

  //disable gui (not connected to the server yet)
  bt_takeImage->setEnabled(0);
  bt_startTimelapse->setEnabled(0);
  bt_stopTimelapse->setEnabled(0);
  bt_analyze->setEnabled(0);
  bt_save->setEnabled(0);
  bt_save->setEnabled(0);
  bt_next->setEnabled(0);
  bt_prev->setEnabled(0);
  check_saveTimelapse->setEnabled(0);
  ed_timelapseSaveName->setEnabled(0);
  spin_interval->setEnabled(0);
  combo_cameraOptions->setEnabled(0);
  option_add->setEnabled(0);
  option_delete->setEnabled(0);
  option_update->setEnabled(0);
  option_iso->setEnabled(0);
  option_ss->setEnabled(0);
  option_desc->setEnabled(0);
  option_name->setEnabled(0);
  combo_leds->setEnabled(0);
  bt_original->setEnabled(0);
  bt_analyzed->setEnabled(0);

  //fit window into desktop size
  QDesktopWidget* desktop = QApplication::desktop();
  QSizeF screenSize = desktop->screenGeometry().size();
  qDebug() << "screen geometry " << screenSize << endl;
  int overscanw = 0;
  int overscanh = 0;
  QSize displaySize =
      QSize(screenSize.width() - overscanw, screenSize.height() - overscanh);
  resize(displaySize);

  show();
}

//display the information for the current image
void igem_MainWindow::showImageInfo() {
  QImage* img = image_list.at(current_image);

  if(!img->text("name").contains("_")) return;

  QStringList split = img->text("name").split("_");
  QString date = split.at(1);
  date.insert(6,"/");
  date.insert(4,"/");
  QString time = split.at(2);
  time.insert(4,":");
  time.insert(2,":");

  ed_date->setText("Timestamp: "+time+", "+date);
  ed_iso->setText("ISO: "+img->text("iso"));
  ed_ss->setText("Shutter: "+img->text("ss"));
}

//TODO
void igem_MainWindow::analyzeImage()
{
  if(!analyzeImageWorker->isRunning()){
    qDebug() << "start analyzing image" << endl;
    analyzeImage_index = current_image;

    QImage* img = image_list.at(current_image);
    //downsample image if computation takes to long
    lowRes = new QImage(img->scaled(800,600,Qt::KeepAspectRatio,Qt::SmoothTransformation));

    analyzeImageWorker->setAnalyzeImage(lowRes);
    analyzeImageWorker->start();
  }
}

//read a camera setting and add to the list of available options
void igem_MainWindow::readSetting(QString s) {
  if (s.length() == 0) return;

  //magic string which replaces linebreaks on the server side
  s.replace("##igem#is#great##", "\n");
  QStringList split = s.split(";");

  if (split.length() != 3) {
    qDebug() << "ERROR: corrupt setting" << endl;
    return;
  }

  //setting is split into: name, description, parameters
  cameraOption newOption;
  newOption.name = split.at(0);
  newOption.desc = split.at(1);

  QStringList split2 = split.at(2).split("-");

  for (int i = 0; i < split2.length(); i++) {
    QStringList split3 = split2.at(i).split(" ");
    if (split3.at(0) == "t")
      newOption.t = split3.at(1).toInt();
    else if (split3.at(0) == "ex")
      newOption.ex = split3.at(1);
    else if (split3.at(0) == "awb")
      newOption.awb = split3.at(1);
    else if (split3.at(0) == "ev")
      newOption.ev = split3.at(1).toInt();
    else if (split3.at(0) == "ss")
      newOption.ss = split3.at(1).toInt();
    else if (split3.at(0) == "awbg") {
      QStringList awbg = split3.at(1).split(",");
      newOption.awbg.first = awbg.at(0).toDouble();
      newOption.awbg.second = awbg.at(1).toDouble();
    } else if (split3.at(0) == "ISO") {
      newOption.iso = split3.at(1).toInt();
    }
  }
  cameraOptionsList.append(newOption);
}

//add all available settings to the combo box
void igem_MainWindow::updateSettingsComboBox() {
  combo_cameraOptions->clear();
  for (int i = 0; i < no_cameraOptions; i++) {
    combo_cameraOptions->addItem(cameraOptionsList.at(i).name);
  }
  activeCameraOption = min(activeCameraOption, cameraOptionsList.length());
  if (no_cameraOptions != 0) {
    combo_cameraOptions->setCurrentIndex(activeCameraOption);
  }
}

//debug function to print all available settings to the console
void igem_MainWindow::printSettings() {
  for (int i = 0; i < cameraOptionsList.length(); i++) {
    cameraOption cOpt = cameraOptionsList.at(i);
    qDebug() << cOpt.name << " " << cOpt.desc << " " << QString::number(cOpt.t)
             << " " << cOpt.ex << " " << cOpt.awb << " "
             << QString::number(cOpt.ev) << " " << QString::number(cOpt.ss)
             << " " << QString::number(cOpt.awbg.first) << ","
             << QString::number(cOpt.awbg.second) << " "
             << QString::number(cOpt.iso) << endl;
  }
}

//activate the user interface
void igem_MainWindow::activateGUI() {
  bt_takeImage->setEnabled(1);
  bt_startTimelapse->setEnabled(1);
  bt_stopTimelapse->setEnabled(0);
  bt_analyze->setEnabled(1);
  bt_save->setEnabled(1);
  bt_save->setEnabled(1);
  bt_next->setEnabled(1);
  bt_prev->setEnabled(1);
  check_saveTimelapse->setEnabled(1);
  ed_timelapseSaveName->setEnabled(1);
  spin_interval->setEnabled(1);
  combo_cameraOptions->setEnabled(1);
  option_add->setEnabled(1);
  option_delete->setEnabled(1);
  option_update->setEnabled(1);
  option_iso->setEnabled(1);
  option_ss->setEnabled(1);
  option_desc->setEnabled(1);
  option_name->setEnabled(1);
  combo_leds->setEnabled(1);
  bt_original->setEnabled(1);
  bt_analyzed->setEnabled(1);
}

//deactivate interface for timelapse
void igem_MainWindow::deactivateGuiForTimelapse() {
  bt_takeImage->setEnabled(0);
  bt_startTimelapse->setEnabled(0);
  bt_stopTimelapse->setEnabled(1);
  bt_save->setEnabled(0);
  bt_save->setEnabled(0);
  bt_next->setEnabled(0);
  bt_prev->setEnabled(0);
  check_saveTimelapse->setEnabled(0);
  ed_timelapseSaveName->setEnabled(0);
  spin_interval->setEnabled(0);
  combo_cameraOptions->setEnabled(0);
  option_add->setEnabled(0);
  option_delete->setEnabled(0);
  option_update->setEnabled(0);
  option_iso->setEnabled(0);
  option_ss->setEnabled(0);
  option_desc->setEnabled(0);
  option_name->setEnabled(0);
  combo_leds->setEnabled(0);
}

//activate interface after timelapse
void igem_MainWindow::activateGuiAfterTimelapse() {
  bt_takeImage->setEnabled(1);
  bt_startTimelapse->setEnabled(1);
  bt_stopTimelapse->setEnabled(0);
  bt_save->setEnabled(1);
  bt_save->setEnabled(1);
  bt_next->setEnabled(1);
  bt_prev->setEnabled(1);
  check_saveTimelapse->setEnabled(1);
  ed_timelapseSaveName->setEnabled(1);
  spin_interval->setEnabled(1);
  combo_cameraOptions->setEnabled(1);
  option_add->setEnabled(1);
  option_delete->setEnabled(1);
  option_update->setEnabled(1);
  option_iso->setEnabled(1);
  option_ss->setEnabled(1);
  option_desc->setEnabled(1);
  option_name->setEnabled(1);
  combo_leds->setEnabled(1);
}

//fit the image into the current window size
void igem_MainWindow::resizeEvent(QResizeEvent* event) {
  QRectF bounds = gs_scene->itemsBoundingRect();
  gv_view->fitInView(bounds, Qt::KeepAspectRatio);
}

//start a network request with a specified url
//onClose=1 if the program is terminating and a timelapse has to be stopped
void igem_MainWindow::startNetworkRequest(QUrl url, bool onClose = 0) {

  //dont make a new request if a normal one is still pending
  if (networkRequestPending && !onClose) {
    qDebug() << "ERROR: network request still pending" << endl;
    return;
  }

  //server not connected, try to connect and call this request later
  if (!serverIsRunning && !url.url().contains("/status") && !onClose) {
    url_serverNotConnected = url;
    serverRunning();
    return;
  }

  //start the network request
  networkRequestPending = 1;
  qDebug() << "startNetworkRequest: " << url.url() << endl;
  networkRequest->setUrl(url);
  networkReply = networkManager->get(*networkRequest);
  timer_request->start(1000); //kill request after a second
}

//analyze incomming network reply
void igem_MainWindow::networkReplyHandler(QNetworkReply* reply) {
  if (!isClosing ||
      QString(reply->request().url().url()).contains("timelapse/stop?end=1"))
    networkRequestPending = 0;

  //check the request url
  QUrl request_url = reply->request().url();
  QString request = request_url.url();
  if (request.contains("takeimage")) {
    QString imageName = (QString)reply->readAll();
    if (!imageName.contains("no setting")) {
      statusBar->showMessage("image taken");
      takeImageFinished(imageName);
    } else {
      statusBar->showMessage("could not take image");
    }
  } else if (request.contains("downloadimage")) {
    loadImageFinished(reply);
  } else if (request.contains("fileexists")) {

  } else if (request.contains("timelapse/start")) {
    statusBar->showMessage("timelapse started");
    qDebug() << "timelapse/start reply" << endl;
    timer_timelapse->start();
  } else if (request.contains("timelapse/stop")) {
    statusBar->showMessage("timelapse stoped");
    qDebug() << "timelapse/stop reply" << endl;
    activateGuiAfterTimelapse();
  } else if (request.contains("timelapse/images")) {
    loadTimeseriesImageFinished(reply);
  } else if (request.contains("settings/all")) {
    QString allSettings = (QString)reply->readAll();
    qDebug() << allSettings << endl;

    if (allSettings != "") { //read all settings
      combo_cameraOptions->clear();
      cameraOptionsList.clear();
      receivedSettings = 1;
      QStringList singleSettings =
          allSettings.split("\n", QString::SkipEmptyParts);
      for (int i = 0; i < singleSettings.length(); i++) {
        readSetting(singleSettings.at(i));
      }
    } else
      return;

    no_cameraOptions = cameraOptionsList.length();
    qDebug() << "no_cameraOptions " << no_cameraOptions << endl;
    printSettings();
    updateSettingsComboBox();
    setCameraOption(activeCameraOption);
    progressBar->setValue(100);

    if (url_serverNotConnected.url() != "") {
      startNetworkRequest(url_serverNotConnected);
      url_serverNotConnected = QUrl("");
    }
  } else if (request.contains("/settings/add")) {
    QString answer = (QString)reply->readAll();
    qDebug() << answer << endl;
    if (answer == "setting added") {
      statusBar->showMessage("setting added");
      no_cameraOptions++;
      activeCameraOption = no_cameraOptions - 1;
      getSettingsAll();
    }
  } else if (request.contains("/settings/update")) {
    QString answer = (QString)reply->readAll();
    qDebug() << answer << endl;
    if (answer == "setting updated") {
      statusBar->showMessage("setting updated");
      getSettingsAll();
    }
  } else if (request.contains("/settings/delete")) {
    QString answer = (QString)reply->readAll();
    qDebug() << answer << endl;
    if (answer == "setting deleted") {
      statusBar->showMessage("setting deleted");
      activeCameraOption = 0;
      no_cameraOptions--;
      getSettingsAll();
    } else {
      statusBar->showMessage("could not delete setting");
      qDebug() << "could not delete setting" << endl;
    }
  } else if (request.contains("/status")) {
    QString answer = (QString)reply->readAll();
    if (answer == "running") {
      progressBar->setValue(25);
      serverIsRunning = 1;
      serverConnectAttempts = 0;
      isInit = 0;
      timer_serverRunning->stop();
      activateGUI();
      qDebug() << "serverIsRunning " << serverIsRunning << endl;
      progressBar->setValue(50);

      getSettingsAll();
    } else {
      serverIsRunning = 0;
      serverConnectAttempts++;
      //reenter ip address
      if (serverConnectAttempts == 10) {
        timer_serverRunning->stop();
        bool ok = 0;
        QString rpiIP = QInputDialog::getText(
            this, "Enter IP Address of Raspberry Pi", "IP Address:",
            QLineEdit::Normal, local_ip, &ok);
        if (!ok) {
          url_server = QUrl("http://" + local_ip + ":1233");
        } else {
          url_server = QUrl("http://" + rpiIP + ":1233");
        }
        serverConnectAttempts = 0;
        timer_serverRunning->start();
      } else {
        answer = "trying to connect";
      }
    }
    statusBar->showMessage("Server: " + answer);
    qDebug() << "server status: " << answer << endl;
  } else {
    qDebug() << "ERROR: unknown network reply!" << endl;
  }
}

void igem_MainWindow::networkReplyError(QNetworkReply::NetworkError e) {
  qDebug() << "networkReplyError: " << e;
}

//start server connect request
void igem_MainWindow::serverRunning() {
  QDateTime currentTime = QDateTime::currentDateTime();
  startNetworkRequest(url_server.url() + url_serverRunning.url() +
                      "?timestamp=" + QString::number(currentTime.toTime_t()) +
                      "&init=" + QString::number(isInit));
}

//start take image request
void igem_MainWindow::takeImage() {
  QString led = combo_leds->currentText();
  led.replace("nm", "");
  startNetworkRequest(url_server.url() + url_takeImage.url() +
                      cameraOptionsList.at(activeCameraOption).name + "&led=" +
                      led);
  statusBar->showMessage("taking image");
}

//image taken -> load it
void igem_MainWindow::takeImageFinished(QString imageName) {
  if (imageName != "") {
    loadImage(imageName);
  }
}

//start a time lapse shooting
void igem_MainWindow::startTimelapse() {
  QString led = combo_leds->currentText();
  led.replace("nm", "");
  startNetworkRequest(url_server.url() + url_timelapseStart.url() +
                      "?interval=" + QString::number(timelapseInterval) +
                      "&name=" + cameraOptionsList.at(activeCameraOption).name +
                      "&led=" + led);
  timer_timelapse->start();
  timelapseRunning = 1;

  deactivateGuiForTimelapse();
  updateSettingsComboBox(); //display the last activated settings
}

//stop the time lapse
void igem_MainWindow::stopTimelapse() {
  timer_timelapse->stop();
  startNetworkRequest(url_server.url() + url_timelapseStop.url() + "?end=" +
                      QString::number(isClosing));
  timelapseRunning = 0;
}

//save the current displayed image
void igem_MainWindow::saveImage() {
  if (no_images == 0) {
    statusBar->showMessage("No image");
    return;
  }

  QString saveFileName = fileDialog->getSaveFileName(
      this, "Save image", QDir::currentPath(), tr("Images (*.jpg)"));
  if (saveFileName == "") return;

  if (!saveFileName.endsWith(".jpg")) {
    saveFileName.append(".jpg");
  }
  qDebug() << "saveFileName = " << saveFileName << endl;

  //TODO analyzed image
  bool ok = image_list.at(current_image)->save(saveFileName, 0, 100);

  if (ok) {
    statusBar->showMessage("saved image: " + saveFileName);
    qDebug() << "saved image " << saveFileName << endl;
  } else {
    statusBar->showMessage("could not save image: " + saveFileName);
    qDebug() << "could not save image" << endl;
  }
}

//start request to get all available camera settings
void igem_MainWindow::getSettingsAll() {
  startNetworkRequest(url_server.url() + url_settingsAll.url());
}

//start request to add a camera setting
void igem_MainWindow::addSetting() {
  stringstream newSetting;
  if (option_name->text().isEmpty()) return;
  QString name = option_name->text();
  QString desc = option_desc->toPlainText();

  newSetting << "-t 500 -ex night -awb off -ev 0 -ss " << option_ss->value()
             << " -awbg 1.5,1.2 -ISO " << option_iso->value();
  startNetworkRequest(url_server.url() + url_addSetting.url() + "?name=" +
                      name + "&desc=" + desc + "&param=" +
                      QString::fromStdString(newSetting.str()));
}

//request to update the current setting
void igem_MainWindow::updateSetting() {
  stringstream newSetting;
  QString name = cameraOptionsList.at(activeCameraOption).name;
  if (option_name->text() != name) return;
  QString desc = option_desc->toPlainText();
  newSetting << "-t 500 -ex night -awb off -ev 0 -ss " << option_ss->value()
             << " -awbg 1.5,1.2 -ISO " << option_iso->value();
  startNetworkRequest(url_server.url() + url_updateSetting.url() + "?name=" +
                      name + "&desc=" + desc + "&param=" +
                      QString::fromStdString(newSetting.str()));
}

//start request to delete the current setting
void igem_MainWindow::deleteSetting() {
  QString name = cameraOptionsList.at(activeCameraOption).name;
  startNetworkRequest(url_server.url() + url_deleteSetting.url() + "?name=" +
                      name);
}

//the user changed a camera setting option
void igem_MainWindow::settingChanged(QString) {
  cameraOption selectedOpt = cameraOptionsList.at(activeCameraOption);
  bool optExists = 0;
  int optExistsIndex = -1;
  for (int i = 0; i < cameraOptionsList.length(); i++) {
    if (cameraOptionsList.at(i).name == option_name->text()) {
      optExists = 1;
      optExistsIndex = i;
    }
  }

  QString col = QApplication::palette().text().color().name();
  QString col2 = "green";
  if (!optExists) {
    option_add->setStyleSheet("QPushButton {color: "+col2+"}");
    option_update->setStyleSheet("QPushButton {color: " + col + "}");
    option_desc->setStyleSheet("QPushButton {color: " + col + "}");
    option_iso->setStyleSheet("QPushButton {color: " + col + "}");
    option_ss->setStyleSheet("QPushButton {color: " + col + "}");
  }
  if (optExists) {
    option_add->setStyleSheet("QPushButton {color: " + col + "}");
    cameraOption existingOpt = cameraOptionsList.at(optExistsIndex);

    bool diff = 0;

    if (selectedOpt.desc != existingOpt.desc) {
      option_desc->setStyleSheet("QPushButton {color: "+col2+"}");
      diff = 1;
    } else {
      option_desc->setStyleSheet("QPushButton {color: " + col + "}");
    }

    if (selectedOpt.iso != existingOpt.iso) {
      option_iso->setStyleSheet("QPushButton {color: "+col2+"}");
      diff = 1;
    } else {
      option_iso->setStyleSheet("QPushButton {color: " + col + "}");
    }

    if (selectedOpt.ss != existingOpt.ss) {
      option_ss->setStyleSheet("QPushButton {color: "+col2+"}");
      diff = 1;
    } else {
      option_ss->setStyleSheet("QPushButton {color: " + col + "}");
    }

    if (diff) {
      option_update->setStyleSheet("QPushButton {color: "+col2+" }");
    }
  }
}

//request an image from the server
void igem_MainWindow::loadImage(QString fileName) {
  newImageName = fileName;
  newImageOptions = cameraOptionsList.at(activeCameraOption);
  startNetworkRequest(url_server.url() + url_downloadImage.url() + fileName);
}

//image request returned, load the image
void igem_MainWindow::loadImageFinished(QNetworkReply* reply) {
  qDebug() << "result received";
  QByteArray data = (QByteArray)reply->readAll();
  curr_image = new QImage();
  curr_image->loadFromData(data);

  curr_image->setText("name", newImageName);
  curr_image->setText("iso", QString::number(newImageOptions.iso));
  curr_image->setText("ss", QString::number(newImageOptions.ss));

  image_list.append(curr_image);
  image_info_list.append(newImageName);

  analyzed_image_list.append(curr_image);

  no_images = image_list.length();
  current_image = no_images - 1;

  qDebug() << "new image #" << no_images << " " << newImageName << endl;

  displayImage(curr_image);
  showImageInfo();

  statusBar->showMessage("loaded image");

  if (timer_timelapse->isActive() &&
      check_saveTimelapse->isChecked()) {  // we're in timelapse mode
    saveTimeseriesImage(); //save image if checked
  }
}

//request the latest time lapse image
void igem_MainWindow::loadTimeseriesImage() {
  startNetworkRequest(url_server.url() + url_getTimelapseImages.url());
}

//time series image request returned, load the image
void igem_MainWindow::loadTimeseriesImageFinished(QNetworkReply* reply) {
  QString images = (QString)reply->readAll();
  QStringList imageList = images.split(",", QString::SkipEmptyParts);
  int no_img = imageList.length();
  if (no_img > no_timelapseImages) {
    QString new_img = imageList.last();
    no_timelapseImages++;
    currentTimelapseImage++;
    loadImage(new_img);
  }
}

//automatically save the current timeseries image
void igem_MainWindow::saveTimeseriesImage() {
  QString saveFileName =
      saveDirTimelapse + "_" +
      QString::number(currentTimelapseImage * timelapseInterval) + ".jpg";
  bool ok = image_list.at(current_image)->save(saveFileName, 0, 100);

  if (ok) {
    statusBar->showMessage("saved timeseries image: " + saveFileName);
    qDebug() << "saved timelapse image " << saveFileName << endl;
  } else {
    statusBar->showMessage("could not save timeseries image: " + saveFileName);
    qDebug() << "could not save timelapse image" << endl;
  }
}

//display a camera option
void igem_MainWindow::setCameraOption(int option) {
  qDebug() << "selected camera option " << option << endl;
  activeCameraOption = option;

  cameraOption opt = cameraOptionsList.at(option);
  option_name->setText(opt.name);
  option_desc->setText(opt.desc);
  option_iso->setValue(opt.iso);
  option_ss->setValue(opt.ss);
}

//set the interval between time lapse images
void igem_MainWindow::setTimelapseInterval(int interval) {
  timelapseInterval = interval;
}

//show the previous image
void igem_MainWindow::prevImage() {
  if (current_image > 0) {
    current_image--;
    displayImage(image_list.at(current_image));
    showImageInfo();
    qDebug() << "current image #" << current_image << endl;
  }
}

//show the next image
void igem_MainWindow::nextImage() {
  if (current_image < no_images - 1) {
    current_image++;
    displayImage(image_list.at(current_image));
    showImageInfo();
    qDebug() << "current image #" << current_image << endl;
  }
}

//display a given image
void igem_MainWindow::displayImage(QImage* image) {
  gs_scene->clear();
  gs_scene->addPixmap(QPixmap::fromImage(*image));
  QRectF bounds = gs_scene->itemsBoundingRect();
  gv_view->fitInView(bounds, Qt::KeepAspectRatio);
  gv_view->update();
}

//automatic saving is activated
void igem_MainWindow::checkSaveTimelapse(bool check) {
  if (check) {
    QString saveDir = fileDialog->getSaveFileName(
        this, "Select directory and base filename", QDir::currentPath());
    qDebug() << "saveDir " << saveDir << endl;
    if (saveDir != "") {
      saveDirTimelapse = saveDir;
      statusBar->showMessage(tr("Automatic timelapse saving on"));
      ed_timelapseSaveName->setEnabled(true);
      ed_timelapseSaveName->setText(saveDirTimelapse);
    } else {
      statusBar->showMessage(tr("Automatic timelapse saving off"));
      check_saveTimelapse->setChecked(false);
      ed_timelapseSaveName->clear();
      ed_timelapseSaveName->setEnabled(false);
    }
  }
}

//user changed the save directory
void igem_MainWindow::saveDirChanged() {
  QString newDir = ed_timelapseSaveName->text();
  int i = newDir.lastIndexOf("/");
  QString path = newDir;
  path.truncate(i);

  qDebug() << "test " << path << endl;

  QFileInfo fi(path);
  QFileInfo fi2(newDir);
  if (fi.isDir() && !fi2.isDir()) {
    saveDirTimelapse = newDir;
    qDebug() << "newSaveDir " << newDir << endl;
    statusBar->showMessage("timelapse save dir changed to: " + newDir);
  } else {
    ed_timelapseSaveName->setText(saveDirTimelapse);
  }
}

//cancel a network request
void igem_MainWindow::cancelNetworkRequest() {
  if (networkReply != 0) {
    if (networkReply->isRunning() &&
        networkRequest->url().url().contains("/status"))
      networkReply->abort();
  }
}
