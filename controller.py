import logging
import time 
import cflib.crtp
import threading
from cflib.crazyflie import Crazyflie

 
class Main:

    def __init__(self):


        self.crazyflie = Crazyflie()
        cflib.crtp.init_drivers()
 
        print("before open link")
        self.crazyflie.open_link("radio://0/10/250K")
 
        self.crazyflie.connectSetupFinished.add_callback(self.connectSetupFinished)
        
        time.sleep(10)
    
        t1 = threading.Thread(target=self.MyThread1)
        t1.start()    
        
        print("after link is opened")       
    
    def MyThread1(self):             
        roll    = 0.0
        pitch   = 0.0
        yawrate = 0
        thrust  = 10001
        
        print("sending data")
        
        while(True):
            result = self.crazyflie.commander.send_setpoint(roll, pitch, yawrate, thrust)
            time.sleep(0.1)


    def connectSetupFinished(self, linkURI):
        """
        Called once connection is done
        """ 
Main()

