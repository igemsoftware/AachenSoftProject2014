iGEM TEAM AACHEN 2014

this is our little github repos for the iGEM competition 2014.

Team Aachen has started as a WetLab Team, however we've also had a few software side projects which we would like to share with you.

/SerialClient
The SerialClient application (C#, Windows Framework) can be used to automatically read data from our own developed OD/F device. Furthermore it generates nice graphics from the received data and offers multiple export options.

/TimeseriesLabeller
The TimeseriesLabeller can be used to add a watermark and description to jpg's which can then be put into an animated gif. This eases a lot of manual work!

/measurarty
Measurarty is our image analysis software for agar-chip analysis. This mainly does an automatic image segmentation using SRM, thresholding and our own smoothness index. This project has been developed in Matlab.

/arduino/odf/*
The arduino codes in the folder are for our OD/F device which can be used to either measure optical density or fluorescence.

/backend
The backend is a webserver that provides an API to control our measurement device WatsOn. The server was implemented in python using the flask framework. The main tasks are to control an arduino board over a serial connection and to trigger the Raspberry Pi camera to take images that are returned to the user.

/arduino/watson
The WatsOn arduino code controls a Peltier heater and turns on LEDs after receiving commands from the backend.

/watson_gui
Finally watson_gui contains the Qt-GUI for WatsOn. With this software you can control and communicate with the backend on the device. It also contains our own ported implementation of the image analysis pipeline from measurarty.
