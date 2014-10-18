###
###     From http://2014.igem.org/Team:Aachen
###

import time
import serial
import atexit
import sys
import os
import subprocess
import socket, re
from time import sleep
import os.path
from flask import request, send_file
from threading import Thread


from flask import Flask
app = Flask(__name__)

ser = None

#execute argument 'cmd' in commandline
def executecmd(cmd):
    p = subprocess.Popen( cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT )
    lines = p.stdout.readlines()
    return lines
   
def readsettings():
    try:
        o = open('settings.txt','r')
        settings = o.readlines()
        o.close()
        return settings
    except:
        return "error"
            
def writesettings(settings):
    try:
        w = open('settings.txt','w')
        w.write(settings)
        w.close()
        return "saved"
    except:
        return "error"

def get_ipv4_address():
    """
    Returns IP address of current machine.
    adapted from http://stackoverflow.com/questions/166506/finding-local-ip-addresses-using-pythons-stdlib/1947766, user: fccoelho
    """
    
    p = subprocess.Popen(["ifconfig"], stdout=subprocess.PIPE)
    ifc_resp = p.communicate()
    patt = re.compile(r'inet\s*\w*\S*:\s*(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})')
    resp = patt.findall(ifc_resp[0])
    for res in resp:
        if not res.startswith('127.'):
            return res    
            
                
           
class timelapsethread(Thread):
    def __init__(self):
        a = 42
        
        
    def initall(self,interval,ser,param,led):
        Thread.__init__(self)
        self.running = False
        # interval of timelapse
        self.interval = interval
        # list of all taken images
        self.images=[]
        # connection to arduino
        self.ser = ser
        # parameter for raspistill (camera.sh)
        self.param = param
        # identifier of led to turn on (450 = 450nm, 480 = 480nm)
        self.led = led
    
    #start timelapse thread    
    def run(self):
        # count number of taken images
        counter=0
        
        # if thread is not stopped
        while self.running:
            # send command to arduino to turn on leds
            if self.led == "450":
                self.ser.write("5")
            else:
                self.ser.write("8")
            print "leds_on"
            # wait for leds to turn on
            sleep(2)
            
            # create path to next image
            fileName = 'testtimelapse' + '_' + str(counter)
            filePath = ''+fileName
            
            #take image
            result = executecmd('./camera.sh %s %s' %(filePath, self.param) )
            print result
            
            if len(result)>0:
                # append name of new image to list 'images'
                image = str(result[0]).rstrip('\n')
                if self.running:
                    self.images.append(image)
            
            # turn off leds that are turned on        
            self.ser.write("o")
            print "leds_off"
            
            # wait for 'self.interval' millisecond to tke next image
            sleep(int(self.interval))
            counter+=1

        self.running = False
        print "thread finished"  
        return "thread finished"


"""
## start timelapse
#      url: /timelapse/start?interval=5&name=1&led=8
#arguments: interval = [5sec-3600sec], name = [name of setting], led = [450, 480]
#   output: ["thread started","wrong parameter 'interval'","error reading settings file","no setting with this name","no setting available"]
"""
@app.route('/timelapse/start')
def threadingstart():    
    name = request.args.get('name')
    interval = request.args.get('interval')
    led = request.args.get('led')
    settings = readsettings()
    
    if settings == "error":
        return "error reading settings file"

    if len(settings)>1:
        # search for setting with matching name in list of settings
        for setting in settings[1:]:
            [s_name,s_desc,s_param]=setting.split(';')
            s_name = s_name.strip()
            s_desc = s_desc.strip()
            s_param = s_param.strip()
            
            if s_name==name:
                if interval.isdigit():
                    if 0< int(interval):
                        # start timelapse thread 
                        imagethread.initall(interval,ser,s_param,led)
                        imagethread.running = True
                        imagethread.start()
                                
                        return "thread started"
                    else:
                        return "wrong parameter 'interval'"
        return "no setting with this name"
    return "no setting available"
    
"""
## stop timelapse
#   url: /timelapse/stop
#output: ["thread stopped","no thread running"]
"""
@app.route('/timelapse/stop')
def threadingstop():
    try:
        if imagethread.running==True:
            imagethread.running = False
            imagethread.join()
            return "thread stopped"
    except:
        a = 42
        
    return "no thread running"
    

"""
## get timelapse image names
#   url: /timelapse/images
#output: [image_timestamp_count_optionnumber.jpg,image_timestamp_count_optionnumber.jpg,...]
--> files can be downloaded through the url '/downloadimage'
"""
@app.route('/timelapse/images')
def images():
    try:
        images = imagethread.images
        # return list of all taken images
        return str(images).replace("'",'').replace('[','').replace(']','').replace(' ','')
    except:
        return ""

"""
## timelapse status
#   url: /timelapse/status
#output: ["running","not running"]
"""
@app.route('/timelapse/status')
def timelapsestatus():
    try:
        if imagethread.running:
            return "running"
        return "not running"
    except:
        return "not running"
        
"""
## download image
#      url: /downloadimage?filepath=image_name.jpg
#arguments: filepath
#   output: [File to download,"wrong file extension","file doesnt exists"]
"""
@app.route('/downloadimage')
def servefile():
    filepath = request.args.get('filepath')
    
    # check if requested filename ends with '.jpg'
    if not filepath.endswith(".jpg"):
        return "wrong file extension"
        
    if os.path.isfile(filepath):
        # return file to user
        return send_file(filepath, mimetype='image/jpeg')
    else:
        return "file doesnt exists"
    
"""
## file exists?
#      url: /fileexists?filepath=image_timestamp_count_optionnumber.jpg
#arguments: filepath
#   output: [true,false]
"""    
@app.route("/fileexists")
def fileexists():
    filepath = request.args.get('filepath')
    
    if filepath != '':
        if os.path.isfile(filepath):
            return "true"
        else:
            return "false"
        

"""
## take single image:
#      url: /takeimage?name=1&led=480
#arguments: name = [name of setting], led = [450, 480]
#   output: [filepath, error]
"""
@app.route("/takeimage")
def takeimage():
    name = request.args.get('name')
    led = request.args.get('led')
    settings = readsettings()
    
    if settings == "error":
        return "error reading settings file"

    if len(settings)>1:
        # search for setting with matching name in list of settings
        for setting in settings[1:]:
            [s_name,s_desc,s_param]=setting.split(';')
            s_name = s_name.strip()
            s_desc = s_desc.strip()
            s_param = s_param.strip()
            if s_name==name:
                             
                # send command to arduino to turn on leds
                if led == "450":
                    ser.write("5")
                else:
                    ser.write("8")
                print "leds_on"
                # wait for leds to turn on
                sleep(2)
                
                # create path to image
                baseFileName = 'image'
                dir = ''
                fileName = baseFileName+"_"
                filePath = dir+fileName
                
                #take image
                result = executecmd('./camera.sh %s %s' %(filePath, s_param))
                print result
                
                # turn off leds that are turned on 
                ser.write("o")
                print "leds_off"
                
                # return path to image
                result = str(result)
                return result.rstrip('\n').replace("'",'').replace("\\n",'').replace('[','').replace(']','').replace(' ','')
                break
                
        return "no setting with this name"
            
         
    return "no setting available"
    
    
"""
## is server running?
#              url: /status?timestamp=1346274273
#optional argument: timestamp = [unix timestamp], init = [1]
#           output: status = ["running",error]
"""
@app.route('/status')
def status():
    timestamp = request.args.get('timestamp')
    init = request.args.get('init')
    
    # set date on Raspberry Pi
    if len(timestamp)==10 and timestamp.isdigit():
        if timestamp != "":
            result = executecmd('sudo ./setdate.sh %s' %(timestamp))
    
    # if init is set stop previously started thread if not already stopped        
    if init.isdigit():
        if int(init)==1:
            try:
                if imagethread.running==True:
                    imagethread.running = False
                    imagethread.join()
            except:
                a = 42
              
    return "running"   


"""
## get all saved settings
#          url: /settings/all
#       output: [name;desc;parameter\nname;desc;parameter, "no settings", "error"]
#sample output: 1;Desc1;-t 500 -ex night -awb off -ev 0 -ss 100000 -awbg 1.5,1.2 -ISO 100\n2;Desc2;-t 500 -ex night -awb off -ev 0 -ss 200000 -awbg 1.5,1.2 -ISO 100
"""
@app.route('/settings/all')
def getall():
    settings = readsettings()

    if len(settings)>1:
        return "".join(settings[1:]).strip()
    else:
        return "no settings"
    return "error"
 
"""
## get single setting
#        url: /settings/single?name=1
#  arguments: name = [name of setting]
#     output: [name;desc;parameter,"error reading settings file","no setting with this name"]
sample output: 1;Desc1;-t 500 -ex night -awb off -ev 0 -ss 100000 -awbg 1.5,1.2 -ISO 100
"""
@app.route('/settings/single')
def getbyname():
    name = request.args.get('name')
    settings = readsettings()
    
    if settings == "error":
        return "error reading settings file"

    if len(settings)>1:
        
        exists=False    
        
        for setting in settings[1:]:
            [s_name,s_desc,s_param]=setting.split(';')
            s_name = s_name.strip()
            s_desc = s_desc.strip()
            s_param = s_param.strip()
            if s_name==name:
                exists=True
                return setting
                break
                
        return "no setting with this name" 

"""
## add new setting
#      url: /settings/add?name=1&desc=First line##igem#is#great##second line of description&param=-t 500 -ex night -awb off -ev 0 -ss 100000 -awbg 1.5,1.2 -ISO 100
#arguments: name = [name of setting], desc = [description of setting], param = [parameter of setting]
#   output: ["setting added","error reading settings file","already exists"]
"""
@app.route('/settings/add')
def add():
    name = request.args.get('name').strip()
    # replace linebreaks with '##igem#is#great##' to save the setting to the settings file
    desc = request.args.get('desc').strip().replace('\n','##igem#is#great##')
    param = request.args.get('param').strip()
    settings = readsettings()
    
    if settings == "error":
        return "error reading settings file"

    if len(settings)>1:

        exists=False    
        
        for setting in settings[1:]:
            [s_name,s_desc,s_param]=setting.split(';')
            s_name = s_name.strip()
            s_desc = s_desc.strip()
            s_param = s_param.strip()
            if s_name==name:                
                exists=True
                break
        
        # if name doesnt exists in settings file than add a new setting        
        if exists==False:
            writesettings("".join(settings).strip() + '\n' + ("%s;%s;%s" %(name,desc,param)))
            return "setting added"
        else:
            return "already exists" 
            
            
"""
## update existing setting
#      url: /settings/update?name=1&desc=updated desc&param=-t 500 -ex night -awb off -ev 0 -ss 100000 -awbg 1.5,1.2 -ISO 100
#arguments: name = [name of setting], desc = [description of setting], param = [parameter of setting]
#   output: ["setting updated","error reading settings file","setting not found","%s is read-only"]
"""
@app.route('/settings/update')
def update():
    name = request.args.get('name').strip()
    desc = request.args.get('desc').strip().replace('\n','##igem#is#great##')
    param = request.args.get('param').strip()
    settings = readsettings()
    
    if settings == "error":
        return "error reading settings file"
    
    # default settings cannot be changed    
    if name in ['1','2','3','4']:
        return "%s is read-only" % name
        
    if len(settings)>1:
        updated=False    
            
        result = settings[0:1]
        for setting in settings[1:]:
            [s_name,s_desc,s_param]=setting.split(';')
            s_name = s_name.strip()
            s_desc = s_desc.strip()
            s_param = s_param.strip()
            
            # if setting with the name exists update the values 
            if s_name.strip()==name.strip():
                result.append("%s;%s;%s" %(name,desc,param+'\n'))
                updated=True
            else:                
                result.append(setting)

        if updated==True:
            writesettings("".join(result))
            return "setting updated"
        else:
            return "setting not found" 
            
             
"""
## delete setting
#      url: /settings/delete?name=1
#arguments: name = [name of setting]
#   output: ["setting deleted","error reading settings file","setting not found","%s is read-only"]
"""
@app.route('/settings/delete')
def delete():
    name = request.args.get('name')
    settings = readsettings()
    
    if settings == "error":
        return "error reading settings file"
    
    # default settings cannot be deleted
    if name in ['1','2','3']:
        return "%s is read-only" % name
            
                
    if len(settings)>1:
        exists=False    
            
        result = settings[0:1]
        for setting in settings[1:]:
            if setting.count(";")<2:
                continue
            [s_name,s_desc,s_param]=setting.split(';')
            if s_name.strip()==name.strip():
                exists=True
            else:                
                result.append(setting)

        if exists==True:
            writesettings("".join(result).strip())
            return "setting deleted"
        else:
            return "setting not found"

"""
Main page
"""
@app.route('/')
def main():
    return "Hello. Full documentation on http://2014.igem.org/Team:Aachen/Project/Measurement_Device."  
    
    
if __name__ == "__main__":
    imagethread = timelapsethread()    
    
    connected = False
    
    # search for connected arduino
    locations=[]
    for i in range(0,4):
        locations.append('/dev/ttyACM'+str(i))
        locations.append('/dev/ttyUSB'+str(i))
    
    for device in locations:
        try:
            print "Trying...",device
            ser = serial.Serial(device, 9600)
            break
        except:
            print "Failed to connect on",device

    while not connected:
        serin = ser.read()
        connected = True
        print "Connected"
     
    sleep(5)   
    
    print "ip: " + get_ipv4_address()
    
    # start flask server    
    app.run(host='0.0.0.0', port=1233, debug=True)
    
    
