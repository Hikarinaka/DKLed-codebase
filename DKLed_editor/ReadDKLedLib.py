from enum import Enum
import sys
from CustomQWidgets import QtComboBoxUnScroll, Functional_Window
from PyQt5 import QtWidgets
from PyQt5.QtCore import QRegularExpression, Qt

from DKLedProjecLayout import DKLed_Project_Layout_Types

class Layout_Types_Comparison:
    def __init__(self, N = "", T = DKLed_Project_Layout_Types.Nothing):
        self.ControlledName = N
        self.LayoutType = T

class HCDFirstParameter(Enum):
    Nothing = 0
    Int = 1  # int (usually in HEX)
    Percentage = 2
    String = 3  # free string
    FormattedStr = 4  # string of special characters
    FileIndex = 5  # File index
    InputEvent = 6  # input events (buttons and virtual buttons)
    Button = 7  # real button index
    VButton = 8  # virtual button index
    Port = 9  # Port (any)
    PortRx = 10
    PortTx = 11
    UART = 12  # Port (not USB)
    UARTrx = 13
    UARTtx = 14
    I2C = 15
    Output = 16  # Output (LED, Servo, etc) index
    SpiLED = 17  # smartLED output index
    Servo = 18  # Servo (PWM) output index
    Dependant = 19  # controller aware
    DependantInt = 20  # controller aware number (max and min are set by controller parameters)

    # controller aware enums:
    TotalOutputs = 26  # number of outputs in this controller
    TotalLEDOutputs = 27
    TotalServoOutputs = 28

    TotalInputs = 31
    TotalEncoderInputs = 32
    TotalNotMappedButtons = 33
    TotalEvents = 34

    # Working settings/triggers
    SettingPWMDiscreet = 41  # Discretization for PWM
    DefaultingPWMDiscreet = 42  # sets PWM Discretization to default
    SettingMaxBrightness = 43
    DefaultingMaxBrightness = 44
    SettingFAFrameNumber = 45
    SettingFADelay = 46
    SettingRelativePauseWithUpdate = 47
    SettingRelativePause = 48
    SettingAbsoluteTime = 49
    SettingAbsoluteTimeEventWithUpdate = 50
    SettingAbsoluteTimeEvent = 51
    SettingRNDSeed = 52

    #Timeline associativity
    TimeLineSetup = 100 #command can be only at the start sile 0000.hcd
    TimeLineLED = 101 #where screens are and S-commands are generated
    TimeLineServo = 102
    TimeLinePort = 103 #AnyPort
    TimeLineUSB = 104 #USB only
    TimeLineUARTIn = 105 #I/O ports only
    TimeLineUARTOut = 106
    TimeLineEvents = 107 #Change Button events
    TimeLineNavigation = 108 #subprograms and (un)conditional jump-to-file stuff

class HCDCompartibleFlags:
    # flags for indicating working mode
    # incompartible modes are modes that can not be triggered in controller at the same time
    # so command can work with both of them, but that modes can not be required or in stop list at the same time
    InEveryMode = 0xFFFF  # all modes compartible
    InNoMode = 0  # no mode compartible
    InLedMode = 1  # this command works in every mode that supports LEDs
    InServoMode = 2  # this command works in every mode that supports servos
    # fast Video mode is incompartible with servo mode, so InServoMode = NOT Fast Video mode
    InFastVideomode = 4
    InFileMode = 8  # this command overrides with Led, Servo and Fast modes
    InAnyButFileMode = 16  # incompartible with file mode

    InSingleInputMode = 32
    InCombinationInputMode = 64  # Single Input and Combination modes are incompartible

    InTextReceiveMode = 128  # ports in text receiving mode, incompartible with Byte receiving mode
    InByteReceivingMode = 256  # post in byte receiving mode
    InUSBCommunicationMode = 512

    InWS2812Compartibility = 1024
    InSK6812Compartibility = 2048
    InRealtimeServoMode = 4096

class MaximumNumberOf:
    Outputs = 0
    Buttons = 0
    ButtonEvents = 0
    Ports = 0
    Encoders = 0
    Microphones = 0

class DKLed_Specs:
    def __init__(self, name_="---", outs_=8, btns_=5, portlines_ = 4, totalbtn_ = 64, encd_ = 2):
        self.name = name_
        self.out = outs_
        self.btn = btns_
        if totalbtn_<btns_:
            totalbtn_ = btns_
        self.totalbtn = totalbtn_
        self.encd = encd_
        self.uart =0
        self.mic = 0
        self.btncomb = False #true if button combination is possible
        #self.wifi = 0
        #self.blth = 0
        self.fastvid = False
        self.devices = []
        self.uartModesIN = []
        self.uartModesOUT = []
        self.uartTypes = []
        self.uartLines = []
        self.uartTotalLines = portlines_
        self.CodeVersion = ""
        self.USBSupport = False
        self.DefaultPWMDiscreet = 512
        self.DefaultMaxPWMDiscreet = 1024
        self.DefaultMinPWMDiscreet = 64
        self.DefaultMinPWMPosition = 0 #minimum value for Start and Stop PWM positions
        self.DefaultMaxPWMPosition = 10000 #maximum value for Start and Stop PWM positions
        self.DefaultStartPWMPosition = 500
        self.DefaultStopPWMPosition = 2500
        self.DefaultMinPWMFreq = 20 #Hz
        self.DefaultMaxPWMFreq = 1600 #Hz
        self.DefaultPWMRealtime = False
        self.DefaultMaxRandomFileList = 200
        self.DefaultMinFastAnimFreq = 20 #Hz
        self.DefaultMaxFastAnimFreq = 25600 #frequency for 1 pixel frame, actual_max_frequency = MaxPWMFreq*16/frame_length
        self.DefaultMaxBrightness = 32
        self.DefaultMinBrightness = 1
        self.DefaultMinButtonSetWaitTimeout = 20 #ms
        self.DefaultMaxButtonSetWaitTimeout = 10000 #ms
        self.DefaultButtonCombinationSetWaitTimeout = 500 #ms
        self.DefaultMinUARTWaitTimeout = 0  # ms
        self.DefaultMaxUARTWaitTimeout = 65535  # ms
        self.DefaultMinUSBWaitTimeout = 0  # ms
        self.DefaultMaxUSBWaitTimeout = 65535  # ms
        self.DefaultMinRelativeSpeed = 1 #%
        self.DefaultMaxRelativeSpeed = 10000 #%
        self.DefaultMinAbsoluteSpeed = 1 #%
        self.DefaultMaxAbsoluteSpeed = 10000 #%
        self.LedsPerOutput = 512#maximum number of pixels per output

        if MaximumNumberOf.Outputs < outs_:
            MaximumNumberOf.Outputs = outs_
        if MaximumNumberOf.Buttons < btns_:
            MaximumNumberOf.Buttons = btns_
        if MaximumNumberOf.ButtonEvents < totalbtn_:
            MaximumNumberOf.ButtonEvents = totalbtn_
        if MaximumNumberOf.Encoders < encd_:
            MaximumNumberOf.Encoders = encd_

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
        if len(self.uartLines) < self.uart:
            self.uartLines.append(1)
        if len(self.uartModesOUT) < self.uart:
            self.uartModesOUT.append(None)
            self.uartModesOUT[self.uart - 1] = []
        self.uartModesIN[u - 1] = arr
        if MaximumNumberOf.Ports < u:
            MaximumNumberOf.Ports = u

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
        if len(self.uartLines) < self.uart:
            self.uartLines.append(1)
        if len(self.uartModesOUT) < self.uart:
            self.uartModesOUT.append(None)
        elif self.uartModesOUT[u - 1]:
            self.uartModesOUT[u - 1].clear()
        self.uartModesOUT[u - 1] = arr
        if MaximumNumberOf.Ports < u:
            MaximumNumberOf.Ports = u

    def uart_count_Lines(self,u,Ln):
        if u < 1:
            u = 1
        if u > self.uart:
            self.uart += 1
            u = self.uart
        if len(self.uartLines) < self.uart:
            self.uartLines.append(1)
        if len(self.uartModesIN) < self.uart:
            self.uartModesIN.append(None)
            self.uartModesIN[self.uart-1] = []
        if len(self.uartModesOUT) < self.uart:
            self.uartModesOUT.append(None)
            self.uartModesOUT[self.uart - 1] = []
        if len(self.uartTypes) < self.uart:
            self.uartTypes.append(None)
        self.uartLines[u - 1] = Ln
        if MaximumNumberOf.Ports < u:
            MaximumNumberOf.Ports = u
        self.uartTotalLines = 0
        for u in range(0,self.uart):
            self.uartTotalLines += self.uartLines[u]

class DKLed_Periph_Specs:
    def __init__(self, name_="---", outs_=8, btns_=5, portlines_=4, totalbtn_ = 64, encd_ = 2, ET = DKLed_Project_Layout_Types.Nothing):
        self.name = name_
        self.subname = ""
        self.out = outs_
        self.btn = btns_
        if totalbtn_<btns_:
            totalbtn_ = btns_
        self.totalbtn = totalbtn_
        self.uartTotalLines = portlines_
        self.encd = encd_
        self.EntityType = ET

        self.PWMDiscreet = 512  # how many steps for PWM discretization
        self.PWMFrequency = 50 #Hz
        self.PWMMinImpulse = 500 #microseconds
        self.PWMMaxImpulse = 2500 #microseconds

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
        self.ControllerAndFileLabel = QtWidgets.QLabel("nothing", self.ParentPage, )
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
            if self.QBox_GroupChoice.count() > 0:
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
            if self.QBox_CommandChoice.count() > 0:
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
                    num_ = CommandItself.Parameter1Count
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
        comm_ = ""
        #comm_ = HCD_Command_Groups[ind2].CommandList[ind3].text0
        #if HCD_Command_Groups[ind2].CommandList[ind3].text1:
        par1arr = []
        par2arr = [0, 1]
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

def Create_Possible_Controller_List():

    global Encoder_Color_Highlights

    with open("Library/Controllers.txt") as ControllerDescription:
        ControllerDescriptionLines = [line.strip() for line in ControllerDescription]
    ControllerDescription.close()
    Ports = 0
    CName = ""
    PortsNames = []
    PortsIndex = []
    PortsLines = []
    ThisController = None
    ThisController = DKLed_Specs()
    for i in range(len(ControllerDescriptionLines)):
        line = ControllerDescriptionLines[i]
        splline = line.split("=", 1)
        line2 = splline[0].upper()
        line3 = line2.replace(" ", "")
        numpar = 0
        if len(splline) == 2:
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
            #ThisController.DefaultPWMDiscreet = 512
            #ThisController.DefaultMaxBrightness = 32
            #ThisController.DefaultMinBrightness = 32
            PortsNames.clear()
            PortsIndex.clear()
        elif 'USBSUPP' in line3:
            val = False
            if ('TRUE' in splline[1].upper()) or ('YES' in splline[1].upper()):
                val = True
            ThisController.USBSupport = val
        elif 'DEFAULT' in line3:
            if ('PWM' in line3) or ('SERV' in line3):
                if 'REAL' in line3:
                    val = False
                    if ('TRUE' in splline[1].upper()) or ('YES' in splline[1].upper()):
                        val = True
                    ThisController.DefaultPWMRealtime = val
                elif 'STEPS' in line3:
                    if 'MAX' in line3:
                        ThisController.DefaultMaxPWMDiscreet = numpar
                    elif 'MIN' in line3:
                        ThisController.DefaultMinPWMDiscreet = numpar
                    else:
                        ThisController.DefaultPWMDiscreet = numpar
                    if ThisController.DefaultPWMDiscreet < ThisController.DefaultMinPWMDiscreet:
                        ThisController.DefaultPWMDiscreet = ThisController.DefaultMinPWMDiscreet
                    if ThisController.DefaultPWMDiscreet > ThisController.DefaultMaxPWMDiscreet:
                        ThisController.DefaultPWMDiscreet = ThisController.DefaultMaxPWMDiscreet
                elif ('POS' in line3) or ('PER' in line3):
                    if 'MAX' in line3:
                        ThisController.DefaultMaxPWMPosition = numpar
                    elif 'MIN' in line3:
                        ThisController.DefaultMinPWMPosition = numpar
                    elif ('START' in line3) or ('BEG' in line3):
                        ThisController.DefaultStartPWMPosition = numpar
                    elif ('STOP' in line3) or ('END' in line3):
                        ThisController.DefaultStopPWMPosition = numpar
                elif 'FREQ' in line3:
                    if 'MAX' in line3:
                        ThisController.DefaultMaxPWMFreq = numpar
                    elif 'MIN' in line3:
                        ThisController.DefaultMinPWMFreq = numpar
            elif 'BRIGHT' in line3:
                if 'MAX' in line3:
                    ThisController.DefaultMaxBrightness = numpar
                elif 'MIN' in line3:
                    ThisController.DefaultMinBrightness = numpar
            elif 'FAST' in line3:
                if 'MAX' in line3:
                    ThisController.DefaultMaxFastAnimFreq = numpar
                elif 'MIN' in line3:
                    ThisController.DefaultMinFastAnimFreq = numpar
            elif 'FILE' in line3:
                ThisController.DefaultMaxRandomFileList = numpar
            elif 'BUT' in line3:
                if 'MAX' in line3:
                    ThisController.DefaultMaxButtonSetWaitTimeout = numpar
                elif 'MIN' in line3:
                    ThisController.DefaultMinButtonSetWaitTimeout = numpar
                elif 'COMB' in line3:
                    ThisController.DefaultButtonCombinationSetWaitTimeout = numpar
            elif ('PAUS' in line3) or ('DEL' in line3):
                if 'REL' in line3:
                    if 'MAX' in line3:
                        ThisController.DefaultMaxRelativeSpeed = numpar
                    elif 'MIN' in line3:
                        ThisController.DefaultMinRelativeSpeed = numpar
                elif 'ABS' in line3:
                    if 'MAX' in line3:
                        ThisController.DefaultMaxAbsoluteSpeed = numpar
                    elif 'MIN' in line3:
                        ThisController.DefaultMinAbsoluteSpeed = numpar
            elif ('UART' in line3) or ('USART' in line3):
                if 'MAX' in line3:
                    ThisController.DefaultMaxUARTWaitTimeout = numpar
                elif 'MIN' in line3:
                    ThisController.DefaultMinUARTWaitTimeout = numpar
            elif 'USB' in line3:
                if 'MAX' in line3:
                    ThisController.DefaultMaxUSBWaitTimeout = numpar
                elif 'MIN' in line3:
                    ThisController.DefaultMinUSBWaitTimeout = numpar
        elif 'PORT' in line3:
            if 'PORT:' in line3:
                Ports += 1
                PortsNames.append("")
                PortsIndex.append(0)
                PortsLines.append(1)
            if Ports == 0:
                Ports = 1
            if len(PortsIndex) == 0:
                PortsIndex.append(0)
            if len(PortsLines) == 0:
                PortsLines.append(1)
            if len(PortsNames) == 0:
                PortsNames.append("")
            if 'NAME' in line3:
                PortsNames[Ports - 1] = splline[1].replace(" ", "")
            elif 'NUMBER' in line3:
                if numpar>Ports:
                    numpar = Ports
                PortsIndex[Ports-1] = numpar
            elif 'RECEIVE' in line3:
                LedEntries = splline[1].split('"')
                LedEntries2 = []
                for j in range(len(LedEntries)):
                    if not ("," in LedEntries[j]) and LedEntries[j].replace(" ", ""):
                        LedEntries2.append(LedEntries[j])
                ThisController.uart_modes_IN(PortsNames[Ports - 1], PortsIndex[Ports-1], *LedEntries2)
            elif 'TRANSMIT' in line3:
                LedEntries = splline[1].split('"')
                LedEntries2 = []
                for j in range(len(LedEntries)):
                    if not ("," in LedEntries[j]) and LedEntries[j].replace(" ", ""):
                        LedEntries2.append(LedEntries[j])
                ThisController.uart_modes_OUT(PortsNames[Ports - 1], PortsIndex[Ports-1], *LedEntries2)
            elif ('LINE' in line3) or ('WIR' in line3):
                if numpar > 0:
                    PortsLines[Ports - 1] = numpar
                elif ('TWOW' in splline[1].upper()) :
                    PortsLines[Ports - 1] = 1
                elif ('DUAL' in splline[1].upper()) or ('TWO' in splline[1].upper()) or ('DOU' in splline[1].upper()) or ('ONEW' in splline[1].upper()):
                    PortsLines[Ports - 1] = 2
                elif ('DUPL' in splline[1].upper()) or ('ON' in splline[1].upper()) or ('SIN' in splline[1].upper()):
                    PortsLines[Ports - 1] = 1
                ThisController.uart_count_Lines(PortsIndex[Ports-1],PortsLines[Ports - 1])
        elif 'NAME' in line3:
            if 'OUT' in line3:
                LedEntries = splline[1].split('"')
                Tp = DKLed_Project_Layout_Types.Nothing
                if 'RELAY' in line3:
                    Tp = DKLed_Project_Layout_Types.Relay
                elif ('SCREEN' in line3) or ('SPILED' in line3):
                    Tp = DKLed_Project_Layout_Types.LedScreen
                elif 'WS2812C' in line3:
                    Tp = DKLed_Project_Layout_Types.WS2815C
                elif 'WS2812' in line3:
                    Tp = DKLed_Project_Layout_Types.WS2812B
                elif 'SK6812' in line3:
                    Tp = DKLed_Project_Layout_Types.SK6812
                elif 'SERVO' in line3:
                    Tp = DKLed_Project_Layout_Types.Servo
                elif 'STRIPE' in line3:
                    Tp = DKLed_Project_Layout_Types.Ledstripe
                elif 'DRIVE' in line3:
                    Tp = DKLed_Project_Layout_Types.Drive
                elif 'PWM' in line3:
                    Tp = DKLed_Project_Layout_Types.PWM
                elif ('MIX' in line3) and (('SK' in line3) or ('WS' in line3)):
                    Tp = DKLed_Project_Layout_Types.WSSKMix
                for j in range(len(LedEntries)):
                    if not ("," in LedEntries[j]) and LedEntries[j].replace(" ", ""):
                        OutputNames2LayoutTypes.append(Layout_Types_Comparison(LedEntries[j], Tp))
            else:
                CName = splline[1].replace(" ", "")
                # print(line + "  :" + CName)
                ThisController.name = CName
        elif 'EVE' in line3:
            ThisController.totalbtn = numpar
            if MaximumNumberOf.ButtonEvents < ThisController.totalbtn:
                MaximumNumberOf.ButtonEvents = ThisController.totalbtn
        elif ('ENC' in line3) and not ('COLOR' in line3):
            ThisController.encd = numpar
            if MaximumNumberOf.Encoders < ThisController.encd:
                MaximumNumberOf.Encoders = ThisController.encd
        elif 'MICRO' in line3:
            ThisController.mic = numpar
            if MaximumNumberOf.Microphones < ThisController.mic:
                MaximumNumberOf.Microphones = ThisController.mic
        elif 'FAST' in line3:
            val = False
            if ('TRUE' in splline[1].upper()) or ('YES' in splline[1].upper()):
                val = True
            ThisController.fastvid = val
        elif 'COMBINATION' in line3:
            val = False
            if ('TRUE' in splline[1].upper()) or ('YES' in splline[1].upper()):
                val = True
            ThisController.btncomb = val
        elif 'SPILED' in line3:
            if ('COUNT' in line3) or ('NUMB' in line3):
                ThisController.LedsPerOutput = numpar
            else:
                LedEntries = splline[1].split('"')
                LedEntries2 = []
                for j in range(len(LedEntries)):
                    if not ("," in LedEntries[j]) and LedEntries[j].replace(" ", ""):
                        LedEntries2.append(LedEntries[j])
                        if not (LedEntries[j] in Fast_Animation_Entry_GreenLight):
                            Fast_Animation_Entry_GreenLight.append(LedEntries[j])
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
        elif 'INP' in line3:
            ThisController.btn = numpar
            if MaximumNumberOf.Buttons < ThisController.btn:
                MaximumNumberOf.Buttons = ThisController.btn
        elif 'OUT' in line3:
            ThisController.out = numpar
            if MaximumNumberOf.Outputs < ThisController.out:
                MaximumNumberOf.Outputs = ThisController.out
        elif 'VERSION' in line3:
            ThisController.CodeVersion = splline[1].replace(" ", "").lower()
        #else:

    #print(Fast_Animation_Entry_Stoplist)
    #for ONm in OutputNames2LayoutTypes:
        #print(str(ONm.LayoutType) + " is called "+ONm.ControlledName)
    Possible_Controllers.append(ThisController)
    del Ports
    del CName
    del PortsNames
    del PortsIndex
    del PortsLines
    ControllerDescriptionLines.clear()
    del ControllerDescriptionLines

def Create_Possible_Periph_List():

    with open("Library/Periph.txt") as ControllerDescription:
        ControllerDescriptionLines = [line.strip() for line in ControllerDescription]
    ControllerDescription.close()
    Ports = 0
    CName = ""

    Possible_Periph.append(DKLed_Periph_Specs("Button", 1, 0, 0, 0, 0, DKLed_Project_Layout_Types.Button)) #0
    Possible_Periph.append(DKLed_Periph_Specs("Keyboard", 2, 0, 0, 0, 0, DKLed_Project_Layout_Types.Keyboard)) #1
    Possible_Periph.append(DKLed_Periph_Specs("Encoder", 3, 0, 0, 0, 0, DKLed_Project_Layout_Types.Encoder))    #2
    Possible_Periph.append(DKLed_Periph_Specs("Relay", 1, 1, 0, 0, 0, DKLed_Project_Layout_Types.Relay))  # 3
    Possible_Periph.append(DKLed_Periph_Specs("Servo", 0, 1, 0, 0, 0, DKLed_Project_Layout_Types.Servo))  # 4
    Possible_Periph[4].subname = "SG90"

    ThisController = DKLed_Periph_Specs()
    for i in range(len(ControllerDescriptionLines)):
        line = ControllerDescriptionLines[i]

        splline = line.split("=", 1)
        line2 = splline[0].upper()
        line3 = line2.replace(" ", "")
        line4 = ""
        numpar = 0
        if len(splline)==2:
            spll2 = splline[1].replace(" ", "")
            line4 = splline[1].upper()
            if spll2.isdigit():
                numpar = int(spll2)
        if ';' in line3:
            line3 = ""

        elif 'PERIPHERY:' in line3:
            if CName != "":
                Possible_Periph.append(ThisController)
                ThisController = DKLed_Periph_Specs()
            Ports = 0
            CName = ""
        elif 'PORT' in line3:
            if ('LINE' in line3) or ('WIR' in line3):
                if numpar > 0:
                    ThisController.uartTotalLines = numpar
                elif ('TWOW' in line4) :
                    ThisController.uartTotalLines = 1
                elif ('DUAL' in line4) or ('TWO' in line4) or ('DOU' in line4) or ('ONEW' in line4):
                    ThisController.uartTotalLines = 2
                elif ('DUPL' in line4) or ('ON' in line4) or ('SIN' in line4):
                    ThisController.uartTotalLines = 1
                elif ('NO' in line4) :
                    ThisController.uartTotalLines = 0
        elif 'PRODUCT' in line3:
            CName = splline[1].replace(" ","")
            LedEntries = splline[1].split('"')
            for j in range(len(LedEntries)):
                if not ("," in LedEntries[j]) and LedEntries[j].replace(" ", ""):
                    CName = LedEntries[j]
            ThisController.subname=CName
        elif 'KIND' in line3:
            ThisController.EntityType = DKLed_Project_Layout_Types.Nothing
            if 'RELAY' in line4:
                ThisController.EntityType = DKLed_Project_Layout_Types.Relay
            elif ('SCREEN' in line4) or ('SPILED' in line4):
                ThisController.EntityType = DKLed_Project_Layout_Types.LedScreen
            elif 'WS2812C' in line4:
                ThisController.EntityType = DKLed_Project_Layout_Types.WS2815C
            elif 'WS2812' in line4:
                ThisController.EntityType = DKLed_Project_Layout_Types.WS2812B
            elif 'SK6812' in line4:
                ThisController.EntityType = DKLed_Project_Layout_Types.SK6812
            elif 'SERVO' in line4:
                ThisController.EntityType = DKLed_Project_Layout_Types.Servo
            elif 'STRIPE' in line4:
                ThisController.EntityType = DKLed_Project_Layout_Types.Ledstripe
            elif 'DRIVE' in line4:
                ThisController.EntityType = DKLed_Project_Layout_Types.Drive
            elif 'PWM' in line4:
                ThisController.EntityType = DKLed_Project_Layout_Types.PWM
        elif 'NAME' in line3:
            CName = splline[1].replace(" ","")
            #print(line + "  :" + CName)
            ThisController.name=CName
        elif 'EVE' in line3:
            ThisController.totalbtn = numpar
            if MaximumNumberOf.ButtonEvents < ThisController.totalbtn:
                MaximumNumberOf.ButtonEvents = ThisController.totalbtn
        elif ('ENC' in line3) and not ('COLOR' in line3):
            ThisController.encd = numpar
            if MaximumNumberOf.Encoders < ThisController.encd:
                MaximumNumberOf.Encoders = ThisController.encd
        elif 'INP' in line3:
            ThisController.btn = numpar
            if MaximumNumberOf.Buttons < ThisController.btn:
                MaximumNumberOf.Buttons = ThisController.btn
        elif 'OUT' in line3:
            ThisController.out = numpar
            if MaximumNumberOf.Outputs < ThisController.out:
                MaximumNumberOf.Outputs = ThisController.out
        elif ('PWM' in line3) or ('SERV' in line3):
            if 'STEPS' in line3:
                ThisController.PWMDiscreet = numpar
            elif ('POS' in line3) or ('PER' in line3):
                if ('START' in line3) or ('BEG' in line3):
                    ThisController.PWMMinImpulse = numpar
                elif ('STOP' in line3) or ('END' in line3):
                    ThisController.PWMMaxImpulse = numpar
            elif 'FREQ' in line3:
                ThisController.PWMFrequency = numpar




    Possible_Periph.append(ThisController)

    del Ports
    del CName
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

def Create_Possible_Commands_List(HCD_Command_Window):
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



Possible_Controllers = []
Possible_Periph = []
HCD_Command_Groups = []
HCD_Command_List = []
HCD_Version_list_Full = []
HCD_Version_association_list = []

UI_Captions = []
Hint_Contents_list0 = []
Hint_Contents_list1 = []
Hint_Contents_list2 = []
Hint_Contents_list = [Hint_Contents_list0, Hint_Contents_list1, Hint_Contents_list2]
Hint_Contents_Show = 3
Hinted_widgets_list = []
Encoder_Color_Highlights = []
Fast_Animation_Entry_Stoplist = []
Fast_Animation_Entry_GreenLight = []

OutputNames2LayoutTypes = []



def Command_Library_Demonstration():
    app = QtWidgets.QApplication([])
    HCD_Command_Window = HCD_Command_Window_Dialog()

    Create_Possible_Controller_List()
    Create_Possible_Commands_List(HCD_Command_Window)

    HCD_Command_Window.Call_Window(True)
    app.exec()

#Command_Library_Demonstration()