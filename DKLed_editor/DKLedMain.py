import gc

from PyQt5 import QtWidgets, uic, QtGui
from PyQt5.QtGui import QIntValidator, QRegularExpressionValidator, QCloseEvent, QBrush, QPen, QPainter, QColor, QFont, QIcon
from PyQt5.QtCore import QRegularExpression, Qt, QSize
from PyQt5.QtWidgets import QInputDialog, QGraphicsView, QGraphicsItem, QGraphicsScene
from PyQt5.QtSerialPort import QSerialPort, QSerialPortInfo
from PyQt5.QtCore import QIODevice
from enum import Enum
import sys

from ReadDKLedLib import HCDFirstParameter, HCDCompartibleFlags, MaximumNumberOf, \
    DKLed_Specs, HCD_Command_Group, HCD_Command,\
    Create_Possible_Commands_List, Create_Possible_Controller_List, Create_UI_Captions_and_Hint_Contents_list,\
    Possible_Controllers, HCD_Version_association_list, HCD_Command_List, HCD_Command_Groups, HCD_Version_list_Full, \
    UI_Captions, Hint_Contents_list, Hint_Contents_Show, Hinted_widgets_list, \
    Encoder_Color_Highlights, Fast_Animation_Entry_Stoplist, HCD_Command_Window_Dialog, Possible_Periph, \
    Create_Possible_Periph_List, OutputNames2LayoutTypes

from DKLedInterfaceNames import Interface_Names, Interface_Style, DKLed_Layout_Icons2, Interface_Icons

from CustomQWidgets import QtComboBoxUnScroll, Functional_Window, QtScrollDrawArea, Program_Windows, Close_All_Windows

from DKLedProjecLayout import DKLed_Project_Layout_Controls, DKLed_Project_Layout_Item, \
    DKLed_Project_Layout_Types, DKLed_Project_Layout_Actions

from DKLedScreenLayout import DKLed_Screen_Layout_Controls, DKLed_Screen_Layout_Actions

from DKLedControllersTab import DKLed_Controller_Controls, DKLed_Screens_Controls

class HCD_Command_Token:
    def __init__(self):
        #connectivity
        self.HCDCommand = None #command description
        self.ParentFile = None #file in which this command is used
        self.Controller = None
        self.PreviousCommand = None #first, previous and next commands in chain
        self.NextCommand = None
        self.FirstCommand = None

        #parameteers:
        self.Parameter1 = [] #list of first parameters
        self.Parameter2 = [] #list of second parameters
        self.Compiled = "" #compiled text

        #widgets:
        self.page = None
        self.layout = None
        self.label1 = None
        self.Parameter1NameLabel = None
        self.Parameter1labels = []
        self.Parameter1Widgets = []
        self.Parameter2labels = []
        self.Parameter2Widgets = []



class DKLed_Output_Instances:
    def __init__(self):
        self.Controller = None #parent Controller class
        self.ParentPage = None #parent Widget
        self.TabName = "none"
        self.index = 0
        self.LayoutObject = None  # pointer to corresponding layout object
        self.Screens = [] #pointer to connected screens or devises
        self.StartScreenIndex = 0
        self.ConnectionPoints = [] #pointer to connection circle element at the layout window (layout object)
        self.LayoutItem = None
        self.ControllerType = 0

        self.LastConnected = []
        self.LastConnectedIndex = []
        self.LastConnectedInternalIndex = []
        self.LastConnectedType = []
        self.Type = DKLed_Project_Layout_Types.OutputChannel
        #created here:
        self.QtComboWidget = None
        self.QtScreenListWidget = None
        self.ScreenListInProgress = False
        self.indexLabel = None
        self.textLabel = None
        self.layout = None
        self.ConnectableDeviceTypes = []
        self.PhysicallyConnected = False

    def Construct_Widget(self):
        self.layout = QtWidgets.QHBoxLayout(self.ParentPage)
        self.indexLabel = QtWidgets.QLabel((hex(self.index)[2:]).lower()+":", self.ParentPage, )
        self.indexLabel.setFixedSize(15, 25)
        self.QtComboWidget = QtComboBoxUnScroll(self.ParentPage)
        #self.QtComboWidget = QtWidgets.QComboBox(self.ParentPage)
        self.QtComboWidget.setFixedSize(130, 25)
        self.QtComboWidget.currentIndexChanged.connect(self.Update_Connectable_List)
        #self.QtComboWidget.setFrame(False)
        #self.QtComboWidget.
        self.textLabel = QtWidgets.QLabel(Interface_Names.ControllerOutput_ConnectToLabel, self.ParentPage, )
        self.textLabel.setFixedHeight(25)
        #self.textLabel.setStyleSheet(" text-align: right; ")
        self.QtScreenListWidget = QtComboBoxUnScroll(self.ParentPage)
        self.QtScreenListWidget.setFixedSize(150, 25)
        self.QtScreenListWidget.addItem(Interface_Names.ControllerOutput_EmptyScreen)
        self.QtScreenListWidget.currentIndexChanged.connect(self.ReRout_Output)
        self.layout.addSpacing(4)
        self.layout.addWidget(self.indexLabel)
        self.layout.addWidget(self.QtComboWidget)
        self.layout.addStretch()
        self.layout.addWidget(self.textLabel)
        self.layout.addWidget(self.QtScreenListWidget)
        self.layout.addSpacing(2)
        self.indexLabel.setVisible(True)
        self.QtComboWidget.setVisible(True)
        self.Update_Instance_List()
        DKLed_Output_Instance_List.append(self)

    def Update_Instance_List(self):
        CheckStoplist = self.Controller.FastAnimCheckbox.isChecked()
        tx = self.QtComboWidget.itemText(self.QtComboWidget.currentIndex())
        #while self.QtComboWidget.itemText(0):
            #self.QtComboWidget.removeItem(0)
        self.QtComboWidget.clear()
        if self.ControllerType != 0:
            for i in range(len(self.ControllerType.devices)):
                if not (CheckStoplist
                        and (self.ControllerType.devices[i] in Fast_Animation_Entry_Stoplist)
                        and self.ControllerType.fastvid):
                    self.QtComboWidget.addItem(self.ControllerType.devices[i])
                    if self.ControllerType.devices[i] == tx:
                        self.QtComboWidget.setCurrentIndex(i)
        del tx

    def set_visible(self, Vis=True):
        if self.indexLabel:
            self.indexLabel.setVisible(Vis)
        if self.QtComboWidget:
            self.QtComboWidget.setVisible(Vis)
        if self.QtScreenListWidget:
            self.QtScreenListWidget.setVisible(Vis)
        if self.textLabel:
            self.textLabel.setVisible(Vis)

    def Update_Connectable_List(self):
        tx = self.QtComboWidget.itemText(self.QtComboWidget.currentIndex())
        self.ConnectableDeviceTypes.clear()
        if tx:
            for ON2 in OutputNames2LayoutTypes:
                if ON2.ControlledName == tx:
                    self.ConnectableDeviceTypes.append(ON2.LayoutType)
            self.Update_Actuators_List()
            #print("connectableList updated for output "+str(self.index))
        del tx

    def Update_Device_Combobox(self, Tp):
        pass

    def Update_Actuators_List(self, Nm=""):
        self.ScreenListInProgress = True
        tx = self.QtScreenListWidget.itemText(self.QtScreenListWidget.currentIndex())
        self.QtScreenListWidget.clear()
        self.QtScreenListWidget.addItem(Interface_Names.ControllerOutput_EmptyScreen)
        ii = 0
        #print("Updating Output " + str(self.index) + " actuator list. Current choice: " + tx)
        for Sc in Actuators:
            if Sc:
                if Sc.Representation.RepresentedEntityType in self.ConnectableDeviceTypes:
                    AddTL = False
                    for Cp in Sc.ConnectionPoints:
                        if ((Cp.MultyLines or len(Cp.Connects)<1) and Cp.ConnectType == DKLed_Project_Layout_Types.InputChannel):
                            AddTL = True
                    if AddTL or (self in Sc.LastConnected) or Nm == Sc.TabName:
                        #print("Output " + str(self.index) + " adding actuator: " + Sc.TabName)
                        self.QtScreenListWidget.addItem(Sc.TabName)
                        if Sc.TabName == tx:
                            #print(", and it's the matching one")
                            ii = self.QtScreenListWidget.count()-1
        self.QtScreenListWidget.setCurrentIndex(ii)
        if ii == 0 and tx != Interface_Names.ControllerOutput_EmptyScreen:
            self.Disconnect_Output()
        #print("Output "+str(self.index)+" screen index is " + str(ii))
        self.ScreenListInProgress = False
        del tx, ii

    def Respond_to_layout_Action(self, index, Action= DKLed_Project_Layout_Actions.NoAction, Parameter= None):
        pass

    def Connect_Item(self, Obj, Ind=0, IInd=0, Direct=False):
        self.LastConnected.append(Obj)
        self.LastConnectedIndex.append(Ind)
        self.LastConnectedInternalIndex.append(IInd)
        self.LastConnectedType.append(Obj.Type)
        #print("Controller Output " + str(IInd) + " connected through layout with " + Obj.TabName + " index #" + str(Ind))
        if Direct:
            Tp = Obj.Representation.RepresentedEntityType
            tx = self.QtComboWidget.itemText(self.QtComboWidget.currentIndex())
            tx2 = ""
            Change = True
            Connect = False
            CheckStoplist = self.Controller.FastAnimCheckbox.isChecked()
            for ON2 in OutputNames2LayoutTypes:
                if ON2.LayoutType == Tp and (ON2.ControlledName in self.ControllerType.devices) \
                        and (not (CheckStoplist and (ON2.ControlledName in Fast_Animation_Entry_Stoplist) and self.ControllerType.fastvid)):
                    if tx == ON2.ControlledName:
                        Change = False
                    else:
                        tx2 = ON2.ControlledName
                    Connect = True
                    break
            if not Connect:
                print("Connection NOT allowed")
                self.Disconnect_Output()
                return False
            if tx2 and Change:
                #print("changing index")
                for ij in range(self.QtComboWidget.count()):
                    if tx2 == self.QtComboWidget.itemText(ij):
                        self.QtComboWidget.setCurrentIndex(ij)
                        break
                self.Update_Actuators_List(Obj.TabName)
            self.ScreenListInProgress = True
            for ik in range(self.QtScreenListWidget.count()):
                if Obj.TabName == self.QtScreenListWidget.itemText(ik):
                    self.QtScreenListWidget.setCurrentIndex(ik)
                    break
            self.ScreenListInProgress = False
        self.PhysicallyConnected = True
        Execute_Output_Connection_Lists_Update(True, self)
        return True

    def ReRout_Output(self):
        if self.ScreenListInProgress == False:
            print("Reconnecting output "+ str(self.index))
            self.PhysicallyConnected = False
            Ind = self.QtScreenListWidget.currentIndex()
            self.Disconnect_Output()
            if Ind > 0:
                tx = self.QtScreenListWidget.itemText(Ind)
                for Sc in Actuators:
                    if Sc:
                        if Sc.TabName == tx:
                            Cp = None
                            DoConn = False
                            for Cp in Sc.ConnectionPoints:
                                if ((Cp.MultyLines or len(Cp.Connects) < 1) and Cp.ConnectType == DKLed_Project_Layout_Types.InputChannel):
                                    DoConn = True
                                    break
                            if Cp and DoConn:
                                self.ConnectionPoints[0].Connect_to(Cp)
                                self.PhysicallyConnected = True
                            break
                self.ScreenListInProgress = True
                self.QtScreenListWidget.setCurrentIndex(Ind)
                self.ScreenListInProgress = False
            del Ind
            Execute_Output_Connection_Lists_Update(True)

    def Disconnect(self, Obj, Direct = False):# disconnect object from this one
        for i in range(len(self.LastConnected) - 1, -1, -1):
            if self.LastConnected[i] == Obj:
                self.LastConnected.pop(i)
                self.LastConnectedIndex.pop(i)
                self.LastConnectedInternalIndex.pop(i)
                self.LastConnectedType.pop(i)
        self.ScreenListInProgress = True
        self.QtScreenListWidget.setCurrentIndex(0)
        self.ScreenListInProgress = False
        Update_all_Output_Connection_Lists()

    def Disconnect_Output(self):
        self.PhysicallyConnected = False
        if len(self.ConnectionPoints):
            if self.ConnectionPoints[0]:
                if len(self.ConnectionPoints[0].Line) > 0:
                    self.ConnectionPoints[0].Line[0].Destroy()
        self.ScreenListInProgress = True
        self.QtScreenListWidget.setCurrentIndex(0)
        self.ScreenListInProgress = False
        Update_all_Output_Connection_Lists()

    def destroy_widget(self):
        self.Screens.clear()
        self.ConnectionPoints.clear()

        for i in range(len(self.LastConnected) - 1, -1, -1):
            self.LastConnected[i].Disconnect(self)

        self.LastConnectedIndex.clear()
        self.LastConnectedInternalIndex.clear()
        self.LastConnectedType.clear()
        self.LastConnected.clear()
        del self.LastConnectedIndex, self.LastConnectedType, self.LastConnected, self.LastConnectedInternalIndex

        del self.index
        del self.ConnectionPoints
        del self.Screens
        del self.StartScreenIndex

        if self in DKLed_Output_Instance_List:
            DKLed_Output_Instance_List.remove(self)

        if type(self.indexLabel) is QtWidgets:
            self.indexLabel.deleteLater()
        if type(self.QtComboWidget) is QtWidgets:
            self.QtComboWidget.deleteLater()
        if type(self.QtScreenListWidget) is QtWidgets:
            self.QtScreenListWidget.deleteLater()
        if type(self.textLabel) is QtWidgets:
            self.textLabel.deleteLater()
        if type(self.layout) is QtWidgets:
            self.layout.deleteLater()
        del self

class DKLed_Port_Instances:
    def __init__(self):

        self.Controller = None #parent Controller class
        self.ParentPage = None #parent Widget
        self.TabName = "none"
        self.LayoutObject = None  # pointer to corresponding layout object
        self.ReceiverPorts = [] #port or objects to connect (layout object)
        self.ConnectionPoints = [] #pointer to connection line at the layout window (layout object)
        self.LayoutItemRx = None
        self.LayoutItemTx = None
        self.index = 0
        self.portType = 0 #0- undef, 1- Uart, 2-SPI, 2-I2C
        self.ControllerType = None
        self.Baudrate = 115200
        self.WaitPeriod = 1000 #wait time in milliseconds

        self.LastConnected = []
        self.LastConnectedIndex = []
        self.LastConnectedInternalIndex = []
        self.LastConnectedType = []
        self.Type = DKLed_Project_Layout_Types.PortChanel
        #created here:
        self.QtComboWidgetBaud = None
        self.QtComboWidgetIn = None
        self.QtComboWidgetOut = None
        self.indexLabel = None
        self.layout = None
        self.Hlayout = None
        self.Hlayout2 = None
        self.QtComboWidgetWait = None
        self.QtCheckboxFrameSync = None

    def Construct_Widget(self):
        self.Hlayout = QtWidgets.QHBoxLayout(self.ParentPage)
        self.Hlayout2 = QtWidgets.QHBoxLayout(self.ParentPage)
        self.layout = QtWidgets.QVBoxLayout(self.ParentPage)
        self.indexLabel = QtWidgets.QLabel(" "+str(self.index)+":", self.ParentPage, )
        self.indexLabel.setFixedSize(50, 25)
        self.QtComboWidgetBaud = QtWidgets.QLineEdit(str(self.Baudrate), self.ParentPage)
        self.QtComboWidgetBaud.setFixedSize(50, 25)
        self.QtComboWidgetBaud.setValidator(IntValuesValidator)
        self.QtComboWidgetBaud.editingFinished.connect(self.check_port_parameter_values)
        self.QtComboWidgetIn = QtComboBoxUnScroll(self.ParentPage)
        self.QtComboWidgetIn.setFixedSize(100, 25)
        self.QtComboWidgetIn.currentIndexChanged.connect(self.Sync_Code_Checkbox_Enabler)
        self.QtComboWidgetOut = QtComboBoxUnScroll(self.ParentPage)
        self.QtComboWidgetOut.setFixedSize(100, 25)
        self.QtComboWidgetOut.currentIndexChanged.connect(self.Sync_Code_Checkbox_Enabler)

        self.QtComboWidgetWait = QtWidgets.QLineEdit(str(self.WaitPeriod), self.ParentPage)
        self.QtComboWidgetWait.setFixedSize(50, 25)
        self.QtComboWidgetWait.setValidator(IntValuesValidator)
        self.QtComboWidgetWait.editingFinished.connect(self.check_port_parameter_values)

        self.QtCheckboxFrameSync = QtWidgets.QCheckBox(Interface_Names.ControllerPort_FrameSyncLabel, self.ParentPage)
        self.QtCheckboxFrameSync.setFixedHeight(25)
        self.QtCheckboxFrameSync.setChecked(False)
        self.QtCheckboxFrameSync.setEnabled(False)

        self.Hlayout.addSpacing(4)
        self.Hlayout.addWidget(self.indexLabel)
        self.Hlayout.addWidget(self.QtComboWidgetBaud)
        self.Hlayout.addWidget(self.QtComboWidgetIn)
        self.Hlayout.addWidget(self.QtComboWidgetOut)
        self.Hlayout.addWidget(self.QtComboWidgetWait)
        self.Hlayout.addStretch()
        self.Hlayout.addSpacing(2)

        self.Hlayout2.addSpacing(116)
        self.Hlayout2.addWidget(self.QtCheckboxFrameSync)

        self.layout.addLayout(self.Hlayout)
        self.layout.addLayout(self.Hlayout2)

        self.indexLabel.setVisible(True)
        self.QtComboWidgetBaud.setVisible(True)
        self.QtComboWidgetIn.setVisible(True)
        self.QtComboWidgetOut.setVisible(True)
        self.QtComboWidgetWait.setVisible(True)
        self.Update_Instance_List()

    def Update_Instance_List(self):
        txIn = self.QtComboWidgetIn.itemText(self.QtComboWidgetIn.currentIndex())
        txOut = self.QtComboWidgetOut.itemText(self.QtComboWidgetOut.currentIndex())
        self.QtComboWidgetIn.clear()
        self.QtComboWidgetOut.clear()
        IIn = 0
        IOut = 0
        if self.ControllerType:
            if self.index < len(self.ControllerType.uartTypes):
                str_ = self.ControllerType.uartTypes[self.index]
                self.indexLabel.setText(str_ + " (" + str(self.index + 1) + "):")
                for i in range(len(self.ControllerType.uartModesIN[self.index])):
                    self.QtComboWidgetIn.addItem(self.ControllerType.uartModesIN[self.index][i])
                    if self.ControllerType.uartModesIN[self.index][i] == txIn:
                        IIn = i
                for i in range(len(self.ControllerType.uartModesOUT[self.index])):
                    self.QtComboWidgetOut.addItem(self.ControllerType.uartModesOUT[self.index][i])
                    if self.ControllerType.uartLines[self.index] < 2:
                        self.QtComboWidgetIn.addItem(self.ControllerType.uartModesOUT[self.index][i])
                        if self.ControllerType.uartModesOUT[self.index][i] == txIn:
                            IIn = i + len(self.ControllerType.uartModesIN[self.index])
                    if self.ControllerType.uartModesOUT[self.index][i] == txOut:
                        IOut = i
                if len(self.ControllerType.uartModesIN[self.index]) == 0:
                    self.QtComboWidgetIn.addItem(" ")
                if len(self.ControllerType.uartModesOUT[self.index]) == 0:
                    self.QtComboWidgetOut.addItem(" ")
                self.QtComboWidgetIn.setCurrentIndex(IIn)
                self.QtComboWidgetOut.setCurrentIndex(IOut)
                #del outbox
            else:
                self.QtComboWidgetIn.addItem(" ")
                self.QtComboWidgetOut.addItem(" ")
            self.check_port_parameter_values()
            self.Sync_Code_Checkbox_Enabler()
        del txIn, txOut, IIn, IOut

    def check_port_parameter_values(self):
        val = 0
        if self.QtComboWidgetBaud.text().isdigit():
            val = int(self.QtComboWidgetBaud.text())
        if val < 600:
            val = 600
        if val > 128000:
            val = 128000
        self.Baudrate = val
        self.QtComboWidgetBaud.setText(str(self.Baudrate))
        val = 0
        if self.QtComboWidgetWait.text().isdigit():
            val = int(self.QtComboWidgetWait.text())
        if val < self.ControllerType.DefaultMinUARTWaitTimeout:
            val = self.ControllerType.DefaultMinUARTWaitTimeout
        if val > self.ControllerType.DefaultMaxUARTWaitTimeout:
            val = self.ControllerType.DefaultMaxUARTWaitTimeout
        self.WaitPeriod = val
        self.QtComboWidgetWait.setText(str(self.WaitPeriod))
        del val

    def Sync_Code_Checkbox_Enabler(self):
        txIn = self.QtComboWidgetIn.itemText(self.QtComboWidgetIn.currentIndex()).upper()
        txOut = self.QtComboWidgetOut.itemText(self.QtComboWidgetOut.currentIndex()).upper()
        if 'DKLED' in txOut or (('DKLED' in txIn) and self.ControllerType.uartLines[self.index] < 2):
            self.QtCheckboxFrameSync.setEnabled(True)
        else:
            self.QtCheckboxFrameSync.setEnabled(False)

    def set_visible(self, Vis=True):
        if self.indexLabel:
            self.indexLabel.setVisible(Vis)
        if self.QtComboWidgetBaud:
            self.QtComboWidgetBaud.setVisible(Vis)
        if self.QtComboWidgetIn and self.QtComboWidgetOut:
            self.QtComboWidgetIn.setVisible(Vis)
            self.QtComboWidgetOut.setVisible(Vis)
            self.QtComboWidgetIn.setFixedSize(100, 25)
            if self.index < len(self.ControllerType.uartTypes):
                if self.ControllerType.uartLines[self.index] < 2:
                    self.QtComboWidgetOut.setVisible(False)
                    self.QtComboWidgetIn.setFixedSize(206, 25)
        if self.QtComboWidgetWait:
            self.QtComboWidgetWait.setVisible(Vis)
        if self.QtCheckboxFrameSync:
            self.QtCheckboxFrameSync.setVisible(Vis)

    def Respond_to_layout_Action(self, index, Action = DKLed_Project_Layout_Actions.NoAction, Parameter = None):
        pass

    def Connect_Item(self, Obj, Ind=0, IInd=0, Direct=False):
        self.LastConnected.append(Obj)
        self.LastConnectedIndex.append(Ind)
        self.LastConnectedInternalIndex.append(IInd)
        self.LastConnectedType.append(Obj.Type)
        return True

    def Disconnect(self, Obj, Direct = False):# disconnect object from this one
        for i in range(len(self.LastConnected) - 1, -1, -1):
            if self.LastConnected[i] == Obj:
                self.LastConnected.pop(i)
                self.LastConnectedIndex.pop(i)
                self.LastConnectedInternalIndex.pop(i)
                self.LastConnectedType.pop(i)

    # destroy widget is called when a new controller is constructed
    # and there is something in it's dedicated list of child widgets
    def destroy_widget(self):
        del self.index
        del self.WaitPeriod
        del self.Baudrate
        del self.portType
        del self.ControllerType
        self.ConnectionPoints.clear()
        for i in range(len(self.LastConnected) - 1, -1, -1):
            self.LastConnected[i].Disconnect(self)

        self.LastConnectedIndex.clear()
        self.LastConnectedInternalIndex.clear()
        self.LastConnectedType.clear()
        self.LastConnected.clear()
        del self.LastConnectedIndex, self.LastConnectedType, self.LastConnected, self.LastConnectedInternalIndex
        self.ReceiverPorts.clear()
        del self.ConnectionPoints
        del self.ReceiverPorts

        if type(self.indexLabel) is QtWidgets:
            self.indexLabel.deleteLater()
        if type(self.QtComboWidgetIn) is QtWidgets:
            self.QtComboWidgetIn.deleteLater()
        if type(self.QtComboWidgetOut) is QtWidgets:
            self.QtComboWidgetOut.deleteLater()
        if type(self.QtComboWidgetWait) is QtWidgets:
            self.QtComboWidgetWait.deleteLater()
        if type(self.QtComboWidgetBaud) is QtWidgets:
            self.QtComboWidgetBaud.deleteLater()
        if type(self.QtCheckboxFrameSync) is QtWidgets:
            self.QtCheckboxFrameSync.deleteLater()
        if type(self.Hlayout) is QtWidgets:
            self.Hlayout.deleteLater()
        if type(self.Hlayout2) is QtWidgets:
            self.Hlayout2.deleteLater()
        if type(self.layout) is QtWidgets:
            self.layout.deleteLater()
        del self

class DKLed_Button_Instances:
    def __init__(self):
        self.Controller = None #parent Controller class
        self.ParentPage = None #parent Widget
        self.TabName = "none"
        self.index = 0
        self.LayoutObject = None  # pointer to corresponding layout object
        self.Keyboard = None #pointer to connected buttons, keyboards or encoders (phisical object)
        self.LastConnected = [] #objects that have a link to this object as a connected one
        self.LastConnectedIndex = []
        self.LastConnectedInternalIndex = []
        self.LastConnectedType = []

        self.LastConnected.append(None)
        self.LastConnectedIndex.append(0)
        self.LastConnectedInternalIndex.append(0)
        self.LastConnectedType.append(DKLed_Project_Layout_Types.Nothing)
        self.KeyPin = 0 #connected pin index at keyboards or encoders (layout object)
        self.ConnectionPoints = [] #pointer to connection blobs at layout window (layout object)
        self.LayoutItem = None
        self.ControllerType =None
        self.btncombCheckbox = None
        self.btnPhysicallyConnected = False
        #created here:
        self.QtBTNWidget = None
        self.color = 0
        self.Type = DKLed_Project_Layout_Types.InputChannel

    def Construct_Widget(self):
        self.color = 0xf0f0f0
        self.QtBTNWidget = QtWidgets.QPushButton((hex(self.index)[2:]).lower(), self.ParentPage)
        self.QtBTNWidget.setStyleSheet("border-color: #909090; border-width: 2px;"
                                       "border-style: outset; border-radius: 6px;"
                                       "background-color: #" + hex(self.color)[2:])
        self.QtBTNWidget.setFixedSize(28, 25)
        self.QtBTNWidget.setVisible(True)
        self.Update_Instance_List()
        self.QtBTNWidget.clicked.connect(self.press_btn_event)

    def Update_Instance_List(self):
        val = True
        val2 = True
        color_ = 0
        if self.btncombCheckbox:
            if self.btncombCheckbox.isChecked():
                val2 = False
                if (1<<(self.index)) > self.ControllerType.totalbtn:
                    val = False
        if self.ControllerType:
            self.QtBTNWidget.setText((hex(self.index)[2:]).lower())

            if val2:
                for i in range(self.ControllerType.encd):
                    if self.Controller.Encoders[i].QtCheckWidget.isChecked():
                        if self.index - 1 == self.Controller.Encoders[i].choiceA:
                            self.QtBTNWidget.setText("CW")
                            color_ = self.Controller.Encoders[i].color
                            val = False
                        if self.index - 1 == self.Controller.Encoders[i].choiceB:
                            self.QtBTNWidget.setText("CCW")
                            color_ = self.Controller.Encoders[i].color
                            val = False
        self.QtBTNWidget.setEnabled(val)
        self.represent_if_btn_connected(not val, color_)
        del val
        del val2

    def press_btn_event(self):
        if self.btnPhysicallyConnected:
            self.Disconnect_Button()
        else:
            self.btnPhysicallyConnected = False
            Obj, i = Connect_Button_Dialog()
            self.connect_button(Obj, i)
        self.Update_Instance_List()

    def connect_button(self, Obj, i):
        if Obj:
            if i < len(Obj.ConnectionPoints):
                self.btnPhysicallyConnected = True
                self.ConnectionPoints[0].Connect_to(Obj.ConnectionPoints[i])
            else:
                print("index of connection point for object to connect to controller input is out of range")

    def Disconnect_Button(self):
        self.btnPhysicallyConnected = False
        if len(self.ConnectionPoints):
            if self.ConnectionPoints[0]:
                if len(self.ConnectionPoints[0].Line) > 0:
                    self.ConnectionPoints[0].Line[0].Destroy()

    def represent_if_btn_connected(self,Encd=False,color_ = 0):
        val2 = self.btnPhysicallyConnected or Encd
        if color_==0:
            color_=self.color
        if self.btncombCheckbox:
            if self.btncombCheckbox.isChecked():
                if (1<<(self.index)) > self.ControllerType.totalbtn:
                    val2 = False
        if val2:
            self.QtBTNWidget.setStyleSheet("border-color: #404040; border-width: 2px;"
                                           "border-style: outset; border-radius: 6px;"
                                           "background-color: #" + hex(color_)[2:])
        else:
            self.QtBTNWidget.setStyleSheet("border-color: #909090; border-width: 2px;"
                                           "border-style: outset; border-radius: 6px;"
                                           "background-color: #" + hex(color_)[2:])
        del val2

    def set_visible(self, Vis=True):
        if self.QtBTNWidget:
            self.QtBTNWidget.setVisible(Vis)

    def Respond_to_layout_Action(self, index, Action = DKLed_Project_Layout_Actions.NoAction, Parameter = None):
        pass

    def Connect_Item(self, Obj, Ind=0, IInd=0, Direct=False):
        self.btnPhysicallyConnected = True
        self.represent_if_btn_connected()
        self.LastConnected[0] = Obj
        self.LastConnectedIndex[0] = Ind
        self.LastConnectedInternalIndex[0] = IInd
        self.LastConnectedType[0] = Obj.Type
        if isinstance(Obj, Encoder_Phisical) and Direct:
            if (Ind > 0) and (Ind < 3):
                inList = False
                firstFreeEnc = self.ControllerType.encd
                chosenEnc = 0

                for i in range(self.ControllerType.encd-1,-1,-1):
                    if self.Controller.Encoders[i].QtCheckWidget.isChecked() == False:
                        firstFreeEnc = i
                    if self.Controller.Encoders[i].Connected_Encoder_Keyboard == Obj and self.Controller.Encoders[i].QtCheckWidget.isChecked():
                        inList = True
                        chosenEnc = i
                        break
                if not inList:
                    if firstFreeEnc < self.ControllerType.encd and (not self.btncombCheckbox.isChecked()):
                        #print("Adding btn "+str(self.index - 1) + " to encoder#"+str(firstFreeEnc))
                        self.Controller.Encoders[firstFreeEnc].choiceA = self.index - 1
                        self.Controller.Encoders[firstFreeEnc].choiceB = self.index - 1
                        self.Controller.Encoders[firstFreeEnc].Connected_Encoder_Keyboard = Obj
                        self.Controller.Encoders[firstFreeEnc].QtCheckWidget.setCheckState(Qt.Checked)
                    else:
                        #print("no free encoder slots")
                        self.Disconnect_Button()
                        return False
                else:
                    #print("updating encoder "+ str(chosenEnc)+ " output connections")
                    if Ind == 1:
                        self.Controller.Encoders[chosenEnc].choiceA = self.index - 1
                    else:
                        self.Controller.Encoders[chosenEnc].choiceB = self.index - 1
                    self.Controller.Encoders[firstFreeEnc].Update_Instance_List()
        return True

    def Disconnect(self, Obj, Direct = False):# disconnect object from this one
        #print("Input #"+(hex(self.index)[2:]).lower() + " disconnected")
        self.btnPhysicallyConnected = False
        self.represent_if_btn_connected()
        if self.LastConnected[0] == Obj:
            if isinstance(Obj, Encoder_Phisical) and Direct:
                if (self.LastConnectedIndex[0] > 0) and (self.LastConnectedIndex[0] < 3):
                    for l_ in Obj.ConnectionPoints[3-self.LastConnectedIndex[0]].Line:
                        if l_:
                            l_.Destroy()
                    for i in range(self.ControllerType.encd):
                        if self.Controller.Encoders[i].Connected_Encoder_Keyboard == Obj:
                            self.Controller.Encoders[i].QtCheckWidget.setCheckState(Qt.Unchecked)
                            break
                    #print("Encoder #" + str(i))
                #print("button #" + str(self.index) + " disconnected from " + str(Obj.TabName) + ", connect " + str(self.LastConnectedIndex[0]))
            self.LastConnected[0] = None
            self.LastConnectedIndex[0] = 0
            self.LastConnectedInternalIndex[0] = self.index
            self.LastConnectedType[0] = DKLed_Project_Layout_Types.Nothing

    def destroy_widget(self):
        self.ConnectionPoints.clear()

        for i in range(len(self.LastConnected)-1, -1, -1):
            self.LastConnected[i].Disconnect(self)

        self.LastConnectedIndex.clear()
        self.LastConnectedInternalIndex.clear()
        self.LastConnectedType.clear()
        self.LastConnected.clear()
        del self.LastConnectedIndex, self.LastConnectedType, self.LastConnected, self.LastConnectedInternalIndex
        del self.index
        del self.ConnectionPoints
        del self.KeyPin

        if type(self.QtBTNWidget) is QtWidgets:
            self.QtBTNWidget.deleteLater()
        del self

class DKLed_Encoder_Instances:
    def __init__(self):
        self.Controller = None  # parent Controller class
        self.ParentPage = None  # parent Widget
        self.TabName = "none"
        self.index = 0
        self.LayoutObject = None #pointer to corresponding layout object
        self.buttons = []  # pointer to connected buttons (layout object)
        self.pins = [] # pins within this controller (layout object)
        self.ConnectionPoints = []  # pointer to connection line at the layout window (layout object)
        self.ControllerType = None
        self.btncombCheckbox = None

        self.LastConnected = []
        self.LastConnectedIndex = []
        self.LastConnectedInternalIndex = []
        self.LastConnectedType = []
        self.Connected_Encoder_Keyboard = None
        self.Type = DKLed_Project_Layout_Types.Encoder
        # created here:
        # A and B are corresponding button pins in the connected Controller
        self.QtComboWidgetA = None
        self.QtComboWidgetB = None
        self.QtCheckWidget = None
        self.indexLabel = None
        self.btnChoiceLabel = None
        self.layout = None
        self.choiceA = 0 #current choices
        self.choiceB = 0
        self.color = 0
        self.preChoiceA = 0 #inital choices
        self.preChoiceB = 0

    def Construct_Widget(self):
        #global MaximumNumberOf_Buttons
        #global Encoder_Color_Highlights
        self.layout = QtWidgets.QHBoxLayout(self.ParentPage)
        self.indexLabel = QtWidgets.QLabel("  Encoder "+str(self.index)+":", self.ParentPage, )
        self.indexLabel.setFixedSize(64, 25)
        if len(Encoder_Color_Highlights) > self.index:
            self.color = Encoder_Color_Highlights[self.index]
        else:
            self.color = 0xF0F0F0
        self.indexLabel.setStyleSheet("background-color: #" + hex(self.color)[2:])
        self.QtCheckWidget = QtWidgets.QCheckBox("connect to inputs CW", self.ParentPage)
        self.QtCheckWidget.setChecked(False)
        self.QtCheckWidget.setFixedHeight(25)
        self.btnChoiceLabel = QtWidgets.QLabel(", CCW", self.ParentPage, )
        self.btnChoiceLabel.setFixedHeight(25)
        self.QtComboWidgetA = QtComboBoxUnScroll(self.ParentPage)
        self.QtComboWidgetA.setFixedSize(32, 25)
        self.QtComboWidgetA.setMaxVisibleItems(MaximumNumberOf.Buttons)
        self.QtComboWidgetA.addItem("1")
        #self.QtComboWidgetA.addItem("2")
        #self.QtComboWidgetA.setCurrentIndex(0)
        self.QtComboWidgetB = QtComboBoxUnScroll(self.ParentPage)
        self.QtComboWidgetB.setFixedSize(32, 25)
        self.QtComboWidgetB.setMaxVisibleItems(MaximumNumberOf.Buttons)
        self.QtComboWidgetB.addItem("1")
        #self.QtComboWidgetA.addItem("2")
        #self.QtComboWidgetB.setCurrentIndex(1)
        self.choiceA = 0
        self.choiceB = 1

        self.layout.addSpacing(4)
        self.layout.addWidget(self.indexLabel)
        self.layout.addWidget(self.QtCheckWidget)
        self.layout.addWidget(self.QtComboWidgetA)
        self.layout.addWidget(self.btnChoiceLabel)
        self.layout.addWidget(self.QtComboWidgetB)
        self.layout.addStretch()
        self.layout.addSpacing(2)
        self.indexLabel.setVisible(True)
        self.QtCheckWidget.setVisible(True)
        self.btnChoiceLabel.setVisible(True)
        self.QtComboWidgetA.setVisible(True)
        self.QtComboWidgetB.setVisible(True)
        self.Update_Instance_List()

        self.QtCheckWidget.stateChanged.connect(self.Update_Instance_List)
        self.QtCheckWidget.stateChanged.connect(self.avoid_collisions)

        self.QtComboWidgetA.currentIndexChanged.connect(self.avoid_collsionsA)
        self.QtComboWidgetB.currentIndexChanged.connect(self.avoid_collsionsB)

    def Update_Instance_List(self):
        #self.choiceA = self.QtComboWidgetA.currentIndex()
        #self.choiceB = self.QtComboWidgetB.currentIndex()
        #while self.QtComboWidgetA.itemText(0):
        #    self.QtComboWidgetA.removeItem(0)
        #while self.QtComboWidgetB.itemText(0):
        #    self.QtComboWidgetB.removeItem(0)
        val = True
        if self.btncombCheckbox:
            val = not (self.btncombCheckbox.isChecked())
        self.QtCheckWidget.setEnabled(val)
        self.btnChoiceLabel.setEnabled(val)
        val = val & self.QtCheckWidget.isChecked()
        if val:
            #print("Encoder "+str(self.index)+" checked")
            if self.Connected_Encoder_Keyboard == None:
                self.Connected_Encoder_Keyboard, i = Connect_Encoder_Dialog()
            if self.Connected_Encoder_Keyboard == None:
                val = False
                self.QtCheckWidget.setCheckState(Qt.Unchecked)
        else:
            #print("Encoder "+str(self.index)+" unchecked")
            self.Reconstruct_encoder_connections()
            self.Connected_Encoder_Keyboard = None
        self.btncombCheckbox.setDisabled(False)
        self.QtComboWidgetA.setEnabled(val)
        self.QtComboWidgetB.setEnabled(val)
        #print(self.Connected_Encoder_Keyboard)
        if self.ControllerType:
            for o in self.Controller.Encoders:
                if o:
                    if o.index < self.ControllerType.encd:
                        if o.QtCheckWidget.isChecked():
                            self.btncombCheckbox.setDisabled(True)
            if (self.QtComboWidgetA.count() < self.ControllerType.btn):
                for i in range(self.QtComboWidgetA.count(), self.ControllerType.btn):
                    self.QtComboWidgetA.addItem((hex(i + 1)[2:]).lower())
                    self.QtComboWidgetB.addItem((hex(i + 1)[2:]).lower())
            elif (self.QtComboWidgetA.count() > self.ControllerType.btn):
                while (self.QtComboWidgetA.count() > self.ControllerType.btn):
                    self.QtComboWidgetA.removeItem(self.QtComboWidgetA.count() - 1)
                    self.QtComboWidgetB.removeItem(self.QtComboWidgetB.count() - 1)
            if self.choiceB < self.ControllerType.btn:
                self.QtComboWidgetB.setCurrentIndex(self.choiceB)
            else:
                self.QtComboWidgetB.setCurrentIndex(self.ControllerType.btn - 1)
                #self.avoid_collsionsB()
            if self.choiceA < self.ControllerType.btn:
                self.QtComboWidgetA.setCurrentIndex(self.choiceA)
            else:
                self.QtComboWidgetA.setCurrentIndex(self.ControllerType.btn - 1)
            if self.choiceB == self.choiceA:
                self.avoid_collsionsB()
        else:
            self.Reconstruct_encoder_connections()
        del val

    def set_visible(self, Vis=True):
            if self.indexLabel:
                self.indexLabel.setVisible(Vis)
            if self.QtCheckWidget:
                self.QtCheckWidget.setVisible(Vis)
            if self.btnChoiceLabel:
                self.btnChoiceLabel.setVisible(Vis)
            if self.QtComboWidgetA:
                self.QtComboWidgetA.setVisible(Vis)
            if self.QtComboWidgetB:
                self.QtComboWidgetB.setVisible(Vis)

    def Respond_to_layout_Action(self, index, Action = DKLed_Project_Layout_Actions.NoAction, Parameter = None):
        pass

    def Reconstruct_encoder_connections(self, Re = False):
        if self.Connected_Encoder_Keyboard:
            if Re:
                pass
            else:
                #print ("line 1 to destroy")
                if len(self.Connected_Encoder_Keyboard.ConnectionPoints[1].Line):
                    if self.Connected_Encoder_Keyboard.ConnectionPoints[1].Line[0]:
                        self.Connected_Encoder_Keyboard.ConnectionPoints[1].Line[0].Destroy()
                #print("line 1 destroyed")
                if len(self.Connected_Encoder_Keyboard.ConnectionPoints[2].Line):
                    if self.Connected_Encoder_Keyboard.ConnectionPoints[2].Line[0]:
                        self.Connected_Encoder_Keyboard.ConnectionPoints[2].Line[0].Destroy()
                #print("line 2 destroyed")

    # Clockwise
    def avoid_collsionsA(self):
        if self.Connected_Encoder_Keyboard:
            if self.Connected_Encoder_Keyboard.ConnectionPoints[1]:
                if len(self.Connected_Encoder_Keyboard.ConnectionPoints[1].Line):
                    if self.Connected_Encoder_Keyboard.ConnectionPoints[1].Line[0]:
                        self.Connected_Encoder_Keyboard.ConnectionPoints[1].Line[0].Destroy()
        self.choiceA = self.QtComboWidgetA.currentIndex()
        lst = []
        for i in range(self.ControllerType.encd):
            if not (self.Controller.Encoders[i] is self):
                if (self.Controller.Encoders[i].QtCheckWidget.isChecked()):
                    lst.append(self.Controller.Encoders[i].choiceA)
                    lst.append(self.Controller.Encoders[i].choiceB)
        lst.append(self.choiceB)
        while (self.choiceA in lst) and (self.choiceA > 0):
            self.choiceA -= 1
        while (self.choiceA in lst) and ((self.QtComboWidgetA.count() - 1) > self.choiceA):
            self.choiceA += 1
        if self.choiceA != self.QtComboWidgetA.currentIndex():
            self.QtComboWidgetA.setCurrentIndex(self.choiceA)
        lst.append(self.choiceA)
        for i in range(self.ControllerType.btn):
            if self.Controller.Inputs[i].index - 1 in lst:
                if self.Controller.Inputs[i].index - 1 == self.choiceA:
                    self.Controller.Inputs[i].QtBTNWidget.setText("CW")
                    self.Controller.Inputs[i].Disconnect_Button()
                    self.Controller.Inputs[i].connect_button(self.Connected_Encoder_Keyboard, 1)
                    self.Controller.Inputs[i].represent_if_btn_connected(True, self.color)

                self.Controller.Inputs[i].QtBTNWidget.setEnabled(False)
            else:
                self.Controller.Inputs[i].QtBTNWidget.setText((hex(i + 1)[2:]).lower())
                self.Controller.Inputs[i].represent_if_btn_connected(False)
                self.Controller.Inputs[i].QtBTNWidget.setEnabled(True)
        del lst

    # Counterclockwise
    def avoid_collsionsB(self):
        if self.Connected_Encoder_Keyboard:
            if self.Connected_Encoder_Keyboard.ConnectionPoints[2]:
                if len(self.Connected_Encoder_Keyboard.ConnectionPoints[2].Line):
                    if self.Connected_Encoder_Keyboard.ConnectionPoints[2].Line[0]:
                        self.Connected_Encoder_Keyboard.ConnectionPoints[2].Line[0].Destroy()
        self.choiceB = self.QtComboWidgetB.currentIndex()
        lst = []
        for i in range(self.ControllerType.encd):
            if not (self.Controller.Encoders[i] is self):
                if (self.Controller.Encoders[i].QtCheckWidget.isChecked()):
                    lst.append(self.Controller.Encoders[i].choiceA)
                    lst.append(self.Controller.Encoders[i].choiceB)
        lst.append(self.choiceA)
        while (self.choiceB in lst) and ((self.QtComboWidgetB.count() - 1) > self.choiceB):
            self.choiceB += 1
        while (self.choiceB in lst) and (self.choiceB > 0):
            self.choiceB -= 1
        if self.choiceB != self.QtComboWidgetB.currentIndex():
            self.QtComboWidgetB.setCurrentIndex(self.choiceB)
        lst.append(self.choiceB)
        for i in range(self.ControllerType.btn):
            if self.Controller.Inputs[i].index - 1 in lst:
                if self.Controller.Inputs[i].index - 1 == self.choiceB:
                    self.Controller.Inputs[i].Disconnect_Button()
                    self.Controller.Inputs[i].connect_button(self.Connected_Encoder_Keyboard, 2)
                    self.Controller.Inputs[i].represent_if_btn_connected(True, self.color)
                    self.Controller.Inputs[i].QtBTNWidget.setText("CCW")
                self.Controller.Inputs[i].QtBTNWidget.setEnabled(False)
            else:
                self.Controller.Inputs[i].QtBTNWidget.setText((hex(i + 1)[2:]).lower())
                self.Controller.Inputs[i].represent_if_btn_connected(False)
                self.Controller.Inputs[i].QtBTNWidget.setEnabled(True)
        del lst

    def avoid_collisions(self):
        if self.QtCheckWidget.isChecked():
            self.avoid_collsionsA()
            self.avoid_collsionsB()
        else:
            for i in range(self.ControllerType.btn):
                if self.Controller.Inputs[i].index - 1 in [self.choiceA, self.choiceB]:
                    self.Controller.Inputs[i].QtBTNWidget.setText((hex(i+1)[2:]).lower())
                    self.Controller.Inputs[i].represent_if_btn_connected(False)
                    self.Controller.Inputs[i].QtBTNWidget.setEnabled(True)

    def Connect_Item(self, Obj, Ind=0, IInd=0, Direct=False):
        self.LastConnected.append(Obj)
        self.LastConnectedIndex.append(Ind)
        self.LastConnectedInternalIndex.append(IInd)
        self.LastConnectedType.append(Obj.Type)
        return True

    def Disconnect(self, Obj, Direct = False):# disconnect object from this one
        for i in range(len(self.LastConnected) - 1, -1, -1):
            if self.LastConnected[i] == Obj:
                self.LastConnected.pop(i)
                self.LastConnectedIndex.pop(i)
                self.LastConnectedInternalIndex.pop(i)
                self.LastConnectedType.pop(i)

    def destroy_widget(self):
        del self.index
        del self.ControllerType
        self.ConnectionPoints.clear()
        self.buttons.clear()
        self.pins.clear()

        for i in range(len(self.LastConnected) - 1, -1, -1):
            self.LastConnected[i].Disconnect(self)

        self.LastConnectedIndex.clear()
        self.LastConnectedInternalIndex.clear()
        self.LastConnectedType.clear()
        self.LastConnected.clear()
        del self.LastConnectedIndex, self.LastConnectedType, self.LastConnected, self.LastConnectedInternalIndex

        del self.ConnectionPoints
        del self.buttons
        del self.pins

        if type(self.indexLabel) is QtWidgets:
            self.indexLabel.deleteLater()
        if type(self.QtComboWidgetA) is QtWidgets:
            self.QtComboWidgetA.deleteLater()
        if type(self.QtComboWidgetB) is QtWidgets:
            self.QtComboWidgetB.deleteLater()
        if type(self.QtCheckWidget) is QtWidgets:
            self.QtCheckWidget.deleteLater()
        if type(self.btnChoiceLabel) is QtWidgets:
            self.btnChoiceLabel.deleteLater()
        if type(self.layout) is QtWidgets:
            self.layout.deleteLater()
        del self



class Controller:
    def __init__(self):

        self.ID = "****"
        self.index = 0
        self.Connected = False

        self.Inputs = [] #list of inputs class instances (DKLed_Button_Instances)
        self.Encoders = [] #list of connected DKLed_Encoder_Instances
        self.Outputs = [] #list of DKLed_Output_Instances
        self.OutputCount = 0 #just in case
        self.Ports = [] #list of DKLed_Port_Instances
        self.I2cAddress = 0
        self.Scenarios = []
        self.ButtonMode = 0  # button mode: 0-standard, 1-combination
        self.CurrentHCDVersionIndex = 0 #index of current HCD version index in HCD_Version_list_Full array
        self.CurrentWorkingFlags = HCDCompartibleFlags.InEveryMode #modes of commands this controller is in
        #parameters
        self.PWMDiscreet = 512 #how many steps for PWM discretization
        self.PWMFrequency = 50
        self.PWMMinImpulse = 500
        self.PWMMaxImpulse = 2500
        self.MaxBrightness = 32
        self.RelativeSpeed = 100
        self.AbsoluteSpeed = 100
        self.CurrentBrightness = 32
        self.USBWaitTimeout = 10
        self.ButtonRepeat = False
        self.ButtonCombination = False
        self.ButtonRepeatTimeout = 10000
        self.ButtonCombinationTimeout = 500
        self.FastAnimationCheckbox = False
        self.RealtimeServoCheckboxState = False
        #self.UARTBaudrate = [] #stored in DKLed_Port_Instances
        #self.LayoutObject = None  # pointer to corresponding layout object
        #extra windows:
        self.TerminalWindow = None
        self.ControllerFilesExplorerWindow = None
        self.OptionalWindow = None
        #interface Widgets:
        self.ParentTabWidget = None
        self.ParentTabLayout = None
        self.Page = None
        self.controllerTypeSelect = None
        self.BrightnessLabel = None
        self.BrightnessValue = None
        self.FastAnimCheckbox = None
        self.GotoWindowButton = None
        self.RealtimeServoCheckbox = None
        self.ServoStepsSettingLabel = None
        self.ServoStepsSettingValue = None
        self.ServoFrequencySettingLabel = None
        self.ServoFrequencySettingValue = None
        self.ServoImpulseLengthLabel1 = None
        self.ServoImpulseLengthValue1 = None
        self.ServoImpulseLengthLabel2 = None
        self.ServoImpulseLengthValue2 = None
        self.ButtonRepeatLabel = None
        self.ButtonRepeatValue = None
        self.BtnCombinationDurationLabel = None
        self.BtnCombinationDurationValue = None
        self.idBox = None
        self.controllerCodeVersionLabel = None
        self.COMchoice = None
        self.IOconf = None
        self.IOconfigs = None
        self.IOconfigs_Layout = None
        self.IO_Output_header = None
        self.IO_Input_header = None
        self.IO_Input_header_comb = None
        self.USBLabel = None
        self.USBWaitLabel = None
        self.USBWaitValue = None
        #Project layout widgets:
        self.Representation = None
        self.GotoWindowButton2 = None
        self.ConnectionPoints=[]

        self.LastConnected = []
        self.LastConnectedIndex = []
        self.LastConnectedInternalIndex = []
        self.LastConnectedType = []
        self.Type = DKLed_Project_Layout_Types.LEDController

    def add_tab(self, Tab, name_="none"):
        self.TabName = name_
        self.Connected = False
        self.Page = QtWidgets.QWidget(None, )
        self.ParentTabLayout = Tab
        self.ParentTabWidget = Tab.ControllerTab #ui.tabWidget_2
        self.index = self.ParentTabWidget.addTab(self.Page, self.TabName)

    def change_tab_name(self, Newname):
        self.TabName = Newname
        self.ParentTabWidget.setTabText(self.index, self.TabName)

    def update_fast_animation_flag(self):
        self.FastAnimationCheckbox = False
        self.RealtimeServoCheckboxState = False
        if self.FastAnimCheckbox:
            self.FastAnimationCheckbox = self.FastAnimCheckbox.isChecked()
            if self.FastAnimCheckbox.isChecked():
                #controller in fast animation mode
                self.CurrentWorkingFlags |= HCDCompartibleFlags.InFastVideomode | HCDCompartibleFlags.InLedMode
                self.CurrentWorkingFlags &= ~HCDCompartibleFlags.InServoMode
            else:
                #Led or Hybrid mode
                self.CurrentWorkingFlags |= HCDCompartibleFlags.InServoMode | HCDCompartibleFlags.InLedMode
                self.CurrentWorkingFlags &= ~HCDCompartibleFlags.InFastVideomode
        if self.RealtimeServoCheckbox:
            self.RealtimeServoCheckboxState = self.RealtimeServoCheckbox.isChecked()
            if self.RealtimeServoCheckboxState:
                self.RealtimeServoCheckbox.setText(Interface_Names.Controller_RealtimeServoCheckbox_True)#+"Servo realtime mode (currently Realtime)"
            else:
                self.RealtimeServoCheckbox.setText(Interface_Names.Controller_RealtimeServoCheckbox_False)#+"Servo realtime mode (currently Framebased)")
        self.update_servo_parameter_interactability()
        #print("Controller " + self.TabName + " in mode: " + hex(self.CurrentWorkingFlags))

    def update_servo_parameter_interactability(self):
        self.CurrentWorkingFlags &= ~HCDCompartibleFlags.InServoMode
        self.CurrentWorkingFlags &= ~HCDCompartibleFlags.InRealtimeServoMode
        ServoInteractable = False
        for i in range(MaximumNumberOf.Outputs):
            tx = self.Outputs[i].QtComboWidget.itemText(self.Outputs[i].QtComboWidget.currentIndex())
            if tx in Fast_Animation_Entry_Stoplist:
                ServoInteractable = True
                self.CurrentWorkingFlags |= HCDCompartibleFlags.InServoMode
        if self.RealtimeServoCheckbox:
            self.RealtimeServoCheckbox.setEnabled(ServoInteractable)
            if self.RealtimeServoCheckboxState:
                self.CurrentWorkingFlags |= HCDCompartibleFlags.InRealtimeServoMode
        if self.ServoStepsSettingValue:
            self.ServoStepsSettingValue.setEnabled(ServoInteractable)
        if self.ServoStepsSettingLabel:
            self.ServoStepsSettingLabel.setEnabled(ServoInteractable)
        if self.ServoFrequencySettingValue:
            self.ServoFrequencySettingValue.setEnabled(ServoInteractable)
        if self.ServoFrequencySettingLabel:
            self.ServoFrequencySettingLabel.setEnabled(ServoInteractable)
        if self.ServoImpulseLengthValue1:
            self.ServoImpulseLengthValue1.setEnabled(ServoInteractable)
        if self.ServoImpulseLengthValue2:
            self.ServoImpulseLengthValue2.setEnabled(ServoInteractable)
        if self.ServoImpulseLengthLabel1:
            self.ServoImpulseLengthLabel1.setEnabled(ServoInteractable)
        if self.ServoImpulseLengthLabel2:
            self.ServoImpulseLengthLabel2.setEnabled(ServoInteractable)
        del ServoInteractable
        # print("Controller " + self.TabName + " in mode: " + hex(self.CurrentWorkingFlags))

    def check_controller_parameter_values(self):
        val = 0
        if self.ServoStepsSettingValue:
            if self.ServoStepsSettingValue.text().isdigit():
                val = int(self.ServoStepsSettingValue.text())
            if val < Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].DefaultMinPWMDiscreet:
                val = Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].DefaultMinPWMDiscreet
            if val > Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].DefaultMaxPWMDiscreet:
                val = Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].DefaultMaxPWMDiscreet
            self.PWMDiscreet = val
            self.ServoStepsSettingValue.setText(str(self.PWMDiscreet))
        if self.USBWaitValue:
            val = 0
            if self.USBWaitValue.text().isdigit():
                val = int(self.USBWaitValue.text())
            if val < Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].DefaultMinUSBWaitTimeout:
                val = Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].DefaultMinUSBWaitTimeout
            if val > Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].DefaultMaxUSBWaitTimeout:
                val = Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].DefaultMaxUSBWaitTimeout
            self.USBWaitTimeout = val
            self.USBWaitValue.setText(str(self.USBWaitTimeout))
        if self.BrightnessValue:
            val = 0
            if self.BrightnessValue.text().isdigit():
                val = int(self.BrightnessValue.text())
            if val < Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].DefaultMinBrightness:
                val = Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].DefaultMinBrightness
            if val > Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].DefaultMaxBrightness:
                val = Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].DefaultMaxBrightness
            self.CurrentBrightness = val
            self.BrightnessValue.setText(str(self.CurrentBrightness))
        if self.ServoFrequencySettingValue:
            val = 0
            if self.ServoFrequencySettingValue.text().isdigit():
                val = int(self.ServoFrequencySettingValue.text())
            if val < Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].DefaultMinPWMFreq:
                val = Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].DefaultMinPWMFreq
            if val > Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].DefaultMaxPWMFreq:
                val = Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].DefaultMaxPWMFreq
            self.PWMFrequency = val
            self.ServoFrequencySettingValue.setText(str(self.PWMFrequency))
        if self.ServoImpulseLengthValue1 and self.ServoImpulseLengthValue2:
            val = 0
            if self.ServoImpulseLengthValue1.text().isdigit():
                val = int(self.ServoImpulseLengthValue1.text())
            if val < Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].DefaultMinPWMPosition:
                val = Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].DefaultMinPWMPosition
            if val > Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].DefaultMaxPWMPosition:
                val = Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].DefaultMaxPWMPosition
            self.PWMMinImpulse = val
            if self.ServoImpulseLengthValue2.text().isdigit():
                val = int(self.ServoImpulseLengthValue2.text())
            if val < Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].DefaultMinPWMPosition:
                val = Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].DefaultMinPWMPosition
            if val > Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].DefaultMaxPWMPosition:
                val = Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].DefaultMaxPWMPosition
            self.PWMMaxImpulse = val
            if self.PWMMinImpulse > self.PWMMaxImpulse:
                val = self.PWMMinImpulse
                self.PWMMinImpulse = self.PWMMaxImpulse
                self.PWMMaxImpulse = val
            self.ServoImpulseLengthValue1.setText(str(self.PWMMinImpulse))
            self.ServoImpulseLengthValue2.setText(str(self.PWMMaxImpulse))
        if self.ButtonRepeatValue:
            val = 0
            if self.ButtonRepeatValue.text().isdigit():
                val = int(self.ButtonRepeatValue.text())
            if val < Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].DefaultMinButtonSetWaitTimeout:
                val = Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].DefaultMinButtonSetWaitTimeout
            if val > Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].DefaultMaxButtonSetWaitTimeout:
                val = Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].DefaultMaxButtonSetWaitTimeout
            self.ButtonRepeatTimeout = val
            self.ButtonRepeatValue.setText(str(self.ButtonRepeatTimeout))
        if self.BtnCombinationDurationValue:
            val = 0
            if self.BtnCombinationDurationValue.text().isdigit():
                val = int(self.BtnCombinationDurationValue.text())
            if val < Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].DefaultMinButtonSetWaitTimeout:
                val = Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].DefaultMinButtonSetWaitTimeout
            if val > Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].DefaultMaxButtonSetWaitTimeout:
                val = Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].DefaultMaxButtonSetWaitTimeout
            self.ButtonCombinationTimeout = val
            self.BtnCombinationDurationValue.setText(str(self.ButtonCombinationTimeout))
        del val

    def update_fast_combination_flag(self):
        if self.IO_Input_header_comb:
            self.ButtonCombination = self.IO_Input_header_comb.isChecked()
            if self.ButtonCombination:
                # combination of buttons
                self.CurrentWorkingFlags |= HCDCompartibleFlags.InCombinationInputMode
                self.CurrentWorkingFlags &= ~HCDCompartibleFlags.InSingleInputMode
            else:
                # single input
                self.CurrentWorkingFlags &= ~HCDCompartibleFlags.InCombinationInputMode
                self.CurrentWorkingFlags |= HCDCompartibleFlags.InSingleInputMode
            #print("Controller " + self.TabName + " in mode: " + hex(self.CurrentWorkingFlags))
            self.BtnCombinationDurationLabel.setEnabled(self.ButtonCombination)
            self.BtnCombinationDurationValue.setEnabled(self.ButtonCombination)
        if self.ButtonRepeatLabel:
            self.ButtonRepeat = self.ButtonRepeatLabel.isChecked()
            if self.ButtonRepeatValue:
                self.ButtonRepeatValue.setEnabled(self.ButtonRepeat)

    def Return_from_Window(self):
        self.GotoWindowButton.setText(Interface_Names.Controller_GotoWindowButton_ToWindow)
        self.index = self.ParentTabWidget.insertTab(self.index, self.Page, self.TabName)
        self.ParentTabWidget.setCurrentIndex(self.index)
        self.Page.show()
        return True

    def Move_to_Window(self):
        if self.GotoWindowButton.text() == Interface_Names.Controller_GotoWindowButton_ToWindow:
            self.GotoWindowButton.setText(Interface_Names.Controller_GotoWindowButton_ToTab)
            if self.OptionalWindow is None:
                self.OptionalWindow = Functional_Window()
                #self.OptionalWindow.PrepareLayout()
            self.OptionalWindow.setWindowTitle(Interface_Names.Controller_OptionalWindow + self.TabName)
            self.OptionalWindow.setFixedWidth(self.ParentTabLayout.Style.ControllerConfigAreaWidth + self.ParentTabLayout.Style.WindowFromTabExtraWidth)
            self.OptionalWindow.setMinimumHeight(self.ParentTabLayout.Style.ControllerConfigAreaHeight + self.ParentTabLayout.Style.WindowFromTabExtraHeight)
            self.OptionalWindow.ParentAction = self.Return_from_Window
            self.OptionalWindow.Install_Widget(self.Page)
        else:
            if self.OptionalWindow:
                self.OptionalWindow.close()

    def fill_tab(self):
        #global MaximumNumberOf_Outputs
        #global MaximumNumberOf_ButtonEvents
        #global MaximumNumberOf_Buttons
        #global MaximumNumberOf_Ports
        #global MaximumNumberOf_Encoders

        self.CurrentWorkingFlags = HCDCompartibleFlags.InEveryMode
        self.CurrentWorkingFlags &= ~HCDCompartibleFlags.InFileMode
        self.PWMDiscreet = 512
        self.CurrentBrightness = 1024

        mainlay = QtWidgets.QHBoxLayout(self.Page)
        layv2 = QtWidgets.QVBoxLayout(self.Page)

        labl1 = QtWidgets.QLabel(Interface_Names.Controller_ControllerTypeInitialLabel, self.Page, )
        labl1.setFixedHeight(25)
        self.controllerTypeSelect = QtWidgets.QComboBox(self.Page)
        self.controllerTypeSelect.setFixedSize(80, 25)
        self.controllerTypeSelect.addItem("- - -")
        self.controllerCodeVersionLabel = QtWidgets.QLabel(" ", self.Page, )
        self.controllerCodeVersionLabel.setFixedHeight( 25)
        for pd in Possible_Controllers:
            self.controllerTypeSelect.addItem(pd.name)
        self.controllerTypeSelect.currentIndexChanged.connect(self.Change_Visibility_Configs)

        btn1 = QtWidgets.QPushButton(Interface_Names.Controller_RemoveControllerButton, self.Page)
        btn1.setFixedSize(60, 25)
        if self.index == 0:
            btn1.setDisabled(True)
        btn1.clicked.connect(self.Destroy)
        self.GotoWindowButton = QtWidgets.QPushButton(Interface_Names.Controller_GotoWindowButton_ToWindow, self.Page)
        self.GotoWindowButton.setFixedSize(90, 25)
        self.GotoWindowButton.clicked.connect(self.Move_to_Window)

        layh1 = QtWidgets.QHBoxLayout(self.Page)

        layh1.addWidget(labl1)
        layh1.addWidget(self.controllerTypeSelect)
        layh1.addWidget(self.controllerCodeVersionLabel)
        layh1.addStretch()
        layh1.addWidget(self.GotoWindowButton)
        layh1.addWidget(btn1)

        labl2 = QtWidgets.QLabel(Interface_Names.Controller_IDInitialLabel, self.Page, )
        labl2.setFixedHeight(25)
        self.idBox = QtWidgets.QLineEdit(self.ID, self.Page)
        self.idBox.setFixedSize(60, 25)
        self.idBox.setValidator(UIdValidator)
        #self.idBox.setVerticalScrollBarPolicy(1)
        btn21 = QtWidgets.QPushButton(Interface_Names.Controller_GetIDButton, self.Page)
        btn21.setFixedSize(40, 25)
        btn21.setDisabled(True)
        self.COMchoice = QtWidgets.QComboBox(self.Page)
        self.COMchoice.setFixedSize(60, 25)
        btn22 = QtWidgets.QPushButton(Interface_Names.Controller_ConnectToControllerDeviceButton, self.Page)
        btn22.setFixedSize(70, 25)
        btn23 = QtWidgets.QPushButton(Interface_Names.Controller_OpenTerminalButton, self.Page)
        btn23.setFixedSize(100, 25)
        btn23.setDisabled(True)

        layh2 = QtWidgets.QHBoxLayout(self.Page)

        layh2.addWidget(labl2)
        layh2.addWidget(self.idBox)
        layh2.addWidget(btn21)
        layh2.addStretch()

        layh21 = QtWidgets.QHBoxLayout(self.Page)
        layh21.addWidget(btn22)
        layh21.addWidget(self.COMchoice)
        layh21.addWidget(btn23)
        layh21.addStretch()

        self.IOconf = QtWidgets.QScrollArea(self.Page)
        self.IOconf.setVerticalScrollBarPolicy(Qt.ScrollBarAlwaysOn)
        self.IOconf.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self.IOconf.setMinimumWidth(self.ParentTabLayout.Style.ControllerConfigAreaWidth)
        self.IOconf.setMinimumHeight(self.ParentTabLayout.Style.ControllerConfigAreaHeight)
        self.IOconf.setWidgetResizable(True)

        self.IOconfigs = QtWidgets.QFrame(self.IOconf)

        self.IOconfigs_Layout = QtWidgets.QVBoxLayout(self.IOconfigs)
        self.IOconfigs_Layout.setContentsMargins(5, 5, 5, 5)

        self.IO_Output_header = QtWidgets.QPushButton(Interface_Names.Controller_IO_Output_header_True, self.IOconfigs)
        self.IO_Output_header.setStyleSheet("QPushButton { text-align: left; }")
        self.IO_Output_header.setDisabled(True)
        self.BrightnessLabel = QtWidgets.QLabel(Interface_Names.Controller_BrightnessLabel, self.IOconfigs)
        self.BrightnessLabel.setFixedHeight(25)
        self.BrightnessValue = QtWidgets.QLineEdit(str(self.CurrentBrightness), self.IOconfigs)
        self.BrightnessValue.setFixedSize(50, 25)
        self.BrightnessValue.setValidator(IntValuesValidator)#setInputMask("0000000")
        self.BrightnessValue.editingFinished.connect(self.check_controller_parameter_values)
        layh50a = QtWidgets.QHBoxLayout(self.IOconfigs)
        layh50a.addSpacing(4)
        layh50a.addWidget(self.BrightnessLabel)
        layh50a.addWidget(self.BrightnessValue)
        layh50a.addStretch()
        layh50a.addSpacing(2)
        self.FastAnimCheckbox = QtWidgets.QCheckBox(Interface_Names.Controller_FastAnimCheckbox, self.IOconfigs)
        self.FastAnimCheckbox.setChecked(False)
        self.FastAnimationCheckbox = False
        self.CurrentWorkingFlags &= ~HCDCompartibleFlags.InFastVideomode
        self.FastAnimCheckbox.setEnabled(False)
        self.FastAnimCheckbox.stateChanged.connect(self.update_fast_animation_flag)
        #self.FastAnimCheckbox.stateChanged.connect(self.Change_Visibility_Configs)
        layh50 = QtWidgets.QHBoxLayout(self.IOconfigs)
        layh50.addSpacing(4)
        layh50.addWidget(self.FastAnimCheckbox)
        layh50.addStretch()
        layh50.addSpacing(2)
        self.RealtimeServoCheckbox = QtWidgets.QCheckBox(Interface_Names.Controller_RealtimeServoCheckbox_False, self.IOconfigs)
        self.RealtimeServoCheckbox.setChecked(False)
        self.RealtimeServoCheckboxState = False
        self.CurrentWorkingFlags &= ~HCDCompartibleFlags.InRealtimeServoMode
        self.RealtimeServoCheckbox.setEnabled(False)
        self.RealtimeServoCheckbox.stateChanged.connect(self.update_fast_animation_flag)
        layh50b = QtWidgets.QHBoxLayout(self.IOconfigs)
        layh50b.addSpacing(4)
        layh50b.addWidget(self.RealtimeServoCheckbox)
        layh50b.addStretch()
        layh50b.addSpacing(2)
        self.ServoStepsSettingLabel = QtWidgets.QLabel(Interface_Names.Controller_ServoStepsSettingLabel, self.IOconfigs)
        self.ServoStepsSettingLabel.setFixedSize(160,25)
        self.ServoStepsSettingValue = QtWidgets.QLineEdit(str(self.PWMDiscreet), self.IOconfigs)
        self.ServoStepsSettingValue.setFixedSize(50, 25)
        self.ServoStepsSettingValue.setValidator(IntValuesValidator)#setInputMask("0000000")
        self.ServoStepsSettingValue.editingFinished.connect(self.check_controller_parameter_values)
        layh50c = QtWidgets.QHBoxLayout(self.IOconfigs)
        layh50c.addSpacing(4)
        layh50c.addWidget(self.ServoStepsSettingLabel)
        layh50c.addWidget(self.ServoStepsSettingValue)
        layh50c.addStretch()
        layh50c.addSpacing(2)
        self.ServoFrequencySettingLabel = QtWidgets.QLabel(Interface_Names.Controller_ServoFrequencySettingLabel, self.IOconfigs)
        self.ServoFrequencySettingLabel.setFixedSize(160,25)
        self.ServoFrequencySettingValue = QtWidgets.QLineEdit(str(self.PWMFrequency), self.IOconfigs)
        self.ServoFrequencySettingValue.setFixedSize(50, 25)
        self.ServoFrequencySettingValue.setValidator(IntValuesValidator)
        self.ServoFrequencySettingValue.editingFinished.connect(self.check_controller_parameter_values)
        layh50d = QtWidgets.QHBoxLayout(self.IOconfigs)
        layh50d.addSpacing(4)
        layh50d.addWidget(self.ServoFrequencySettingLabel)
        layh50d.addWidget(self.ServoFrequencySettingValue)
        layh50d.addStretch()
        layh50d.addSpacing(2)
        self.ServoImpulseLengthLabel1 = QtWidgets.QLabel(Interface_Names.Controller_ServoImpulseLengthLabel1, self.IOconfigs)
        self.ServoImpulseLengthLabel1.setFixedSize(160,25)
        self.ServoImpulseLengthValue1 = QtWidgets.QLineEdit(str(self.PWMMinImpulse), self.IOconfigs)
        self.ServoImpulseLengthValue1.setFixedSize(50, 25)
        self.ServoImpulseLengthValue1.setValidator(IntValuesValidator)
        self.ServoImpulseLengthValue1.editingFinished.connect(self.check_controller_parameter_values)
        self.ServoImpulseLengthLabel2 = QtWidgets.QLabel(Interface_Names.Controller_ServoImpulseLengthLabel2, self.IOconfigs)
        self.ServoImpulseLengthLabel2.setFixedSize(15,25)
        self.ServoImpulseLengthValue2 = QtWidgets.QLineEdit(str(self.PWMMaxImpulse), self.IOconfigs)
        self.ServoImpulseLengthValue2.setFixedSize(50, 25)
        self.ServoImpulseLengthValue2.setValidator(IntValuesValidator)
        self.ServoImpulseLengthValue2.editingFinished.connect(self.check_controller_parameter_values)
        layh50e = QtWidgets.QHBoxLayout(self.IOconfigs)
        layh50e.addSpacing(4)
        layh50e.addWidget(self.ServoImpulseLengthLabel1)
        layh50e.addWidget(self.ServoImpulseLengthValue1)
        layh50e.addWidget(self.ServoImpulseLengthLabel2)
        layh50e.addWidget(self.ServoImpulseLengthValue2)
        layh50e.addStretch()
        layh50e.addSpacing(2)

        self.IO_Input_header = QtWidgets.QPushButton(Interface_Names.Controller_IO_Input_header_True, self.IOconfigs)
        self.IO_Input_header.setStyleSheet("QPushButton { text-align: left; }")
        self.IO_Input_header.setDisabled(True)
        self.IO_Input_header_comb = QtWidgets.QCheckBox("Combination mode", self.IOconfigs)
        self.IO_Input_header_comb.setDisabled(True)
        self.IO_Input_header_comb.stateChanged.connect(self.update_fast_combination_flag)
        layh51a = QtWidgets.QHBoxLayout(self.IOconfigs)
        layh51a.addSpacing(4)
        layh51a.addWidget(self.IO_Input_header_comb)
        layh51a.addStretch()
        layh51a.addSpacing(2)
        self.BtnCombinationDurationLabel = QtWidgets.QLabel("Combination input window (ms):", self.IOconfigs)
        self.BtnCombinationDurationLabel.setFixedHeight(25)
        self.BtnCombinationDurationValue = QtWidgets.QLineEdit(str(self.ButtonCombinationTimeout), self.IOconfigs)
        self.BtnCombinationDurationValue.setFixedSize(50, 25)
        self.BtnCombinationDurationValue.setValidator(IntValuesValidator)
        self.BtnCombinationDurationValue.editingFinished.connect(self.check_controller_parameter_values)
        layh51b = QtWidgets.QHBoxLayout(self.IOconfigs)
        layh51b.addSpacing(4)
        layh51b.addWidget(self.BtnCombinationDurationLabel)
        layh51b.addWidget(self.BtnCombinationDurationValue)
        layh51b.addStretch()
        layh51b.addSpacing(2)
        self.ButtonRepeatLabel = QtWidgets.QCheckBox("Repeat button action every (ms):", self.IOconfigs)
        self.ButtonRepeatLabel.setFixedHeight(25)
        self.ButtonRepeatLabel.stateChanged.connect(self.update_fast_combination_flag)
        self.ButtonRepeatValue = QtWidgets.QLineEdit(str(self.ButtonRepeatTimeout), self.IOconfigs)
        self.ButtonRepeatValue.setFixedSize(50, 25)
        self.ButtonRepeatValue.setValidator(IntValuesValidator)#setInputMask("0000000")
        self.ButtonRepeatValue.editingFinished.connect(self.check_controller_parameter_values)
        self.ButtonRepeatValue.setEnabled(self.ButtonRepeat)
        layh51c = QtWidgets.QHBoxLayout(self.IOconfigs)
        layh51c.addSpacing(4)
        layh51c.addWidget(self.ButtonRepeatLabel)
        layh51c.addWidget(self.ButtonRepeatValue)
        layh51c.addStretch()
        layh51c.addSpacing(2)

        self.IO_UART_header = QtWidgets.QPushButton(Interface_Names.Controller_IO_UART_header_True, self.IOconfigs)
        self.IO_UART_header.setStyleSheet("QPushButton { text-align: left; }")
        self.IO_UART_header.setDisabled(True)

        self.IO_UART_header01 = QtWidgets.QLabel("Port", self.IOconfigs)
        self.IO_UART_header02 = QtWidgets.QLabel("Baudrate", self.IOconfigs)
        self.IO_UART_header03 = QtWidgets.QLabel("Input format", self.IOconfigs)
        self.IO_UART_header04 = QtWidgets.QLabel("Output format", self.IOconfigs)
        self.IO_UART_header05 = QtWidgets.QLabel("Timeout", self.IOconfigs)
        self.IO_UART_header01.setFixedSize(50, 20)
        self.IO_UART_header02.setFixedSize(50, 20)
        self.IO_UART_header03.setFixedSize(100, 20)
        self.IO_UART_header04.setFixedSize(100, 20)
        self.IO_UART_header05.setFixedSize(50, 20)
        layh52 = QtWidgets.QHBoxLayout(self.IOconfigs)
        layh52.addSpacing(4)
        layh52.addWidget(self.IO_UART_header01)
        layh52.addWidget(self.IO_UART_header02)
        layh52.addWidget(self.IO_UART_header03)
        layh52.addWidget(self.IO_UART_header04)
        layh52.addWidget(self.IO_UART_header05)
        layh52.addStretch()
        layh52.addSpacing(2)

        self.IO_Screens_header = QtWidgets.QPushButton(Interface_Names.Controller_IO_Screens_header_True, self.IOconfigs)
        self.IO_Screens_header.setStyleSheet("QPushButton { text-align: left; }")
        self.IO_Screens_header.setDisabled(True)
        #self.IO_Screens_header.setVisible(False)
        self.IO_Files_header = QtWidgets.QPushButton(Interface_Names.Controller_IO_Files_header_True, self.IOconfigs)
        self.IO_Files_header.setStyleSheet("QPushButton { text-align: left; }")
        self.IO_Files_header.setDisabled(True)
        #self.IO_Files_header.setVisible(False)

        self.IO_Output_header.clicked.connect(self.Change_Visibility_Outputs)
        self.IO_Input_header.clicked.connect(self.Change_Visibility_Inputs)
        self.IO_UART_header.clicked.connect(self.Change_Visibility_Ports)
        self.IO_Screens_header.clicked.connect(self.Change_Visibility_Screens)
        self.IO_Files_header.clicked.connect(self.Change_Visibility_Files)

        for i in range(MaximumNumberOf.Outputs):
            if len(self.Outputs) <= i:
                self.Outputs.append(None)
            if type(self.Outputs[i]) is DKLed_Output_Instances:
                self.Outputs[i].destroy_widget()
            temp = DKLed_Output_Instances()
            temp.Controller = self
            temp.TabName = self.TabName + " output "+str(i)
            temp.ControllerType = Possible_Controllers[0]
            if self.controllerTypeSelect.currentIndex() > 0:
                temp.ControllerType = Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1]
            temp.index = i
            temp.ParentPage = self.IOconfigs
            temp.Construct_Widget()
            temp.QtComboWidget.currentIndexChanged.connect(self.update_servo_parameter_interactability)
            self.FastAnimCheckbox.stateChanged.connect(temp.Update_Instance_List)
            self.Outputs[i] = temp
        self.CurrentWorkingFlags &= ~HCDCompartibleFlags.InServoMode

        for i in range(MaximumNumberOf.Encoders):
            if len(self.Encoders) <= i:
                self.Encoders.append(None)
            if type(self.Encoders[i]) is DKLed_Encoder_Instances:
                self.Encoders[i].destroy_widget()
            temp = DKLed_Encoder_Instances()
            temp.btncombCheckbox = self.IO_Input_header_comb
            temp.Controller = self
            temp.ControllerType = Possible_Controllers[0]
            if self.controllerTypeSelect.currentIndex() > 0:
                temp.ControllerType = Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1]
            temp.index = i
            temp.ParentPage = self.IOconfigs
            temp.Construct_Widget()
            self.IO_Input_header_comb.stateChanged.connect(temp.Update_Instance_List)
            self.Encoders[i] = temp

        laybtns = []
        j = 0
        laybtns.append(None)
        laybtns[j] = QtWidgets.QHBoxLayout(self.IOconfigs)
        laybtns[j].addSpacing(24)
        for i in range(MaximumNumberOf.Buttons):
            if i >= (j+1)*8:
                laybtns[j].addStretch()
                laybtns[j].addSpacing(2)
                j += 1
                laybtns.append(None)
                laybtns[j] = QtWidgets.QHBoxLayout(self.IOconfigs)
                laybtns[j].addSpacing(24)
            if len(self.Inputs) <= i:
                self.Inputs.append(None)
            if type(self.Inputs[i]) is DKLed_Button_Instances:
                self.Inputs[i].destroy_widget()
            temp = DKLed_Button_Instances()
            temp.btncombCheckbox = self.IO_Input_header_comb
            temp.Controller = self
            temp.ControllerType = Possible_Controllers[0]
            if self.controllerTypeSelect.currentIndex() > 0:
                temp.ControllerType = Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1]
            temp.index = i + 1
            temp.ParentPage = self.IOconfigs
            temp.Construct_Widget()
            laybtns[j].addWidget(temp.QtBTNWidget)
            self.IO_Input_header_comb.stateChanged.connect(temp.Update_Instance_List)
            self.Inputs[i] = temp
        laybtns[j].addStretch()
        laybtns[j].addSpacing(2)

        for i in range(MaximumNumberOf.Ports):
            if len(self.Ports) <= i:
                self.Ports.append(None)
            if  type(self.Ports[i]) is DKLed_Port_Instances:
                self.Ports[i].destroy_widget()
            temp = DKLed_Port_Instances()
            temp.Baudrate = 115200
            temp.WaitPeriod = 1000
            temp.Controller = self
            temp.ControllerType = Possible_Controllers[0]
            if self.controllerTypeSelect.currentIndex() > 0:
                temp.ControllerType = Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1]
            temp.index = i
            temp.ParentPage = self.IOconfigs
            temp.Construct_Widget()
            self.Ports[i] = temp
        self.USBLabel = QtWidgets.QLabel("  USB Communication: yes", self.IOconfigs)
        self.USBLabel.setFixedHeight(25)
        self.USBWaitLabel = QtWidgets.QLabel("Wait timeout:", self.IOconfigs)
        self.USBWaitLabel.setFixedHeight(25)
        self.USBWaitValue = QtWidgets.QLineEdit(str(self.USBWaitTimeout), self.IOconfigs)
        self.USBWaitValue.setFixedSize(50, 25)
        self.USBWaitValue.setValidator(IntValuesValidator)#setInputMask("0000000")
        self.USBWaitValue.editingFinished.connect(self.check_controller_parameter_values)
        layh52a = QtWidgets.QHBoxLayout(self.IOconfigs)
        layh52a.addSpacing(4)
        layh52a.addWidget(self.USBWaitLabel)
        layh52a.addWidget(self.USBWaitValue)
        layh52a.addStretch()
        layh52a.addSpacing(2)

        self.IOconfigs_Layout.addWidget(self.IO_Output_header)
        self.IOconfigs_Layout.addLayout(layh50a)
        self.IOconfigs_Layout.addLayout(layh50)
        for i in range(len(self.Outputs)):
            self.IOconfigs_Layout.addLayout(self.Outputs[i].layout)
        self.IOconfigs_Layout.addLayout(layh50b)
        self.IOconfigs_Layout.addLayout(layh50c)
        self.IOconfigs_Layout.addLayout(layh50d)
        self.IOconfigs_Layout.addLayout(layh50e)
        self.IOconfigs_Layout.addWidget(self.IO_Input_header)
        self.IOconfigs_Layout.addLayout(layh51a)
        self.IOconfigs_Layout.addLayout(layh51b)
        for i in range(len(laybtns)):
            self.IOconfigs_Layout.addLayout(laybtns[i])
        for i in range(len(self.Encoders)):
            self.IOconfigs_Layout.addLayout(self.Encoders[i].layout)
        self.IOconfigs_Layout.addLayout(layh51c)
        self.IOconfigs_Layout.addWidget(self.IO_UART_header)
        self.IOconfigs_Layout.addWidget(self.USBLabel)
        self.IOconfigs_Layout.addLayout(layh52a)
        self.IOconfigs_Layout.addLayout(layh52)
        for i in range(len(self.Ports)):
            self.IOconfigs_Layout.addLayout(self.Ports[i].layout)
        self.IOconfigs_Layout.addWidget(self.IO_Screens_header)
        self.IOconfigs_Layout.addWidget(self.IO_Files_header)
        self.IOconfigs_Layout.addStretch()

        self.IOconf.setWidget(self.IOconfigs)
        self.Change_Visibility_Outputs()
        self.Change_Visibility_Inputs()
        self.Change_Visibility_Ports()
        self.Change_Visibility_Screens()
        self.Change_Visibility_Files()


        labl3 = QtWidgets.QLabel("Files at controller", self.Page, )
        btn31 = QtWidgets.QPushButton("Browse", self.Page)
        btn31.setFixedHeight(25)
        btn31.setDisabled(True)
        btn32 = QtWidgets.QPushButton("Upload", self.Page)
        btn32.setFixedHeight(25)
        btn32.setDisabled(True)

        self.InitFileExploreButton = QtWidgets.QPushButton("Explore initialisation scenario sequence", self.Page)
        self.InitFileExploreButton.setFixedHeight(25)

        layh3 = QtWidgets.QHBoxLayout(self.Page)

        layh3.addWidget(btn31)
        layh3.addWidget(btn32)
        layh3.addStretch()

        layv2.addLayout(layh1)
        layv2.addLayout(layh2)
        layv2.addLayout(layh21)
        layv2.addWidget(self.IOconf)
        layv2.addWidget(labl3)
        layv2.addLayout(layh3)
        layv2.addWidget(self.InitFileExploreButton)

        #self.layPage=layv2
        mainlay.addLayout(layv2)
        mainlay.addStretch(50)

        self.Page.setLayout(mainlay)
        self.ParentTabWidget.setCurrentIndex(self.index)
        self.Page.show()

    def create_layout_entity(self,LayoutPrj_):
        self.ConnectionPoints.clear()
        self.Representation = DKLed_Project_Layout_Item()
        self.Representation.RepresentedEntity = self
        self.Representation.RepresentedEntityType = DKLed_Project_Layout_Types.LEDController
        self.Representation.RepresentedEntitySpecs = None
        if self.controllerTypeSelect.currentIndex() > 0:
            self.Representation.RepresentedEntitySpecs = Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1]
        self.Representation.LayoutControl = LayoutPrj_

        self.GotoWindowButton2 = QtWidgets.QPushButton("")
        self.GotoWindowButton2.setFixedSize(LayoutStyle.Iconsize)
        self.GotoWindowButton2.setStyleSheet(LayoutStyle.IconStyle)
        self.GotoWindowButton2.clicked.connect(self.Move_to_Window)
        self.GotoWindowButton2.setIcon(LayoutIcons.ControllerSettingsIcon)
        self.GotoWindowButton2.setIconSize(LayoutStyle.Iconsize)

        self.Representation.Construct_Item(self.TabName, self.GotoWindowButton2)

        #rou = self.Representation.Sync_with_Phisics_Device(0, DKLed_Project_Layout_Types.OutputChannel, self, 0)
        #self.ConnectionPoints.append(rou)

    def Update_layout_entity(self):

        if self.controllerTypeSelect.currentIndex() > 0:
            self.Representation.RepresentedEntitySpecs = Possible_Controllers[
                self.controllerTypeSelect.currentIndex() - 1]
        else:
            self.Representation.RepresentedEntitySpecs = None
        self.Representation.Update_Item()
        #reclaim connections
        self.ConnectionPoints.clear()
        if self.controllerTypeSelect.currentIndex() > 0:
            for i in range(Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].out):
                rou = self.Representation.Sync_with_Phisics_Device(i, 0, DKLed_Project_Layout_Types.OutputChannel, self.Outputs[i], i)
                self.ConnectionPoints.append(rou)
                self.Outputs[i].ConnectionPoints.clear()
                self.Outputs[i].ConnectionPoints.append(rou)
            for i in range(Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].btn):
                rou = self.Representation.Sync_with_Phisics_Device(i, 0, DKLed_Project_Layout_Types.InputChannel, self.Inputs[i], i)
                self.ConnectionPoints.append(rou)
                self.Inputs[i].ConnectionPoints.clear()
                self.Inputs[i].ConnectionPoints.append(rou)
            for i in range(Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].uart):
                rou = self.Representation.Sync_with_Phisics_Device(i, 0, DKLed_Project_Layout_Types.PortChanel, self.Ports[i], i)
                self.ConnectionPoints.append(rou)
                self.Ports[i].ConnectionPoints.clear()
                self.Ports[i].ConnectionPoints.append(rou)
                rou = self.Representation.Sync_with_Phisics_Device(i, 1, DKLed_Project_Layout_Types.PortChanel, self.Ports[i], i + 512)
                if rou:
                    self.ConnectionPoints.append(rou)
                    self.Ports[i].ConnectionPoints.append(rou)

    def Disconnect(self, Obj, Direct = False):# disconnect object from this one
        for i in range(len(self.LastConnected) - 1, -1, -1):
            if self.LastConnected[i] == Obj:
                self.LastConnected.pop(i)
                self.LastConnectedIndex.pop(i)
                self.LastConnectedInternalIndex.pop(i)
                self.LastConnectedType.pop(i)

    def Respond_to_layout_Action(self, index, Action =  DKLed_Project_Layout_Actions.NoAction, Parameter=None):
        pass

    def Remove_Connection_Point(self, P=None):
        if P:
            #print(self.ConnectionPoints)
            if P in self.ConnectionPoints:
                self.ConnectionPoints.remove(P)
                if P.ConnectType == DKLed_Project_Layout_Types.OutputChannel:
                    if P.No < MaximumNumberOf.Outputs:
                        if P in self.Outputs[P.No].ConnectionPoints:
                            self.Outputs[P.No].ConnectionPoints.remove(P)
                elif P.ConnectType == DKLed_Project_Layout_Types.InputChannel:
                    if P.No < MaximumNumberOf.Buttons:
                        if P in self.Inputs[P.No].ConnectionPoints:
                            self.Inputs[P.No].ConnectionPoints.remove(P)
                elif P.ConnectType == DKLed_Project_Layout_Types.PortChanel:
                    if P.No < MaximumNumberOf.Ports:
                        if P in self.Ports[P.No].ConnectionPoints:
                            for ii in range(len(self.Ports[P.No].ConnectionPoints) - 1, -1, -1):
                                if self.Ports[P.No].ConnectionPoints[ii] == P:
                                    self.Ports[P.No].ConnectionPoints.pop(ii)

    def Change_Visibility_Outputs(self):
        Visible_ = False
        if (self.IO_Output_header.text() == Interface_Names.Controller_IO_Output_header_False):
            if (self.controllerTypeSelect.currentIndex() > 0):
                Visible_ = True
            self.IO_Output_header.setText(Interface_Names.Controller_IO_Output_header_True)
        else:
            self.IO_Output_header.setText(Interface_Names.Controller_IO_Output_header_False)
        self.BrightnessValue.setVisible(Visible_)
        self.BrightnessLabel.setVisible(Visible_)
        if Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].DefaultMaxBrightness == Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].DefaultMinBrightness:
            self.BrightnessValue.setVisible(False)
            self.BrightnessLabel.setVisible(False)
        self.FastAnimCheckbox.setVisible(Visible_)
        if not Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].fastvid:
            self.FastAnimCheckbox.setVisible(False)
        self.RealtimeServoCheckbox.setVisible(False)
        if Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].DefaultPWMRealtime:
            self.RealtimeServoCheckbox.setVisible(Visible_)
        self.ServoStepsSettingValue.setVisible(Visible_)
        self.ServoStepsSettingLabel.setVisible(Visible_)
        self.ServoFrequencySettingValue.setVisible(Visible_)
        self.ServoFrequencySettingLabel.setVisible(Visible_)
        if Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].DefaultMinPWMFreq == Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].DefaultMaxPWMFreq:
            self.ServoFrequencySettingValue.setVisible(False)
            self.ServoFrequencySettingLabel.setVisible(False)
        self.ServoImpulseLengthValue1.setVisible(Visible_)
        self.ServoImpulseLengthLabel1.setVisible(Visible_)
        self.ServoImpulseLengthValue2.setVisible(Visible_)
        self.ServoImpulseLengthLabel2.setVisible(Visible_)
        if Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].DefaultMinPWMPosition == Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].DefaultMaxPWMPosition:
            self.ServoImpulseLengthValue1.setVisible(False)
            self.ServoImpulseLengthLabel1.setVisible(False)
            self.ServoImpulseLengthValue2.setVisible(False)
            self.ServoImpulseLengthLabel2.setVisible(False)

        self.BrightnessValue.setText(str(self.CurrentBrightness))
        self.ServoStepsSettingValue.setText(str(self.PWMDiscreet))
        self.ServoFrequencySettingValue.setText(str(self.PWMFrequency))
        self.ServoImpulseLengthValue1.setText(str(self.PWMMinImpulse))
        self.ServoImpulseLengthValue2.setText(str(self.PWMMaxImpulse))

        for i in range(len(self.Outputs)):
            if (self.controllerTypeSelect.currentIndex() == 0) or (Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].out <= i):
                Visible_ = False
            self.Outputs[i].set_visible(Visible_)
        del Visible_

    def Change_Visibility_Inputs(self):
        Visible_ = False
        if (self.IO_Input_header.text() == Interface_Names.Controller_IO_Input_header_False):
            if (self.controllerTypeSelect.currentIndex() > 0):
                Visible_ = True
            self.IO_Input_header.setText(Interface_Names.Controller_IO_Input_header_True)
        else:
            self.IO_Input_header.setText(Interface_Names.Controller_IO_Input_header_False)
        self.IO_Input_header_comb.setVisible(Visible_)
        self.BtnCombinationDurationLabel.setVisible(Visible_)
        self.BtnCombinationDurationValue.setVisible(Visible_)
        if not Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].btncomb:
            self.IO_Input_header_comb.setVisible(False)
            self.BtnCombinationDurationLabel.setVisible(False)
            self.BtnCombinationDurationValue.setVisible(False)
        self.ButtonRepeatValue.setVisible(Visible_)
        self.ButtonRepeatLabel.setVisible(Visible_)
        if Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].DefaultMaxButtonSetWaitTimeout == Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].DefaultMinButtonSetWaitTimeout:
            self.ButtonRepeatValue.setVisible(False)
            self.ButtonRepeatLabel.setVisible(False)
        Vis2=Visible_
        for i in range(len(self.Inputs)):
            if (self.controllerTypeSelect.currentIndex() == 0) or (Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].btn <= i):
                Vis2 = False
            self.Inputs[i].set_visible(Vis2)
        for i in range(len(self.Encoders)):
            if (self.controllerTypeSelect.currentIndex() == 0) or (Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].encd <= i):
                Visible_ = False
            self.Encoders[i].set_visible(Visible_)
        self.ButtonRepeatValue.setText(str(self.ButtonRepeatTimeout))
        del Vis2
        del Visible_

    def Change_Visibility_Ports(self):
        Visible_ = False
        if self.IO_UART_header.text() == Interface_Names.Controller_IO_UART_header_False:
            if (self.controllerTypeSelect.currentIndex() > 0) and (Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].uart > 0):
                Visible_ = True
            self.IO_UART_header.setText(Interface_Names.Controller_IO_UART_header_True)
        else:
            self.IO_UART_header.setText(Interface_Names.Controller_IO_UART_header_False)
        self.IO_UART_header01.setVisible(Visible_)
        self.IO_UART_header02.setVisible(Visible_)
        self.IO_UART_header03.setVisible(Visible_)
        self.IO_UART_header04.setVisible(Visible_)
        self.IO_UART_header05.setVisible(Visible_)

        if Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].USBSupport:
            self.USBLabel.setVisible(Visible_)
            self.CurrentWorkingFlags |= HCDCompartibleFlags.InUSBCommunicationMode
            self.USBWaitLabel.setVisible(Visible_)
            self.USBWaitValue.setVisible(Visible_)
            if Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].DefaultMinUSBWaitTimeout == Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].DefaultMaxUSBWaitTimeout:
                self.USBWaitLabel.setVisible(False)
                self.USBWaitValue.setVisible(False)
                #self.USBWaitTimeout = Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].DefaultMinUSBWaitTimeout
            self.USBWaitValue.setText(str(self.USBWaitTimeout))
        else:
            self.USBLabel.setVisible(False)
            self.USBWaitLabel.setVisible(False)
            self.USBWaitValue.setVisible(False)
            self.CurrentWorkingFlags &= ~HCDCompartibleFlags.InUSBCommunicationMode
        for i in range(len(self.Ports)):
            if (self.controllerTypeSelect.currentIndex() == 0) \
                or (Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].uart <= i):
                Visible_ = False
            self.Ports[i].set_visible(Visible_)
        #print(str(Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].USBSupport))

    def Change_Visibility_Screens(self):
        Visible_ = False
        if (self.IO_Screens_header.text() == Interface_Names.Controller_IO_Screens_header_False):
            if (self.controllerTypeSelect.currentIndex() > 0):
                Visible_ = True
            self.IO_Screens_header.setText(Interface_Names.Controller_IO_Screens_header_True)
        else:
            self.IO_Screens_header.setText(Interface_Names.Controller_IO_Screens_header_False)
        del Visible_

    def Change_Visibility_Files(self):
        Visible_ = False
        if (self.IO_Files_header.text() == Interface_Names.Controller_IO_Files_header_False):
            if (self.controllerTypeSelect.currentIndex() > 0):
                Visible_ = True
            self.IO_Files_header.setText(Interface_Names.Controller_IO_Files_header_True)
        else:
            self.IO_Files_header.setText(Interface_Names.Controller_IO_Files_header_False)
        del Visible_

    def Change_Visibility_Configs(self):
        if self.controllerTypeSelect.currentIndex() > 0:
            self.CurrentWorkingFlags = HCDCompartibleFlags.InEveryMode
            self.PWMDiscreet = Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].DefaultPWMDiscreet
            self.CurrentBrightness = Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].DefaultMaxBrightness
            self.PWMMinImpulse = Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].DefaultStartPWMPosition
            self.PWMMaxImpulse = Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].DefaultStopPWMPosition
            self.ButtonRepeatTimeout = Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].DefaultMaxButtonSetWaitTimeout
            self.ButtonCombinationTimeout = Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].DefaultButtonCombinationSetWaitTimeout

            self.Update_layout_entity()
            for o in self.Outputs:
                o.ControllerType = Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1]
                if o.index < Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].out:
                    o.Update_Instance_List()
            for o in self.Inputs:
                o.ControllerType = Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1]
                if o.index - 1 < Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].btn:
                    o.Update_Instance_List()
            BtnCombEn = True
            for o in self.Encoders:
                o.ControllerType = Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1]
                if o.index < Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].encd:
                    o.Update_Instance_List()
                    o.avoid_collisions()
                    if o.QtCheckWidget.isChecked():
                        BtnCombEn = False
                else:
                    o.Reconstruct_encoder_connections()
            for o in self.Inputs:
                o.ControllerType = Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1]
                if o.index - 1 < Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].btn:
                    o.Update_Instance_List()

            for o in self.Ports:
                o.ControllerType = Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1]
                if o.index < Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].uart:
                    o.Update_Instance_List()
            self.IO_Input_header.setDisabled(False)
            self.IO_Output_header.setDisabled(False)
            self.IO_Screens_header.setDisabled(True)
            self.IO_Files_header.setDisabled(False)
            self.controllerCodeVersionLabel.setText(Interface_Names.Controller_controllerCodeVersionLabel + Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].CodeVersion)
            self.CurrentHCDVersionIndex = -1
            for i in range(len(HCD_Version_list_Full)):
                if HCD_Version_list_Full[i] == Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].CodeVersion:
                    self.CurrentHCDVersionIndex = i
                    #print("controller ver " + HCD_Version_list_Full[i] + " in "+str(self.CurrentHCDVersionIndex))
            self.BrightnessValue.setText(str(self.CurrentBrightness))
            self.ServoStepsSettingValue.setText(str(self.PWMDiscreet))
            self.ServoFrequencySettingValue.setText(str(self.PWMFrequency))
            self.ServoImpulseLengthValue1.setText(str(self.PWMMinImpulse))
            self.ServoImpulseLengthValue2.setText(str(self.PWMMaxImpulse))
            self.ButtonRepeat = self.ButtonRepeatLabel.isChecked()
            self.ButtonCombination = self.IO_Input_header_comb.isChecked() and BtnCombEn
            self.ButtonRepeatValue.setText(str(self.ButtonRepeatTimeout))
            self.BtnCombinationDurationValue.setText(str(self.ButtonCombinationTimeout))
            if HCD_Command_Window.CurrentController == self:
                if HCD_Command_Window.DialogWindow.isVisible():
                    HCD_Command_Window.Call_Window()
            if Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].uart > 0:
                self.IO_UART_header.setDisabled(False)
                self.CurrentWorkingFlags |= (HCDCompartibleFlags.InByteReceivingMode | HCDCompartibleFlags.InTextReceiveMode)
            else:
                self.IO_UART_header.setDisabled(True)
                self.CurrentWorkingFlags &= ~(
                            HCDCompartibleFlags.InByteReceivingMode | HCDCompartibleFlags.InTextReceiveMode)
            if Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].fastvid:
                self.FastAnimCheckbox.setEnabled(True)
                if self.FastAnimCheckbox.isChecked():
                    # controller in fast animation mode
                    self.CurrentWorkingFlags |= HCDCompartibleFlags.InFastVideomode | HCDCompartibleFlags.InLedMode
                    self.CurrentWorkingFlags &= ~HCDCompartibleFlags.InServoMode
                else:
                    # Led or Hybrid mode
                    self.CurrentWorkingFlags |= HCDCompartibleFlags.InServoMode | HCDCompartibleFlags.InLedMode
                    self.CurrentWorkingFlags &= ~HCDCompartibleFlags.InFastVideomode
            else:
                self.FastAnimCheckbox.setEnabled(False)
                self.CurrentWorkingFlags |= HCDCompartibleFlags.InServoMode | HCDCompartibleFlags.InLedMode
                self.CurrentWorkingFlags &= ~HCDCompartibleFlags.InFastVideomode
            self.update_servo_parameter_interactability()
            if Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].btncomb:
                self.IO_Input_header_comb.setEnabled(BtnCombEn)
                self.IO_Input_header_comb.setChecked(self.ButtonCombination)
                if self.ButtonCombination:
                    # combination of buttons
                    self.CurrentWorkingFlags |= HCDCompartibleFlags.InCombinationInputMode
                    self.CurrentWorkingFlags &= ~HCDCompartibleFlags.InSingleInputMode
                else:
                    # single input
                    self.CurrentWorkingFlags &= ~HCDCompartibleFlags.InCombinationInputMode
                    self.CurrentWorkingFlags |= HCDCompartibleFlags.InSingleInputMode
                self.BtnCombinationDurationLabel.setEnabled(self.ButtonCombination)
                self.BtnCombinationDurationValue.setEnabled(self.ButtonCombination)
            else:
                self.IO_Input_header_comb.setEnabled(False)
                self.CurrentWorkingFlags |= HCDCompartibleFlags.InSingleInputMode
                self.CurrentWorkingFlags &= ~HCDCompartibleFlags.InCombinationInputMode

            self.check_controller_parameter_values()
        else:
            self.CurrentWorkingFlags = HCDCompartibleFlags.InNoMode
            self.PWMDiscreet = 512
            self.CurrentBrightness = 32
            self.controllerCodeVersionLabel.setText(" ")
            self.IO_Input_header.setDisabled(True)
            self.IO_Output_header.setDisabled(True)
            self.IO_Screens_header.setDisabled(True)
            self.IO_Files_header.setDisabled(True)
            self.IO_UART_header.setDisabled(True)
            self.FastAnimCheckbox.setEnabled(False)
            self.Update_layout_entity()

        self.Change_Visibility_Outputs()
        self.Change_Visibility_Outputs()
        self.Change_Visibility_Inputs()
        self.Change_Visibility_Inputs()
        self.Change_Visibility_Ports()
        self.Change_Visibility_Ports()

        #print("Controller " + self.TabName + " in mode: " + bin(self.CurrentWorkingFlags) + " Descrete =" + str(self.PWMDiscreet) + " Brightness =" + str(self.MaxBrightness))

    def Destroy(self, Kind = None):
        print ("destroy " + self.TabName)
        print("it's indestructable yet")
        return False

class Button_Phisical:
    def __init__(self):
        self.index = 0
        self.Connected = False
        self.TabName = "Button"

        #dependable entities
        self.Controller = None
        self.DKLed_Button_Instance = None #currently connected Controller input
        self.Last_DKLed_Button_Instance = None #stores the last value of currently connected input so it will be possibly reconnected

        #Project layout widgets:
        self.Representation = None
        self.GotoWindowButton2 = None
        self.ConnectionPoints = []#pointers to EllipseCallable objects

        self.LastConnected = []
        self.LastConnectedIndex = []
        self.LastConnectedInternalIndex = []
        self.LastConnectedType = []
        self.Type = DKLed_Project_Layout_Types.Button
        self.OutputCount = 1
        self.InputCount = 0

    def create_layout_entity(self, LayoutPrj_):
        self.ConnectionPoints.clear()
        self.Representation = DKLed_Project_Layout_Item()
        self.Representation.RepresentedEntity = self
        self.Representation.RepresentedEntityType = DKLed_Project_Layout_Types.Button
        self.Representation.RepresentedEntitySpecs = Possible_Periph[0]
        self.Representation.LayoutControl = LayoutPrj_

        self.Representation.Construct_Item(self.TabName)
        self.Representation.Update_Item()

        rou = self.Representation.Sync_with_Phisics_Device(0, 0, DKLed_Project_Layout_Types.OutputChannel, self, 0)
        self.ConnectionPoints.append(rou)

    def Update_layout_entity(self):
        print("Button layout item updates")

        self.Representation.RepresentedEntitySpecs = Possible_Periph[0]
        self.Representation.Update_Item()
        self.ConnectionPoints.clear()
        rou = self.Representation.Sync_with_Phisics_Device(0, 0, DKLed_Project_Layout_Types.OutputChannel, self, 0)
        self.ConnectionPoints.append(rou)

    def Respond_to_layout_Action(self, index, Action=DKLed_Project_Layout_Actions.NoAction, Parameter=None):
        pass

    def Connect_Item(self, Obj, Ind=0, IInd=0, Direct=False):
        self.LastConnected.append(Obj)
        self.LastConnectedIndex.append(Ind)
        self.LastConnectedInternalIndex.append(IInd)
        self.LastConnectedType.append(Obj.Type)
        #print("Button " + self.TabName + " list of connections length: "+ str(len(self.LastConnected)))
        return True

    def Disconnect(self, Obj, Direct = False):# disconnect object from this one
        #print("Button " + self.TabName + " disconnected")
        for i in range(len(self.LastConnected) - 1, -1, -1):
            if self.LastConnected[i] == Obj:
                self.LastConnected.pop(i)
                self.LastConnectedIndex.pop(i)
                self.LastConnectedInternalIndex.pop(i)
                self.LastConnectedType.pop(i)

    def Remove_Connection_Point(self, P=None):
        if P:
            if P in self.ConnectionPoints:
                self.ConnectionPoints.remove(P)

    def Destroy(self, Kind = None):
        if Kind == DKLed_Project_Layout_Types.Button or Kind == DKLed_Project_Layout_Types.SimpleDevice:
            self.Representation.Empty_Item()
            print(self.TabName + " emptied")
            self.ConnectionPoints.clear()
            self.LastConnected.clear()
            self.LastConnectedIndex.clear()
            self.LastConnectedInternalIndex.clear()
            self.LastConnectedType.clear()

            self.index = 0
            self.Connected = False
            self.TabName = ""
            self.Controller = None
            self.DKLed_Button_Instance = None #currently connected Controller input
            self.Last_DKLed_Button_Instance = None #stores the last value of currently connected input so it will be possibly reconnected
            self.Representation = None
            self.GotoWindowButton2 = None
            self.Type = DKLed_Project_Layout_Types.Nothing
            self.OutputCount = 1
            self.InputCount = 0
            print("  resetted")

            if self in Keyboards:
                Keyboards.remove(self)
            print("    destroyed")
            del self
            #gc.collect()
            return True
        else:
            return False

class Keyboard_Phisical:
    def __init__(self):
        self.index = 0
        self.Connected = False
        self.TabName = "Keyboard"

        #dependable entities
        self.Controller = None
        self.DKLed_Button_Instance = None #currently connected Controller input
        self.Last_DKLed_Button_Instance = None #stores the last value of currently connected input so it will be possibly reconnected

        #Project layout widgets:
        self.Representation = None
        self.GotoWindowButton2 = None
        self.GotoWindowButton3 = None
        self.ConnectionPoints = []#pointers to EllipseCallable objects

        self.LastConnected = []
        self.LastConnectedIndex = []
        self.LastConnectedInternalIndex = []
        self.LastConnectedType = []
        self.Type = DKLed_Project_Layout_Types.Keyboard
        self.OutputCount = 2
        self.InputCount = 0

    def create_layout_entity(self, LayoutPrj_):
        self.ConnectionPoints.clear()
        self.Representation = DKLed_Project_Layout_Item()
        self.Representation.RepresentedEntity = self
        self.Representation.RepresentedEntityType = DKLed_Project_Layout_Types.Keyboard
        self.Representation.RepresentedEntitySpecs = Possible_Periph[1]
        self.Representation.LayoutControl = LayoutPrj_

        self.GotoWindowButton2 = QtWidgets.QPushButton("")
        self.GotoWindowButton2.setFixedSize(LayoutStyle.Iconsize)
        self.GotoWindowButton2.setStyleSheet(LayoutStyle.IconStyle)
        self.GotoWindowButton2.clicked.connect(self.Add_extra_output)
        self.GotoWindowButton2.setIcon(LayoutIcons.AddButtonToKbdIcon)
        self.GotoWindowButton2.setIconSize(LayoutStyle.Iconsize)

        self.GotoWindowButton3 = QtWidgets.QPushButton("")
        self.GotoWindowButton3.setFixedSize(LayoutStyle.Iconsize)
        self.GotoWindowButton3.setStyleSheet(LayoutStyle.IconStyle)
        self.GotoWindowButton3.clicked.connect(self.Delete_extra_output)
        self.GotoWindowButton3.setIcon(LayoutIcons.DeleteButtonFromKbdIcon)
        self.GotoWindowButton3.setIconSize(LayoutStyle.Iconsize)

        self.Representation.Construct_Item(self.TabName, self.GotoWindowButton2)
        self.Representation.Add_SettingsButton(self.GotoWindowButton3)
        self.Representation.Update_Item(0, self.OutputCount, 0)

        for i in range(self.OutputCount):
            rou = self.Representation.Sync_with_Phisics_Device(i, 0, DKLed_Project_Layout_Types.OutputChannel, self, i)
            self.ConnectionPoints.append(rou)

    def Update_layout_entity(self):

        self.Representation.RepresentedEntitySpecs = Possible_Periph[1]
        self.Representation.Update_Item(0, self.OutputCount, 0)
        self.ConnectionPoints.clear()
        for i in range(self.OutputCount):
            rou = self.Representation.Sync_with_Phisics_Device(i, 0, DKLed_Project_Layout_Types.OutputChannel, self, i)
            self.ConnectionPoints.append(rou)

    def Add_extra_output(self):
        self.OutputCount = self.OutputCount + 1
        self.Update_layout_entity()

    def Delete_extra_output(self):
        if self.OutputCount>2:
            self.OutputCount = self.OutputCount - 1
            self.Update_layout_entity()

    def Respond_to_layout_Action(self, index, Action = DKLed_Project_Layout_Actions.NoAction, Parameter = None):
        pass

    def Connect_Item(self, Obj, Ind=0, IInd=0, Direct=False):
        self.LastConnected.append(Obj)
        self.LastConnectedIndex.append(Ind)
        self.LastConnectedInternalIndex.append(IInd)
        self.LastConnectedType.append(Obj.Type)
        #print("kbd connect to "+ str(Ind))
        #print("Button " + self.TabName + " list of connections length: "+ str(len(self.LastConnected)))
        return True

    def Disconnect(self, Obj, Direct = False):# disconnect object from this one
        #print("Button " + self.TabName + " disconnected")
        for i in range(len(self.LastConnected) - 1, -1, -1):
            if self.LastConnected[i] == Obj:
                self.LastConnected.pop(i)
                self.LastConnectedIndex.pop(i)
                self.LastConnectedInternalIndex.pop(i)
                self.LastConnectedType.pop(i)

    def Remove_Connection_Point(self, P=None):
        if P:
            if P in self.ConnectionPoints:
                self.ConnectionPoints.remove(P)

    def Destroy(self, Kind = None):
        if Kind == DKLed_Project_Layout_Types.Keyboard or Kind == DKLed_Project_Layout_Types.SimpleDevice:
            self.Representation.Empty_Item()
            print(self.TabName)
            self.ConnectionPoints.clear()
            self.LastConnected.clear()
            self.LastConnectedIndex.clear()
            self.LastConnectedInternalIndex.clear()
            self.LastConnectedType.clear()

            self.index = 0
            self.Connected = False
            self.TabName = ""
            self.Controller = None
            self.DKLed_Button_Instance = None #currently connected Controller input
            self.Last_DKLed_Button_Instance = None #stores the last value of currently connected input so it will be possibly reconnected
            self.Representation = None
            if type(self.GotoWindowButton2) is QtWidgets:
                self.GotoWindowButton2.deleteLater()
            self.GotoWindowButton2 = None
            if type(self.GotoWindowButton3) is QtWidgets:
                self.GotoWindowButton3.deleteLater()
            self.GotoWindowButton3 = None
            self.Type = DKLed_Project_Layout_Types.Nothing
            self.OutputCount = 1
            self.InputCount = 0
            if self in Keyboards:
                Keyboards.remove(self)
            print("    destroyed")
            del self
            #gc.collect()
            return True
        else:
            return False

class Encoder_Phisical:
    def __init__(self):
        self.index = 0
        self.Connected = False
        self.TabName = "Button"

        #dependable entities
        self.Controller = None
        self.DKLed_Button_Instance = None #currently connected Controller input
        self.Last_DKLed_Button_Instance = None #stores the last value of currently connected input so it will be possibly reconnected

        #Project layout widgets:
        self.Representation = None
        self.GotoWindowButton2 = None
        self.ConnectionPoints = []#pointers to EllipseCallable objects

        self.LastConnected = []
        self.LastConnectedIndex = []
        self.LastConnectedInternalIndex = []
        self.LastConnectedType = []
        self.Type = DKLed_Project_Layout_Types.Encoder
        self.OutputCount = 3
        self.InputCount = 0

    def create_layout_entity(self, LayoutPrj_):
        self.ConnectionPoints.clear()
        self.Representation = DKLed_Project_Layout_Item()
        self.Representation.RepresentedEntity = self
        self.Representation.RepresentedEntityType = DKLed_Project_Layout_Types.Encoder
        self.Representation.RepresentedEntitySpecs = Possible_Periph[2]
        self.Representation.LayoutControl = LayoutPrj_

        self.Representation.Construct_Item(self.TabName)
        self.Representation.Update_Item()

        for i in range(self.OutputCount):
            rou = self.Representation.Sync_with_Phisics_Device(i, 0, DKLed_Project_Layout_Types.OutputChannel, self, i)
            self.ConnectionPoints.append(rou)

    def Update_layout_entity(self):
        self.Representation.RepresentedEntitySpecs = Possible_Periph[2]
        self.Representation.Update_Item()
        self.ConnectionPoints.clear()
        for i in range(self.OutputCount):
            rou = self.Representation.Sync_with_Phisics_Device(i, 0, DKLed_Project_Layout_Types.OutputChannel, self, i)
            self.ConnectionPoints.append(rou)

    def Respond_to_layout_Action(self, index, Action = DKLed_Project_Layout_Actions.NoAction, Parameter = None):
        pass

    def Connect_Item(self, Obj, Ind=0, IInd=0, Direct=False):
        self.LastConnected.append(Obj)
        self.LastConnectedIndex.append(Ind)
        self.LastConnectedInternalIndex.append(IInd)
        self.LastConnectedType.append(Obj.Type)
        #print("Encoder " + self.TabName + " list of connections length: "+ str(len(self.LastConnected)))
        return True

    def Disconnect(self, Obj, Direct = False):# disconnect object from this one
        #print("Button " + self.TabName + " disconnected")
        for i in range(len(self.LastConnected) - 1, -1, -1):
            if self.LastConnected[i] == Obj:
                self.LastConnected.pop(i)
                self.LastConnectedIndex.pop(i)
                self.LastConnectedInternalIndex.pop(i)
                self.LastConnectedType.pop(i)

    def Remove_Connection_Point(self, P=None):
        if P:
            if P in self.ConnectionPoints:
                self.ConnectionPoints.remove(P)

    def Destroy(self, Kind = None):
        if Kind == DKLed_Project_Layout_Types.Encoder or Kind == DKLed_Project_Layout_Types.SimpleDevice:
            self.Representation.Empty_Item()
            print(self.TabName)
            self.ConnectionPoints.clear()
            self.LastConnected.clear()
            self.LastConnectedIndex.clear()
            self.LastConnectedInternalIndex.clear()
            self.LastConnectedType.clear()

            self.index = 0
            self.Connected = False
            self.TabName = ""
            self.Controller = None
            self.DKLed_Button_Instance = None #currently connected Controller input
            self.Last_DKLed_Button_Instance = None #stores the last value of currently connected input so it will be possibly reconnected
            self.Representation = None
            self.GotoWindowButton2 = None
            self.Type = DKLed_Project_Layout_Types.Nothing
            self.OutputCount = 1
            self.InputCount = 0
            if self in Keyboards:
                Keyboards.remove(self)
            print("    destroyed")
            del self
            #gc.collect()
            return True
        else:
            return False

class Relay_Phisical:
    def __init__(self):
        self.index = 0
        self.Connected = False
        self.TabName = "Relay"

        #dependable entities
        self.Controller = None
        self.DKLed_Button_Instance = None #currently connected Controller input
        self.Last_DKLed_Button_Instance = None #stores the last value of currently connected input so it will be possibly reconnected

        #Project layout widgets:
        self.Representation = None
        self.GotoWindowButton2 = None
        self.ConnectionPoints = []#pointers to EllipseCallable objects

        self.LastConnected = []
        self.LastConnectedIndex = []
        self.LastConnectedInternalIndex = []
        self.LastConnectedType = []
        self.Type = DKLed_Project_Layout_Types.Button
        self.OutputCount = 1
        self.InputCount = 1

    def create_layout_entity(self, LayoutPrj_):
        self.ConnectionPoints.clear()
        self.Representation = DKLed_Project_Layout_Item()
        self.Representation.RepresentedEntity = self
        self.Representation.RepresentedEntityType = DKLed_Project_Layout_Types.Relay
        self.Representation.RepresentedEntitySpecs = Possible_Periph[3]
        self.Representation.LayoutControl = LayoutPrj_

        self.Representation.Construct_Item(self.TabName)
        self.Representation.Update_Item()

        rou = self.Representation.Sync_with_Phisics_Device(0, 0, DKLed_Project_Layout_Types.InputChannel, self, 0)
        self.ConnectionPoints.append(rou)
        rou = self.Representation.Sync_with_Phisics_Device(0, 0, DKLed_Project_Layout_Types.OutputChannel, self, 0)
        self.ConnectionPoints.append(rou)

        Update_all_Output_Connection_Lists()

    def Update_layout_entity(self):
        print("Relay layout item updates")

        self.Representation.RepresentedEntitySpecs = Possible_Periph[3]
        self.Representation.Update_Item()
        self.ConnectionPoints.clear()
        rou = self.Representation.Sync_with_Phisics_Device(0, 0, DKLed_Project_Layout_Types.InputChannel, self, 0)
        self.ConnectionPoints.append(rou)
        rou = self.Representation.Sync_with_Phisics_Device(0, 0, DKLed_Project_Layout_Types.OutputChannel, self, 0)
        self.ConnectionPoints.append(rou)
        Update_all_Output_Connection_Lists()

    def Respond_to_layout_Action(self, index, Action = DKLed_Project_Layout_Actions.NoAction, Parameter = None):
        pass

    def Connect_Item(self, Obj, Ind=0, IInd=0, Direct=False):
        self.LastConnected.append(Obj)
        self.LastConnectedIndex.append(Ind)
        self.LastConnectedInternalIndex.append(IInd)
        self.LastConnectedType.append(Obj.Type)
        #print("Button " + self.TabName + " list of connections length: "+ str(len(self.LastConnected)))
        Update_all_Output_Connection_Lists()
        if isinstance(Obj, Relay_Phisical):
            Execute_Output_Connection_Lists_Update()
        return True

    def Disconnect(self, Obj, Direct = False):# disconnect object from this one
        #print("Relay " + self.TabName + " disconnected")
        for i in range(len(self.LastConnected) - 1, -1, -1):
            if self.LastConnected[i] == Obj:
                self.LastConnected.pop(i)
                self.LastConnectedIndex.pop(i)
                self.LastConnectedInternalIndex.pop(i)
                self.LastConnectedType.pop(i)
        Update_all_Output_Connection_Lists()

    def Remove_Connection_Point(self, P=None):
        if P:
            if P in self.ConnectionPoints:
                self.ConnectionPoints.remove(P)

    def Destroy(self, Kind= None):
        if Kind == DKLed_Project_Layout_Types.Relay or Kind == DKLed_Project_Layout_Types.SimpleDevice:
            self.Representation.Empty_Item()
            print(self.TabName)
            self.ConnectionPoints.clear()
            self.LastConnected.clear()
            self.LastConnectedIndex.clear()
            self.LastConnectedInternalIndex.clear()
            self.LastConnectedType.clear()

            self.index = 0
            self.Connected = False
            self.TabName = ""
            self.Controller = None
            self.DKLed_Button_Instance = None #currently connected Controller input
            self.Last_DKLed_Button_Instance = None #stores the last value of currently connected input so it will be possibly reconnected
            self.Representation = None
            self.GotoWindowButton2 = None
            self.Type = DKLed_Project_Layout_Types.Nothing
            self.OutputCount = 1
            self.InputCount = 0
            if self in Actuators:
                Actuators.remove(self)
            Update_all_Output_Connection_Lists()
            print("    destroyed")
            del self
            #gc.collect()
            return True
        else:
            return False

class Servo_Phisical:
    def __init__(self):
        self.index = 0
        self.Connected = False
        self.TabName = "Servo"

        #dependable entities
        self.Controller = None
        self.DKLed_Button_Instance = None #currently connected Controller input
        self.Last_DKLed_Button_Instance = None #stores the last value of currently connected input so it will be possibly reconnected

        #Project layout widgets:
        self.Representation = None
        self.GotoWindowButton1 = None
        self.GotoWindowButton2 = None
        self.GotoWindowButton3 = None
        self.ConnectionPoints = []#pointers to EllipseCallable objects

        self.LastConnected = []
        self.LastConnectedIndex = []
        self.LastConnectedInternalIndex = []
        self.LastConnectedType = []
        self.Type = DKLed_Project_Layout_Types.Button
        self.OutputCount = 1
        self.InputCount = 1

        self.ChosenPeriph = 4

    def create_layout_entity(self, LayoutPrj_):
        self.ConnectionPoints.clear()
        self.ChosenPeriph = 4
        self.Representation = DKLed_Project_Layout_Item()
        self.Representation.RepresentedEntity = self
        self.Representation.RepresentedEntityType = DKLed_Project_Layout_Types.Servo
        self.Representation.RepresentedEntitySpecs = Possible_Periph[self.ChosenPeriph]
        self.Representation.LayoutControl = LayoutPrj_

        self.GotoWindowButton1 = QtComboBoxUnScroll()
        self.GotoWindowButton1.setFixedSize(LayoutStyle.Iconsize.width() + LayoutStyle.ConnectionHOffset,
                                            int(LayoutStyle.Iconsize.height()/2))
        for pp in Possible_Periph:
            if pp.EntityType == DKLed_Project_Layout_Types.Servo:
                self.GotoWindowButton1.addItem(pp.subname)
        self.GotoWindowButton1.currentIndexChanged.connect(self.Update_Servo_Specs)

        self.GotoWindowButton2 = QtWidgets.QDial()
        self.GotoWindowButton2.setFixedSize(LayoutStyle.Iconsize)
        self.GotoWindowButton2.setStyleSheet(LayoutStyle.IconStyle)
        self.GotoWindowButton2.setNotchesVisible(True)
        self.GotoWindowButton2.setMinimum(0)
        self.GotoWindowButton2.setMaximum(Possible_Periph[self.ChosenPeriph].PWMDiscreet)
        self.GotoWindowButton2.setSingleStep(int(Possible_Periph[self.ChosenPeriph].PWMDiscreet/128))
        self.GotoWindowButton2.setPageStep(int(Possible_Periph[self.ChosenPeriph].PWMDiscreet/8))
        self.GotoWindowButton2.setNotchTarget(3.7)
        self.GotoWindowButton2.setSliderPosition(0)

        self.GotoWindowButton3 = QtWidgets.QLineEdit("0")
        self.GotoWindowButton3.setFixedSize(LayoutStyle.Iconsize.width(), int(LayoutStyle.Iconsize.height()/2))
        self.GotoWindowButton3.setValidator(IntValuesValidator)  # setInputMask("0000000")
        self.GotoWindowButton3.editingFinished.connect(self.Change_State_Val)
        self.GotoWindowButton2.valueChanged.connect(self.Change_State_Dial)

        self.Representation.Construct_Item(self.TabName)
        self.Representation.Add_SettingsButton(self.GotoWindowButton2, 0, 1.5)
        self.Representation.Add_SettingsButton(self.GotoWindowButton3, 0, 1.5)
        self.Representation.Add_SettingsButton(self.GotoWindowButton1, 0, -1)
        self.Representation.Update_Item()

        rou = self.Representation.Sync_with_Phisics_Device(0, 0, DKLed_Project_Layout_Types.InputChannel, self, 0)
        self.ConnectionPoints.append(rou)

        Update_all_Output_Connection_Lists()

    def Update_layout_entity(self):
        print("Servo layout item updates")

        self.Representation.RepresentedEntitySpecs = Possible_Periph[4]
        self.Representation.Update_Item()
        self.ConnectionPoints.clear()
        rou = self.Representation.Sync_with_Phisics_Device(0, 0, DKLed_Project_Layout_Types.InputChannel, self, 0)
        self.ConnectionPoints.append(rou)
        Update_all_Output_Connection_Lists()

    def Update_Servo_Specs(self):
        tx = self.GotoWindowButton1.itemText(self.GotoWindowButton1.currentIndex())
        print(self.TabName + " new spec name = "+tx)
        for i in range(len(Possible_Periph)):
            if Possible_Periph[i].subname == tx:
                print("new specs are: desc="+str(Possible_Periph[i].PWMDiscreet))
                val = self.GotoWindowButton2.value()
                val = int(val * (Possible_Periph[i].PWMDiscreet/Possible_Periph[self.ChosenPeriph].PWMDiscreet))
                self.ChosenPeriph = i
                self.GotoWindowButton2.setMaximum(Possible_Periph[self.ChosenPeriph].PWMDiscreet)
                self.GotoWindowButton2.setSliderPosition(val)
                break

    def Update_Servo_PWM(self, D, F, MinI, MaxI):
        pass

    def Change_State_Val(self):
        val = 0
        if self.GotoWindowButton3.text().isdigit():
            val = int(self.GotoWindowButton3.text())
        if val < 0:
            val = Possible_Periph[self.ChosenPeriph].PWMDiscreet
        if val >= Possible_Periph[self.ChosenPeriph].PWMDiscreet:
            val = Possible_Periph[self.ChosenPeriph].PWMDiscreet - 1
        self.GotoWindowButton2.setSliderPosition(val)

    def Change_State_Dial(self):
        val = self.GotoWindowButton2.value()
        self.GotoWindowButton3.setText(str(val))
        self.Change_State_apply()

    def Change_State(self, val=0):
        if val < 0:
            val = Possible_Periph[self.ChosenPeriph].PWMDiscreet
        if val >= Possible_Periph[self.ChosenPeriph].PWMDiscreet:
            val = Possible_Periph[self.ChosenPeriph].PWMDiscreet - 1
        self.GotoWindowButton2.setSliderPosition(val)

    def Change_State_apply(self):
        # print("Servo position = "+self.GotoWindowButton3.text())
        pass

    def Respond_to_layout_Action(self, index, Action = DKLed_Project_Layout_Actions.NoAction, Parameter = None):
        pass

    def Connect_Item(self, Obj, Ind=0, IInd=0, Direct=False):
        self.LastConnected.append(Obj)
        self.LastConnectedIndex.append(Ind)
        self.LastConnectedInternalIndex.append(IInd)
        self.LastConnectedType.append(Obj.Type)
        #print("Button " + self.TabName + " list of connections length: "+ str(len(self.LastConnected)))
        Update_all_Output_Connection_Lists()
        if isinstance(Obj, Relay_Phisical):
            Execute_Output_Connection_Lists_Update()
        return True

    def Disconnect(self, Obj, Direct = False):# disconnect object from this one
        #print("Relay " + self.TabName + " disconnected")
        for i in range(len(self.LastConnected) - 1, -1, -1):
            if self.LastConnected[i] == Obj:
                self.LastConnected.pop(i)
                self.LastConnectedIndex.pop(i)
                self.LastConnectedInternalIndex.pop(i)
                self.LastConnectedType.pop(i)
        Update_all_Output_Connection_Lists()

    def Remove_Connection_Point(self, P=None):
        if P:
            if P in self.ConnectionPoints:
                self.ConnectionPoints.remove(P)

    def Destroy(self, Kind= None):
        if Kind == DKLed_Project_Layout_Types.Relay or Kind == DKLed_Project_Layout_Types.SimpleDevice:
            self.Representation.Empty_Item()
            print(self.TabName)
            self.ConnectionPoints.clear()
            self.LastConnected.clear()
            self.LastConnectedIndex.clear()
            self.LastConnectedInternalIndex.clear()
            self.LastConnectedType.clear()

            self.index = 0
            self.Connected = False
            self.TabName = ""
            self.Controller = None
            self.DKLed_Button_Instance = None #currently connected Controller input
            self.Last_DKLed_Button_Instance = None #stores the last value of currently connected input so it will be possibly reconnected
            self.Representation = None
            if type(self.GotoWindowButton1) is QtWidgets:
                self.GotoWindowButton1.deleteLater()
            self.GotoWindowButton1 = None
            if type(self.GotoWindowButton2) is QtWidgets:
                self.GotoWindowButton2.deleteLater()
            self.GotoWindowButton2 = None
            if type(self.GotoWindowButton3) is QtWidgets:
                self.GotoWindowButton3.deleteLater()
            self.GotoWindowButton3 = None
            self.Type = DKLed_Project_Layout_Types.Nothing
            self.OutputCount = 1
            self.InputCount = 0
            if self in Actuators:
                Actuators.remove(self)
            Update_all_Output_Connection_Lists()
            print("    destroyed")
            del self
            #gc.collect()
            return True
        else:
            return False

class SPILed:
    def __init__(self, Sc, Ch, Prv=None, Nxt=None):
        self.Screen = Sc
        self.Channel = Ch
        self.PosX = 0
        self.PosY = 0

        self.Prev = Prv #previous and next LED in sequence
        self.Next = Nxt

    def GetCurrentColor(self):
        pass


class Screen:
    def __init__(self):

        self.index = 0
        self.Connected = False
        self.TabName = "none"

        self.Inputs = [] #list of inputs class instances (DKLed_Button_Instances)
        self.Outputs = [] #list of DKLed_Output_Instances
        self.OutputCount = 8 #just in case
        self.Scenarios = []
        self.Controller = None
        self.Controllers = []
        self.ActiveControllerIndex = 0
        self.CurrentHCDVersionIndex = 0 #index of current HCD version index in HCD_Version_list_Full array
        self.CurrentWorkingFlags = HCDCompartibleFlags.InEveryMode #modes of commands this controller is in
        #parameters
        self.MaxBrightness = 32
        self.CurrentBrightness = 32
        self.GammaCorrection = 1
        self.GammaBrightness = 255
        #extra windows:
        self.OptionalWindow = None
        #interface Widgets:
        self.ParentTabWidget = None
        self.ParentTabLayout = None
        self.Page = None
        self.controllerTypeSelect = None
        self.BrightnessLabel = None
        self.BrightnessValue = None

        self.mainlay = None
        self.ToolBar = None
        self.ToolBarEvent = DKLed_Screen_Layout_Actions.NoAction
        self.LEDMartixBar = None
        self.GotoWindowButton = None
        self.ChanelButtons = []
        self.ChanelButtonsMooveUp = None
        self.ChanelButtonsMooveDown = None
        self.ChanelButtonsMooveOffset = 0 #how many buttons are over Up limit and hidden
        self.SelectedChannel = 0
        self.ChanelButtonsLay = None
        self.AddChanelButton = None
        self.RemoveEmptyChannels = None
        self.SaveScreenMap = None
        self.InsertLEDMatrix = None
        self.AddLEDButton = None
        self.RemoveLEDButton = None
        self.ChannelsLayout = None #big scroll area

        #Project layout widgets:
        self.Representation = None
        self.GotoWindowButton2 = None
        self.GotoWindowButton3 = None
        self.GotoWindowButton4 = None
        self.ConnectionPoints = []

        self.LastConnected = []
        self.LastConnectedIndex = []
        self.LastConnectedInternalIndex = []
        self.LastConnectedType = []
        self.Type = DKLed_Project_Layout_Types.LedScreen

    def add_tab(self, Tab, name_="none"):
        self.TabName = name_
        self.Connected = False
        self.Page = QtWidgets.QWidget(None, )
        self.ParentTabLayout = Tab
        self.ParentTabWidget = Tab.ControllerTab #ui.tabWidget_2
        self.index = self.ParentTabWidget.addTab(self.Page, self.TabName)

    def change_tab_name(self, Newname):
        self.TabName = Newname
        self.ParentTabWidget.setTabText(self.index, self.TabName)

    def Return_from_Window(self):
        self.GotoWindowButton.setText(Interface_Names.Screen_GotoWindowButton_ToWindow)
        self.index = self.ParentTabWidget.insertTab(self.index, self.Page, self.TabName)
        self.ParentTabWidget.setCurrentIndex(self.index)
        self.Page.show()
        return True

    def Move_to_Window(self):
        if self.GotoWindowButton.text() == Interface_Names.Screen_GotoWindowButton_ToWindow:
            self.GotoWindowButton.setText(Interface_Names.Screen_GotoWindowButton_ToTab)
            if self.OptionalWindow is None:
                self.OptionalWindow = Functional_Window()
                #self.OptionalWindow.PrepareLayout()
            self.OptionalWindow.setWindowTitle(Interface_Names.Screen_OptionalWindow + self.TabName)
            self.OptionalWindow.setMinimumWidth(self.ParentTabLayout.Style.ControllerConfigAreaWidth + self.ParentTabLayout.Style.WindowFromTabExtraWidth)
            self.OptionalWindow.setMinimumHeight(self.ParentTabLayout.Style.ControllerConfigAreaHeight + self.ParentTabLayout.Style.WindowFromTabExtraHeight)
            self.OptionalWindow.ParentAction = self.Return_from_Window
            self.OptionalWindow.Install_Widget(self.Page)
        else:
            if self.OptionalWindow:
                self.OptionalWindow.close()

    def fill_tab(self):
        self.mainlay = QtWidgets.QVBoxLayout(self.Page)

        btn1 = QtWidgets.QPushButton(Interface_Names.Screen_RemoveScreenButton, self.Page)
        btn1.setFixedSize(60, 25)
        btn1.clicked.connect(self.Destroy)
        self.GotoWindowButton = QtWidgets.QPushButton(Interface_Names.Screen_GotoWindowButton_ToWindow, self.Page)
        self.GotoWindowButton.setFixedSize(90, 25)
        self.GotoWindowButton.clicked.connect(self.Move_to_Window)

        layh1 = QtWidgets.QHBoxLayout(self.Page)

        layh1.addStretch()
        layh1.addWidget(self.GotoWindowButton)
        layh1.addWidget(btn1)

        self.ChanelButtonsLay = QtWidgets.QVBoxLayout(self.Page)
        self.ChanelButtonsMooveUp = QtWidgets.QPushButton("^", self.Page)
        self.ChanelButtonsMooveDown = QtWidgets.QPushButton("v", self.Page)
        self.ChanelButtonsMooveUp.setFixedSize(25, 20)  # self.ParentTabLayout.Style.ToolIconsize)
        self.ChanelButtonsMooveUp.setStyleSheet("border-color: #909090; border-width: 2px;"
                                            "border-style: outset; border-radius: 6px;"
                                            "background-color: #f0f0f0")
        self.ChanelButtonsMooveDown.setFixedSize(25, 20)  # self.ParentTabLayout.Style.ToolIconsize)
        self.ChanelButtonsMooveDown.setStyleSheet("border-color: #909090; border-width: 2px;"
                                            "border-style: outset; border-radius: 6px;"
                                            "background-color: #f0f0f0")
        self.ChanelButtonsMooveUp.clicked.connect(self.Move_Channel_Layout_Bar_Down)
        self.ChanelButtonsMooveDown.clicked.connect(self.Move_Channel_Layout_Bar_Up)
        self.ChanelButtonsMooveUp.setEnabled(False)
        self.ChanelButtonsMooveDown.setEnabled(False)

        self.Update_Channel_Layout_Bar()

        self.ToolBar = QtWidgets.QToolBar(self.Page)
        self.ToolBar.setOrientation(Qt.Horizontal)
        self.ToolBar.setIconSize(self.ParentTabLayout.Style.ToolIconsize)
        self.AddChanelButton = self.ToolBar.addAction(self.ParentTabLayout.Icons.AddChannelToScreenIcon, Interface_Names.Screen_AddChannelButton)
        self.RemoveEmptyChannels = self.ToolBar.addAction(self.ParentTabLayout.Icons.RemoveEmptyChannelsFromScreenIcon, Interface_Names.Screen_RemoveEmptyChannelsButton)
        self.SaveScreenMap = self.ToolBar.addAction(self.ParentTabLayout.Icons.SaveScreenMapIcon, Interface_Names.Screen_SaveScreenMapButton)
        self.ToolBar.addSeparator()
        self.InsertLEDMatrix = self.ToolBar.addAction(self.ParentTabLayout.Icons.InsertLEDMatrixToScreenIcon,
                                                          Interface_Names.Screen_InsertLEDMatrixButton)
        self.InsertLEDMatrix.triggered.connect(self.Set_Layout_Action_AddMatrix)
        self.LEDMartixBar = QtComboBoxUnScroll(self.ToolBar)
        self.LEDMartixBar.setFixedSize(self.ParentTabLayout.Style.ToolIconsize.width()+24, self.ParentTabLayout.Style.ToolIconsize.height())
        self.LEDMartixBar.setIconSize(self.ParentTabLayout.Style.ToolIconsize)
        for i in range(16):
            self.LEDMartixBar.addItem(self.ParentTabLayout.Icons.LEDMatrixIcon[i], "")
        self.LEDMartixBar.setMaxVisibleItems(self.ParentTabLayout.Style.IconComboBoxMaxVisibleItems)
        self.ToolBar.addWidget(self.LEDMartixBar)
        self.AddLEDButton = self.ToolBar.addAction(self.ParentTabLayout.Icons.AddSingleLEDToScreenIcon, Interface_Names.Screen_AddLEDButton)
        self.AddLEDButton.triggered.connect(self.Set_Layout_Action_AppendLed)
        self.RemoveLEDButton = self.ToolBar.addAction(self.ParentTabLayout.Icons.RemoveSingleLEDFromScreenIcon, Interface_Names.Screen_RemoveLEDButton)
        self.RemoveLEDButton.triggered.connect(self.Set_Layout_Action_DeleteLed)

        layh2 = QtWidgets.QHBoxLayout(self.Page)
        layh2.addLayout(self.ChanelButtonsLay)
        self.ChannelsLayout = DKLed_Screen_Layout_Controls(layh2, self)
        self.ChannelsLayout.Update_ChannelCount(self.OutputCount)

        self.mainlay.addLayout(layh1)
        self.mainlay.addWidget(self.ToolBar)
        self.mainlay.addLayout(layh2)

        self.Page.setLayout(self.mainlay)
        self.ParentTabWidget.setCurrentIndex(self.index)
        self.Page.show()

    def Update_Channel_Layout_Bar(self):
        lc = self.ChanelButtonsLay.count()
        if lc > 0:
            for i in range(lc-1, -1, -1):
                self.ChanelButtonsLay.removeItem(self.ChanelButtonsLay.itemAt(i))
        self.ChanelButtonsLay.addWidget(self.ChanelButtonsMooveUp)
        for j in range(self.OutputCount):
            if len(self.ChanelButtons) <= j:
                self.ChanelButtons.append(None)
                self.ChanelButtons[j] = QtWidgets.QPushButton(hex(j)[2], self.Page)
                self.ChanelButtons[j].pressed.connect(self.Find_Pressed_Channel)
            self.ChanelButtons[j].setText((hex(j)[2]).lower())
            self.ChanelButtons[j].setFixedSize(25,25) #self.ParentTabLayout.Style.ToolIconsize)
            oc = "#909090"
            if self.SelectedChannel == j:
                ColorProbe.setHsv(int(j * 360 / self.OutputCount), 150, 250)
                oc = self.ParentTabLayout.Style.OutlineSelectedColor.name(QColor.HexRgb)
            else:
                ColorProbe.setHsv(int(j * 360 / self.OutputCount), 50, 250)
                oc = self.ParentTabLayout.Style.OutlineLightColor.name(QColor.HexRgb)
            cl = ColorProbe.name(QColor.HexRgb)
            self.ChanelButtons[j].setStyleSheet("border-color: "+oc+"; border-width: 2px;"
                                       "border-style: outset; border-radius: 6px;"
                                       "background-color: " + cl)

            self.ChanelButtons[j].setVisible(True)
            self.ChanelButtonsLay.addWidget(self.ChanelButtons[j])
        self.ChanelButtonsLay.addWidget(self.ChanelButtonsMooveDown)
        self.ChanelButtonsLay.addStretch()
        self.Update_Channel_Layout_Bar_Visibility()
        #print("Button lay elements"+str(self.ChanelButtonsLay.count()))

    def Update_Channel_Layout_Bar_Visibility(self):
        self.ChanelButtonsMooveUp.setEnabled(False)
        self.ChanelButtonsMooveDown.setEnabled(False)
        if self.OutputCount > self.ParentTabLayout.Style.MaximumScreenChannelButtonsVisible:
            if self.ChanelButtonsMooveOffset > 0:
                self.ChanelButtonsMooveDown.setEnabled(True)
            if self.OutputCount - self.ChanelButtonsMooveOffset > self.ParentTabLayout.Style.MaximumScreenChannelButtonsVisible:
                self.ChanelButtonsMooveUp.setEnabled(True)
            for j in range(self.OutputCount):
                if j>= self.ChanelButtonsMooveOffset and j < (self.ChanelButtonsMooveOffset + self.ParentTabLayout.Style.MaximumScreenChannelButtonsVisible):
                    self.ChanelButtons[j].setVisible(True)
                else:
                    self.ChanelButtons[j].setVisible(False)

    def Move_Channel_Layout_Bar_Up(self):
        if self.ChanelButtonsMooveOffset > 0:
            self.ChanelButtonsMooveOffset -= 1
            #print("screen channel offset = "+str(self.ChanelButtonsMooveOffset))
            self.Update_Channel_Layout_Bar_Visibility()

    def Move_Channel_Layout_Bar_Down(self):
        if self.ChanelButtonsMooveOffset < self.OutputCount - self.ParentTabLayout.Style.MaximumScreenChannelButtonsVisible:
            self.ChanelButtonsMooveOffset += 1
            #print("screen channel offset = " + str(self.ChanelButtonsMooveOffset))
            self.Update_Channel_Layout_Bar_Visibility()

    def Set_Layout_Action_None(self):
        self.InsertLEDMatrix.setIcon(self.ParentTabLayout.Icons.InsertLEDMatrixToScreenIcon)
        self.AddLEDButton.setIcon(self.ParentTabLayout.Icons.AddSingleLEDToScreenIcon)
        self.RemoveLEDButton.setIcon(self.ParentTabLayout.Icons.RemoveSingleLEDFromScreenIcon)

    def Set_Layout_Action_AddMatrix(self):
        self.Set_Layout_Action_None()
        if self.ToolBarEvent == DKLed_Screen_Layout_Actions.NewMatrix:
            self.ToolBarEvent = DKLed_Screen_Layout_Actions.NoAction
        else:
            self.ToolBarEvent = DKLed_Screen_Layout_Actions.NewMatrix
            self.InsertLEDMatrix.setIcon(self.ParentTabLayout.Icons.InsertLEDMatrixToScreenHighlightIcon)

    def Set_Layout_Action_AppendLed(self):
        self.Set_Layout_Action_None()
        if self.ToolBarEvent == DKLed_Screen_Layout_Actions.AppendNewLed:
            self.ToolBarEvent = DKLed_Screen_Layout_Actions.NoAction
        else:
            self.ToolBarEvent = DKLed_Screen_Layout_Actions.AppendNewLed
            self.AddLEDButton.setIcon(self.ParentTabLayout.Icons.AddSingleLEDToScreenHighlightIcon)

    def Set_Layout_Action_InsertLed(self):
        pass

    def Set_Layout_Action_DeleteLed(self):
        self.Set_Layout_Action_None()
        if self.ToolBarEvent == DKLed_Screen_Layout_Actions.DelLed:
            self.ToolBarEvent = DKLed_Screen_Layout_Actions.NoAction
        else:
            self.ToolBarEvent = DKLed_Screen_Layout_Actions.DelLed
            self.RemoveLEDButton.setIcon(self.ParentTabLayout.Icons.RemoveSingleLEDFromScreenHighlightIcon)

    def Find_Pressed_Channel(self):
        for j in range(self.OutputCount):
            #self.ChanelButtons[j].setStyleSheet("border-color: #909090; border-width: 2px; border-style: outset; border-radius: 6px; background-color: " + cl)
            oc = "#909090"
            if self.ChanelButtons[j].isDown():
                ColorProbe.setHsv(int(j * 360 / self.OutputCount), 150, 250)
                oc = self.ParentTabLayout.Style.OutlineSelectedColor.name(QColor.HexRgb)
                self.SelectedChannel = j
            else:
                ColorProbe.setHsv(int(j * 360 / self.OutputCount), 50, 250)
                oc = self.ParentTabLayout.Style.OutlineLightColor.name(QColor.HexRgb)
            cl = ColorProbe.name(QColor.HexRgb)
            self.ChanelButtons[j].setStyleSheet("border-color: "+oc+"; border-width: 2px;"
                                                "border-style: outset; border-radius: 6px;"
                                                "background-color: " + cl)
        #print("Channel pressed:"+str(self.SelectedChannel))

    def create_layout_entity(self,LayoutPrj_):
        self.ConnectionPoints.clear()
        self.Representation = DKLed_Project_Layout_Item()
        self.Representation.RepresentedEntity = self
        self.Representation.RepresentedEntityType = DKLed_Project_Layout_Types.LedScreen
        self.Representation.RepresentedEntitySpecs = Possible_Periph[3]
        self.Representation.LayoutControl = LayoutPrj_

        self.GotoWindowButton2 = QtWidgets.QPushButton("")
        self.GotoWindowButton2.setFixedSize(LayoutStyle.Iconsize)
        self.GotoWindowButton2.setStyleSheet(LayoutStyle.IconStyle)
        self.GotoWindowButton2.clicked.connect(self.Move_to_Window)
        self.GotoWindowButton2.setIcon(LayoutIcons.ScreenSettingsIcon)
        self.GotoWindowButton2.setIconSize(LayoutStyle.Iconsize)

        self.Representation.Construct_Item(self.TabName, self.GotoWindowButton2)
        self.Representation.Update_Item(self.OutputCount, self.OutputCount, 0)

        for i in range(self.OutputCount):
            rou = self.Representation.Sync_with_Phisics_Device(i, 0, DKLed_Project_Layout_Types.InputChannel, self, i)
            self.ConnectionPoints.append(rou)
            rou = self.Representation.Sync_with_Phisics_Device(i, 0, DKLed_Project_Layout_Types.OutputChannel, self, i)
            self.ConnectionPoints.append(rou)

        Update_all_Output_Connection_Lists()

    def Update_layout_entity(self):

        print("Screen layout item updates")

        self.Representation.RepresentedEntitySpecs = Possible_Periph[3]
        self.Representation.Update_Item(self.OutputCount, self.OutputCount, 0)
        self.ConnectionPoints.clear()
        for i in range(self.OutputCount):
            rou = self.Representation.Sync_with_Phisics_Device(i, 0, DKLed_Project_Layout_Types.InputChannel, self, i)
            self.ConnectionPoints.append(rou)
            rou = self.Representation.Sync_with_Phisics_Device(i, 0, DKLed_Project_Layout_Types.OutputChannel, self, i)
            self.ConnectionPoints.append(rou)
        Update_all_Output_Connection_Lists()

    def Connect_Item(self, Obj, Ind=0, IInd=0, Direct=False):
        self.LastConnected.append(Obj)
        self.LastConnectedIndex.append(Ind)
        self.LastConnectedInternalIndex.append(IInd)
        self.LastConnectedType.append(Obj.Type)
        #print("Screen Output " + str(IInd) + " connected through layout with " + Obj.TabName + ", index #" + str(Ind))
        Update_all_Output_Connection_Lists()
        if isinstance(Obj, Screen):
            Execute_Output_Connection_Lists_Update()
        return True

    def Disconnect(self, Obj, Direct = False):# disconnect object from this one
        for i in range(len(self.LastConnected) - 1, -1, -1):
            if self.LastConnected[i] == Obj:
                self.LastConnected.pop(i)
                self.LastConnectedIndex.pop(i)
                self.LastConnectedInternalIndex.pop(i)
                self.LastConnectedType.pop(i)

    def Respond_to_layout_Action(self, index, Action =  DKLed_Project_Layout_Actions.NoAction, Parameter = None):
        pass

    def Remove_Connection_Point(self, P=None):
        if P:
            if P in self.ConnectionPoints:
                self.ConnectionPoints.remove(P)

    def Destroy(self, Kind = None):
        print ("destroy " + self.TabName)
        print("it's indestructable yet")
        return False


def Update_Command_Group_Combobox(Qbox, PQbox):
    ind = 0
    if PQbox:
        ind = PQbox.currentIndex()
    while Qbox.itemText(0):
        Qbox.removeItem(0)
    for i in range(len(HCD_Version_association_list[ind])):
        Qbox.addItem(HCD_Command_Groups[HCD_Version_association_list[ind][i]].GroupName)

def Update_Command_List_Combobox(Qbox, PQbox, empt_="- - -"):
    ind = 0
    if PQbox:
        ind = PQbox.currentIndex()
    while Qbox.itemText(0):
        Qbox.removeItem(0)
    for i in range(len(HCD_Command_Groups[ind].CommandList)):
        Qbox.addItem(HCD_Command_Groups[ind].CommandList[i].name)
    if len(HCD_Command_Groups[ind].CommandList) == 0:
        Qbox.addItem(empt_)
    #Update_Command_List_Tab_CommandList()

def Update_Command_List_Tab_CommandGroup():
    Update_Command_Group_Combobox(ui.Command_Section, ui.Command_Ver)

def Update_Command_List_Tab_CommandList():
    Update_Command_List_Combobox(ui.Command_itself, ui.Command_Section)

def Show_Command_Window():
    HCD_Command_Window.Call_Window(True)
    if len(Controllers) > 0:
        HCD_Command_Window.CurrentController = Controllers[0]
    #print("controllers len = "+str(len(Controllers)))

def Add_Controller_Tab():
    add_dialog = QtWidgets.QInputDialog() #ui
    add_dialog.setCancelButtonText(Interface_Names.Dialogue_AddControllerTab_CancelButtonText)
    add_dialog.setOkButtonText(Interface_Names.Dialogue_AddControllerTab_OkButtonText)
    add_dialog.setWindowTitle(Interface_Names.Dialogue_AddControllerTab_WindowTitle)
    add_dialog.setLabelText(Interface_Names.Dialogue_AddControllerTab_LabelText)
    name_ = Interface_Names.Dialogue_AddControllerTab_DefaultName + str(len(Controllers) + 1)
    add_dialog.setInputMode(QInputDialog.TextInput)
    add_dialog.setTextValue(name_)
    ok = add_dialog.exec()
    if ok:
        Controllers2 = Controller()
        Controllers.append(Controllers2)
        if add_dialog.textValue() == "":
            add_dialog.setTextValue(Controllers2.TabName)
        Controllers2.add_tab(ControllerSettingsTab, add_dialog.textValue())
        Controllers2.fill_tab()
        Controllers2.create_layout_entity(LayoutPrj)
        Execute_Output_Connection_Lists_Update()
    add_dialog.destroy(True, False)
    return Controllers2

def Add_Button_Tab():
    global KeyboardsIndex
    add_dialog = QtWidgets.QInputDialog() #ui
    add_dialog.setCancelButtonText(Interface_Names.Dialogue_AddButtonTab_CancelButtonText)
    add_dialog.setOkButtonText(Interface_Names.Dialogue_AddButtonTab_OkButtonText)
    add_dialog.setWindowTitle(Interface_Names.Dialogue_AddButtonTab_WindowTitle)
    add_dialog.setLabelText(Interface_Names.Dialogue_AddButtonTab_LabelText)
    name_ = Interface_Names.Dialogue_AddButtonTab_DefaultName + str(KeyboardsIndex)
    add_dialog.setInputMode(QInputDialog.TextInput)
    add_dialog.setTextValue(name_)
    ok = add_dialog.exec()
    Controllers2 = None
    if ok:
        KeyboardsIndex += 1
        Controllers2 = Button_Phisical()
        Keyboards.append(Controllers2)
        if add_dialog.textValue() == "":
            add_dialog.setTextValue(Controllers2.TabName)
        Controllers2.TabName = add_dialog.textValue()
        Controllers2.create_layout_entity(LayoutPrj)
    add_dialog.destroy(True, False)
    return Controllers2

def Add_Keyboard_Tab():
    global KeyboardsIndex
    add_dialog = QtWidgets.QInputDialog() #ui
    add_dialog.setCancelButtonText(Interface_Names.Dialogue_AddKeyboardTab_CancelButtonText)
    add_dialog.setOkButtonText(Interface_Names.Dialogue_AddKeyboardTab_OkButtonText)
    add_dialog.setWindowTitle(Interface_Names.Dialogue_AddKeyboardTab_WindowTitle)
    add_dialog.setLabelText(Interface_Names.Dialogue_AddKeyboardTab_LabelText)
    name_ = Interface_Names.Dialogue_AddKeyboardTab_DefaultName + str(KeyboardsIndex)
    add_dialog.setInputMode(QInputDialog.TextInput)
    add_dialog.setTextValue(name_)
    ok = add_dialog.exec()
    Controllers2 = None
    if ok:
        KeyboardsIndex +=1
        Controllers2 = Keyboard_Phisical()
        Keyboards.append(Controllers2)
        if add_dialog.textValue() == "":
            add_dialog.setTextValue(Controllers2.TabName)
        Controllers2.TabName = add_dialog.textValue()
        Controllers2.create_layout_entity(LayoutPrj)
    add_dialog.destroy(True, False)
    return Controllers2

def Add_Encoder_Tab():
    global KeyboardsIndex
    add_dialog = QtWidgets.QInputDialog() #ui
    add_dialog.setCancelButtonText(Interface_Names.Dialogue_AddEncoderTab_CancelButtonText)
    add_dialog.setOkButtonText(Interface_Names.Dialogue_AddEncoderTab_OkButtonText)
    add_dialog.setWindowTitle(Interface_Names.Dialogue_AddEncoderTab_WindowTitle)
    add_dialog.setLabelText(Interface_Names.Dialogue_AddEncoderTab_LabelText)
    name_ = Interface_Names.Dialogue_AddEncoderTab_DefaultName + str(KeyboardsIndex)
    add_dialog.setInputMode(QInputDialog.TextInput)
    add_dialog.setTextValue(name_)
    ok = add_dialog.exec()
    Controllers2 = None
    if ok:
        KeyboardsIndex +=1
        Controllers2 = Encoder_Phisical()
        Keyboards.append(Controllers2)
        if add_dialog.textValue() == "":
            add_dialog.setTextValue(Controllers2.TabName)
        Controllers2.TabName = add_dialog.textValue()
        Controllers2.create_layout_entity(LayoutPrj)
    add_dialog.destroy(True, False)
    return Controllers2

def Add_Relay_Tab():
    global ActuatorsIndex
    add_dialog = QtWidgets.QInputDialog() #ui
    add_dialog.setCancelButtonText(Interface_Names.Dialogue_AddRelayTab_CancelButtonText)
    add_dialog.setOkButtonText(Interface_Names.Dialogue_AddRelayTab_OkButtonText)
    add_dialog.setWindowTitle(Interface_Names.Dialogue_AddRelayTab_WindowTitle)
    add_dialog.setLabelText(Interface_Names.Dialogue_AddRelayTab_LabelText)
    name_ = Interface_Names.Dialogue_AddRelayTab_DefaultName + str(ActuatorsIndex)
    add_dialog.setInputMode(QInputDialog.TextInput)
    add_dialog.setTextValue(name_)
    ok = add_dialog.exec()
    Controllers2 = None
    if ok:
        ActuatorsIndex +=1
        Controllers2 = Relay_Phisical()
        Actuators.append(Controllers2)
        if add_dialog.textValue() == "":
            add_dialog.setTextValue(Controllers2.TabName)
        Controllers2.TabName = add_dialog.textValue()
        Controllers2.create_layout_entity(LayoutPrj)
        Execute_Output_Connection_Lists_Update()
    add_dialog.destroy(True, False)
    return Controllers2

def Add_Servo_Tab():
    global ActuatorsIndex
    add_dialog = QtWidgets.QInputDialog() #ui
    add_dialog.setCancelButtonText(Interface_Names.Dialogue_AddServoTab_CancelButtonText)
    add_dialog.setOkButtonText(Interface_Names.Dialogue_AddServoTab_OkButtonText)
    add_dialog.setWindowTitle(Interface_Names.Dialogue_AddServoTab_WindowTitle)
    add_dialog.setLabelText(Interface_Names.Dialogue_AddServoTab_LabelText)
    name_ = Interface_Names.Dialogue_AddServoTab_DefaultName + str(ActuatorsIndex)
    add_dialog.setInputMode(QInputDialog.TextInput)
    add_dialog.setTextValue(name_)
    ok = add_dialog.exec()
    Controllers2 = None
    if ok:
        ActuatorsIndex +=1
        Controllers2 = Servo_Phisical()
        Actuators.append(Controllers2)
        if add_dialog.textValue() == "":
            add_dialog.setTextValue(Controllers2.TabName)
        Controllers2.TabName = add_dialog.textValue()
        Controllers2.create_layout_entity(LayoutPrj)
        Execute_Output_Connection_Lists_Update()
    add_dialog.destroy(True, False)
    return Controllers2

def Add_Screen_Tab():
    global ActuatorsIndex
    add_dialog = QtWidgets.QInputDialog() #ui
    add_dialog.setCancelButtonText(Interface_Names.Dialogue_AddScreenTab_CancelButtonText)
    add_dialog.setOkButtonText(Interface_Names.Dialogue_AddScreenTab_OkButtonText)
    add_dialog.setWindowTitle(Interface_Names.Dialogue_AddScreenTab_WindowTitle)
    add_dialog.setLabelText(Interface_Names.Dialogue_AddScreenTab_LabelText)
    name_ = Interface_Names.Dialogue_AddScreenTab_DefaultName + str(ActuatorsIndex)
    add_dialog.setInputMode(QInputDialog.TextInput)
    add_dialog.setTextValue(name_)
    ok = add_dialog.exec()
    Controllers2 = None
    if ok:
        ActuatorsIndex += 1
        Controllers2 = Screen()
        Actuators.append(Controllers2)
        if add_dialog.textValue() == "":
            add_dialog.setTextValue(Controllers2.TabName)
        Controllers2.add_tab(ScreensRoutingTab, add_dialog.textValue())
        Controllers2.fill_tab()
        Controllers2.create_layout_entity(LayoutPrj)
        Execute_Output_Connection_Lists_Update()
    add_dialog.destroy(True, False)
    return Controllers2

def Connect_Button_Dialog():
    add_dialog = QtWidgets.QInputDialog() #ui
    add_dialog.setCancelButtonText("Discard")
    add_dialog.setOkButtonText("Connect")
    add_dialog.setWindowTitle("Connect to button")
    add_dialog.setLabelText("Connect input to:")
    NewBtnLabel = "New button"
    Obj = None
    SL = []
    tx = ""
    for k in range(len(Keyboards)):
        if Keyboards[k].Type == DKLed_Project_Layout_Types.Keyboard:
            for j in range(Keyboards[k].OutputCount):
                if len(Keyboards[k].ConnectionPoints[j].Connects)<1:
                    SL.append(Keyboards[k].TabName + ", output "+str(j))
        elif Keyboards[k].Type == DKLed_Project_Layout_Types.Encoder:
            if len(Keyboards[k].ConnectionPoints[0].Connects)<1:
                SL.append(Keyboards[k].TabName + ", Button output")
        elif Keyboards[k].Type == DKLed_Project_Layout_Types.Button:
            if len(Keyboards[k].LastConnected)<1:
                tx = Keyboards[k].TabName
                SL.append(tx)
            else:
                if Keyboards[k].LastConnected[0] == None:
                    tx = Keyboards[k].TabName
                    SL.append(tx)
            #print("append to free button list:"+tx)
            if tx == NewBtnLabel:
                NewBtnLabel = NewBtnLabel + " "
    SL.append(NewBtnLabel)
    add_dialog.setComboBoxItems(SL)
    add_dialog.setOption(QInputDialog.UseListViewForComboBoxItems, True)
    ok = add_dialog.exec()
    ind = 0
    if ok:
        tx = add_dialog.textValue()
        if tx == NewBtnLabel:
            Obj = Add_Button_Tab()
        else:
            for k in range(len(Keyboards)):
                if Keyboards[k].Type == DKLed_Project_Layout_Types.Keyboard:
                    for j in range(Keyboards[k].OutputCount):
                        if tx == (Keyboards[k].TabName + ", output " + str(j)):
                            print(tx)
                            Obj = Keyboards[k]
                            ind = j
                            break
                    else:
                        continue
                    break
                elif Keyboards[k].Type == DKLed_Project_Layout_Types.Encoder:
                    if len(Keyboards[k].ConnectionPoints[0].Connects) < 1:
                        if tx == Keyboards[k].TabName + ", Button output":
                            print(tx)
                            Obj = Keyboards[k]
                            break
                elif Keyboards[k].Type == DKLed_Project_Layout_Types.Button:
                    if len(Keyboards[k].LastConnected) < 1:
                        if tx == Keyboards[k].TabName:
                            print(tx)
                            Obj = Keyboards[k]
                            break
                    else:
                        if Keyboards[k].LastConnected[0] == None:
                            if tx == Keyboards[k].TabName:
                                print(tx)
                                Obj = Keyboards[k]
                                break
    add_dialog.destroy(True, False)
    return Obj, ind

def Connect_Encoder_Dialog():
    add_dialog = QtWidgets.QInputDialog() #ui
    add_dialog.setCancelButtonText("Discard")
    add_dialog.setOkButtonText("Connect")
    add_dialog.setWindowTitle("Connect to Encoder")
    add_dialog.setLabelText("Connect to:")
    NewBtnLabel = "New Encoder"
    Obj = None
    SL = []
    tx = ""
    for k in range(len(Keyboards)):
        if Keyboards[k].Type == DKLed_Project_Layout_Types.Encoder:
            if len(Keyboards[k].ConnectionPoints[1].Connects)<1:
                tx = Keyboards[k].TabName
                SL.append(tx)
            if tx == NewBtnLabel:
                NewBtnLabel = NewBtnLabel + " "
    SL.append(NewBtnLabel)
    add_dialog.setComboBoxItems(SL)
    add_dialog.setOption(QInputDialog.UseListViewForComboBoxItems, True)
    ok = add_dialog.exec()
    ind = 0
    if ok:
        tx = add_dialog.textValue()
        if tx == NewBtnLabel:
            Obj = Add_Encoder_Tab()
        else:
            for k in range(len(Keyboards)):
                if Keyboards[k].Type == DKLed_Project_Layout_Types.Encoder:
                    if len(Keyboards[k].ConnectionPoints[1].Connects) < 1:
                        if tx == Keyboards[k].TabName:
                            print("Connect Encoder to "+tx)
                            Obj = Keyboards[k]
                            ind = k
                            break

    add_dialog.destroy(True, False)
    return Obj, ind

def Update_all_Output_Connection_Lists():
    global OutputsActuatorListsAwaitUpdation
    OutputsActuatorListsAwaitUpdation = True

def Execute_Output_Connection_Lists_Update(ex=False, Obj=None):
    global OutputsActuatorListsAwaitUpdation
    if OutputsActuatorListsAwaitUpdation or ex:
        #print("Start updating output lists                ---")
        for CO in DKLed_Output_Instance_List:
            if CO != Obj:
                CO.Update_Actuators_List()
        OutputsActuatorListsAwaitUpdation = False




# Application init is moved to Interface_Icons
# as Python requires all the fields of Interface_Icons to be already initialized
# but it is possible only after application initiation
# (icons can be loaded only after that)
# and as Interface_Icons is located in Names file
# because creating it here makes a cyclical import
# (Main imports something from ProjectLayout that imports something from Main)
# it is stored there

#app = QtWidgets.QApplication([])
#Interface_Icons = DKLed_Layout_Icons2()
ui = uic.loadUi("Library/Main_window.ui")
ui.setWindowTitle("DKLed Editor")

Main_Window = Functional_Window()
Main_Window.ParentAction = Close_All_Windows
Main_Window.setWindowTitle("DKLed Editor")


HCD_Command_Window = HCD_Command_Window_Dialog()
#HCD_Command_Window.Call_Window(True)

def Set_Tooltip_Text(i, enabl=Hint_Contents_Show):
    if (enabl < 3) and (i < len(Hint_Contents_list)):
        return Hint_Contents_list[enabl][i]
    return ""

def Update_Tooltip_Text_over_application(enabl=Hint_Contents_Show):
    for i in range(len(Hinted_widgets_list)):
        if hasattr(Hinted_widgets_list[i][0], 'setTabToolTip') and (len(Hinted_widgets_list[i]) == 3):
            Hinted_widgets_list[i][0].setTabToolTip(Hinted_widgets_list[i][1],
                                                    Set_Tooltip_Text(Hinted_widgets_list[i][2], enabl))
        elif hasattr(Hinted_widgets_list[i][0], 'setToolTip') and (len(Hinted_widgets_list[i]) == 2):
            Hinted_widgets_list[i][0].setToolTip(Set_Tooltip_Text(Hinted_widgets_list[i][1], enabl))


Controllers = []
PeripheryControllers = []
Keyboards = []
KeyboardsIndex = 0
Actuators = []
ActuatorsIndex = 0
DKLed_Output_Instance_List = []
OutputsActuatorListsAwaitUpdation = False

UIdFormat = QRegularExpression("[0-9,A-Z.`~!@#$%^&<>?]{4}")
UIdValidator = QRegularExpressionValidator(UIdFormat)
IntValuesValidator = QIntValidator(0, 10000000)

LayoutPrj = DKLed_Project_Layout_Controls(ui.Tab2horizontalLayout)
LayoutStyle = LayoutPrj.Style
LayoutIcons = LayoutPrj.Icons

ColorProbe = QColor()

ControllerSettingsTab = DKLed_Controller_Controls(ui.verticalLayout_3)
ScreensRoutingTab = DKLed_Screens_Controls(ui.ScreenEditLayout)



Create_Possible_Controller_List()
Create_Possible_Periph_List()
Create_Possible_Commands_List(HCD_Command_Window)
for i in range(len(HCD_Version_list_Full)):
    ui.Command_Ver.addItem(HCD_Version_list_Full[i])
ui.Command_Ver.currentIndexChanged.connect(Update_Command_List_Tab_CommandGroup)
ui.Command_Section.currentIndexChanged.connect(Update_Command_List_Tab_CommandList)
ui.Command_Add_Button.clicked.connect(Show_Command_Window)

Update_Command_List_Tab_CommandGroup()

Create_UI_Captions_and_Hint_Contents_list()

Hinted_widgets_list.append([ui.Main_Tabs, 0, 0])
Hinted_widgets_list.append([ui.Main_Tabs, 1, 1])
Update_Tooltip_Text_over_application()



Controllers.append(None)
Controllers[0] = Controller()
Controllers[0].TabName = "Default"
Controllers[0].add_tab(ControllerSettingsTab, Controllers[0].TabName)
Controllers[0].fill_tab()
Controllers[0].create_layout_entity(LayoutPrj)

ControllerSettingsTab.ActionAddController.triggered.connect(Add_Controller_Tab)

ScreensRoutingTab.ActionAddScreen.triggered.connect(Add_Screen_Tab)

LayoutPrj.ActionAddController.triggered.connect(Add_Controller_Tab)
LayoutPrj.ActionAddButton.triggered.connect(Add_Button_Tab)
LayoutPrj.ActionAddKeyboard.triggered.connect(Add_Keyboard_Tab)
LayoutPrj.ActionAddEncoder.triggered.connect(Add_Encoder_Tab)
LayoutPrj.ActionAddRelay.triggered.connect(Add_Relay_Tab)
LayoutPrj.ActionAddServo.triggered.connect(Add_Servo_Tab)
LayoutPrj.ActionAddScreen.triggered.connect(Add_Screen_Tab)
LayoutPrj.ActionDeleteConnection.triggered.connect(Execute_Output_Connection_Lists_Update)
LayoutPrj.ActionDeleteLayoutItem.triggered.connect(Execute_Output_Connection_Lists_Update)

Main_Window.Install_Widget(ui)

#ui.show()
#app.exec()
Interface_Icons.app.exec()
