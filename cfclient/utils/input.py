#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#     ||          ____  _ __                           
#  +------+      / __ )(_) /_______________ _____  ___ 
#  | 0xBC |     / __  / / __/ ___/ ___/ __ `/_  / / _ \
#  +------+    / /_/ / / /_/ /__/ /  / /_/ / / /_/  __/
#   ||  ||    /_____/_/\__/\___/_/   \__,_/ /___/\___/
#
#  Copyright (C) 2011-2013 Bitcraze AB
#
#  Crazyflie Nano Quadcopter Client
#
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License
#  as published by the Free Software Foundation; either version 2
#  of the License, or (at your option) any later version.
#  
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.

#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

"""
The input module that will read joysticks/input devices and send control set-points to
the Crazyflie. It will also accept settings from the UI.

This module can use different drivers for reading the input device data. Currently it can
just use the PyGame driver but in the future there will be a Linux and Windows driver that can
bypass PyGame.

When reading values from inputdevice a config is used to map axis and buttons to control functions
for the Crazyflie.
"""

__author__ = 'Bitcraze AB'
__all__ = ['JoystickReader']

import sys
import time
import os
import glob
import traceback
import logging
import shutil
import copy

logger = logging.getLogger(__name__)

from pygamereader import PyGameReader
from cfclient.utils.config import Config
from cfclient.utils.config_manager import ConfigManager

from PyQt4 import Qt, QtCore, QtGui, uic
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4.Qt import *

class JoystickReader(QThread):
    """Thread that will read input from devices/joysticks and send control-setponts to
    the Crazyflie"""
    PITCH_AXIS_ID  = 0
    ROLL_AXIS_ID   = 1
    YAW_AXIS_ID    = 2
    THRUST_AXIS_ID = 3

    # Incomming signal to start input capture
    startInputSignal = pyqtSignal(str, str)
    # Incomming signal to stop input capture
    stopInputSignal = pyqtSignal()
    # Incomming signal to set min/max thrust
    updateMinMaxThrustSignal = pyqtSignal(int, int)
    # Incomming signal to set roll/pitch calibration
    update_trim_roll_signal = pyqtSignal(float)
    update_trim_pitch_signal = pyqtSignal(float)
    # Incomming signal to set max roll/pitch angle
    updateMaxRPAngleSignal = pyqtSignal(int)
    # Incomming signal to set max yaw rate
    updateMaxYawRateSignal = pyqtSignal(int)
    # Incomming signal to set thrust lowering slew rate limiting
    updateThrustLoweringSlewrateSignal = pyqtSignal(int, int)

    # Configure the aixs: Axis id, joystick axis id and inverse or not
    updateAxisConfigSignal = pyqtSignal(int, int, float)
    # Start detection of variation for joystick axis
    detectAxisVarSignal = pyqtSignal()
    
    # Outgoing signal for device found
    inputUpdateSignal = pyqtSignal(float, float, float, float)
    # Outgoing signal for when pitch/roll calibration has been updated
    calUpdateSignal = pyqtSignal(float, float)
    # Outgoing signal for emergency stop
    emergencyStopSignal = pyqtSignal(bool)

    inputDeviceErrorSignal = pyqtSignal('QString')

    sendControlSetpointSignal = pyqtSignal(float, float, float, int)

    discovery_signal = pyqtSignal(object)

    inputConfig = []
    
    def __init__(self):
        QThread.__init__(self)
        #self.moveToThread(self)

        # TODO: Should be OS dependant
        self.inputdevice = PyGameReader()

        self.startInputSignal.connect(self.startInput)
        self.stopInputSignal.connect(self.stopInput)
        self.updateMinMaxThrustSignal.connect(self.updateMinMaxThrust)
        self.update_trim_roll_signal.connect(self._update_trim_roll)
        self.update_trim_pitch_signal.connect(self._update_trim_pitch)
        self.updateMaxRPAngleSignal.connect(self.updateMaxRPAngle)
        self.updateThrustLoweringSlewrateSignal.connect(self.updateThrustLoweringSlewrate)
        self.updateMaxYawRateSignal.connect(self.updateMaxYawRate)

        self.maxRPAngle = 0
        self.thrustDownSlew = 0
        self.thrustSlewEnabled = False
        self.slewEnableLimit = 0
        self.maxYawRate = 0
        self.detectAxis = False
        self.emergencyStop = False

        self.oldThrust = 0

        self._trim_roll = Config().get("trim_roll")
        self._trim_pitch = Config().get("trim_pitch")

        # TODO: The polling interval should be set from config file
        self.readTimer = QTimer()
        self.readTimer.setInterval(10);
        self.connect(self.readTimer, SIGNAL("timeout()"), self.readInput)

        self._discovery_timer = QTimer()
        self._discovery_timer.setInterval(1000);
        self.connect(self._discovery_timer, SIGNAL("timeout()"), self._do_device_discovery)
        self._discovery_timer.start()    

        self._available_devices = {}

        # Check if user config exists, otherwise copy files
        if not os.path.isdir(ConfigManager().configs_dir):
            logger.info("No user config found, copying dist files")
            os.makedirs(ConfigManager().configs_dir)
            for f in glob.glob(sys.path[0] + "/cfclient/configs/input/[A-Za-z]*.json"):
                shutil.copy2(f, ConfigManager().configs_dir)

    def _do_device_discovery(self):
        devs = self.getAvailableDevices()
        
        if len(devs):
            self.discovery_signal.emit(devs)
            self._discovery_timer.stop()

    def getAvailableDevices(self):
        """List all available input devices."""
        devs = self.inputdevice.getAvailableDevices()

        for d in devs:
            self._available_devices[d["name"]] = d["id"]

        return devs 

    def enableRawReading(self, deviceId):
        """Enable raw reading of the input device with id deviceId. This is used to
        get raw values for setting up of input devices. Values are read without using a mapping."""
        self.inputdevice.enableRawReading(deviceId)

    def disableRawReading(self):
        """Disable raw reading of input device."""
        self.inputdevice.disableRawReading()

    def readRawValues(self):
        """ Read raw values from the input device."""
        return self.inputdevice.readRawValues()

    # Fix for Ubuntu... doing self.moveToThread will not work without this
    # since it seems that the default implementation of run will not call exec_ to process
    # events.
    def run(self):
        self.exec_()

    @pyqtSlot(str, str)
    def startInput(self, device_name, config_name):
        """Start reading input from the device with name device_name using config config_name"""
        try:
            device_id = self._available_devices[device_name]
            self.inputdevice.startInput(device_id, ConfigManager().get_config(config_name))
            self.readTimer.start()
        except Exception:
            self.inputDeviceErrorSignal.emit("Error while opening/initializing input device\n\n%s" % (traceback.format_exc()))

    @pyqtSlot()
    def stopInput(self):
        """Stop reading from the input device."""
        self.readTimer.stop()

    @pyqtSlot(int)
    def updateMaxYawRate(self, maxRate):
        """Set a new max yaw rate value."""
        self.maxYawRate = maxRate

    @pyqtSlot(int)
    def updateMaxRPAngle(self, maxAngle):
        """Set a new max roll/pitch value."""
        self.maxRPAngle = maxAngle

    @pyqtSlot(int, int)
    def updateThrustLoweringSlewrate(self, thrustDownSlew, slewLimit):
        """Set new values for limit where the slewrate control kicks in and
        for the slewrate."""
        self.thrustDownSlew = thrustDownSlew
        self.slewEnableLimit = slewLimit
        if (thrustDownSlew > 0):
            self.thrustSlewEnabled = True
        else:
            self.thrustSlewEnabled = False

    def setCrazyflie(self, cf):
        """Set the referance for the Crazyflie"""
        self.cf = cf

    @pyqtSlot(int, int)
    def updateMinMaxThrust(self, minThrust, maxThrust):
        """Set a new min/max thrust limit."""
        self.minThrust = minThrust
        self.maxThrust = maxThrust

    @pyqtSlot(float)
    def _update_trim_roll(self, trim_roll):
        """Set a new value for the roll trim."""
        self._trim_roll = trim_roll

    @pyqtSlot(float)
    def _update_trim_pitch(self, trim_pitch):
        """Set a new value for the trim trim."""
        self._trim_pitch = trim_pitch

    @pyqtSlot()
    def readInput(self):
        """Read input data from the selected device"""
        try:
            data = self.inputdevice.readInput()
            roll = data["roll"] * self.maxRPAngle
            pitch = data["pitch"] * self.maxRPAngle
            thrust = data["thrust"]
            yaw = data["yaw"]
            raw_thrust = data["thrust"]
            emergency_stop = data["estop"]
            trim_roll = data["rollcal"]
            trim_pitch = data["pitchcal"]

            if self.emergencyStop != emergency_stop:
                self.emergencyStop = emergency_stop
                self.emergencyStopSignal.emit(self.emergencyStop)

            # Thust limiting (slew, minimum and emergency stop)
            if raw_thrust<0.05 or emergency_stop:
                thrust=0
            else:
                thrust = self.minThrust + thrust * (self.maxThrust - self.minThrust)
            if self.thrustSlewEnabled == True and self.slewEnableLimit > thrust and not emergency_stop:
                if self.oldThrust > self.slewEnableLimit:
                    self.oldThrust = self.slewEnableLimit
                if thrust < (self.oldThrust - (self.thrustDownSlew/100)):
                    thrust = self.oldThrust - self.thrustDownSlew/100
                if raw_thrust < 0 or thrust < self.minThrust:
                    thrust = 0
            self.oldThrust = thrust

            # Yaw deadband
            # TODO: Add to input device config?
            if yaw < -0.2 or yaw > 0.2:
                if yaw < 0:
                    yaw = (yaw + 0.2) * self.maxYawRate * 1.25
                else:
                    yaw = (yaw - 0.2) * self.maxYawRate * 1.25
            else:
                self.yaw = 0

            if trim_roll != 0 or trim_pitch != 0:
                self._trim_roll += trim_roll
                self._trim_pitch += trim_pitch
                self.calUpdateSignal.emit(self._trim_roll, self._trim_pitch)

            self.inputUpdateSignal.emit(roll, pitch, yaw, thrust)
            trimmed_rol = roll + self._trim_roll
            trimmed_pitch = pitch + self._trim_pitch
            self.sendControlSetpointSignal.emit(trimmed_rol, trimmed_pitch,
                                                yaw, thrust)
        except Exception:
            logger.warning("Exception while reading inputdevice: %s", traceback.format_exc())
            self.inputDeviceErrorSignal.emit("Error reading from input device\n\n%s"%traceback.format_exc())
            self.readTimer.stop()


