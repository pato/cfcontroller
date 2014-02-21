import logging
import sys
import time 
import cflib.crtp
import threading
from cflib.crazyflie import Crazyflie

 
class Main:

    def __init__(self):


        self.crazyflie = Crazyflie()
        cflib.crtp.init_drivers()
 
        print("Trying to open link")
        self.crazyflie.open_link("radio://0/10/250K")
 
        self.crazyflie.connectSetupFinished.add_callback(self.connectSetupFinished)
        
        time.sleep(10)
    
        t1 = threading.Thread(target=self.FlyThread)
        t1.start()    
        
        print("Link is opened")       

    
    def FlyThread(self):             
        roll    = 0.0
        pitch   = 0.0
        yawrate = 0
        thrust  = 10001 # minimum thrust required to power motors
        
        print("In FlyThread")
        
        #while(True):
        for x in range(0,100):
            result = self.crazyflie.commander.send_setpoint(roll, pitch, yawrate, thrust+x)
            time.sleep(0.1)

        self.crazyflie.close_link()

    def connectSetupFinished(self, linkURI):
        # Called when connection is done
        print("Connection is finished");
        #self.FlyThread()

Main()
