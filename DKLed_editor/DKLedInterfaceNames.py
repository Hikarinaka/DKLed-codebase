from PyQt5 import QtWidgets, uic, QtGui
from PyQt5.QtGui import QIntValidator, QRegularExpressionValidator, QCloseEvent, QBrush, QPen, QPainter, QColor, \
    QFont, QIcon, QPainterPath, QPixmap, QMouseEvent
from PyQt5.QtCore import QRegularExpression, Qt, QSize, QEvent, QPointF
from PyQt5.QtWidgets import QInputDialog, QGraphicsView, QGraphicsItem, QGraphicsScene, \
    QStyle, QStyleOptionGraphicsItem, QGraphicsEllipseItem, QGraphicsPathItem, QGraphicsRectItem
from PyQt5.QtSerialPort import QSerialPort, QSerialPortInfo
from PyQt5.QtCore import QIODevice
from enum import Enum

class DKLed_Interface_Names:
    def __init__(self):
        self.MainWindow_ControllerTab_AddControllerButton = "Add Controller"

        self.MainWindow_LayoutTab_AddControllerTool = "Add Controller"
        self.MainWindow_LayoutTab_AddMP3Tool = "Add UART MP3 Player"
        self.MainWindow_LayoutTab_AddDeviceTool = "Add UART/SPI/I2C Device"
        self.MainWindow_LayoutTab_AddScreenTool = "Add spi LED Screen"
        self.MainWindow_LayoutTab_AddServoTool = "Add Servo drive"
        self.MainWindow_LayoutTab_AddRelayTool = "Add electronic Switch"
        self.MainWindow_LayoutTab_AddButtonTool = "Add single Button"
        self.MainWindow_LayoutTab_AddEncoderTool = "Add Encoder"
        self.MainWindow_LayoutTab_AddKeyboardTool = "Add Keyboard"
        self.MainWindow_LayoutTab_DeleteConnectionTool = "Delete highlighted Connections"
        self.MainWindow_LayoutTab_DeleteLayoutItemTool = "Delete highlighted Items"

        self.Dialogue_AddControllerTab_CancelButtonText = "Discard"
        self.Dialogue_AddControllerTab_OkButtonText = "Add"
        self.Dialogue_AddControllerTab_WindowTitle = "Add controller"
        self.Dialogue_AddControllerTab_LabelText = "New Controller name in project"
        self.Dialogue_AddControllerTab_DefaultName = "DKLed "

        self.Dialogue_AddButtonTab_CancelButtonText = "Discard"
        self.Dialogue_AddButtonTab_OkButtonText = "Add"
        self.Dialogue_AddButtonTab_WindowTitle = "Add Button"
        self.Dialogue_AddButtonTab_LabelText = "New Button name in project"
        self.Dialogue_AddButtonTab_DefaultName = "Button "

        self.Dialogue_AddKeyboardTab_CancelButtonText = "Discard"
        self.Dialogue_AddKeyboardTab_OkButtonText = "Add"
        self.Dialogue_AddKeyboardTab_WindowTitle = "Add Keyboard"
        self.Dialogue_AddKeyboardTab_LabelText = "New Keyboard name in project"
        self.Dialogue_AddKeyboardTab_DefaultName = "Keyboard "

        self.Dialogue_AddEncoderTab_CancelButtonText = "Discard"
        self.Dialogue_AddEncoderTab_OkButtonText = "Add"
        self.Dialogue_AddEncoderTab_WindowTitle = "Add Encoder"
        self.Dialogue_AddEncoderTab_LabelText = "New Encoder name in project"
        self.Dialogue_AddEncoderTab_DefaultName = "Encoder "

        self.Dialogue_AddRelayTab_CancelButtonText = "Discard"
        self.Dialogue_AddRelayTab_OkButtonText = "Add"
        self.Dialogue_AddRelayTab_WindowTitle = "Add Electrical Switch"
        self.Dialogue_AddRelayTab_LabelText = "New Electrical Switch name in project"
        self.Dialogue_AddRelayTab_DefaultName = "Relay "

        self.Dialogue_AddServoTab_CancelButtonText = "Discard"
        self.Dialogue_AddServoTab_OkButtonText = "Add"
        self.Dialogue_AddServoTab_WindowTitle = "Add Servo Drive"
        self.Dialogue_AddServoTab_LabelText = "New Servo drive name in project"
        self.Dialogue_AddServoTab_DefaultName = "Servo "

        self.Dialogue_AddScreenTab_CancelButtonText = "Discard"
        self.Dialogue_AddScreenTab_OkButtonText = "Add"
        self.Dialogue_AddScreenTab_WindowTitle = "Add Screen"
        self.Dialogue_AddScreenTab_LabelText = "New SPI LED Screen name in project"
        self.Dialogue_AddScreenTab_DefaultName = "Screen "

        self.Controller_RealtimeServoCheckbox_True = "Servo realtime mode (currently Realtime)"
        self.Controller_RealtimeServoCheckbox_False = "Servo realtime mode (currently Framebased)"
        self.Controller_GotoWindowButton_ToWindow = "Open in Window"
        self.Controller_GotoWindowButton_ToTab = "Return to tab"
        self.Controller_OptionalWindow = "Controller: "
        self.Controller_IO_Output_header_True = "Outputs V"
        self.Controller_IO_Output_header_False = "Outputs >"
        self.Controller_IO_Input_header_True = "Buttons V"
        self.Controller_IO_Input_header_False = "Buttons >"
        self.Controller_IO_UART_header_True = "I/O Ports V"
        self.Controller_IO_UART_header_False = "I/O Ports >"
        self.Controller_IO_Screens_header_True = "Screens V"
        self.Controller_IO_Screens_header_False = "Screens >"
        self.Controller_IO_Files_header_True = "Scenarios V"
        self.Controller_IO_Files_header_False = "Scenarios >"
        self.Controller_controllerCodeVersionLabel = "HCD ver. "
        self.Controller_ControllerTypeInitialLabel = "Controller"
        self.Controller_RemoveControllerButton = "Remove"
        self.Controller_IDInitialLabel = "ID:"
        self.Controller_GetIDButton = "Get"
        self.Controller_ConnectToControllerDeviceButton = "Connect"
        self.Controller_OpenTerminalButton = "Open Terminal"
        self.Controller_BrightnessLabel = "Inital pixel brightness:"
        self.Controller_FastAnimCheckbox = "enable Fast Animation mode"
        self.Controller_ServoStepsSettingLabel = "Servo/PWM steps:"
        self.Controller_ServoFrequencySettingLabel = "Servo/PWM frequency (Hz):"
        self.Controller_ServoImpulseLengthLabel1 = "Servo/PWM impulse length (us):"
        self.Controller_ServoImpulseLengthLabel2 = "to"
        # self.Controller_
        # self.Controller_
        # self.Controller_
        # self.Controller_

        self.ControllerPort_FrameSyncLabel = "Emit frame synchronisation code"
        self.ControllerOutput_ConnectToLabel = "connect to"
        self.ControllerOutput_EmptyScreen = "(none)"

        self.Screen_GotoWindowButton_ToWindow = "Open in Window"
        self.Screen_GotoWindowButton_ToTab = "Return to tab"
        self.Screen_OptionalWindow = "Screen: "
        self.Screen_RemoveScreenButton = "Remove"
        self.Screen_AddChannelButton = "Add chanel"
        self.Screen_RemoveEmptyChannelsButton = "Remove empty Channels"
        self.Screen_InsertLEDMatrixButton = "Add LED matrix to Channel"
        self.Screen_AddLEDButton = "Add single LED to Channel"
        self.Screen_RemoveLEDButton = "Remove single LED from Channel"
        self.Screen_SaveScreenMapButton = "Save Screen"

class DKLed_Layout_Icons2:
    def __init__(self):
        self.app = QtWidgets.QApplication([])
        #self.app.setStyleSheet(Interface_Style.GLOBAL_STYLE)
        print("loading icons")

        self.ButtonPx = QPixmap("Library/Icons/Icon (11).png")
        self.ButtonOutPx = QPixmap("Library/Icons/Icon (19).png")

        self.ScreenBackgroundPx = QPixmap("Library/Icons/Background (0).png")
        self.ScreenBackgroundScaledPx = self.ScreenBackgroundPx.scaledToWidth(Interface_Style.ScreenLayoutMarkerStep, Qt.SmoothTransformation)

        self.EncoderOutPx = QPixmap("Library/Icons/Icon (27).png")
        self.EncoderCWOutPx = QPixmap("Library/Icons/Icon (29).png")
        self.EncoderCCWOutPx = QPixmap("Library/Icons/Icon (28).png")

        self.RelayOutPx = QPixmap("Library/Icons/Icon (101).png")
        self.ServoInPx = QPixmap("Library/Icons/Icon (84).png")

        self.ControllerSettingsPx = QPixmap("Library/Icons/Icon (10).png")
        self.ControllerSettingsIcon = QIcon(self.ControllerSettingsPx)
        self.ButtonSettingsPx = QPixmap("Library/Icons/Icon (13).png")
        self.ButtonSettingsIcon = QIcon(self.ButtonSettingsPx)
        self.ScreenSettingsPx = QPixmap("Library/Icons/Icon (52).png")
        self.ScreenSettingsIcon = QIcon(self.ScreenSettingsPx)

        self.AddButtonToKbdPx = QPixmap("Library/Icons/Icon (107).png")
        self.AddButtonToKbdIcon = QIcon(self.AddButtonToKbdPx)
        self.DeleteButtonFromKbdPx = QPixmap("Library/Icons/Icon (106).png")
        self.DeleteButtonFromKbdIcon = QIcon(self.DeleteButtonFromKbdPx)

        self.AddControllerPx = QPixmap("Library/Icons/Icon (2).png")
        self.AddControllerIcon = QIcon(self.AddControllerPx)
        self.AddButtonPx = QPixmap("Library/Icons/Icon (18).png")
        self.AddButtonIcon = QIcon(self.AddButtonPx)
        self.AddEncoderPx = QPixmap("Library/Icons/Icon (30).png")
        self.AddEncoderIcon = QIcon(self.AddEncoderPx)
        self.AddKeyboardPx = QPixmap("Library/Icons/Icon (94).png")
        self.AddKeyboardIcon = QIcon(self.AddKeyboardPx)

        self.AddScreenPx = QPixmap("Library/Icons/Icon (51).png")
        self.AddScreenIcon = QIcon(self.AddScreenPx)
        self.AddServoPx = QPixmap("Library/Icons/Icon (86).png")
        self.AddServoIcon = QIcon(self.AddServoPx)
        self.AddRelayPx = QPixmap("Library/Icons/Icon (103).png")
        self.AddRelayIcon = QIcon(self.AddRelayPx)

        self.AddMP3Px = QPixmap("Library/Icons/Icon (66).png")
        self.AddMP3Icon = QIcon(self.AddMP3Px)
        self.AddDevicePx = QPixmap("Library/Icons/Icon (77).png")
        self.AddDeviceIcon = QIcon(self.AddDevicePx)

        self.DeleteConnectionPx = QPixmap("Library/Icons/Icon (105).png")
        self.DeleteConnectionIcon = QIcon(self.DeleteConnectionPx)
        self.DeleteLayoutItemPx = QPixmap("Library/Icons/Icon (108).png")
        self.DeleteLayoutItemIcon = QIcon(self.DeleteLayoutItemPx)

        self.AddChannelToScreenPx = QPixmap("Library/Icons/Icon (48).png")
        self.AddChannelToScreenIcon = QIcon(self.AddChannelToScreenPx)
        self.RemoveEmptyChannelsFromScreenPx = QPixmap("Library/Icons/Icon (109).png")
        self.RemoveEmptyChannelsFromScreenIcon = QIcon(self.RemoveEmptyChannelsFromScreenPx)
        self.SaveScreenMapPx = QPixmap("Library/Icons/Icon (54).png")
        self.SaveScreenMapIcon = QIcon(self.SaveScreenMapPx)
        self.InsertLEDMatrixToScreenPx = QPixmap("Library/Icons/Icon (45).png")
        self.InsertLEDMatrixToScreenIcon = QIcon(self.InsertLEDMatrixToScreenPx)
        self.AddSingleLEDToScreenPx = QPixmap("Library/Icons/Icon (37).png")
        self.AddSingleLEDToScreenIcon = QIcon(self.AddSingleLEDToScreenPx)
        self.RemoveSingleLEDFromScreenPx = QPixmap("Library/Icons/Icon (111).png")
        self.RemoveSingleLEDFromScreenIcon = QIcon(self.RemoveSingleLEDFromScreenPx)
        self.InsertLEDMatrixToScreenHighlightPx = QPixmap("Library/Icons/Icon (112).png")
        self.InsertLEDMatrixToScreenHighlightIcon = QIcon(self.InsertLEDMatrixToScreenHighlightPx)
        self.AddSingleLEDToScreenHighlightPx = QPixmap("Library/Icons/Icon (113).png")
        self.AddSingleLEDToScreenHighlightIcon = QIcon(self.AddSingleLEDToScreenHighlightPx)
        self.RemoveSingleLEDFromScreenHighlightPx = QPixmap("Library/Icons/Icon (114).png")
        self.RemoveSingleLEDFromScreenHighlightIcon = QIcon(self.RemoveSingleLEDFromScreenHighlightPx)


        #self.LEDMatrix03Px = QPixmap("Library/Icons/Icons-matrix_0-3.png")
        #self.LEDMatrix03Icon = QIcon(self.LEDMatrix03Px)
        #self.LEDMatrix0CPx = QPixmap("Library/Icons/Icons-matrix_0-C.png")
        #self.LEDMatrix0CIcon = QIcon(self.LEDMatrix0CPx)
        #self.LEDMatrix0FhPx = QPixmap("Library/Icons/Icons-matrix_0-F_h.png")
        #self.LEDMatrix0FhIcon = QIcon(self.LEDMatrix0FhPx)
        #self.LEDMatrix0FvPx = QPixmap("Library/Icons/Icons-matrix_0-F_v.png")
        #self.LEDMatrix0FvIcon = QIcon(self.LEDMatrix0FvPx)
        #self.LEDMatrix30Px = QPixmap("Library/Icons/Icons-matrix_3-0.png")
        #self.LEDMatrix30Icon = QIcon(self.LEDMatrix30Px)
        #self.LEDMatrix3ChPx = QPixmap("Library/Icons/Icons-matrix_3-C_h.png")
        #self.LEDMatrix3ChIcon = QIcon(self.LEDMatrix3ChPx)
        #self.LEDMatrix3CvPx = QPixmap("Library/Icons/Icons-matrix_3-C_v.png")
        #self.LEDMatrix3CvIcon = QIcon(self.LEDMatrix3CvPx)
        #self.LEDMatrix3FPx = QPixmap("Library/Icons/Icons-matrix_3-F.png")
        #self.LEDMatrix3FIcon = QIcon(self.LEDMatrix3FPx)

        self.LEDMatrixPx = []
        self.LEDMatrixIcon = []
        for i in range(16):
            self.LEDMatrixPx.append(QPixmap("Library/Icons/Icons-matrix ("+str(i)+").png"))
            self.LEDMatrixIcon.append(QIcon(self.LEDMatrixPx[i]))

        print("Icons loaded")

class DKLed_Layout_Style:
    def __init__(self):
        self.GLOBAL_STYLE = """QMainWindow, Functional_Window, QInputDialog {
                            border-color: hsv(100, 120, 20);
                            background-color: hsv(170, 40, 200); 
                            border-style: outset; border-radius: 4px;
                            }
                            QFrame{
                            background-color: hsv(170, 40, 200); 
                            }
                            QTabWidget::pane {
                            border: 2px solid hsv(120, 120, 100);
                            border-style: outset; border-radius: 4px;
                            }
                            QTabWidget::tab-bar {
                            alignment: left;
                            }
                            """

        self.IconStyle = "border-color: #909090; border-width: 2px; " \
                         "border-style: outset; border-radius: 6px;" \
                         "background-color: #f0f0f0"
        #self.Iconsize = QSize(64,64)
        self.Iconsize = QSize(48, 48)
        self.ToolIconsize = QSize(32, 32)
        self.ToolIconGap = 16
        self.ItemIconsize = QSize(48, 48)
        #outlines
        self.OutlineLightColor = QColor(0x909090)
        self.OutlineColor = QColor(0x808080)
        self.OutlineSelectedColor = QColor(0x40, 0x40, 0x40, 0x90)
        self.ConnectColor = QColor(0x707070)
        self.ConnectSelectedColor = QColor(0x70, 0xf0, 0x70, 0x90)
        self.ConnectConstructColor = QColor(0x70, 0x70, 0xf0, 0x90)
        self.ConnectHoverColor = QColor(0x40, 0x90, 0xff, 0xff)
        #Fills
        self.BackgroundColor = QColor(0xfafafa)
        self.FillMainColor = QColor(0xf0f0f0)
        self.FillInpColor = QColor(0xf0c0c0)
        self.FillOutColor = QColor(0xc0c0f0)
        self.FillPortColor = QColor(0xe0e0a0)

        self.ConnectWidth = 3
        self.OutlineWidth = 2
        self.OutlineSelectedWidth = 3

        self.ConnectionPointRadius = 5
        self.ConnectionVOffset = 20
        self.ConnectionHOffset = 25
        self.ConnectionPointSpacing = 26

        self.ControllerConfigAreaWidth = 408
        self.ControllerConfigAreaHeight = 300

        self.WindowFromTabExtraWidth = 20
        self.WindowFromTabExtraHeight = 300

        self.IconComboBoxMaxVisibleItems = 16

        self.MaximumScreenChannelCount = 16
        self.MaximumScreenChannelButtonsVisible = 10
        self.ScreenLayoutMarkerStep = 16


Interface_Names = DKLed_Interface_Names()
Interface_Style = DKLed_Layout_Style()
Interface_Icons = DKLed_Layout_Icons2()

