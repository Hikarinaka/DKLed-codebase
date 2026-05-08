
from PyQt5 import QtWidgets, uic
from PyQt5.QtGui import QIntValidator, QRegularExpressionValidator, QCloseEvent
from PyQt5.QtCore import QRegularExpression, Qt
from PyQt5.QtWidgets import QInputDialog
from PyQt5.QtSerialPort import QSerialPort, QSerialPortInfo
from PyQt5.QtCore import QIODevice
from enum import Enum
import sys

class HCDFirstParameter(Enum):
    Nothing = 0
    Int = 1             #  int (usually in HEX)
    Percentage = 2
    String = 3          #  free string
    FormattedStr = 4    #  string of special characters
    FileIndex = 5          #  File index
    InputEvent = 6      #  input events (buttons and virtual buttons)
    Button = 7          #  real button index
    VButton = 8         #  virtual button index
    Port = 9            #  Port (any)
    UART = 10            #  Port (not USB)
    Output = 11         #  Output (LED, Servo, etc) index
    SpiLED = 12         #  smartLED output index
    Servo = 13          #  Servo (PWM) output index
    Dependant = 14      #  controller aware
    DependantInt = 15   #  controller aware number (max and min are set by controller parameters)

    #controller aware enums:
    TotalOutputs = 21   #  number of outputs in this controller
    TotalLEDOutputs = 22
    TotalServoOutputs = 23

    TotalInputs = 31
    TotalEncoderInputs = 32
    TotalNotMappedButtons = 33
    TotalEvents = 34

    #Working settings
    SettingPWMDiscreet = 41 #Discretization for PWM
    DefaultingPWMDiscreet = 42 #sets PWM Discretization to default
    SettingMaxBrightness = 43
    DefaultingMaxBrightness = 44
    SettingFAFrameNumber = 45
    SettingFAFrameSize = 46


class HCDCompartibleFlags:
    #flags for indicating working mode
    #incompartible modes are modes that can not be triggered in controller at the same time
    #so command can work with both of them, but that modes can not be required or in stop list at the same time
    InEveryMode = 0xFFFF #all modes compartible
    InNoMode = 0 #no mode compartible
    InLedMode = 1 #this command works in every mode that supports LEDs
    InServoMode = 2 #this command works in every mode that supports servos
    #fast Video mode is incompartible with servo mode, so InServoMode = NOT Fast Video mode
    InFastVideomode = 4
    InFileMode = 8 #this command overrides with Led, Servo and Fast modes
    InAnyButFileMode = 16 #incompartible with file mode

    InSingleInputMode = 32
    InCombinationInputMode = 64 #Single Input and Combination modes are incompartible

    InTextReceiveMode = 128 #ports in text receiving mode, incompartible with Byte receiving mode
    InByteReceivingMode = 256 #post in byte receiving mode
    InUSBCommunicationMode = 512

    InWS2812Compartibility = 1024
    InSK6812Compartibility = 2048

class DKLed_Specs:
    def __init__(self, name_="I5O8U2", outs_=8, btns_=5, totalbtn_=64, encd_=1, fastvid_=True, btncomb_=False):
        self.name = name_
        self.out = outs_
        self.btn = btns_
        self.totalbtn = totalbtn_
        self.encd = encd_
        self.uart = 0
        self.mic = 0
        self.btncomb = btncomb_ #true if button combination is possible
        #self.wifi = 0
        #self.blth = 0
        self.fastvid = fastvid_
        self.devices = []
        self.uartModesIN = []
        self.uartModesOUT = []
        self.uartTypes = []
        self.CodeVersion = ""
        self.USBSupport = False
        self.DefaultPWMDiscreet = 512
        self.DefaultMaxBrightness = 32
        global MaximumNumberOf_Outputs
        global MaximumNumberOf_ButtonEvents
        global MaximumNumberOf_Buttons
        global MaximumNumberOf_Encoders
        if MaximumNumberOf_Outputs < outs_:
            MaximumNumberOf_Outputs = outs_
        if MaximumNumberOf_Buttons < btns_:
            MaximumNumberOf_Buttons = btns_
        if MaximumNumberOf_ButtonEvents < totalbtn_:
            MaximumNumberOf_ButtonEvents = totalbtn_
        if MaximumNumberOf_Encoders < encd_:
            MaximumNumberOf_Encoders = encd_

    def add_device(self, *devs):
        for dev in devs:
            self.devices.append(dev)

    def uart_modes_IN(self, PType, u, *modes):
        if u < 1:
            u = 1
        if u > self.uart:
            self.uart += 1
            u = self.uart
        arr = []
        for mode in modes:
            arr.append(mode)
        if len(self.uartTypes) < self.uart:
            self.uartTypes.append(None)
        self.uartTypes[u-1] = PType
        if len(self.uartModesIN) < self.uart:
            self.uartModesIN.append(None)
        elif self.uartModesIN[u - 1]:
            self.uartModesIN[u - 1].clear()
        if len(self.uartModesOUT) < self.uart:
            self.uartModesOUT.append(None)
            self.uartModesOUT[self.uart - 1] = []
        self.uartModesIN[u - 1] = arr
        global MaximumNumberOf_Ports
        if MaximumNumberOf_Ports < u:
            MaximumNumberOf_Ports = u

    def uart_modes_OUT(self, PType, u, *modes):
        if u < 1:
            u = 1
        if u > self.uart:
            self.uart += 1
            u = self.uart
        arr = []
        for mode in modes:
            arr.append(mode)
        if len(self.uartTypes) < self.uart:
            self.uartTypes.append(None)
        self.uartTypes[u-1] = PType
        if len(self.uartModesIN) < self.uart:
            self.uartModesIN.append(None)
            self.uartModesIN[self.uart-1] = []
        if len(self.uartModesOUT) < self.uart:
            self.uartModesOUT.append(None)
        elif self.uartModesOUT[u - 1]:
            self.uartModesOUT[u - 1].clear()
        self.uartModesOUT[u - 1] = arr
        global MaximumNumberOf_Ports
        if MaximumNumberOf_Ports < u:
            MaximumNumberOf_Ports = u

class HCD_Command_Group:
    def __init__(self):
        self.GroupName = "" #what to show to user
        self.GroupDescription = "" #description of the group - what it does
        self.HCDVersion = [] #list of supported versions of HCD controllers
        self.GroupIndex = 0
        self.GroupReference = "" #the "Command group" field
        self.CompartibleFlags = HCDCompartibleFlags.InEveryMode #modes that this command group can appear
        self.RequiredFlags = HCDCompartibleFlags.InNoMode #modes that this commnad group can not be visible
        self.CommandList = [] #links to command objects

class HCD_Command:
    def __init__(self):
        self.group = None #associated HCD_Command_Group instance
        self.name = ""
        self.description = "" #for some text explaining how the command works
        self.stackable = False #if the command can be used inside button event
        self.CompartibleFlags = HCDCompartibleFlags.InEveryMode #what flags may be on for this command to be available
        self.RequiredFlags = HCDCompartibleFlags.InNoMode #what flags must be on for this command to be available
        self.SetsFlags = HCDCompartibleFlags.InNoMode #which flag will this command set in the process
        self.ClearsFlags = HCDCompartibleFlags.InNoMode #which flags to clear
        self.text0 = "" #the starting text of a command for (ex "M96Q" in M96Q 12 P 0011)
        self.text1 = "" #the second parameter declaration (ex "P" in M96Q 12 P 0011)
        self.text2 = "" #last text if needed
        self.Parameter1Type = HCDFirstParameter.Nothing
        self.Parameter1Depends = HCDFirstParameter.Nothing
        self.Parameter1ChangesValue = HCDFirstParameter.Nothing
        self.Parameter1Name = ""
        self.Parameter1Signable = False
        self.Parameter1Count = 0 #0 - controller specs dependant, 1+ - number
        self.Parameter1Length = 0 #0 - any, 1+ - number of hex symbols
        self.Parameter1Entries = [] #for int parameters it's min and max values
        self.Parameter1EntryNames = []

        self.CommandWindowDialog = None

    def Form_Command_text(self, Controller, par1cnt, par2cnt):
        comm_ = self.text0
        if self.Parameter1Type == HCDFirstParameter.Int:
            if type(par1cnt) is list:
                num_ = self.Parameter1Count
                if num_ == 0:  # controller dependant
                    num_ = 1
                    if not (Controller is None):
                        num_ = 2
                if len(par1cnt) < num_:
                    num_ = len(par1cnt)
                for i in range(num_):
                    tx = par1cnt[i]
                    if len(tx) > 0:
                        if (tx[0] == "+") or (tx[0] == "-"):
                            comm_ += tx[0]
                            tx = tx[1:]
                    nn = self.Parameter1Entries[0]
                    if tx.isdigit():
                        nn = int(tx)
                        if nn < self.Parameter1Entries[0]:
                            nn = self.Parameter1Entries[0]
                        if nn > self.Parameter1Entries[1]:
                            nn = self.Parameter1Entries[1]
                    tx = hex(nn)[2:]
                    while len(tx) < self.Parameter1Length:
                        tx = "0" + tx
                    comm_ += tx + " "
                    del tx
                    del nn
                del num_

        comm_ += " " + self.text1
        comm_ += " " + self.text2
        par1cnt.clear()
        return comm_

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

class HCD_Command_Window_Dialog:
    def __init__(self):
        self.currentCommand = None
        self.DialogWindow = None
        self.ConstructedCommand = ""
        self.QBox_VC_UpdateNext = False
        self.QBox_GC_UpdateNext = False
        self.QBox_CC_UpdateNext = False
        self.CurrentController = None

        self.ParentPage = None
        self.layout = None
        self.CommandDescriptionLabel = None
        self.GroupDescriptionLabel = None
        self.CommandFormLabel = None
        self.CommandChoiceLabel = None
        self.QBox_VersionChoice = None
        self.QBox_GroupChoice = None
        self.QBox_CommandChoice = None
        self.ControllerAndFileLabel = None

        self.Parameter1NameLabel = None
        self.Parameter1Grid = None
        self.Parameter1WidgetsIntField = []
        self.Parameter2labels = []
        self.Parameter2Widgets = []

    def Call_Window(self, ShowChoice=True):
        if self.DialogWindow is None:
            self.Construct_Window()
        nam = "nothing"
        if not (self.CurrentController is None):
            scn = "not selected"
            nam = "Controller: " + self.CurrentController.TabName + "\nScenario file: "+scn
            self.QBox_VersionChoice.setEnabled(True)
            if self.CurrentController.CurrentHCDVersionIndex != -1:
                if self.QBox_VersionChoice.currentIndex() != self.CurrentController.CurrentHCDVersionIndex:
                    self.QBox_VersionChoice.setCurrentIndex(self.CurrentController.CurrentHCDVersionIndex)
                self.QBox_VersionChoice.setEnabled(False)

        self.ControllerAndFileLabel.setText(nam)
        self.CommandChoiceLabel.setVisible(ShowChoice)
        self.QBox_GroupChoice.setVisible(ShowChoice)
        self.QBox_VersionChoice.setVisible(ShowChoice)
        self.QBox_CommandChoice.setVisible(ShowChoice)
        hgt = 1
        if ShowChoice:
            hgt = 25
        self.CommandChoiceLabel.setFixedHeight(hgt)
        self.QBox_CommandChoice.setFixedHeight(hgt)
        self.QBox_GroupChoice.setFixedHeight(hgt)
        self.QBox_VersionChoice.setFixedHeight(hgt)
        self.DialogWindow.show()

    def Construct_Window(self):
        self.DialogWindow = Functional_Window()
        self.DialogWindow.ParentAction = self.Close_Window_Event
        self.DialogWindow.setWindowTitle("Add HCD command")
        self.DialogWindow.setFixedWidth(300)
        self.ParentPage = QtWidgets.QWidget()

        self.layout = QtWidgets.QVBoxLayout(self.ParentPage)

        self.CommandDescriptionLabel = QtWidgets.QLabel("Description:", self.ParentPage, )
        self.CommandDescriptionLabel.setText("No command selected")
        self.CommandDescriptionLabel.setWordWrap(True)
        self.CommandDescriptionLabel.setAlignment(Qt.AlignLeft | Qt.AlignTop)
        self.Parameter1NameLabel = QtWidgets.QLabel("", self.ParentPage, )
        self.Parameter1NameLabel.setVisible(False)

        self.GroupDescriptionLabel = QtWidgets.QLabel("Description:", self.ParentPage, )
        self.GroupDescriptionLabel.setText("No Command group selected")
        self.GroupDescriptionLabel.setWordWrap(True)
        self.GroupDescriptionLabel.setAlignment(Qt.AlignLeft | Qt.AlignTop)

        self.CommandFormLabel = QtWidgets.QTextEdit(self.ParentPage)
        self.CommandFormLabel.setTextInteractionFlags(Qt.TextSelectableByMouse | Qt.TextSelectableByKeyboard)

        self.CommandChoiceLabel = QtWidgets.QLabel("Chose command", self.ParentPage, )
        self.ControllerAndFileLabel = QtWidgets.QLabel("nothing" , self.ParentPage, )
        self.QBox_VersionChoice = QtWidgets.QComboBox(self.ParentPage)
        self.QBox_VC_UpdateNext = False
        self.QBox_GroupChoice = QtWidgets.QComboBox(self.ParentPage)
        self.QBox_GC_UpdateNext = False
        self.QBox_CommandChoice = QtWidgets.QComboBox(self.ParentPage)
        self.QBox_CC_UpdateNext = False
        for i in range(len(HCD_Version_list_Full)):
            self.QBox_VersionChoice.addItem(HCD_Version_list_Full[i])
        self.QBox_VC_UpdateNext = True

        self.Parameter1Grid = QtWidgets.QGridLayout(self.ParentPage)

        self.layout.addWidget(self.ControllerAndFileLabel)
        self.layout.addWidget(self.CommandChoiceLabel)
        self.layout.addWidget(self.QBox_VersionChoice)
        self.layout.addWidget(self.QBox_GroupChoice)
        self.layout.addWidget(self.GroupDescriptionLabel)
        self.layout.addWidget(self.QBox_CommandChoice)
        self.layout.addWidget(self.CommandDescriptionLabel)
        self.layout.addSpacing(20)
        self.layout.addWidget(self.Parameter1NameLabel)
        self.layout.addLayout(self.Parameter1Grid)
        self.layout.addWidget(self.CommandFormLabel)

        self.layout.addStretch()

        self.Update_Tab_CommandGroup()
        self.QBox_VersionChoice.currentIndexChanged.connect(self.Update_Tab_CommandGroup)
        self.QBox_GroupChoice.currentIndexChanged.connect(self.Update_Tab_CommandList)
        self.QBox_CommandChoice.currentIndexChanged.connect(self.Update_Command_Description)

        self.DialogWindow.Install_Widget(self.ParentPage)

    def Update_Tab_CommandGroup(self):
        if self.QBox_VC_UpdateNext:
            #print("command window first box update")
            ind = 0
            if self.QBox_VersionChoice:
                ind = self.QBox_VersionChoice.currentIndex()
            self.QBox_GC_UpdateNext = False
            tx = ""
            if self.QBox_GroupChoice.count()>0:
                tx = self.QBox_GroupChoice.itemText(self.QBox_GroupChoice.currentIndex())
            while self.QBox_GroupChoice.itemText(0):
                self.QBox_GroupChoice.removeItem(0)
            ch = 0
            notfound = True
            for i in range(len(HCD_Version_association_list[ind])):
                self.QBox_GroupChoice.addItem(HCD_Command_Groups[HCD_Version_association_list[ind][i]].GroupName)
                if tx == HCD_Command_Groups[HCD_Version_association_list[ind][i]].GroupName:
                    ch = i
                    notfound = False
            self.QBox_GroupChoice.setCurrentIndex(ch)
            self.QBox_GC_UpdateNext = notfound
            self.Update_Tab_CommandList()
        self.QBox_GC_UpdateNext = True

    def Update_Tab_CommandList(self):
        if self.QBox_GC_UpdateNext:
            #print("command window second box update")
            ind = 0
            if self.QBox_GroupChoice:
                ind = self.QBox_GroupChoice.currentIndex()
                #print(HCD_Command_Groups[ind].GroupName + "   "+ HCD_Command_Groups[ind].GroupDescription)
                self.GroupDescriptionLabel.setText(HCD_Command_Groups[ind].GroupDescription)
            self.QBox_CC_UpdateNext = False
            tx = ""
            if self.QBox_CommandChoice.count()>0:
                tx = self.QBox_CommandChoice.itemText(self.QBox_CommandChoice.currentIndex())
            while self.QBox_CommandChoice.itemText(0):
                self.QBox_CommandChoice.removeItem(0)
            ch = 0
            notfound = True
            for i in range(len(HCD_Command_Groups[ind].CommandList)):
                self.QBox_CommandChoice.addItem(HCD_Command_Groups[ind].CommandList[i].name)
                if tx == HCD_Command_Groups[ind].CommandList[i].name:
                    ch = i
                    notfound = False
            if len(HCD_Command_Groups[ind].CommandList) == 0:
                self.QBox_CommandChoice.addItem("- - -")
                ch = 0
                notfound = True
            self.QBox_CommandChoice.setCurrentIndex(ch)
            self.QBox_CC_UpdateNext = notfound
            #print("update of commands")
            self.Update_Command_Description()
        self.QBox_CC_UpdateNext = True

    def Update_Command_Description(self):
        if self.QBox_CC_UpdateNext:
            #print("command window third box update")
            # ind1 = self.QBox_VersionChoice.currentIndex()
            ind2 = self.QBox_GroupChoice.currentIndex()
            ind3 = self.QBox_CommandChoice.currentIndex()
            # HCD_Version_association_list[ind1][ind2]
            # HCD_Command_Groups[ind2].CommandList[ind3]
            if len(HCD_Command_Groups[ind2].CommandList):
                #CommandItself = HCD_Command()
                CommandItself = HCD_Command_Groups[ind2].CommandList[ind3]
                self.CommandDescriptionLabel.setText(CommandItself.description)

                self.Parameter1NameLabel.setVisible(False)
                self.Parameter1NameLabel.setFixedHeight(1)

                if len(self.Parameter1WidgetsIntField):
                    for i in range(len(self.Parameter1WidgetsIntField)):
                        self.Parameter1WidgetsIntField[i].setVisible(False)
                        self.Parameter1WidgetsIntField[i].setFixedHeight(20)
                if CommandItself.Parameter1Type == HCDFirstParameter.Int:
                    self.Parameter1NameLabel.setText(CommandItself.Parameter1Name)
                    self.Parameter1NameLabel.setVisible(True)
                    self.Parameter1NameLabel.setFixedHeight(20)
                    num_= CommandItself.Parameter1Count
                    if num_ == 0: #controller dependant
                        num_ = 1
                        if not (self.CurrentController is None):
                            num_ = 2
                    while len(self.Parameter1WidgetsIntField) < num_:
                        wid = QtWidgets.QLineEdit(self.ParentPage)
                        self.Parameter1WidgetsIntField.append(wid)
                        wid.textEdited.connect(self.Form_Command)
                    nm = ""
                    if CommandItself.Parameter1Signable:
                        nm = "#"
                    if CommandItself.Parameter1Length == 0:
                        nm += "0000000"
                    else:
                        for i in range(CommandItself.Parameter1Length):
                            nm += "0"
                    for i in range(num_):
                        self.Parameter1WidgetsIntField[i].setVisible(True)
                        self.Parameter1WidgetsIntField[i].setFixedHeight(20)
                        self.Parameter1WidgetsIntField[i].setText(str(CommandItself.Parameter1Entries[0]))
                        self.Parameter1WidgetsIntField[i].setInputMask(nm)
                        y = i % 4
                        x = i//4
                        self.Parameter1Grid.addWidget(self.Parameter1WidgetsIntField[i], x, y, Qt.AlignVCenter | Qt.AlignLeft)
                        del x, y
                    del nm, num_

                self.Form_Command()

            else:
                self.CommandDescriptionLabel.setText("empty command list for this version and command group")
                self.CommandFormLabel.setText(" ")

    def Form_Command(self):
        ind2 = self.QBox_GroupChoice.currentIndex()
        ind3 = self.QBox_CommandChoice.currentIndex()
        comm_=""
        #comm_ = HCD_Command_Groups[ind2].CommandList[ind3].text0
        #if HCD_Command_Groups[ind2].CommandList[ind3].text1:
        par1arr = []
        par2arr = [0,1]
        if HCD_Command_Groups[ind2].CommandList[ind3].Parameter1Type == HCDFirstParameter.Int:
            num_ = HCD_Command_Groups[ind2].CommandList[ind3].Parameter1Count
            if num_ == 0:  # controller dependant
                num_ = 1
                if not (self.CurrentController is None):
                    num_ = 2
            if len(self.Parameter1WidgetsIntField) < num_:
                num_ = len(self.Parameter1WidgetsIntField)
            for i in range(num_):
                par1arr.append(self.Parameter1WidgetsIntField[i].text())
            del num_
        comm_ = HCD_Command_Groups[ind2].CommandList[ind3].Form_Command_text(self.CurrentController, par1arr, par2arr)
        del ind2
        del ind3
        par1arr.clear()
        par2arr.clear()
        self.CommandFormLabel.setText(comm_)
        return comm_

    def Close_Window_Event(self):
        return True

class QtComboBoxUnScroll(QtWidgets.QComboBox):
    def __init__(self, *args, **kwargs):
        super(QtComboBoxUnScroll, self).__init__(*args, **kwargs)

    def wheelEvent(self, event):
        event.ignore()

class DKLed_Output_Instances:
    def __init__(self):
        self.Controller = None #parent Controller class
        self.ParentPage = None #parent Widget
        self.index = 0
        self.LayoutObject = None  # pointer to corresponding layout object
        self.Screens = [] #pointer to connected screens or devises
        self.StartScreenIndex = 0
        self.ConnectionLines = [] #pointer to connection line at the layout window (layout object)
        self.ControllerType = 0
        #created here:
        self.QtComboWidget = None
        self.QtScreenListWidget = None
        self.indexLabel = None
        self.textLabel = None
        self.layout = None

    def Construct_Widget(self):
        self.layout = QtWidgets.QHBoxLayout(self.ParentPage)
        self.indexLabel = QtWidgets.QLabel((hex(self.index)[2:]).lower()+":", self.ParentPage, )
        self.indexLabel.setFixedSize(15, 25)
        self.QtComboWidget = QtComboBoxUnScroll(self.ParentPage)
        #self.QtComboWidget = QtWidgets.QComboBox(self.ParentPage)
        self.QtComboWidget.setFixedSize(130, 25)
        #self.QtComboWidget.setFrame(False)
        #self.QtComboWidget.
        self.textLabel = QtWidgets.QLabel("connect to", self.ParentPage, )
        self.textLabel.setFixedHeight(25)
        #self.textLabel.setStyleSheet(" text-align: right; ")
        self.QtScreenListWidget = QtComboBoxUnScroll(self.ParentPage)
        self.QtScreenListWidget.setFixedSize(150, 25)
        self.QtScreenListWidget.addItem("(none)")
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

    def Update_Instance_List(self):
        CheckStoplist = self.Controller.FastAnimCheckbox.isChecked()
        tx = self.QtComboWidget.itemText(self.QtComboWidget.currentIndex())
        while self.QtComboWidget.itemText(0):
            self.QtComboWidget.removeItem(0)
        if self.ControllerType != 0:
            for i in range(len(self.ControllerType.devices)):
                if not (CheckStoplist
                        and (self.ControllerType.devices[i] in Fast_Animation_Entry_Stoplist)
                        and self.ControllerType.fastvid):
                    self.QtComboWidget.addItem(self.ControllerType.devices[i])
                    if self.ControllerType.devices[i] == tx:
                        self.QtComboWidget.setCurrentIndex(i)

    def set_visible(self, Vis=True):
        if self.indexLabel:
            self.indexLabel.setVisible(Vis)
        if self.QtComboWidget:
            self.QtComboWidget.setVisible(Vis)
        if self.QtScreenListWidget:
            self.QtScreenListWidget.setVisible(Vis)
        if self.textLabel:
            self.textLabel.setVisible(Vis)

    def destroy_widget(self):
        self.Screens.clear()
        self.ConnectionLines.clear()
        del self.index
        del self.ConnectionLines
        del self.Screens
        del self.StartScreenIndex

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
        self.LayoutObject = None  # pointer to corresponding layout object
        self.ReceiverPorts = [] #port or objects to connect (layout object)
        self.ConnectionLines = [] #pointer to connection line at the layout window (layout object)
        self.index = 0
        self.portType = 0 #0- undef, 1- Uart, 2-SPI, 2-I2C
        self.ControllerType = 0
        self.Baudrate = 115200
        self.WaitPeriod = 1000 #wait time in milliseconds
        #created here:
        self.QtComboWidgetBaud = None
        self.QtComboWidgetIn = None
        self.QtComboWidgetOut = None
        self.indexLabel = None
        self.layout = None
        self.QtComboWidgetWait = None

    def Construct_Widget(self):
        self.layout = QtWidgets.QHBoxLayout(self.ParentPage)
        self.indexLabel = QtWidgets.QLabel(" "+str(self.index)+":", self.ParentPage, )
        self.indexLabel.setFixedSize(50, 25)
        self.QtComboWidgetBaud = QtWidgets.QLineEdit(str(self.Baudrate), self.ParentPage)
        self.QtComboWidgetBaud.setFixedSize(50, 25)
        self.QtComboWidgetBaud.setValidator(QIntValidator(600, 256000))
        self.QtComboWidgetIn = QtComboBoxUnScroll(self.ParentPage)
        self.QtComboWidgetIn.setFixedSize(100, 25)
        self.QtComboWidgetOut = QtComboBoxUnScroll(self.ParentPage)
        self.QtComboWidgetOut.setFixedSize(100, 25)

        self.QtComboWidgetWait = QtWidgets.QLineEdit(str(self.WaitPeriod), self.ParentPage)
        self.QtComboWidgetWait.setFixedSize(50, 25)
        self.QtComboWidgetWait.setValidator(QIntValidator(500, 10000))

        self.layout.addSpacing(4)
        self.layout.addWidget(self.indexLabel)
        self.layout.addWidget(self.QtComboWidgetBaud)
        self.layout.addWidget(self.QtComboWidgetIn)
        self.layout.addWidget(self.QtComboWidgetOut)
        self.layout.addWidget(self.QtComboWidgetWait)
        self.layout.addStretch()
        self.layout.addSpacing(2)

        self.indexLabel.setVisible(True)
        self.QtComboWidgetBaud.setVisible(True)
        self.QtComboWidgetIn.setVisible(True)
        self.QtComboWidgetOut.setVisible(True)
        self.QtComboWidgetWait.setVisible(True)
        self.Update_Instance_List()

    def Update_Instance_List(self):
        txIn = self.QtComboWidgetIn.itemText(self.QtComboWidgetIn.currentIndex())
        txOut = self.QtComboWidgetOut.itemText(self.QtComboWidgetOut.currentIndex())
        while self.QtComboWidgetIn.itemText(0):
            self.QtComboWidgetIn.removeItem(0)
        while self.QtComboWidgetOut.itemText(0):
            self.QtComboWidgetOut.removeItem(0)
        if self.ControllerType != 0:
            if self.index < len(self.ControllerType.uartTypes):
                str_ = self.ControllerType.uartTypes[self.index]
                self.indexLabel.setText(str_ + " (" + str(self.index + 1) + "):")
                for i in range(len(self.ControllerType.uartModesIN[self.index])):
                    self.QtComboWidgetIn.addItem(self.ControllerType.uartModesIN[self.index][i])
                    if self.ControllerType.uartModesIN[self.index][i] == txIn:
                        self.QtComboWidgetIn.setCurrentIndex(i)
                if len(self.ControllerType.uartModesIN[self.index]) == 0:
                    self.QtComboWidgetIn.addItem(" ")
                for i in range(len(self.ControllerType.uartModesOUT[self.index])):
                    self.QtComboWidgetOut.addItem(self.ControllerType.uartModesOUT[self.index][i])
                    if self.ControllerType.uartModesOUT[self.index][i] == txOut:
                        self.QtComboWidgetOut.setCurrentIndex(i)
                if len(self.ControllerType.uartModesOUT[self.index]) == 0:
                    self.QtComboWidgetOut.addItem(" ")
            else:
                self.QtComboWidgetIn.addItem(" ")
                self.QtComboWidgetOut.addItem(" ")

    def set_visible(self, Vis=True):
        if self.indexLabel:
            self.indexLabel.setVisible(Vis)
        if self.QtComboWidgetBaud:
            self.QtComboWidgetBaud.setVisible(Vis)
        if self.QtComboWidgetIn:
            self.QtComboWidgetIn.setVisible(Vis)
        if self.QtComboWidgetOut:
            self.QtComboWidgetOut.setVisible(Vis)
        if self.QtComboWidgetWait:
            self.QtComboWidgetWait.setVisible(Vis)

    # destroy widget is called when a new controller is constructed
    # and there is something in it's dedicated list of child widgets
    def destroy_widget(self):
        del self.index
        del self.WaitPeriod
        del self.Baudrate
        del self.portType
        del self.ControllerType
        self.ConnectionLines.clear()
        self.ReceiverPorts.clear()
        del self.ConnectionLines
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
        if type(self.layout) is QtWidgets:
            self.layout.deleteLater()
        del self

class DKLed_Button_Instances:
    def __int__(self):
        self.Controller = None #parent Controller class
        self.ParentPage = None #parent Widget
        self.index = 0
        self.LayoutObject = None  # pointer to corresponding layout object
        self.Keyboards = [] #pointer to connected keyboards or encoders (layout object)
        self.KeyPins = [] #pointer to connected pins at keyboards or encoders (layout object)
        self.ConnectionLines = [] #pointer to connection line at the layout window (layout object)
        self.ControllerType = 0
        self.btncombCheckbox = 0
        #created here:
        self.QtBTNWidget = None
        self.color = 0

    def Construct_Widget(self):
        self.color = 0xf0f0f0
        self.QtBTNWidget = QtWidgets.QPushButton((hex(self.index)[2:]).lower(), self.ParentPage)
        self.QtBTNWidget.setStyleSheet("border-color: #909090; border-width: 2px;"
                                       "border-style: outset; border-radius: 6px;"
                                       "background-color: #" + hex(self.color)[2:])
        self.QtBTNWidget.setFixedSize(28, 25)

        self.QtBTNWidget.setVisible(True)
        self.Update_Instance_List()

    def Update_Instance_List(self):
        val = True
        val2 = True
        if self.btncombCheckbox !=0:
            val2 = not (self.btncombCheckbox.isChecked())
        if self.ControllerType != 0:
            self.QtBTNWidget.setText((hex(self.index)[2:]).lower())
            self.QtBTNWidget.setStyleSheet("border-color: #909090; border-width: 2px;"
                                       "border-style: outset; border-radius: 6px;"
                                       "background-color: #" + hex(self.color)[2:])
            if val2:
                for i in range(self.ControllerType.encd):
                    if self.Controller.Encoders[i].QtCheckWidget.isChecked():
                        if self.index - 1 == self.Controller.Encoders[i].choiceA:
                            self.QtBTNWidget.setStyleSheet("border-color: #909090; border-width: 2px;"
                                       "border-style: outset; border-radius: 6px;"
                                       "background-color: #" + hex(self.Controller.Encoders[i].color)[2:])
                            self.QtBTNWidget.setText("A")
                            val = False
                        if self.index - 1 == self.Controller.Encoders[i].choiceB:
                            self.QtBTNWidget.setStyleSheet("border-color: #909090; border-width: 2px;"
                                       "border-style: outset; border-radius: 6px;"
                                       "background-color: #" + hex(self.Controller.Encoders[i].color)[2:])
                            self.QtBTNWidget.setText("B")
                            val = False
        self.QtBTNWidget.setEnabled(val)
        del val
        del val2

    def set_visible(self, Vis=True):
        if self.QtBTNWidget:
            self.QtBTNWidget.setVisible(Vis)

    def destroy_widget(self):
        self.Keyboards.clear()
        self.KeyPins.clear()
        self.ConnectionLines.clear()
        del self.index
        del self.ConnectionLines
        del self.Keyboards
        del self.KeyPins

        if type(self.QtBTNWidget) is QtWidgets:
            self.QtBTNWidget.deleteLater()
        del self

class DKLed_Encoder_Instances:
    def __init__(self):
        self.Controller = None  # parent Controller class
        self.ParentPage = None  # parent Widget
        self.index = 0
        self.LayoutObject = None #pointer to corresponding layout object
        self.buttons = []  # pointer to connected buttons (layout object)
        self.pins = [] # pins within this controller (layout object)
        self.ConnectionLines = []  # pointer to connection line at the layout window (layout object)
        self.ControllerType = 0
        self.btncombCheckbox = 0
        # created here:
        self.QtComboWidgetA = None
        self.QtComboWidgetB = None
        self.QtCheckWidget = None
        self.indexLabel = None
        self.btnChoiceLabel = None
        self.layout = None
        self.choiceA=0
        self.choiceB=0
        self.color = 0
        self.preChoiceA=0
        self.preChoiceB=0

    def Construct_Widget(self):
        global MaximumNumberOf_Buttons
        global Encoder_Color_Highlights
        self.layout = QtWidgets.QHBoxLayout(self.ParentPage)
        self.indexLabel = QtWidgets.QLabel("  Encoder "+str(self.index)+":", self.ParentPage, )
        self.indexLabel.setFixedSize(64, 25)
        if len(Encoder_Color_Highlights) > self.index:
            self.color = Encoder_Color_Highlights[self.index]
        else:
            self.color = 0xF0F0F0
        self.indexLabel.setStyleSheet("background-color: #" + hex(self.color)[2:])
        self.QtCheckWidget = QtWidgets.QCheckBox("connect to inputs A", self.ParentPage)
        self.QtCheckWidget.setChecked(False)
        self.QtCheckWidget.setFixedSize(112, 25)
        self.btnChoiceLabel = QtWidgets.QLabel(", B", self.ParentPage, )
        self.btnChoiceLabel.setFixedSize(13, 25)
        self.QtComboWidgetA = QtComboBoxUnScroll(self.ParentPage)
        self.QtComboWidgetA.setFixedSize(36, 25)
        self.QtComboWidgetA.setMaxVisibleItems(MaximumNumberOf_Buttons)
        self.QtComboWidgetA.addItem("1")
        #self.QtComboWidgetA.addItem("2")
        #self.QtComboWidgetA.setCurrentIndex(0)
        self.QtComboWidgetB = QtComboBoxUnScroll(self.ParentPage)
        self.QtComboWidgetB.setFixedSize(36, 25)
        self.QtComboWidgetB.setMaxVisibleItems(MaximumNumberOf_Buttons)
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
        if self.btncombCheckbox !=0:
            val = not (self.btncombCheckbox.isChecked())
        self.QtCheckWidget.setEnabled(val)
        self.btnChoiceLabel.setEnabled(val)
        val = val & self.QtCheckWidget.isChecked()
        self.QtComboWidgetA.setEnabled(val)
        self.QtComboWidgetB.setEnabled(val)
        del val
        if self.ControllerType != 0:
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

    def avoid_collsionsA(self):
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
                    self.Controller.Inputs[i].QtBTNWidget.setText("A")
                    self.Controller.Inputs[i].QtBTNWidget.setStyleSheet("border-color: #909090; border-width: 2px;"
                                       "border-style: outset; border-radius: 6px;"
                                       "background-color: #" + hex(self.color)[2:])
                self.Controller.Inputs[i].QtBTNWidget.setEnabled(False)
            else:
                self.Controller.Inputs[i].QtBTNWidget.setText((hex(i + 1)[2:]).lower())
                self.Controller.Inputs[i].QtBTNWidget.setStyleSheet("border-color: #909090; border-width: 2px;"
                                       "border-style: outset; border-radius: 6px;"
                                       "background-color: #" + hex(self.Controller.Inputs[i].color)[2:])
                self.Controller.Inputs[i].QtBTNWidget.setEnabled(True)
        del lst

    def avoid_collsionsB(self):
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
                    self.Controller.Inputs[i].QtBTNWidget.setStyleSheet("border-color: #909090; border-width: 2px;"
                                       "border-style: outset; border-radius: 6px;"
                                       "background-color: #" + hex(self.color)[2:])
                    self.Controller.Inputs[i].QtBTNWidget.setText("B")
                self.Controller.Inputs[i].QtBTNWidget.setEnabled(False)
            else:
                self.Controller.Inputs[i].QtBTNWidget.setText((hex(i+1)[2:]).lower())
                self.Controller.Inputs[i].QtBTNWidget.setStyleSheet("border-color: #909090; border-width: 2px;"
                                       "border-style: outset; border-radius: 6px;"
                                       "background-color: #" + hex(self.Controller.Inputs[i].color)[2:])
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
                    self.Controller.Inputs[i].QtBTNWidget.setStyleSheet("border-color: #909090; border-width: 2px;"
                                       "border-style: outset; border-radius: 6px;"
                                       "background-color: #" + hex(self.Controller.Inputs[i].color)[2:])
                    self.Controller.Inputs[i].QtBTNWidget.setEnabled(True)

    def destroy_widget(self):
        del self.index
        del self.ControllerType
        self.ConnectionLines.clear()
        self.buttons.clear()
        self.pins.clear()
        del self.ConnectionLines
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

class Functional_Window(QtWidgets.QWidget):
    #grandLayout = None
    def __int__(self):
        super().__init__()
        self.ParentAction = None
        self.grandLayout = None

    def PrepareLayout(self):
        if not hasattr(self, 'grandLayout'):
            self.grandLayout = QtWidgets.QHBoxLayout(self)
        else:
            if self.grandLayout is None:
                self.grandLayout = QtWidgets.QHBoxLayout(self)
        self.grandLayout.setContentsMargins(0,0,0,0)

    def Install_Widget(self, obj):
        self.PrepareLayout()
        if obj:
            obj.setParent(self)
            self.grandLayout.addWidget(obj)
            self.setLayout(self.grandLayout)
            obj.show()
        self.show()

    def closeEvent(self, event):
        val = True
        if self.ParentAction:
            val = self.ParentAction()
        if val:
            event.accept()
        else:
            event.ignore()

class Controller:
    #ID = "****"
    #TabName = "none"
    #Page = None
    #Connected = False
    #index = 0

    def __init__(self):

        self.ID = "****"
        self.index = 0
        self.Connected = False

        self.Inputs = []
        self.Encoders = []
        self.Outputs = []
        self.Ports = []
        self.I2cAddress = 0
        self.Scenarios = []
        self.ButtonMode = 0  # button mode: 0-standard, 1-combination
        self.CurrentHCDVersionIndex = 0 #index of current HCD version index in HCD_Version_list_Full array
        self.CurrentWorkingFlags = HCDCompartibleFlags.InEveryMode #modes of commands this controller is in
        self.PWMDiscreet = 512 #how many steps for PWM discretization
        self.MaxBrightness = 32
        #self.LayoutObject = None  # pointer to corresponding layout object
        #extra windows:
        self.TerminalWindow = None
        self.ControllerFilesExplorerWindow = None
        self.OptionalWindow = None
        #interface Widgets:
        self.Page = None
        self.controllerTypeSelect = None
        self.FastAnimCheckbox = None
        self.GotoWindowButton = None
        self.idBox = None
        self.controllerCodeVersionLabel = None
        self.COMchoice = None
        self.IOconfigs = None
        self.IOconfigs_Layout = None
        self.IO_Output_header = None
        self.IO_Input_header = None
        self.IO_Input_header_comb = None
        self.USBLabel = None

    def add_tab(self, name_="none"):
        self.TabName = name_
        self.Connected = False
        self.Page = QtWidgets.QWidget(None, )
        self.index = ui.tabWidget_2.addTab(self.Page, self.TabName)

    def change_tab_name(self, Newname):
        self.TabName = Newname
        ui.tabWidget_2.setTabText(self.index, self.TabName)

    def update_fast_animation_flag(self):
        if self.FastAnimCheckbox:
            if self.FastAnimCheckbox.isChecked():
                #controller in fast animation mode
                self.CurrentWorkingFlags |= HCDCompartibleFlags.InFastVideomode | HCDCompartibleFlags.InLedMode
                self.CurrentWorkingFlags &= ~HCDCompartibleFlags.InServoMode
            else:
                #Led or Hybrid mode
                self.CurrentWorkingFlags |= HCDCompartibleFlags.InServoMode | HCDCompartibleFlags.InLedMode
                self.CurrentWorkingFlags &= ~HCDCompartibleFlags.InFastVideomode
            #print("Controller " + self.TabName + " in mode: " + hex(self.CurrentWorkingFlags))

    def update_fast_combination_flag(self):
        if self.IO_Input_header_comb:
            if self.IO_Input_header_comb.isChecked():
                # combination of buttons
                self.CurrentWorkingFlags |= HCDCompartibleFlags.InCombinationInputMode
                self.CurrentWorkingFlags &= ~HCDCompartibleFlags.InSingleInputMode
            else:
                # single input
                self.CurrentWorkingFlags &= ~HCDCompartibleFlags.InCombinationInputMode
                self.CurrentWorkingFlags |= HCDCompartibleFlags.InSingleInputMode
            #print("Controller " + self.TabName + " in mode: " + hex(self.CurrentWorkingFlags))

    def Return_from_Window(self):
        self.GotoWindowButton.setText("Open in Window")
        self.index = ui.tabWidget_2.insertTab(self.index, self.Page, self.TabName)
        ui.tabWidget_2.setCurrentIndex(self.index)
        self.Page.show()
        return True

    def Move_to_Window(self):
        if self.GotoWindowButton.text() == "Open in Window":
            self.GotoWindowButton.setText("Return to tab")
            if self.OptionalWindow is None:
                self.OptionalWindow = Functional_Window()
                #self.OptionalWindow.PrepareLayout()
            self.OptionalWindow.setWindowTitle("Controller: " + self.TabName)
            self.OptionalWindow.setFixedWidth(428)
            self.OptionalWindow.setMinimumHeight(450)
            self.OptionalWindow.ParentAction = self.Return_from_Window
            self.OptionalWindow.Install_Widget(self.Page)
        else:
            if self.OptionalWindow:
                self.OptionalWindow.close()

    def fill_tab(self):
        global MaximumNumberOf_Outputs
        global MaximumNumberOf_ButtonEvents
        global MaximumNumberOf_Buttons
        global MaximumNumberOf_Ports
        global MaximumNumberOf_Encoders

        self.CurrentWorkingFlags = HCDCompartibleFlags.InEveryMode
        self.PWMDiscreet = 512
        self.MaxBrightness = 32

        mainlay = QtWidgets.QHBoxLayout(self.Page)
        layv2 = QtWidgets.QVBoxLayout(self.Page)

        labl1 = QtWidgets.QLabel("Controller", self.Page, )
        labl1.setFixedHeight(25)
        self.controllerTypeSelect = QtWidgets.QComboBox(self.Page)
        self.controllerTypeSelect.setFixedSize(80, 25)
        self.controllerTypeSelect.addItem("- - -")
        self.controllerCodeVersionLabel = QtWidgets.QLabel(" ", self.Page, )
        self.controllerCodeVersionLabel.setFixedHeight( 25)
        for pd in Possible_Controllers:
            self.controllerTypeSelect.addItem(pd.name)
        self.controllerTypeSelect.currentIndexChanged.connect(self.Change_Visibility_Configs)

        btn1 = QtWidgets.QPushButton("Remove", self.Page)
        btn1.setFixedSize(60, 25)
        if self.index == 0:
            btn1.setDisabled(True)
        self.GotoWindowButton = QtWidgets.QPushButton("Open in Window", self.Page)
        self.GotoWindowButton.setFixedSize(90, 25)
        self.GotoWindowButton.clicked.connect(self.Move_to_Window)

        layh1 = QtWidgets.QHBoxLayout(self.Page)

        layh1.addWidget(labl1)
        layh1.addWidget(self.controllerTypeSelect)
        layh1.addWidget(self.controllerCodeVersionLabel)
        layh1.addStretch()
        layh1.addWidget(self.GotoWindowButton)
        layh1.addWidget(btn1)

        labl2 = QtWidgets.QLabel("ID:", self.Page, )
        labl2.setFixedHeight(25)
        self.idBox = QtWidgets.QLineEdit(self.ID, self.Page)
        self.idBox.setFixedSize(60, 25)
        self.idBox.setValidator(UIdValidator)
        #self.idBox.setVerticalScrollBarPolicy(1)
        btn21 = QtWidgets.QPushButton("Get", self.Page)
        btn21.setFixedSize(40, 25)
        btn21.setDisabled(True)
        self.COMchoice = QtWidgets.QComboBox(self.Page)
        self.COMchoice.setFixedSize(60, 25)
        btn22 = QtWidgets.QPushButton("Connect", self.Page)
        btn22.setFixedSize(70, 25)
        btn23 = QtWidgets.QPushButton("Open Terminal", self.Page)
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

        IOconf = QtWidgets.QScrollArea(self.Page)
        IOconf.setVerticalScrollBarPolicy(Qt.ScrollBarAlwaysOn)
        IOconf.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        IOconf.setMinimumWidth(408)
        IOconf.setMinimumHeight(300)
        IOconf.setWidgetResizable(False)

        self.IOconfigs = QtWidgets.QWidget(IOconf, )

        self.IOconfigs_Layout = QtWidgets.QVBoxLayout(self.IOconfigs)
        self.IOconfigs_Layout.setContentsMargins(5, 5, 5, 5)

        self.IO_Output_header = QtWidgets.QPushButton("Outputs V", self.IOconfigs)
        self.IO_Output_header.setStyleSheet("QPushButton { text-align: left; }")
        self.IO_Output_header.setDisabled(True)
        self.FastAnimCheckbox = QtWidgets.QCheckBox("enable Fast Animation mode", self.IOconfigs)
        self.FastAnimCheckbox.setChecked(False)
        self.FastAnimationCheckbox = False
        self.FastAnimCheckbox.setEnabled(False)
        self.FastAnimCheckbox.stateChanged.connect(self.update_fast_animation_flag)
        #self.FastAnimCheckbox.stateChanged.connect(self.Change_Visibility_Configs)
        layh50 = QtWidgets.QHBoxLayout(self.IOconfigs)
        layh50.addSpacing(4)
        layh50.addWidget(self.FastAnimCheckbox)
        layh50.addStretch()
        layh50.addSpacing(2)

        self.IO_Input_header = QtWidgets.QPushButton("Buttons V", self.IOconfigs)
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

        self.IO_UART_header = QtWidgets.QPushButton("I/O Ports V", self.IOconfigs)
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

        self.IO_Screens_header = QtWidgets.QPushButton("Screens >", self.IOconfigs)
        self.IO_Screens_header.setStyleSheet("QPushButton { text-align: left; }")
        self.IO_Screens_header.setDisabled(True)
        self.IO_Screens_header.setVisible(False)
        self.IO_Files_header = QtWidgets.QPushButton("Scenarios >", self.IOconfigs)
        self.IO_Files_header.setStyleSheet("QPushButton { text-align: left; }")
        self.IO_Files_header.setDisabled(True)
        self.IO_Files_header.setVisible(False)

        self.IO_Output_header.clicked.connect(self.Change_Visibility_Outputs)
        self.IO_Input_header.clicked.connect(self.Change_Visibility_Inputs)
        self.IO_UART_header.clicked.connect(self.Change_Visibility_Ports)

        for i in range(MaximumNumberOf_Outputs):
            if len(self.Outputs) <= i:
                self.Outputs.append(None)
            if type(self.Outputs[i]) is DKLed_Output_Instances:
                self.Outputs[i].destroy_widget()
            temp = DKLed_Output_Instances()
            temp.Controller = self
            temp.ControllerType = Possible_Controllers[0]
            if self.controllerTypeSelect.currentIndex() > 0:
                temp.ControllerType = Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1]
            temp.index = i
            temp.ParentPage = self.IOconfigs
            temp.Construct_Widget()
            self.FastAnimCheckbox.stateChanged.connect(temp.Update_Instance_List)
            self.Outputs[i] = temp

        for i in range(MaximumNumberOf_Encoders):
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
        for i in range(MaximumNumberOf_Buttons):
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

        for i in range(MaximumNumberOf_Ports):
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

        self.IOconfigs_Layout.addWidget(self.IO_Output_header)
        self.IOconfigs_Layout.addLayout(layh50)
        for i in range(len(self.Outputs)):
            self.IOconfigs_Layout.addLayout(self.Outputs[i].layout)
        self.IOconfigs_Layout.addWidget(self.IO_Input_header)
        self.IOconfigs_Layout.addLayout(layh51a)
        for i in range(len(laybtns)):
            self.IOconfigs_Layout.addLayout(laybtns[i])
        for i in range(len(self.Encoders)):
            self.IOconfigs_Layout.addLayout(self.Encoders[i].layout)
        self.IOconfigs_Layout.addWidget(self.IO_UART_header)
        self.IOconfigs_Layout.addWidget(self.USBLabel)
        self.IOconfigs_Layout.addLayout(layh52)
        for i in range(len(self.Ports)):
            self.IOconfigs_Layout.addLayout(self.Ports[i].layout)
        self.IOconfigs_Layout.addWidget(self.IO_Screens_header)
        self.IOconfigs_Layout.addWidget(self.IO_Files_header)
        self.IOconfigs_Layout.addStretch()

        IOconf.setWidget(self.IOconfigs)
        self.Change_Visibility_Outputs()
        self.Change_Visibility_Inputs()
        self.Change_Visibility_Ports()

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
        layv2.addWidget(IOconf)
        layv2.addWidget(labl3)
        layv2.addLayout(layh3)
        layv2.addWidget(self.InitFileExploreButton)

        #self.layPage=layv2
        mainlay.addLayout(layv2)
        mainlay.addStretch(50)

        self.Page.setLayout(mainlay)
        ui.tabWidget_2.setCurrentIndex(self.index)
        self.Page.show()

    def Change_Visibility_Outputs(self):
        Visible_ = False
        if (self.IO_Output_header.text() == "Outputs >"):
            if (self.controllerTypeSelect.currentIndex() > 0):
                Visible_ = True
            self.IO_Output_header.setText("Outputs V")
        else:
            self.IO_Output_header.setText("Outputs >")
        self.FastAnimCheckbox.setVisible(Visible_)
        for i in range(len(self.Outputs)):
            if (self.controllerTypeSelect.currentIndex() == 0) or (Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].out <= i):
                Visible_ = False
            self.Outputs[i].set_visible(Visible_)

    def Change_Visibility_Inputs(self):
        Visible_ = False
        if (self.IO_Input_header.text() == "Buttons >"):
            if (self.controllerTypeSelect.currentIndex() > 0):
                Visible_ = True
            self.IO_Input_header.setText("Buttons V")
        else:
            self.IO_Input_header.setText("Buttons >")
        self.IO_Input_header_comb.setVisible(Visible_)
        Vis2=Visible_
        for i in range(len(self.Inputs)):
            if (self.controllerTypeSelect.currentIndex() == 0) or (Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].btn <= i):
                Vis2 = False
            self.Inputs[i].set_visible(Vis2)
        for i in range(len(self.Encoders)):
            if (self.controllerTypeSelect.currentIndex() == 0) or (Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].encd <= i):
                Visible_ = False
            self.Encoders[i].set_visible(Visible_)

    def Change_Visibility_Ports(self):
        Visible_ = False
        if self.IO_UART_header.text() == "I/O Ports >":
            if (self.controllerTypeSelect.currentIndex() > 0) \
                and (Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].uart > 0):
                Visible_ = True
            self.IO_UART_header.setText("I/O Ports V")
        else:
            self.IO_UART_header.setText("I/O Ports >")
        self.IO_UART_header01.setVisible(Visible_)
        self.IO_UART_header02.setVisible(Visible_)
        self.IO_UART_header03.setVisible(Visible_)
        self.IO_UART_header04.setVisible(Visible_)
        self.IO_UART_header05.setVisible(Visible_)
        for i in range(len(self.Ports)):
            if (self.controllerTypeSelect.currentIndex() == 0) \
                or (Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].uart <= i):
                Visible_ = False
            self.Ports[i].set_visible(Visible_)
        if Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].USBSupport:
            self.USBLabel.setVisible(Visible_)
            self.CurrentWorkingFlags |= HCDCompartibleFlags.InUSBCommunicationMode
        else:
            self.USBLabel.setVisible(False)
            self.CurrentWorkingFlags &= ~HCDCompartibleFlags.InUSBCommunicationMode
        #print(str(Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].USBSupport))

    def Change_Visibility_Configs(self):
        if self.controllerTypeSelect.currentIndex() > 0:
            self.CurrentWorkingFlags = HCDCompartibleFlags.InEveryMode
            self.PWMDiscreet = Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].DefaultPWMDiscreet
            self.MaxBrightness = Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].DefaultMaxBrightness
            for o in self.Outputs:
                o.ControllerType = Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1]
                if o.index < Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].out:
                    o.Update_Instance_List()
            for o in self.Encoders:
                o.ControllerType = Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1]
                if o.index < Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].encd:
                    o.Update_Instance_List()
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
            self.IO_Screens_header.setDisabled(False)
            self.IO_Files_header.setDisabled(False)
            self.controllerCodeVersionLabel.setText("HCD ver. " + Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].CodeVersion)
            self.CurrentHCDVersionIndex = -1
            for i in range(len(HCD_Version_list_Full)):
                if HCD_Version_list_Full[i] == Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].CodeVersion:
                    self.CurrentHCDVersionIndex = i
                    #print("controller ver " + HCD_Version_list_Full[i] + " in "+str(self.CurrentHCDVersionIndex))
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
            if Possible_Controllers[self.controllerTypeSelect.currentIndex() - 1].btncomb:
                self.IO_Input_header_comb.setEnabled(True)
                if self.IO_Input_header_comb.isChecked():
                    # combination of buttons
                    self.CurrentWorkingFlags |= HCDCompartibleFlags.InCombinationInputMode
                    self.CurrentWorkingFlags &= ~HCDCompartibleFlags.InSingleInputMode
                else:
                    # single input
                    self.CurrentWorkingFlags &= ~HCDCompartibleFlags.InCombinationInputMode
                    self.CurrentWorkingFlags |= HCDCompartibleFlags.InSingleInputMode
            else:
                self.IO_Input_header_comb.setEnabled(False)
                self.CurrentWorkingFlags |= HCDCompartibleFlags.InSingleInputMode
                self.CurrentWorkingFlags &= ~HCDCompartibleFlags.InCombinationInputMode
        else:
            self.CurrentWorkingFlags = HCDCompartibleFlags.InNoMode
            self.PWMDiscreet = 512
            self.MaxBrightness = 32
            self.controllerCodeVersionLabel.setText(" ")
            self.IO_Input_header.setDisabled(True)
            self.IO_Output_header.setDisabled(True)
            self.IO_Screens_header.setDisabled(True)
            self.IO_Files_header.setDisabled(True)
            self.IO_UART_header.setDisabled(True)
            self.FastAnimCheckbox.setEnabled(False)

        self.Change_Visibility_Outputs()
        self.Change_Visibility_Outputs()
        self.Change_Visibility_Inputs()
        self.Change_Visibility_Inputs()
        self.Change_Visibility_Ports()
        self.Change_Visibility_Ports()
        print("Controller " + self.TabName + " in mode: "
              + bin(self.CurrentWorkingFlags) + " Descrete =" + str(self.PWMDiscreet)
              + " Brightness =" + str(self.MaxBrightness))





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
    add_dialog = QtWidgets.QInputDialog(ui, )
    add_dialog.setCancelButtonText("Discard")
    add_dialog.setOkButtonText("Add")
    add_dialog.setWindowTitle("Add controller")
    add_dialog.setLabelText("New Controller name in project")
    name_ = "DKLed " + str(len(Controllers) + 1)
    add_dialog.setInputMode(QInputDialog.TextInput)
    add_dialog.setTextValue(name_)
    ok = add_dialog.exec()
    if ok:
        Controllers2 = Controller()
        Controllers.append(Controllers2)
        if add_dialog.textValue() == "":
            add_dialog.setTextValue(Controllers2.TabName)
        Controllers2.add_tab(add_dialog.textValue())
        Controllers2.fill_tab()
    add_dialog.destroy(True, False)

def Create_Possible_Controller_List():

    #Possible_Controllers.append(None)
    #Possible_Controllers[0] = DKLed_Specs()
    #Possible_Controllers[0].add_device("WS2812", "SK6812", "WS2812 & SK6812", "SG90", "PWM", "on/off")
    #Possible_Controllers[0].uart_modes_IN("Uart", 1, "Disable", "Command line", "Byte")
    #Possible_Controllers[0].uart_modes_IN("Uart", 2, "Disable", "Command line", "Byte")
    #Possible_Controllers[0].uart_modes_OUT("Uart", 1, "Disable", "Text", "MP3 player", "JDY com", "DKLed")
    #Possible_Controllers[0].uart_modes_OUT("Uart", 2, "Disable", "Text", "MP3 player", "JDY com", "DKLed")

    #Possible_Controllers.append(None)
    #Possible_Controllers[1] = DKLed_Specs("I6O8U2", 8, 6, 64, 1, True)
    #ossible_Controllers[1].add_device("WS2812", "SK6812", "SG90", "on/off")
    #Possible_Controllers[1].uart_modes_IN("Uart", 1, "Disable", "Command line", "Byte")
    #Possible_Controllers[1].uart_modes_IN("Uart", 2, "Disable", "Command line", "Byte")
    #Possible_Controllers[1].uart_modes_OUT("Uart", 1, "Disable", "Text", "MP3 player", "JDY com", "DKLed")
    #Possible_Controllers[1].uart_modes_OUT("Uart", 2, "Disable", "Text", "MP3 player", "JDY com", "DKLed")

    global MaximumNumberOf_Outputs
    global MaximumNumberOf_ButtonEvents
    global MaximumNumberOf_Buttons
    global MaximumNumberOf_Encoders
    global MaximumNumberOf_Microphones
    global Encoder_Color_Highlights

    with open("Library/Controllers.txt") as ControllerDescription:
        ControllerDescriptionLines = [line.strip() for line in ControllerDescription]
    ControllerDescription.close()
    Ports = 0
    CName = ""
    PortsNames = []
    PortsIndex = []
    ThisController = None
    ThisController = DKLed_Specs()
    for i in range(len(ControllerDescriptionLines)):
        line = ControllerDescriptionLines[i]
        line2 = line.upper()
        line3 = line2.replace(" ", "")
        splline = line.split("=", 1)
        numpar = 0
        if len(splline)==2:
            spll2 = splline[1].replace(" ", "")
            if spll2.isdigit():
                numpar = int(spll2)
        if ';' in line3:
            line3 = ""
        elif 'COLOR' in line3:
            if 'ENC' in line3:
                splline[1] = splline[1].replace(" ", "")
                Encoder_Color_Highlights.append(int(splline[1], 16))
        elif 'CONTROLLER:' in line3:
            if CName != "":
                Possible_Controllers.append(ThisController)
                ThisController = DKLed_Specs()
            Ports = 0
            CName = ""
            ThisController.DefaultPWMDiscreet = 512
            ThisController.DefaultMaxBrightness = 32
            PortsNames.clear()
            PortsIndex.clear()
        elif 'USBSUPP' in line3:
            val = False
            if ('TRUE' in splline[1].upper()) or ('YES' in splline[1].upper()):
                val = True
            ThisController.USBSupport = val
        elif 'PORT' in line3:
            if 'PORT:' in line3:
                Ports += 1
                PortsNames.append("")
                PortsIndex.append(0)
            if Ports == 0:
                Ports = 1
            if len(PortsIndex) == 0:
                PortsIndex.append(0)
            if len(PortsNames) == 0:
                PortsNames.append("")
            if 'NAME' in line3:
                PortsNames[Ports - 1] = splline[1].replace(" ", "")
            if 'NUMBER' in line3:
                splline[1] = splline[1].replace(" ", "")
                PortsIndex[Ports-1] = int(splline[1])
            if 'RECEIVE' in line3:
                LedEntries = splline[1].split('"')
                LedEntries2 = []
                for j in range(len(LedEntries)):
                    if not ("," in LedEntries[j]) and LedEntries[j].replace(" ", ""):
                        LedEntries2.append(LedEntries[j])
                ThisController.uart_modes_IN(PortsNames[Ports - 1], PortsIndex[Ports-1], *LedEntries2)
            if 'TRANSMIT' in line3:
                LedEntries = splline[1].split('"')
                LedEntries2 = []
                for j in range(len(LedEntries)):
                    if not ("," in LedEntries[j]) and LedEntries[j].replace(" ", ""):
                        LedEntries2.append(LedEntries[j])
                ThisController.uart_modes_OUT(PortsNames[Ports - 1], PortsIndex[Ports - 1], *LedEntries2)
        elif 'DEFAULT' in line3:
            if ('STEPS' in line3) and (('PWM' in line3) or ('SERV' in line3)):
                ThisController.DefaultPWMDiscreet = numpar
            elif 'BRIGHT' in line3:
                ThisController.DefaultMaxBrightness = numpar
        else:
            if 'NAME' in line3:
                CName = splline[1].replace(" ","")
                #print(line + "  :" + CName)
                ThisController.name=CName
            elif 'INP' in line3:
                ThisController.btn = numpar
                if MaximumNumberOf_Buttons < ThisController.btn:
                    MaximumNumberOf_Buttons = ThisController.btn
            elif 'OUT' in line3:
                ThisController.out = numpar
                if MaximumNumberOf_Outputs < ThisController.out:
                    MaximumNumberOf_Outputs = ThisController.out
            elif 'EVE' in line3:
                ThisController.totalbtn = numpar
                if MaximumNumberOf_ButtonEvents < ThisController.totalbtn:
                    MaximumNumberOf_ButtonEvents = ThisController.totalbtn
            elif ('ENC' in line3) and not ('COLOR' in line3):
                ThisController.encd = numpar
                if MaximumNumberOf_Encoders < ThisController.encd:
                    MaximumNumberOf_Encoders = ThisController.encd
            elif 'MICRO' in line3:
                ThisController.mic = numpar
                if MaximumNumberOf_Microphones < ThisController.mic:
                    MaximumNumberOf_Microphones = ThisController.mic
            elif 'FAST' in line3:
                val=False
                if ('TRUE' in splline[1].upper()) or ('YES' in splline[1].upper()):
                    val = True
                ThisController.fastvid = val
            elif 'COMBINATION' in line3:
                val = False
                if ('TRUE' in splline[1].upper()) or ('YES' in splline[1].upper()):
                    val = True
                ThisController.btncomb = val
            elif 'SPILED' in line3:
                LedEntries = splline[1].split('"')
                LedEntries2 = []
                for j in range(len(LedEntries)):
                    if not ("," in LedEntries[j]) and LedEntries[j].replace(" ", ""):
                        LedEntries2.append(LedEntries[j])
                ThisController.add_device(*LedEntries2)
            elif 'DEVICE' in line3:
                LedEntries = splline[1].split('"')
                LedEntries2 = []
                for j in range(len(LedEntries)):
                    if not ("," in LedEntries[j]) and LedEntries[j].replace(" ", ""):
                        LedEntries2.append(LedEntries[j])
                        if not (LedEntries[j] in Fast_Animation_Entry_Stoplist):
                            Fast_Animation_Entry_Stoplist.append(LedEntries[j])
                ThisController.add_device(*LedEntries2)
            elif 'VERSION' in line3:
                ThisController.CodeVersion = splline[1].replace(" ", "").lower()


    Possible_Controllers.append(ThisController)
    del Ports
    del CName
    del PortsNames
    del PortsIndex
    ControllerDescriptionLines.clear()
    del ControllerDescriptionLines

def Create_UI_Captions_and_Hint_Contents_list():
    UI_Captions.append("Controllers")
    Hint_Contents_list0.append("Here you can add or remove controller\n"
                              "to the project, set it's parameters,\n"
                              "upload scenarios\n"
                              "or directly control them through console")
    Hint_Contents_list1.append("")
    Hint_Contents_list2.append("")
    UI_Captions.append("Layout")
    Hint_Contents_list0.append("Here you can arrange all your parts\n"
                              "and connect them accordingly\n"
                              "so it will match your actual hardware")
    Hint_Contents_list1.append("")
    Hint_Contents_list2.append("")
    UI_Captions.append("Add Controller")
    Hint_Contents_list0.append("In case you have several DKLed controllers,\n that need to work togather")
    Hint_Contents_list1.append("")
    Hint_Contents_list2.append("")

def Create_Possible_Commands_List():
    with open("Library/Commands.txt") as CommandDescription:
        CommandDescriptionLines = [line.strip() for line in CommandDescription]
    CommandDescription.close()
    HCDVersions = []
    commandList = []
    CommandGroupCount = 0
    CommandGroup = HCD_Command_Group()
    CommandItself = HCD_Command()
    GNotFirst = False
    CNotFirst = False
    for i in range(len(CommandDescriptionLines)):
        line = CommandDescriptionLines[i] #"Something in line = another"
        line2 = line.upper() #"SOMETHING IN LINE = ANOTHER"
        #line3 = line2.replace(" ", "") #"SOMETHINGINLINE=ANOTHER"
        splline = line.split("=", 1)   # ["Something in line ", " another"]
        line3 = splline[0].replace(" ","").upper()
        numpar = 0
        if len(splline)==2:
            spll2 = splline[1].replace(" ", "")
            if spll2.isdigit():
                numpar = int(spll2)
        else:
            splline.append("")
        if ';' in line3:
            line3 = ""
        elif 'HCDVER' in line3:
            LedEntries = splline[1].split('"')
            if len(HCDVersions):
                HCDVersions = []
            for j in range(len(LedEntries)):
                if not ("," in LedEntries[j]) and LedEntries[j].replace(" ", ""):
                    HCDVersions.append(LedEntries[j])
                    if not (LedEntries[j] in HCD_Version_list_Full):
                        HCD_Version_list_Full.append(LedEntries[j])
                        arr = []
                        HCD_Version_association_list.append(arr)
            LedEntries.clear()
        elif 'GROUP' in line3:
            if 'GROUP:' in line3:
                if GNotFirst:
                    CommandGroup = HCD_Command_Group()
                    commandList = []
                CommandGroup.CommandList = commandList
                GNotFirst = True
                CommandGroup.GroupIndex = CommandGroupCount
                CommandGroup.CompartibleFlags = HCDCompartibleFlags.InEveryMode
                CommandGroup.RequiredFlags = HCDCompartibleFlags.InNoMode
                CommandGroupCount += 1
                CommandGroup.HCDVersion = HCDVersions
                HCD_Command_Groups.append(CommandGroup)
            if 'COMMAND' in line3:
                splline[1] = splline[1].replace('"', '').strip(" ")
                CommandGroup.GroupReference = splline[1]
            elif 'NAME' in line3:
                splline[1] = splline[1].replace('"', '').strip(" ")
                CommandGroup.GroupName = splline[1]
            elif 'DESCR' in line3:
                splline[1] = splline[1].replace('"', '').strip(" ")
                CommandGroup.GroupDescription = splline[1]
            elif 'REQUIRE' in line3:
                if 'NONE' in line3:
                    CommandGroup.CompartibleFlags = HCDCompartibleFlags.InEveryMode
                    CommandGroup.RequiredFlags = HCDCompartibleFlags.InNoMode
                else:
                    LedEntries = splline[1].upper().split(',')
                    LedEntries[0] = LedEntries[0].replace(' ', '')
                    Flags = HCDCompartibleFlags.InNoMode
                    if 'LED' in LedEntries[0]:
                        Flags |= HCDCompartibleFlags.InLedMode
                    elif 'SERV' in LedEntries[0]:
                        Flags |= HCDCompartibleFlags.InServoMode
                    elif 'FAST' in LedEntries[0]:
                        Flags |= HCDCompartibleFlags.InFastVideomode
                    elif 'FILE' in LedEntries[0]:
                        Flags |= HCDCompartibleFlags.InFileMode
                    elif ('INDEP' in LedEntries[0]) or ('SINGLE' in LedEntries[0]):
                        Flags |= HCDCompartibleFlags.InSingleInputMode
                    elif 'COMBIN' in LedEntries[0]:
                        Flags |= HCDCompartibleFlags.InCombinationInputMode
                    elif ('COMMAND' in LedEntries[0]) or ('STRING' in LedEntries[0]) or ('TEXT' in LedEntries[0]):
                        Flags |= HCDCompartibleFlags.InTextReceiveMode
                    elif 'BYTE' in LedEntries[0]:
                        Flags |= HCDCompartibleFlags.InByteReceivingMode
                    elif 'USB' in LedEntries[0]:
                        Flags |= HCDCompartibleFlags.InUSBCommunicationMode
                    elif 'SCEN' in LedEntries[0]:
                        Flags |= HCDCompartibleFlags.InAnyButFileMode

                    if len(LedEntries) > 1:
                        if ('ON' in LedEntries[1]) or ('YES' in LedEntries[1]) or ('TRUE' in LedEntries[1]):
                            CommandGroup.RequiredFlags |= Flags
                        else:
                            CommandGroup.CompartibleFlags &= ~Flags
                    #print("Group " + CommandGroup.GroupName + ":")
                    #print("Group Flags (req):"+hex(CommandGroup.RequiredFlags))
                    #print("Group Flags (comp):" + hex(CommandGroup.CompartibleFlags))
                    del Flags
                    LedEntries.clear()
        elif 'COMMAND' in line3:
            if 'COMMAND:' in line3:
                if CNotFirst:
                    CommandItself = HCD_Command()
                CNotFirst = True
                HCD_Command_List.append(CommandItself)
                CommandGroup.CommandList.append(CommandItself)
                CommandItself.group = CommandGroup
                CommandItself.CommandWindowDialog = HCD_Command_Window
                CommandItself.CompartibleFlags = HCDCompartibleFlags.InEveryMode
                CommandItself.RequiredFlags = HCDCompartibleFlags.InNoMode
                CommandItself.SetsFlags = HCDCompartibleFlags.InNoMode
                CommandItself.ClearsFlags = HCDCompartibleFlags.InNoMode
                CommandItself.Parameter1ChangesValue = HCDFirstParameter.Nothing
                CommandItself.Parameter1Depends = HCDFirstParameter.Nothing
                CommandItself.Parameter1Signable = False
                CommandItself.Parameter1Name = ""
                CommandItself.Parameter1Count = 1
                CommandItself.Parameter1Type = HCDFirstParameter.Nothing #No Parameter
                CommandItself.Parameter1Length = 0
                CommandItself.Parameter1Entries = []
            elif 'NAME' in line3:
                splline[1] = splline[1].replace('"', '').strip(" ")
                CommandItself.name = splline[1]
            elif 'DESCR' in line3:
                splline[1] = splline[1].replace('"', '').strip(" ")
                CommandItself.description = splline[1]
            elif 'STACK' in line3: #can be put into button event
                val = False
                if ('TRUE' in splline[1].upper()) or ('YES' in splline[1].upper()):
                    val = True
                CommandItself.stackable = val
            elif 'START' in line3:
                splline[1] = splline[1].replace('"', '').strip(" ")
                CommandItself.text0 = splline[1]
            elif 'SECOND' in line3:
                splline[1] = splline[1].replace('"', '').strip(" ")
                CommandItself.text1 = splline[1]
            elif 'END' in line3:
                splline[1] = splline[1].replace('"', '').strip(" ")
                CommandItself.text2 = splline[1]
            elif 'SET' in line3:
                if 'NONE' in line3:
                    CommandItself.SetsFlags = HCDCompartibleFlags.InNoMode
                else:
                    Entries = splline[1].upper()
                    Flags = HCDCompartibleFlags.InNoMode
                    if 'LED' in Entries:
                        Flags |= HCDCompartibleFlags.InLedMode
                    elif 'SERV' in Entries:
                        Flags |= HCDCompartibleFlags.InServoMode
                    elif 'FAST' in Entries:
                        Flags |= HCDCompartibleFlags.InFastVideomode
                    elif 'FILE' in Entries:
                        Flags |= HCDCompartibleFlags.InFileMode
                    elif ('INDEP' in Entries) or ('SINGLE' in Entries):
                        Flags |= HCDCompartibleFlags.InSingleInputMode
                    elif 'COMBIN' in Entries:
                        Flags |= HCDCompartibleFlags.InCombinationInputMode
                    elif ('COMMAND' in Entries) or ('STRING' in Entries) or ('TEXT' in Entries):
                        Flags |= HCDCompartibleFlags.InTextReceiveMode
                    elif 'BYTE' in Entries:
                        Flags |= HCDCompartibleFlags.InByteReceivingMode
                    elif 'USB' in Entries:
                        Flags |= HCDCompartibleFlags.InUSBCommunicationMode
                    elif 'SCEN' in Entries:
                        Flags |= HCDCompartibleFlags.InAnyButFileMode
                    elif 'WS2812' in Entries:
                        Flags |= HCDCompartibleFlags.InWS2812Compartibility
                    elif 'SK6812' in Entries:
                        Flags |= HCDCompartibleFlags.InSK6812Compartibility

                    CommandItself.SetsFlags |= Flags
                    #print("Command " + CommandItself.name + ":")
                    #print("Command sets Flags:"+hex(CommandItself.SetsFlags))
                    del Flags
                    del Entries
            elif 'CLEAR' in line3:
                if 'NONE' in line3:
                    CommandItself.ClearsFlags = HCDCompartibleFlags.InNoMode
                else:
                    Entries = splline[1].upper()
                    Flags = HCDCompartibleFlags.InNoMode
                    if 'LED' in Entries:
                        Flags |= HCDCompartibleFlags.InLedMode
                    elif 'SERV' in Entries:
                        Flags |= HCDCompartibleFlags.InServoMode
                    elif 'FAST' in Entries:
                        Flags |= HCDCompartibleFlags.InFastVideomode
                    elif 'FILE' in Entries:
                        Flags |= HCDCompartibleFlags.InFileMode
                    elif ('INDEP' in Entries) or ('SINGLE' in Entries):
                        Flags |= HCDCompartibleFlags.InSingleInputMode
                    elif 'COMBIN' in Entries:
                        Flags |= HCDCompartibleFlags.InCombinationInputMode
                    elif ('COMMAND' in Entries) or ('STRING' in Entries) or ('TEXT' in Entries):
                        Flags |= HCDCompartibleFlags.InTextReceiveMode
                    elif 'BYTE' in Entries:
                        Flags |= HCDCompartibleFlags.InByteReceivingMode
                    elif 'USB' in Entries:
                        Flags |= HCDCompartibleFlags.InUSBCommunicationMode
                    elif 'SCEN' in Entries:
                        Flags |= HCDCompartibleFlags.InAnyButFileMode
                    elif 'WS2812' in Entries:
                        Flags |= HCDCompartibleFlags.InWS2812Compartibility
                    elif 'SK6812' in Entries:
                        Flags |= HCDCompartibleFlags.InSK6812Compartibility

                    CommandItself.ClearsFlags |= Flags
                    # print("Command " + CommandItself.name + ":")
                    # print("Command clears Flags:"+hex(CommandItself.ClearsFlags))
                    del Flags
                    del Entries
            elif 'REQUIRE' in line3:
                if 'NONE' in line3:
                    CommandItself.CompartibleFlags = HCDCompartibleFlags.InEveryMode
                    CommandItself.RequiredFlags = HCDCompartibleFlags.InNoMode
                else:
                    LedEntries = splline[1].upper().split(',')
                    LedEntries[0] = LedEntries[0].replace(' ', '')
                    Flags = HCDCompartibleFlags.InNoMode
                    if 'LED' in LedEntries[0]:
                        Flags |= HCDCompartibleFlags.InLedMode
                    elif 'SERV' in LedEntries[0]:
                        Flags |= HCDCompartibleFlags.InServoMode
                    elif 'FAST' in LedEntries[0]:
                        Flags |= HCDCompartibleFlags.InFastVideomode
                    elif 'FILE' in LedEntries[0]:
                        Flags |= HCDCompartibleFlags.InFileMode
                    elif ('INDEP' in LedEntries[0]) or ('SINGLE' in LedEntries[0]):
                        Flags |= HCDCompartibleFlags.InSingleInputMode
                    elif 'COMBIN' in LedEntries[0]:
                        Flags |= HCDCompartibleFlags.InCombinationInputMode
                    elif ('COMMAND' in LedEntries[0]) or ('STRING' in LedEntries[0]) or ('TEXT' in LedEntries[0]):
                        Flags |= HCDCompartibleFlags.InTextReceiveMode
                    elif 'BYTE' in LedEntries[0]:
                        Flags |= HCDCompartibleFlags.InByteReceivingMode
                    elif 'USB' in LedEntries[0]:
                        Flags |= HCDCompartibleFlags.InUSBCommunicationMode
                    elif 'SCEN' in LedEntries[0]:
                        Flags |= HCDCompartibleFlags.InAnyButFileMode
                    elif 'WS2812' in LedEntries[0]:
                        Flags |= HCDCompartibleFlags.InWS2812Compartibility
                    elif 'SK6812' in LedEntries[0]:
                        Flags |= HCDCompartibleFlags.InSK6812Compartibility
                    if len(LedEntries) > 1:
                        if ('ON' in LedEntries[1]) or ('YES' in LedEntries[1]) or ('TRUE' in LedEntries[1]):
                            CommandItself.RequiredFlags |= Flags
                        else:
                            CommandItself.CompartibleFlags &= ~Flags
                    #print("Command " + CommandItself.name + ":")
                    #print("Command Flags (req):"+hex(CommandItself.RequiredFlags))
                    #print("Command Flags (comp):" + hex(CommandItself.CompartibleFlags))
                    del Flags
                    LedEntries.clear()
        elif 'PARAM' in line3:
            if '1' in line3: #first parameter format
                if 'TYPE' in line3:
                    # parameter Type:
                    # 0 - int
                    # 1 - free string
                    # 2 - string of special characters
                    # 3 - File index
                    # 4 - input events (buttons and virtual buttons)
                    # 5 - real button index
                    # 6 - virtual button index
                    # 7 - Port (any)
                    # 8 - Port (not USB)
                    # 9 - Output (LED, Servo, etc) index
                    # 10 - smartLED output index
                    # 11 - Servo (PWM) output index
                    # 13 - content aware
                    line3 = splline[1].upper()
                    if 'SPECIAL' in line3: #string of special characters (controller ID, Uart answers)
                        CommandItself.Parameter1Type = HCDFirstParameter.FormattedStr
                    elif 'STR' in line3: #string (text output, subfolder name...)
                        CommandItself.Parameter1Type = HCDFirstParameter.String
                    elif 'FILE' in line3: #file index (first 4 hex digits in filename)
                        CommandItself.Parameter1Type = HCDFirstParameter.FileIndex
                    elif 'PERIP' in line3:  # UART or other periphery port index
                        CommandItself.Parameter1Type = HCDFirstParameter.UART
                    elif 'PORT' in line3:  # any supported I/O port index
                        CommandItself.Parameter1Type = HCDFirstParameter.Port
                    elif ('INPUT' in line3) or ('EVENT' in line3): #inputs index (in hex)
                        CommandItself.Parameter1Type = HCDFirstParameter.InputEvent
                    elif 'VIRTUALBUTTON' in line3: #only virtual buttons (in hex)
                        CommandItself.Parameter1Type = HCDFirstParameter.VButton
                    elif 'BUTTON' in line3: #only real buttons (in hex)
                        CommandItself.Parameter1Type = HCDFirstParameter.Button
                    elif 'LED' in line3: #smartLED output index
                        CommandItself.Parameter1Type = HCDFirstParameter.SpiLED
                    elif ('SERV' in line3) or ('SWITCH' in line3): #servo output index
                        CommandItself.Parameter1Type = HCDFirstParameter.Servo
                    elif 'OUT' in line3: #Output (LED, Servo, etc) index
                        CommandItself.Parameter1Type = HCDFirstParameter.Output
                    elif ('SLID' in line3) or ('DIAL' in line3):
                        CommandItself.Parameter1Type = HCDFirstParameter.Percentage
                        if len(CommandItself.Parameter1Entries) < 1:
                            CommandItself.Parameter1Entries.append(0)
                            CommandItself.Parameter1Entries.append(65535)
                        while not type(CommandItself.Parameter1Entries[0]) == int:
                            CommandItself.Parameter1Entries.pop(0)
                    elif ('NUMBER' in line3) or ('INT' in line3): #number (Baudrate, brightness, delay, etc)
                        CommandItself.Parameter1Type = HCDFirstParameter.Int
                        if len(CommandItself.Parameter1Entries) < 1:
                            CommandItself.Parameter1Entries.append(0)
                            CommandItself.Parameter1Entries.append(65535)
                        while not type(CommandItself.Parameter1Entries[0]) == int:
                            CommandItself.Parameter1Entries.pop(0)
                elif 'LEN' in line3: #number of symbols in entry, 0 = any number
                    # in INT mode counts only digit symbols, i.e. +- symbols are not counted
                    CommandItself.Parameter1Length = numpar
                elif 'NUMBER' in line3: #number of possible entries, 0 = controller dependant
                    CommandItself.Parameter1Count = numpar
                elif 'RANGE' in line3:
                    if CommandItself.Parameter1Type == HCDFirstParameter.Int:
                        CommandItself.Parameter1Entries.clear()
                        splline[1] = splline[1].replace(" ", "")
                        ent_ = splline[1].split(",", 1)
                        if len(ent_) == 2:
                            if ent_[0].isdigit():
                                CommandItself.Parameter1Entries.append(int(ent_[0]))
                            else:
                                CommandItself.Parameter1Entries.append(0)
                            if ent_[1].isdigit():
                                CommandItself.Parameter1Entries.append(int(ent_[1]))
                            else:
                                CommandItself.Parameter1Entries.append(65535)
                        else:
                            CommandItself.Parameter1Entries.append(0)
                            CommandItself.Parameter1Entries.append(65535)
                elif 'NAME' in line3:
                    splline[1] = splline[1].replace('"', '').strip(" ")
                    CommandItself.Parameter1Name = splline[1]
                elif 'SIGN' in line3:
                    val = False
                    if ('TRUE' in splline[1].upper()) or ('YES' in splline[1].upper()):
                        val = True
                    CommandItself.Parameter1Signable = val
                elif 'SET' in line3:
                    if 'NONE' in line3:
                        CommandItself.Parameter1ChangesValue = HCDFirstParameter.Nothing
                    else:
                        Entries = splline[1].upper()
                        if ('DEFAULT' in Entries) and ('STEPS' in Entries) and (('PWM' in Entries) or ('SERV' in Entries)):
                            CommandItself.Parameter1ChangesValue = HCDFirstParameter.DefaultingPWMDiscreet
                        elif ('STEPS' in Entries) and (('PWM' in Entries) or ('SERV' in Entries)):
                            CommandItself.Parameter1ChangesValue = HCDFirstParameter.SettingPWMDiscreet
                        elif ('DEFAULT' in Entries) and ('BRIGHT' in Entries):
                            CommandItself.Parameter1ChangesValue = HCDFirstParameter.DefaultingMaxBrightness
                        elif 'BRIGHT' in Entries:
                            CommandItself.Parameter1ChangesValue = HCDFirstParameter.SettingMaxBrightness
                        elif ('FRAM' in Entries) and ('UNIQ' in Entries):
                            CommandItself.Parameter1ChangesValue = HCDFirstParameter.SettingFAFrameNumber
                        elif ('FRAM' in Entries) and ('SIZE' in Entries):
                            CommandItself.Parameter1ChangesValue = HCDFirstParameter.SettingFAFrameSize
                        # print("Command " + CommandItself.name + ":")
                        # print("Command sets Flags:"+hex(CommandItself.SetsFlags))
                        del Entries
                elif 'DEPEND' in line3:
                    if 'NONE' in line3:
                        CommandItself.Parameter1Depends = HCDFirstParameter.Nothing
                    else:
                        Entries = splline[1].upper()
                        if ('STEPS' in Entries) and (('PWM' in Entries) or ('SERV' in Entries)):
                            CommandItself.Parameter1Depends = HCDFirstParameter.SettingPWMDiscreet
                        elif 'BRIGHT' in Entries:
                            CommandItself.Parameter1Depends = HCDFirstParameter.SettingMaxBrightness
                        elif ('FRAM' in Entries) and ('UNIQ' in Entries):
                            CommandItself.Parameter1Depends = HCDFirstParameter.SettingFAFrameNumber
                        elif ('FRAM' in Entries) and ('SIZE' in Entries):
                            CommandItself.Parameter1Depends = HCDFirstParameter.SettingFAFrameSize
                        # print("Command " + CommandItself.name + ":")
                        # print("Command sets Flags:"+hex(CommandItself.SetsFlags))
                        del Entries






    CommandDescriptionLines.clear()
    del CommandDescriptionLines
    for i in range(len(HCD_Command_Groups)):
        for j in range(len(HCD_Version_list_Full)):
            if HCD_Version_list_Full[j] in HCD_Command_Groups[i].HCDVersion:
                HCD_Version_association_list[j].append(i)
    #    print("Group no " + str(HCD_Command_Groups[i].GroupIndex) +":")
    #    print(HCD_Command_Groups[i].HCDVersion)
    #    print("Group reference "+ HCD_Command_Groups[i].GroupReference + ", name " + HCD_Command_Groups[i].GroupName)
    #    print("Requires "+ HCD_Command_Groups[i].toggleAttribute + " in state "
    #    + str(HCD_Command_Groups[i].toggleAttributeState))
    #    print(HCD_Command_Groups[i].GroupDescription)
    #print(HCD_Version_association_list)
    for i in range(len(HCD_Version_list_Full)):
        ui.Command_Ver.addItem(HCD_Version_list_Full[i])
    ui.Command_Ver.currentIndexChanged.connect(Update_Command_List_Tab_CommandGroup)
    ui.Command_Section.currentIndexChanged.connect(Update_Command_List_Tab_CommandList)
    ui.Command_Add_Button.clicked.connect(Show_Command_Window)







app = QtWidgets.QApplication([])
ui = uic.loadUi("Library/Main_window.ui")
ui.setWindowTitle("DKLed Editor")

ui.tabWidget_2.setElideMode(3)  # Qt::ElideNone
ui.tabWidget_2.setUsesScrollButtons(True)

Main_Window = Functional_Window()
Main_Window.ParentAction = None

Possible_Controllers = []
HCD_Command_Groups = []
HCD_Command_List = []
HCD_Version_list_Full = []
HCD_Version_association_list = []

Hint_Contents_list0 = []
Hint_Contents_list1 = []
Hint_Contents_list2 = []
Hint_Contents_list = [Hint_Contents_list0, Hint_Contents_list1, Hint_Contents_list2]
Hint_Contents_Show = 3
Hinted_widgets_list = []
Encoder_Color_Highlights = []
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


UI_Captions = []
MaximumNumberOf_Outputs = 0
MaximumNumberOf_Buttons = 0
MaximumNumberOf_ButtonEvents = 0
MaximumNumberOf_Ports = 0
MaximumNumberOf_Encoders = 0
MaximumNumberOf_Microphones = 0

Controllers = []
Fast_Animation_Entry_Stoplist = []

UIdFormat = QRegularExpression("[0-9,A-Z.`~!@#$%^&<>?]{4}")
UIdValidator = QRegularExpressionValidator(UIdFormat)

Create_Possible_Controller_List()
Create_Possible_Commands_List()
Update_Command_List_Tab_CommandGroup()

Create_UI_Captions_and_Hint_Contents_list()

Hinted_widgets_list.append([ui.Main_Tabs, 0, 0])
Hinted_widgets_list.append([ui.Main_Tabs, 1, 1])
Hinted_widgets_list.append([ui.Main_Tabs_Controller_Add_Button, 2])
Update_Tooltip_Text_over_application()


Controllers.append(None)
Controllers[0] = Controller()
Controllers[0].TabName = "Default"
Controllers[0].add_tab(Controllers[0].TabName)
Controllers[0].fill_tab()

ui.Main_Tabs_Controller_Add_Button.clicked.connect(Add_Controller_Tab)

Main_Window.Install_Widget(ui)

#ui.show()
app.exec()
