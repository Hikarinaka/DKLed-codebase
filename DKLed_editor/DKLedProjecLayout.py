from PyQt5 import QtWidgets, uic, QtGui
from PyQt5.QtGui import QIntValidator, QRegularExpressionValidator, QCloseEvent, QBrush, QPen, QPainter, QColor, \
    QFont, QIcon, QPainterPath, QPixmap, QMouseEvent
from PyQt5.QtCore import QRegularExpression, Qt, QSize, QEvent, QPointF
from PyQt5.QtWidgets import QInputDialog, QGraphicsView, QGraphicsItem, QGraphicsScene, \
    QStyle, QStyleOptionGraphicsItem, QGraphicsEllipseItem, QGraphicsPathItem, QGraphicsRectItem
from PyQt5.QtSerialPort import QSerialPort, QSerialPortInfo
from PyQt5.QtCore import QIODevice
from enum import Enum
import sip
import sys
import gc

from CustomQWidgets import QtComboBoxUnScroll, Functional_Window, QtScrollDrawArea, Program_Windows, \
    Close_All_Windows, QGraphicsRectHoverBoard, QGraphicsSceneCallable

from DKLedInterfaceNames import Interface_Names, Interface_Style, Interface_Icons

class DKLed_Project_Layout_Types(Enum):
    Nothing = 0

    LEDController = 1
    ServoController =2
    WIFIController =3

    UARTDevice =10
    MP3Device =11
    Nextion =12

    InputChannel = 30 #one of below
    Button = 31
    Encoder = 32
    Resistor = 33
    Endstop = 34
    Keyboard = 35
    EncoderCW = 36
    EncoderCCW = 37
    RelayOut = 38
    Microphone = 39

    OutputChannel = 40 #one of below
    Relay = 41
    LedScreen = 42
    WS2812B = 43
    SK6812 = 44
    WS2815C = 45
    WSSKMix = 46
    Ledstripe = 47
    Servo = 48
    Drive = 49
    PWM = 50

    PortChanel = 60
    USBPort = 61
    UART = 62
    UARTRxTx = 63
    UARTRx = 64
    UARTTx = 65
    SPI = 66
    SPIMOSI = 67
    SPIMISO = 68
    I2C = 71
    I2CMaster = 72
    I2CSlave = 73

    SimpleDevice = 90

    EveryChannel = 100

class DKLed_Project_Layout_Actions(Enum):
    NoAction = 0
    Connect = 1
    Disconnect = 2

class QGraphicsEllipseItemCallable(QGraphicsEllipseItem):
    def __init__(self,*args, **kwargs):
        super().__init__(*args, **kwargs)
        self.ConnectType = None
        self.SubconnectType = None
        self.ConnectableTypes = []
        self.ConnectionStoplist = []
        self.Item = None #what in Controller, PhisicalButton etc classes thos blob represents
        self.ItemSlotIndex = 0
        self.No = 0
        self.SubNo = 0
        self.NomberOfLines = 0
        self.MultyLines = False
        self.Line = [] #line paths
        self.Connects = []
        self.setAcceptHoverEvents(True)
        self.Lay = None
        self.InProgress=False
        self.UnderPen = QPen(Qt.NoPen)
        self.UnderPen.setColor(QColor(0, 0, 0, 0))
        self.UnderPen.setStyle(Qt.SolidLine)
        self.UnderPen.setWidthF(2)
        self.setPen(self.UnderPen)
        self.NormalPen = self.UnderPen
        self.HighlightedPen = None

    def setPens(self,Normal_,Highlighted_=None):
        self.NormalPen = Normal_
        self.HighlightedPen =Highlighted_
        self.setPen(Normal_)

    def mousePressEvent(self, e):
        #print(e.button())
        if not self.InProgress:
            if e.button()==Qt.LeftButton:
                if not self.Lay.ConnectionInProgress:
                    if self.MultyLines or len(self.Connects) < 1:
                        self.InProgress = True
                        self.Lay.ConnectionInProgress = self
                        SomeLine2 = QGraphicsPathItemCallable()
                        SomeLine2.setPens(self.Lay.PenConnect, self.Lay.PenConnectConstructed)
                        SomeLine2.setStartItem(self, 0, 0)
                        SomeLine2.addIntoScene(self.Lay)
                        if self.ConnectType == DKLed_Project_Layout_Types.InputChannel:
                            SomeLine2.setControlPoints(-60, 0, 0, 0)
                        elif self.ConnectType == DKLed_Project_Layout_Types.OutputChannel:
                            SomeLine2.setControlPoints(60, 0, 0, 0)
                        else:
                            SomeLine2.setControlPoints(0, 60, 0, 0)
                        SomeLine2.setMouseTrack(True)
                        SomeLine2.Highlighted = True
                        self.Lay.LineConnects.append(SomeLine2)
                        self.Lay.PathInProgress = SomeLine2
                else:
                    if (self.SubconnectType in self.Lay.ConnectionInProgress.ConnectableTypes) \
                            and (self.Lay.ConnectionInProgress.SubconnectType in self.ConnectableTypes) \
                            and (not (self.Lay.ConnectionInProgress in self.ConnectionStoplist)):
                        if (self in self.Lay.ConnectionInProgress.Connects) or (
                                self.MultyLines == False and len(self.Connects) > 0):
                            # print("already connected")
                            self.Lay.New_Connection_fail_handler()
                        else:
                            # print("connected!")
                            self.Lay.PathInProgress.setEndItem(self.Lay.Hovered, 0, 0)
                            if self.ConnectType == DKLed_Project_Layout_Types.InputChannel:
                                self.Lay.PathInProgress.setControlPoints(60, 0, -60, 0)
                            elif self.ConnectType == DKLed_Project_Layout_Types.OutputChannel:
                                self.Lay.PathInProgress.setControlPoints(-60, 0, 60, 0)
                            else:
                                self.Lay.PathInProgress.setControlPoints(0, 60, 0, 60)
                            self.Lay.ConnectionInProgress.Connects.append(self)
                            self.Connects.append(self.Lay.ConnectionInProgress)

                            self.Line.append(self.Lay.PathInProgress)
                            self.Lay.ConnectionInProgress.Line.append(self.Lay.PathInProgress)
                            self.Lay.PathInProgress.Highlighted = False
                            self.Lay.PathInProgress.setPens(self.Lay.PenConnect, self.Lay.PenConnectSelected)
                            self.Lay.PathInProgress.EO = self.Lay.ConnectionInProgress
                            self.Lay.PathInProgress.ES = self
                            if self.ConnectType == DKLed_Project_Layout_Types.OutputChannel:
                                self.Lay.PathInProgress.swapStartEnd()
                            if self.Item.Connect_Item(self.Lay.ConnectionInProgress.Item,
                                                   self.Lay.ConnectionInProgress.ItemSlotIndex, self.ItemSlotIndex, True):
                                self.Lay.ConnectionInProgress.Item.Connect_Item(self.Item, self.ItemSlotIndex, self.Lay.ConnectionInProgress.ItemSlotIndex, True)
                            self.Lay.PathInProgress = None
                            self.Lay.ConnectionInProgress.InProgress = False
                            self.Lay.ConnectionInProgress = None
            elif e.button()==Qt.RightButton:
                Hl = True
                for l_ in self.Line:
                    if l_:
                        if l_.isHighlighted():
                            Hl = False
                for l_ in self.Line:
                    if l_:
                        l_.Highlighted = Hl

        pass

    def Connect_to(self,Rou):
        if isinstance(Rou, QGraphicsEllipseItemCallable):
            if Rou.Item:
                SomeLine2 = QGraphicsPathItemCallable()
                SomeLine2.setPens(self.Lay.PenConnect, self.Lay.PenConnectSelected)
                SomeLine2.setStartItem(self, 0, 0)
                SomeLine2.addIntoScene(self.Lay)
                SomeLine2.setEndItem(Rou, 0, 0)
                if self.ConnectType == DKLed_Project_Layout_Types.InputChannel:
                    SomeLine2.setControlPoints(-60, 0, 60, 0)
                elif self.ConnectType == DKLed_Project_Layout_Types.OutputChannel:
                    SomeLine2.setControlPoints(60, 0, -60, 0)
                else:
                    SomeLine2.setControlPoints(0, 60, 0, 60)
                Rou.Connects.append(self)
                self.Connects.append(Rou)
                SomeLine2.EO = Rou
                SomeLine2.ES = self
                if self.ConnectType == DKLed_Project_Layout_Types.InputChannel:
                    SomeLine2.swapStartEnd()
                self.Item.Connect_Item(Rou.Item, Rou.ItemSlotIndex, self.ItemSlotIndex)
                Rou.Item.Connect_Item(self.Item, self.ItemSlotIndex, Rou.ItemSlotIndex)
                self.Line.append(SomeLine2)
                Rou.Line.append(SomeLine2)
                self.Lay.LineConnects.append(SomeLine2)
                #print("connected!")

    def Set_Connectable_Types(self, Type = DKLed_Project_Layout_Types.Nothing):
        if Type == DKLed_Project_Layout_Types.Nothing:
            self.ConnectableTypes.clear()
            self.ConnectableTypes.append(DKLed_Project_Layout_Types.Nothing)
        elif Type == DKLed_Project_Layout_Types.Button or Type == DKLed_Project_Layout_Types.Endstop or Type == DKLed_Project_Layout_Types.Keyboard:
            self.ConnectableTypes.clear()
            self.ConnectableTypes.append(DKLed_Project_Layout_Types.Button)
            self.ConnectableTypes.append(DKLed_Project_Layout_Types.Encoder)
            self.ConnectableTypes.append(DKLed_Project_Layout_Types.Endstop)

        elif Type == DKLed_Project_Layout_Types.Encoder:
            self.ConnectableTypes.clear()
            self.ConnectableTypes.append(DKLed_Project_Layout_Types.Button)
            self.ConnectableTypes.append(DKLed_Project_Layout_Types.Encoder)
            self.ConnectableTypes.append(DKLed_Project_Layout_Types.Endstop)
            self.ConnectableTypes.append(DKLed_Project_Layout_Types.EncoderCW)
            self.ConnectableTypes.append(DKLed_Project_Layout_Types.EncoderCCW)
        elif Type == DKLed_Project_Layout_Types.EncoderCW:
            self.ConnectableTypes.clear()
            self.ConnectableTypes.append(DKLed_Project_Layout_Types.EncoderCW)
        elif Type == DKLed_Project_Layout_Types.EncoderCCW:
            self.ConnectableTypes.clear()
            self.ConnectableTypes.append(DKLed_Project_Layout_Types.EncoderCCW)
        for l_ in self.Line:
            if l_:
                if not ((l_.EO.SubconnectType in l_.ES.ConnectableTypes) and (l_.ES.SubconnectType in l_.EO.ConnectableTypes)):
                    l_.Destroy()

    def hoverEnterEvent(self, e):
        #x = e.scenePos()
        #print("Connector of type "+ str(self.ConnectType)+"/"+str(self.SubconnectType) +" nomber "+ str(self.No))
        if self.Lay:
            self.Lay.Hovered = self
            if self.HighlightedPen:
                if not self.Lay.ConnectionInProgress:
                    if self.MultyLines == True or len(self.Connects) < 1:
                        self.setPen(self.HighlightedPen)
                elif (self.SubconnectType in self.Lay.ConnectionInProgress.ConnectableTypes) \
                        and (self.Lay.ConnectionInProgress.SubconnectType in self.ConnectableTypes)\
                        and (not(self.Lay.ConnectionInProgress in self.ConnectionStoplist)):
                    if not ((self in self.Lay.ConnectionInProgress.Connects) or (
                            self.MultyLines == False and len(self.Connects) > 0)):
                        self.setPen(self.HighlightedPen)
        pass

    def hoverLeaveEvent(self, e):
        if self.Lay:
            self.Lay.Hovered = None
            self.setPen(self.NormalPen)
        #print("Connector of type " + str(self.ConnectType) + " nomber " + str(self.No) + " is no longer under mouse")
        pass

    def Remove_Connection(self, L = None, Itm = None, Direct = False):
        if L:
            if L in self.Line:
                self.Line.remove(L)
        if Itm:
            if Itm in self.Connects:
                self.Connects.remove(Itm)
                Itm.Item.Disconnect(self.Item, Direct)

    def Sync_with_Phisical_Device(self, DevSlot = None, ind = 0):
        self.Item = DevSlot
        self.ItemSlotIndex = ind
        return self

class QGraphicsSceneCursor(QGraphicsEllipseItem):
    def __init__(self,*args, **kwargs):
        super().__init__(*args, **kwargs)

    #def mouseReleaseEvent(self, event: 'QGraphicsSceneMouseEvent'):
        #self.ungrabMouse()

class QGraphicsPathItemCallable(QGraphicsPathItem):
    def __init__(self,*args, **kwargs):
        super().__init__(*args, **kwargs)
        self.SubPath = QGraphicsPathItem()
        self.SubPath2 = QGraphicsPathItem()
        self.C1 = [0,0]
        self.dPos1 = [0, 0]
        self.TrackedStart = None
        self.TrackedEnd = None
        self.C2 = [0,0]
        self.dPos2 = [0,0]
        self.PainterPath = QPainterPath()
        self.PainterPath2 = QPainterPath()
        self.Highlighted = False
        self.UnderPen = QPen(Qt.NoPen)
        self.UnderPen.setColor(QColor(0, 0, 0, 0))
        self.UnderPen.setStyle(Qt.SolidLine)
        self.UnderPen.setWidthF(6)
        self.setPen(self.UnderPen)
        self.NormalPen = self.UnderPen
        self.HighlightedPen = self.UnderPen
        #self.Brush = Qt.NoBrush
        self.setFlag(QGraphicsItem.ItemIsSelectable, False)
        self.setFlag(QGraphicsItem.ItemClipsToShape, False)
        #self.SubPath.setFlag(QGraphicsItem.ItemIsSelectable, False)
        self.Z = [0,20]
        self.PosStore = [0,0,0,0]
        self.ES = None
        self.EO = None
        self.EndToMouse = False
        self.MouseInputSource = None
        self.Useful = True
        self.Lay = None

    def mousePressEvent(self, e):
        self.Highlighted = not self.Highlighted
        if self.Highlighted:
            self.SubPath.setPen(self.HighlightedPen)
            self.SubPath.setZValue(self.Z[1])
        else:
            self.SubPath.setPen(self.NormalPen)
            self.SubPath.setZValue(self.Z[0])
            if self.Lay:
                if self == self.Lay.PathInProgress:
                    self.Lay.New_Connection_fail_handler()
        #print(self.Z)
        #pass

    def addIntoScene(self, Lay_):
        Scene_ = None
        self.Lay = None
        if isinstance(Lay_, DKLed_Project_Layout_Controls):
            Scene_ = Lay_.Scene
            self.Lay = Lay_
        if isinstance(Scene_, QGraphicsScene):
            Scene_.addItem(self)
            Scene_.addItem(self.SubPath)
            Scene_.addItem(self.SubPath2)
        self.MouseInputSource = None
        if isinstance(Scene_, QGraphicsSceneCallable):
            self.MouseInputSource = Scene_

    def setMouseTrack(self, Track_= False):
        self.EndToMouse = False
        if Track_ and self.MouseInputSource:
            self.EndToMouse = True
            self.C2 = [0, 0]

    def Update(self):
        X1 = self.dPos1[0]
        Y1 = self.dPos1[1]
        X2 = self.dPos2[0]
        Y2 = self.dPos2[1]
        if self.TrackedStart:
            X1 += self.TrackedStart.scenePos().x()
            Y1 += self.TrackedStart.scenePos().y()
        if self.TrackedEnd:
            X2 += self.TrackedEnd.scenePos().x()
            Y2 += self.TrackedEnd.scenePos().y()
        if self.EndToMouse:
            X2 = self.MouseInputSource.MX
            Y2 = self.MouseInputSource.MY

        if (X1 != self.PosStore[0]) or (Y1 != self.PosStore[1]) or (X2 != self.PosStore[2]) or (Y2 != self.PosStore[2]):
            self.PosStore[0] = X1
            self.PosStore[1] = Y1
            self.PosStore[2] = X2
            self.PosStore[3] = Y2
            self.PainterPath.clear()
            self.PainterPath2.clear()
            self.PainterPath.moveTo(X1, Y1)
            self.PainterPath.cubicTo(X1 + self.C1[0], Y1 + self.C1[1], X2 + self.C2[0], Y2 + self.C2[1], X2, Y2)
            self.PainterPath.cubicTo(X2 + self.C2[0], Y2 + self.C2[1], X1 + self.C1[0], Y1 + self.C1[1], X1, Y1)
            self.PainterPath2.moveTo(X1, Y1)
            self.PainterPath2.cubicTo(X1 + self.C1[0], Y1 + self.C1[1], X2 + self.C2[0], Y2 + self.C2[1], X2, Y2)

            self.setPath(self.PainterPath)
            self.SubPath.setPath(self.PainterPath2)
            self.SubPath2.setPath(self.PainterPath2)
            if self.Highlighted:
                self.SubPath.setPen(self.HighlightedPen)
                self.SubPath.setZValue(self.Z[1])
            else:
                self.SubPath.setPen(self.NormalPen)
                self.SubPath.setZValue(self.Z[0])

    def setControlPoints(self, dx1, dy1, dx2, dy2):
        self.C1 = [dx1, dy1]
        self.C2 = [dx2, dy2]

    def setStartpoint(self, x, y):
        self.TrackedStart = None
        self.dPos1 = [x, y]

    def setEndpoint(self, x, y):
        self.TrackedEnd = None
        self.dPos2 = [x, y]

    def setStartItem(self, Tracked, x=0, y=0):
        if isinstance(Tracked, QGraphicsItem):
            self.TrackedStart = Tracked
        self.dPos1 = [x, y]

    def setEndItem(self, Tracked, x=0, y=0):
        self.EndToMouse = False
        if isinstance(Tracked, QGraphicsItem):
            self.TrackedEnd = Tracked
        self.dPos2 = [x, y]

    def swapStartEnd(self):
        pp = self.dPos1
        self.dPos1 = self.dPos2
        self.dPos2 = pp
        pp = self.C1
        self.C1 = self.C2
        self.C2 = pp
        ti = self.TrackedEnd
        self.TrackedEnd = self.TrackedStart
        self.TrackedStart = ti

    def setZValues(self, Normal_, Highlighted_):
        self.Z = [Normal_, Highlighted_]
        self.setZValue(self.Z[0])
        self.SubPath2.setZValue(self.Z[0])
        if self.Highlighted:
            self.SubPath.setZValue(self.Z[1])
        else:
            self.SubPath.setZValue(self.Z[0])

    def setPens(self, Normal_, Highlighted_):
        self.NormalPen = Normal_
        self.HighlightedPen = Highlighted_
        if self.Highlighted:
            self.SubPath.setPen(self.HighlightedPen)
            self.SubPath.setZValue(self.Z[1])
        else:
            self.SubPath.setPen(self.NormalPen)
            self.SubPath.setZValue(self.Z[0])
        self.SubPath2.setPen(self.NormalPen)
        self.setPen(self.UnderPen)

    def isHighlighted(self):
        return self.Highlighted

    def Destroy(self, Direct=False):
        if self.ES:
            self.ES.Remove_Connection(self, self.EO, Direct)
        if self.EO:
            self.EO.Remove_Connection(self, self.ES, Direct)

        self.PainterPath.clear()
        self.PainterPath2.clear()
        self.SubPath.scene().removeItem(self.SubPath)
        self.SubPath2.scene().removeItem(self.SubPath2)
        self.scene().removeItem(self)
        self.Z.clear()
        self.PosStore.clear()
        self.C1.clear()
        self.C2.clear()
        self.dPos1.clear()
        self.dPos2.clear()
        del self.SubPath
        del self.SubPath2
        del self.UnderPen
        del self.PainterPath
        del self.PainterPath2
        #sip.delete(self.SubPath)
        #sip.delete(self.SubPath2)
        #sip.delete(self.UnderPen)
        #sip.delete(self.PainterPath)
        #sip.delete(self.PainterPath2)
        #sip.delete(self)
        self.Useful = False
        self.Highlighted = False
        del self
        gc.collect()
        #print("collected "+ str (g))

class DKLed_Project_Layout_Controls:
    def __init__(self, Layoutinto):
        self.Scene = QGraphicsSceneCallable()
        self.View = QtWidgets.QGraphicsView(self.Scene)
        #self.View.setDragMode(QGraphicsView.ScrollHandDrag)
        self.View.setTransformationAnchor(QGraphicsView.AnchorUnderMouse)

        self.Style = Interface_Style
        self.Icons = Interface_Icons

        LayV = QtWidgets.QVBoxLayout()

        self.ToolBar = QtWidgets.QToolBar()
        self.ToolBar.setOrientation(Qt.Vertical)
        self.ToolBar.setIconSize(self.Style.ToolIconsize)
        self.ActionAddController = self.ToolBar.addAction(self.Icons.AddControllerIcon, Interface_Names.MainWindow_LayoutTab_AddControllerTool)
        self.ActionAddMP3 = self.ToolBar.addAction(self.Icons.AddMP3Icon, Interface_Names.MainWindow_LayoutTab_AddMP3Tool)
        self.ActionAddDevice = self.ToolBar.addAction(self.Icons.AddDeviceIcon, Interface_Names.MainWindow_LayoutTab_AddDeviceTool)
        self.ToolBar.addSeparator()
        self.ActionAddScreen = self.ToolBar.addAction(self.Icons.AddScreenIcon, Interface_Names.MainWindow_LayoutTab_AddScreenTool)
        self.ActionAddServo = self.ToolBar.addAction(self.Icons.AddServoIcon, Interface_Names.MainWindow_LayoutTab_AddServoTool)
        self.ActionAddRelay = self.ToolBar.addAction(self.Icons.AddRelayIcon, Interface_Names.MainWindow_LayoutTab_AddRelayTool)
        self.ToolBar.addSeparator()
        self.ActionAddButton = self.ToolBar.addAction(self.Icons.AddButtonIcon, Interface_Names.MainWindow_LayoutTab_AddButtonTool)
        self.ActionAddEncoder = self.ToolBar.addAction(self.Icons.AddEncoderIcon, Interface_Names.MainWindow_LayoutTab_AddEncoderTool)
        self.ActionAddKeyboard = self.ToolBar.addAction(self.Icons.AddKeyboardIcon,Interface_Names.MainWindow_LayoutTab_AddKeyboardTool)
        self.ToolBar.addSeparator()
        self.ActionDeleteConnection = self.ToolBar.addAction(self.Icons.DeleteConnectionIcon, Interface_Names.MainWindow_LayoutTab_DeleteConnectionTool)
        self.ActionDeleteConnection.triggered.connect(self.Delete_Highlighted_Connections)
        self.ActionDeleteLayoutItem = self.ToolBar.addAction(self.Icons.DeleteLayoutItemIcon, Interface_Names.MainWindow_LayoutTab_DeleteLayoutItemTool)
        self.ActionDeleteLayoutItem.triggered.connect(self.Delete_Highlighted_LayoutItems)

        LayV.addWidget(self.ToolBar)
        LayV.addStretch()
        Layoutinto.addLayout(LayV)
        Layoutinto.addWidget(self.View)

        self.LineConnects = [] #links to the line connections
        self.DrawnEntities = []
        self.DrawnEntities.clear()
        self.LineConnectionsPurgeInProgress = False

        self.Hovered = None
        self.PathInProgress = None
        self.ConnectionInProgress = None

        self.ButtonEntities = []
        self.ButtonEntities.clear()

        self.View.setInteractive(True)
        self.View.setViewportUpdateMode(QGraphicsView.FullViewportUpdate)
        self.View.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOn)
        self.View.setVerticalScrollBarPolicy(Qt.ScrollBarAlwaysOn)

        self.PenOutline = QPen(self.Style.OutlineColor)
        self.PenOutline.setCosmetic(True)
        self.PenOutline.setWidthF(self.Style.OutlineWidth)
        self.PenOutline.setCapStyle(Qt.RoundCap)
        self.PenOutline.setJoinStyle(Qt.RoundJoin)
        self.PenOutlineSelected = QPen(self.Style.OutlineSelectedColor)
        self.PenOutlineSelected.setCosmetic(True)
        self.PenOutlineSelected.setWidthF(self.Style.OutlineSelectedWidth)
        self.PenOutlineSelected.setCapStyle(Qt.RoundCap)
        self.PenOutlineSelected.setJoinStyle(Qt.RoundJoin)
        self.PenOutlineSelected.setStyle(Qt.SolidLine)
        self.PenConnect = QPen(self.Style.ConnectColor)
        self.PenConnect.setCosmetic(True)
        self.PenConnect.setWidthF(self.Style.ConnectWidth)
        self.PenConnect.setCapStyle(Qt.RoundCap)
        self.PenConnect.setJoinStyle(Qt.RoundJoin)
        self.PenConnect.setStyle(Qt.DotLine)
        self.PenConnectSelected = QPen(self.PenConnect)
        self.PenConnectSelected.setColor(self.Style.ConnectSelectedColor)
        self.PenConnectSelected.setStyle(Qt.SolidLine)
        self.PenConnectConstructed = QPen(self.PenConnectSelected)
        self.PenConnectConstructed.setColor(self.Style.ConnectConstructColor)
        self.PenConnectHover = QPen(self.PenOutline)
        self.PenConnectHover.setColor(self.Style.ConnectHoverColor)

        self.PenBackground = QPen(Qt.NoPen)
        #self.PenBackground.setWidthF(1)

        self.FillBrush = QBrush(self.Style.FillMainColor)
        self.InpBrush = QBrush(self.Style.FillInpColor)
        self.OutBrush = QBrush(self.Style.FillOutColor)
        self.PortBrush = QBrush(self.Style.FillPortColor)
        self.NoBrush = QBrush(Qt.NoBrush)
        self.MainFont = QFont()

        self.Hoverboard = QGraphicsRectHoverBoard(0,0,20,20)
        self.Scene.addItem(self.Hoverboard)
        self.Hoverboard.S = self.Scene
        self.Hoverboard.Lay = self
        self.Hoverboard.setZValue(-1)
        self.Hoverboard.setPen(self.PenBackground)
        self.Hoverboard.setBrush(self.NoBrush)
        self.Hoverboard.MousePressHandler = self.New_Connection_fail_handler
        #self.Hoverboard.setFlag(QGraphicsItem.ItemIsSelectable, False)

        #self.View.scale(0.5,0.5)

        self.MainPainter = QPainter()
        self.MainPainter.setBrush(self.FillBrush)
        self.MainPainter.setPen(self.PenOutline)

        self.Scene.changed.connect(self.Rect_to_front)
        #self.View.show()

    def Rect_to_front(self):

        #self.Hoverboard.setRect(self.View.sceneRect().x()+2, self.View.sceneRect().y()+2,self.View.sceneRect().width() - 3, self.View.sceneRect().height() - 3)

        Xmin = 0
        Xmax = 0
        Ymin = 0
        Ymax = 0
        CheckBounds=True

        for l_ in self.LineConnects:
            if isinstance(l_, QGraphicsPathItemCallable):
                if l_.Useful:
                    l_.Update()
                else:
                    self.LineConnects.remove(l_)
                    del l_
            else:
                self.LineConnects.remove(l_)
                del l_

        for ent in self.DrawnEntities:
            #if ent is QtWidgets.QGraphicsRectItem:
            if ent.Mainfield.isSelected():
                ent.Mainfield.setZValue(3)
                ent.Mainfield.setPen(self.PenOutlineSelected)
            else:
                ent.Mainfield.setZValue(1)
                ent.Mainfield.setPen(self.PenOutline)
            x1 = ent.Mainfield.x()
            x2 = x1 + ent.Mainfield.boundingRect().width()
            y1 = ent.Mainfield.y()
            y2 = y1 + ent.Mainfield.boundingRect().height()
            if x1<Xmin or CheckBounds:
                Xmin = x1
            if x2> Xmax or CheckBounds:
                Xmax = x2
            if y1<Ymin or CheckBounds:
                Ymin = y1
            if y2> Ymax or CheckBounds:
                Ymax = y2
            CheckBounds = False

        W = self.View.maximumViewportSize().width()-2
        H = self.View.maximumViewportSize().height() - 2
        P = self.View.mapToScene(0,0)
        X0 = P.x()+1
        Y0 = P.y()+1
        self.Hoverboard.setRect(X0, Y0, W, H)

        #Xmax = Xmax + self.Style.ConnectionHOffset
        #Ymax = Ymax + self.Style.ConnectionVOffset
        #Xmin = Xmin - self.Style.ConnectionHOffset
        #Ymin = Ymin - self.Style.ConnectionVOffset
        #if Xmin<X0:
            #W = W + X0 - Xmin
            #X0 = Xmin
        #if Ymin<Y0:
            #H = H + Y0 - Ymin
            #Y0 = Ymin
        #if Xmax > W+X0:
            #W = Xmax - X0
        #if Ymax > H+Y0:
            #H = Ymax - Y0

        #for ent in self.DrawnEntities:
            #ent.setPos(ent.x() - X0, ent.y()-Y0)

        #self.Scene.setSceneRect(X0,Y0, W,H)
        #self.View.ensureVisible(X0,Y0,W,H,4,4)

        self.View.update()

    def New_Connection_fail_handler(self):
        if self.PathInProgress:
            if self.PathInProgress in self.LineConnects:
                self.LineConnects.remove(self.PathInProgress)
            self.PathInProgress.Destroy()
            self.PathInProgress = None
        if self.ConnectionInProgress:
            self.ConnectionInProgress.InProgress = False
        self.ConnectionInProgress = None

    def Delete_Highlighted_Connections(self):
        for i in range(len(self.LineConnects)-1,-1,-1):
            l_ = self.LineConnects[i]
            if isinstance(l_, QGraphicsPathItemCallable):
                if l_.isHighlighted():
                    l_.Destroy(True)
                    self.LineConnects.pop(i)

    def Delete_Highlighted_LayoutItems(self):
        for i in range(len(self.DrawnEntities)-1,-1,-1):
            ent = self.DrawnEntities[i]
            if isinstance(ent, DKLed_Project_Layout_Item):
                if ent.Mainfield.isSelected():
                    ent.Delete_Item(DKLed_Project_Layout_Types.SimpleDevice)

    def button_react(self):
        print("BB is pressed")

class DKLed_Project_Layout_Item:
    def __init__(self):
        self.RepresentedEntity = None #Controller or other thing
        self.RepresentedEntityType = DKLed_Project_Layout_Types.Nothing
        self.RepresentedEntitySpecs = None
        self.LayoutControl = None
        self.Connections = [] #link to connection lines
        self.ConnectionKind = []
        self.ConnectionPoints = [] #positions of connection points
        self.Name = ""
        self.InputCount = 0
        self.OutputCount = 0
        self.PortCount = 0
        self.IconVCount = 1
        self.IconHCount = 1

        #drawebla parts
        self.Mainfield = None
        self.MainfieldOutline = None
        self.Title = None
        self.Paintershape = None
        self.ConnectionShapes = []
        self.ConnectionLabels = []
        self.PixIcons = []
        self.Width = 0
        self.Height = 0
        self.BTNoffsetH = 0

        self.SettingsBTN = []
        self.SettingsBTNdy = []

    def Construct_Item(self, Nam_="", SettingsButton=None):

        self.Connections.clear()
        self.ConnectionKind.clear()
        self.ConnectionPoints.clear()
        self.ConnectionShapes.clear()
        self.ConnectionLabels.clear()
        self.PixIcons.clear()

        self.Paintershape = QPainterPath()
        self.Adjust_size()

        self.Mainfield = self.LayoutControl.Scene.addPath(self.Paintershape, self.LayoutControl.PenOutline, self.LayoutControl.FillBrush)
        self.MainfieldOutline = self.LayoutControl.Scene.addPath(self.Paintershape, self.LayoutControl.PenOutline, self.LayoutControl.NoBrush)
        self.MainfieldOutline.setParentItem(self.Mainfield)
        self.Mainfield.setFlag(QGraphicsItem.ItemIsSelectable, True)
        self.Mainfield.setFlag(QGraphicsItem.ItemIsMovable, True)
        self.Mainfield.setFlag(QGraphicsItem.ItemClipsToShape, True)

        self.Name = Nam_
        self.Title = self.LayoutControl.Scene.addSimpleText(self.Name, self.LayoutControl.MainFont)
        ww = self.Title.boundingRect().width()
        self.Width = self.LayoutControl.Style.Iconsize.width() + 2*self.LayoutControl.Style.ConnectionHOffset
        self.Title.setPos((self.Width-ww)/2, 3)
        self.Title.setParentItem(self.Mainfield)
        self.Add_SettingsButton(SettingsButton)

        self.Mainfield.setPos(self.LayoutControl.View.mapToScene(int((self.LayoutControl.View.maximumViewportSize().width()-self.Width)/2),int((self.LayoutControl.View.maximumViewportSize().height()-self.Height)/2)))
        for ent in self.LayoutControl.DrawnEntities:
            x1 = self.Mainfield.x()
            x2 = ent.Mainfield.x()
            y1 = self.Mainfield.y()
            y2 = ent.Mainfield.y()
            if (abs(x1-x2)<5) and (abs(y1-y2)<5):
                self.Mainfield.setPos(x1+5, y1+5)
            ent.Mainfield.setSelected(False)
        self.LayoutControl.DrawnEntities.append(self)
        self.Mainfield.setSelected(True)

        del ww

    def Adjust_size(self, dx=0, dy=0):
        self.Width = self.LayoutControl.Style.Iconsize.width() + (self.IconHCount-1) * self.LayoutControl.Style.ItemIconsize.width() + 2*self.LayoutControl.Style.ConnectionHOffset+dx
        self.Height = self.LayoutControl.Style.Iconsize.height() + (self.IconVCount-1) * self.LayoutControl.Style.ItemIconsize.height() + 2*self.LayoutControl.Style.ConnectionVOffset+dy

        io = self.InputCount
        if self.OutputCount>io:
            io= self.OutputCount
        pp = self.PortCount
        if io>0:
            io = (io-1)*3
        if pp>0:
            pp = (pp-1)*3

        if self.Height < (self.LayoutControl.Style.ConnectionPointRadius * io + 2*self.LayoutControl.Style.ConnectionVOffset):
            self.Height = (self.LayoutControl.Style.ConnectionPointRadius * io + 2*self.LayoutControl.Style.ConnectionVOffset)
        if self.Width < (self.LayoutControl.Style.ConnectionPointRadius * pp + 2*self.LayoutControl.Style.ConnectionHOffset):
            self.Width = (self.LayoutControl.Style.ConnectionPointRadius * pp + 2 * self.LayoutControl.Style.ConnectionHOffset)
        if not self.Paintershape.isEmpty():
            self.Paintershape.clear()
        self.Paintershape.addRoundedRect(0, 0, self.Width, self.Height, 10, 10)
        if self.Mainfield:
            self.Mainfield.setPath(self.Paintershape)
        if self.MainfieldOutline:
            self.MainfieldOutline.setPath(self.Paintershape)

        del io,pp

    def Add_Input(self, i, y, Nam="0", slot=0):
        pos = [0, y]
        a=self.LayoutControl.Style.ConnectionPointRadius
        self.ConnectionPoints.append(pos)
        Rou = QGraphicsEllipseItemCallable(0 - a, 0 - a, 2 * a, 2 * a)
        Rou.setPos(pos[0], pos[1])
        Rou.setPens(self.LayoutControl.PenOutline, self.LayoutControl.PenConnectHover)
        Rou.setBrush(self.LayoutControl.InpBrush)
        Rou.setParentItem(self.Mainfield)
        Rou.No = i
        Rou.SubNo = slot
        Rou.ConnectType = DKLed_Project_Layout_Types.InputChannel
        Rou.Lay = self.LayoutControl
        #Rou.SceneCursor = self.LayoutControl.Cursor
        self.ConnectionShapes.append(Rou)
        #self.RepresentedEntity.ConnectionPoints.append(Rou)
        Name_ = self.LayoutControl.Scene.addSimpleText(Nam, self.LayoutControl.MainFont)
        Name_.setPos(a+self.LayoutControl.Style.OutlineWidth, 0-Name_.boundingRect().height()/2)
        Name_.setParentItem(Rou)
        self.ConnectionLabels.append(Name_)
        self.ConnectionKind.append([i, DKLed_Project_Layout_Types.InputChannel, DKLed_Project_Layout_Types.Nothing])
        return Rou

    def Add_Output(self, i, y, Nam="W", slot=0):
        #pos = [self.Mainfield.boundingRect().width() - self.LayoutControl.Style.OutlineWidth -0.25, y]
        pos = [self.Width, y]
        a = self.LayoutControl.Style.ConnectionPointRadius
        self.ConnectionPoints.append(pos)
        Rou = QGraphicsEllipseItemCallable(0 - a, 0 - a, 2 * a, 2 * a)
        Rou.setPos(pos[0], pos[1])
        Rou.setPens(self.LayoutControl.PenOutline, self.LayoutControl.PenConnectHover)
        Rou.setBrush(self.LayoutControl.OutBrush)
        Rou.setParentItem(self.Mainfield)
        Rou.No = i
        Rou.SubNo = slot
        Rou.ConnectType = DKLed_Project_Layout_Types.OutputChannel
        Rou.Lay = self.LayoutControl
        #Rou.SceneCursor = self.LayoutControl.Cursor
        self.ConnectionShapes.append(Rou)
        #self.RepresentedEntity.ConnectionPoints.append(Rou)
        Name_ = self.LayoutControl.Scene.addSimpleText(Nam, self.LayoutControl.MainFont)
        Name_.setPos(0-a-Name_.boundingRect().width(), 0-Name_.boundingRect().height()/2)
        Name_.setParentItem(Rou)
        self.ConnectionLabels.append(Name_)
        self.ConnectionKind.append([i, DKLed_Project_Layout_Types.OutputChannel,DKLed_Project_Layout_Types.Nothing])
        return Rou

    def Add_Port(self, i, x, Nam="rx/tx", slot=0):
        #pos = [x, self.Mainfield.boundingRect().height() - self.LayoutControl.Style.OutlineWidth]
        pos = [x, self.Height]
        a = self.LayoutControl.Style.ConnectionPointRadius
        self.ConnectionPoints.append(pos)
        Rou = QGraphicsEllipseItemCallable(0-a, 0-a,2*a,2*a)
        Rou.setPos(pos[0], pos[1])
        Rou.setPens(self.LayoutControl.PenOutline, self.LayoutControl.PenConnectHover)
        Rou.setBrush(self.LayoutControl.PortBrush)
        Rou.setParentItem(self.Mainfield)
        Rou.No = i
        Rou.SubNo = slot
        Rou.ConnectType = DKLed_Project_Layout_Types.PortChanel
        Rou.Lay = self.LayoutControl
        #Rou.SceneCursor = self.LayoutControl.Cursor
        self.ConnectionShapes.append(Rou)
        #self.RepresentedEntity.ConnectionPoints.append(Rou)
        Name_ = self.LayoutControl.Scene.addSimpleText(Nam, self.LayoutControl.MainFont)
        Name_.setRotation(-90)
        Name_.setPos(0 - Name_.boundingRect().height()/2, 0 - a - self.LayoutControl.Style.OutlineWidth)
        Name_.setParentItem(Rou)
        self.ConnectionLabels.append(Name_)
        self.ConnectionKind.append([i, DKLed_Project_Layout_Types.PortChanel, DKLed_Project_Layout_Types.Nothing])
        return Rou

    def Update_Connection_Position_H(self, i, x):
        pos = self.ConnectionShapes[i].pos().y()
        self.ConnectionShapes[i].setPos(x, pos)

    def Add_SettingsButton(self, SettingsButton=None, dx = 0, dy=0):
        if SettingsButton:
            i = len(self.SettingsBTN)
            self.SettingsBTN.append(None)
            self.SettingsBTNdy.append(self.LayoutControl.Style.ConnectionVOffset + (i + dy) * self.LayoutControl.Style.Iconsize.height())
            self.SettingsBTN[i] = self.LayoutControl.Scene.addWidget(SettingsButton)
            self.SettingsBTN[i].setPos((self.Width-SettingsButton.width())/2, self.SettingsBTNdy[i])
            self.SettingsBTN[i].setParentItem(self.Mainfield)

    def Update_Item(self, InC1=0, OuC1=0, PoC1=0):
        InC = 0
        OuC = 0
        PoC = 0
        self.BTNoffsetH = 0
        if self.RepresentedEntitySpecs:
            InC = self.RepresentedEntitySpecs.btn
            OuC = self.RepresentedEntitySpecs.out
            PoC = self.RepresentedEntitySpecs.uartTotalLines

        if InC1>0:
            InC = InC1
        if OuC1>0:
            OuC = OuC1
        if PoC1>0:
            PoC = PoC1

        for i in range(len(self.ConnectionShapes)-1, -1, -1):
            DelPort = False
            if self.ConnectionShapes[i].ConnectType == DKLed_Project_Layout_Types.PortChanel:
                DelPort = True
            elif self.ConnectionShapes[i].ConnectType == DKLed_Project_Layout_Types.OutputChannel and self.ConnectionShapes[i].No >=OuC:
                DelPort = True
            elif self.ConnectionShapes[i].ConnectType == DKLed_Project_Layout_Types.InputChannel and self.ConnectionShapes[i].No >=InC:
                DelPort = True
            if DelPort:
                self.Delete_Connection(i)

        LinC = self.InputCount
        LouC = self.OutputCount
        LpoC = self.PortCount

        self.InputCount = InC
        self.OutputCount = OuC
        self.PortCount = PoC
        self.IconVCount = 1
        self.IconHCount = 1

        for p in self.PixIcons:
            self.LayoutControl.Scene.removeItem(p)
        self.PixIcons.clear()

        if self.RepresentedEntityType == DKLed_Project_Layout_Types.LEDController:
            self.Adjust_size(0, 50)
            for i in range(LinC, self.InputCount):
                rou = self.Add_Input(i,
                                     self.LayoutControl.Style.ConnectionVOffset + i * self.LayoutControl.Style.ConnectionPointRadius * 3,
                                     "I" + hex(i + 1)[2])
                rou.SubconnectType = DKLed_Project_Layout_Types.InputChannel
                #rou.ConnectableTypes.append(DKLed_Project_Layout_Types.Button)
                #rou.ConnectableTypes.append(DKLed_Project_Layout_Types.Encoder)
                #rou.ConnectableTypes.append(DKLed_Project_Layout_Types.EncoderCW)
                #rou.ConnectableTypes.append(DKLed_Project_Layout_Types.EncoderCCW)
                #rou.ConnectableTypes.append(DKLed_Project_Layout_Types.Endstop)
                rou.MultyLines = False
            for i in range(LouC, self.OutputCount):
                rou = self.Add_Output(i,
                                self.LayoutControl.Style.ConnectionVOffset + i * self.LayoutControl.Style.ConnectionPointRadius * 3,
                                "O" + hex(i)[2])
                rou.SubconnectType = DKLed_Project_Layout_Types.OutputChannel
                rou.ConnectableTypes.append(DKLed_Project_Layout_Types.LedScreen)
                rou.ConnectableTypes.append(DKLed_Project_Layout_Types.WS2812B)
                rou.ConnectableTypes.append(DKLed_Project_Layout_Types.WS2815C)
                rou.ConnectableTypes.append(DKLed_Project_Layout_Types.SK6812)
                rou.ConnectableTypes.append(DKLed_Project_Layout_Types.WSSKMix)
                rou.ConnectableTypes.append(DKLed_Project_Layout_Types.Ledstripe)
                rou.ConnectableTypes.append(DKLed_Project_Layout_Types.Servo)
                rou.ConnectableTypes.append(DKLed_Project_Layout_Types.Relay)
                rou.MultyLines = False
            if self.RepresentedEntitySpecs:
                j = 0
                for i in range(0, self.RepresentedEntitySpecs.uart):
                    str_ = self.RepresentedEntitySpecs.uartTypes[i] + " " + str(i + 1)
                    if self.RepresentedEntitySpecs.uartLines[i] < 2:
                        rou = self.Add_Port(i,
                                            self.LayoutControl.Style.ConnectionHOffset + j * self.LayoutControl.Style.ConnectionPointRadius * 3,
                                            str_)
                        if self.RepresentedEntitySpecs.uartTypes[i].upper() == "UART":
                            rou.SubconnectType = DKLed_Project_Layout_Types.UARTRxTx
                            rou.ConnectableTypes.append(DKLed_Project_Layout_Types.UARTRxTx)
                            rou.ConnectableTypes.append(DKLed_Project_Layout_Types.UARTTx)
                            rou.ConnectableTypes.append(DKLed_Project_Layout_Types.UART)
                            rou.ConnectableTypes.append(DKLed_Project_Layout_Types.UARTRx)
                            rou.MultyLines = True
                        elif self.RepresentedEntitySpecs.uartTypes[i].upper() == "I2C":
                            rou.SubconnectType = DKLed_Project_Layout_Types.I2C
                            rou.ConnectableTypes.append(DKLed_Project_Layout_Types.I2C)
                            rou.ConnectableTypes.append(DKLed_Project_Layout_Types.I2CSlave)
                            rou.ConnectableTypes.append(DKLed_Project_Layout_Types.I2CMaster)
                            rou.MultyLines = True
                    else:
                        if self.RepresentedEntitySpecs.uartTypes[i].upper() == "UART":
                            rou = self.Add_Port(i,
                                                self.LayoutControl.Style.ConnectionHOffset + j * self.LayoutControl.Style.ConnectionPointRadius * 3,
                                                str_ + " RX", 0)
                            rou.SubconnectType = DKLed_Project_Layout_Types.UARTRx
                            rou.ConnectableTypes.append(DKLed_Project_Layout_Types.UARTTx)
                            rou.ConnectableTypes.append(DKLed_Project_Layout_Types.UART)
                            rou.ConnectableTypes.append(DKLed_Project_Layout_Types.UARTRxTx)
                            rou.MultyLines = True
                        elif self.RepresentedEntitySpecs.uartTypes[i].upper() == "SPI":
                            rou = self.Add_Port(i,
                                                self.LayoutControl.Style.ConnectionHOffset + j * self.LayoutControl.Style.ConnectionPointRadius * 3,
                                                str_ + " MISO", 0)
                            rou.SubconnectType = DKLed_Project_Layout_Types.SPIMISO
                            rou.ConnectableTypes.append(DKLed_Project_Layout_Types.SPI)
                            rou.ConnectableTypes.append(DKLed_Project_Layout_Types.SPIMISO)
                            rou.MultyLines = True
                        j += 1
                        if self.RepresentedEntitySpecs.uartTypes[i].upper() == "UART":
                            rou = self.Add_Port(i,
                                                self.LayoutControl.Style.ConnectionHOffset + j * self.LayoutControl.Style.ConnectionPointRadius * 3,
                                                str_ + " TX", 1)
                            rou.SubconnectType = DKLed_Project_Layout_Types.UARTTx
                            rou.ConnectableTypes.append(DKLed_Project_Layout_Types.UARTRx)
                            rou.ConnectableTypes.append(DKLed_Project_Layout_Types.UART)
                            rou.ConnectableTypes.append(DKLed_Project_Layout_Types.UARTRxTx)
                            rou.MultyLines = True
                        elif self.RepresentedEntitySpecs.uartTypes[i].upper() == "SPI":
                            rou = self.Add_Port(i,
                                                self.LayoutControl.Style.ConnectionHOffset + j * self.LayoutControl.Style.ConnectionPointRadius * 3,
                                                str_ + " MOSI", 1)
                            rou.SubconnectType = DKLed_Project_Layout_Types.SPIMOSI
                            rou.ConnectableTypes.append(DKLed_Project_Layout_Types.SPI)
                            rou.ConnectableTypes.append(DKLed_Project_Layout_Types.SPIMOSI)
                            rou.MultyLines = True
                    j += 1
                del j
            for i in range(len(self.ConnectionShapes)):
                if self.ConnectionShapes[i].ConnectType == DKLed_Project_Layout_Types.OutputChannel:
                    self.Update_Connection_Position_H(i, self.Width)
                if self.ConnectionShapes[i].ConnectType == DKLed_Project_Layout_Types.InputChannel:
                    if self.RepresentedEntitySpecs:
                        if self.RepresentedEntitySpecs.encd > 0:
                            self.ConnectionShapes[i].Set_Connectable_Types(DKLed_Project_Layout_Types.Encoder)
                        else:
                            self.ConnectionShapes[i].Set_Connectable_Types(DKLed_Project_Layout_Types.Button)
                    else:
                        self.ConnectionShapes[i].Set_Connectable_Types(DKLed_Project_Layout_Types.Button)
            self.Clear_Connectionshapes_Stoplists()
        elif self.RepresentedEntityType == DKLed_Project_Layout_Types.Button:
            #self.InputCount = 0
            #self.OutputCount = 1
            #self.PortCount = 0
            #self.IconVCount = 1
            #self.IconHCount = 1
            self.Adjust_size(0-self.LayoutControl.Style.ConnectionHOffset, 0-self.LayoutControl.Style.ConnectionVOffset+5)
            if LouC == 0:
                rou = self.Add_Output(0,
                                  self.Height/2 - self.LayoutControl.Style.OutlineWidth,
                                  "")
                rou.SubconnectType = DKLed_Project_Layout_Types.Button
                rou.ConnectableTypes.append(DKLed_Project_Layout_Types.InputChannel)
                rou.ConnectableTypes.append(DKLed_Project_Layout_Types.Relay)
                rou.MultyLines = False
            else:
                self.Update_Connection_Position_H(0,self.Width)

            self.Clear_Connectionshapes_Stoplists()
            Px = self.LayoutControl.Icons.ButtonOutPx.scaled(self.LayoutControl.Style.ItemIconsize)
            PPx = self.LayoutControl.Scene.addPixmap(Px)
            PPx.setPos((self.Width - self.LayoutControl.Style.ItemIconsize.width() -
                        self.LayoutControl.Style.ConnectionPointRadius) / 2, self.LayoutControl.Style.ConnectionVOffset)
            PPx.setParentItem(self.Mainfield)
            self.PixIcons.append(PPx)
        elif self.RepresentedEntityType == DKLed_Project_Layout_Types.Keyboard:
            #self.InputCount = 0
            #self.OutputCount = 1
            #self.PortCount = 0
            self.IconVCount = 2
            #self.IconHCount = 1
            self.Adjust_size(0-self.LayoutControl.Style.ConnectionHOffset, 0-self.LayoutControl.Style.ConnectionVOffset+5)
            for i in range(LouC, self.OutputCount):
                rou = self.Add_Output(i,
                                self.LayoutControl.Style.ConnectionVOffset + i * self.LayoutControl.Style.ConnectionPointRadius * 3,
                                hex(i)[2:])
                rou.SubconnectType = DKLed_Project_Layout_Types.Button
                rou.ConnectableTypes.append(DKLed_Project_Layout_Types.InputChannel)
                rou.ConnectableTypes.append(DKLed_Project_Layout_Types.Relay)
                rou.MultyLines = False
            for i in range(len(self.ConnectionShapes)):
                self.Update_Connection_Position_H(i, self.Width)
            self.Clear_Connectionshapes_Stoplists()
            self.BTNoffsetH = -self.LayoutControl.Style.ConnectionPointRadius
        elif self.RepresentedEntityType == DKLed_Project_Layout_Types.Encoder:
            #self.InputCount = 0
            #self.OutputCount = 1
            #self.PortCount = 0
            self.IconVCount = 3
            #self.IconHCount = 1
            self.Adjust_size(0-self.LayoutControl.Style.ConnectionHOffset, 0-self.LayoutControl.Style.ConnectionVOffset+5)
            for i in range(LouC, self.OutputCount):
                rou = self.Add_Output(i,
                                self.LayoutControl.Style.ConnectionVOffset + (i+0.5) * self.LayoutControl.Style.ItemIconsize.height(),
                                "")
                rou.SubconnectType = DKLed_Project_Layout_Types.Button
                rou.ConnectableTypes.append(DKLed_Project_Layout_Types.InputChannel)
                rou.ConnectableTypes.append(DKLed_Project_Layout_Types.Relay)
                rou.MultyLines = False
            for i in range(len(self.ConnectionShapes)):
                if i == 1:
                    self.ConnectionShapes[1].SubconnectType = DKLed_Project_Layout_Types.EncoderCW
                if i == 2:
                    self.ConnectionShapes[2].SubconnectType = DKLed_Project_Layout_Types.EncoderCCW
                self.Update_Connection_Position_H(i, self.Width)
            self.Clear_Connectionshapes_Stoplists()
            Px = self.LayoutControl.Icons.EncoderOutPx.scaled(self.LayoutControl.Style.ItemIconsize)
            PPx = self.LayoutControl.Scene.addPixmap(Px)
            PPx.setPos((self.Width - self.LayoutControl.Style.ItemIconsize.width() -
                        self.LayoutControl.Style.ConnectionPointRadius) / 2, self.LayoutControl.Style.ConnectionVOffset)
            PPx.setParentItem(self.Mainfield)
            self.PixIcons.append(PPx)
            Px = self.LayoutControl.Icons.EncoderCWOutPx.scaled(self.LayoutControl.Style.ItemIconsize)
            PPx = self.LayoutControl.Scene.addPixmap(Px)
            PPx.setPos((self.Width - self.LayoutControl.Style.ItemIconsize.width() -
                        self.LayoutControl.Style.ConnectionPointRadius) / 2, self.LayoutControl.Style.ConnectionVOffset + self.LayoutControl.Style.ItemIconsize.height())
            PPx.setParentItem(self.Mainfield)
            self.PixIcons.append(PPx)
            Px = self.LayoutControl.Icons.EncoderCCWOutPx.scaled(self.LayoutControl.Style.ItemIconsize)
            PPx = self.LayoutControl.Scene.addPixmap(Px)
            PPx.setPos((self.Width - self.LayoutControl.Style.ItemIconsize.width() -
                        self.LayoutControl.Style.ConnectionPointRadius) / 2, self.LayoutControl.Style.ConnectionVOffset + 2*self.LayoutControl.Style.ItemIconsize.height())
            PPx.setParentItem(self.Mainfield)
            self.PixIcons.append(PPx)
        elif self.RepresentedEntityType == DKLed_Project_Layout_Types.Relay:
            #self.InputCount = 0
            #self.OutputCount = 1
            #self.PortCount = 0
            #self.IconVCount = 1
            #self.IconHCount = 1
            self.Adjust_size(0-self.LayoutControl.Style.ConnectionHOffset, 0-self.LayoutControl.Style.ConnectionVOffset)
            if LinC == 0:
                rou = self.Add_Input(0,
                                  self.Height/2 - self.LayoutControl.Style.OutlineWidth,
                                  "")
                rou.SubconnectType = DKLed_Project_Layout_Types.Relay
                rou.ConnectableTypes.append(DKLed_Project_Layout_Types.OutputChannel)
                rou.ConnectableTypes.append(DKLed_Project_Layout_Types.Button)
                rou.ConnectableTypes.append(DKLed_Project_Layout_Types.Encoder)
                rou.ConnectableTypes.append(DKLed_Project_Layout_Types.Endstop)
                rou.ConnectableTypes.append(DKLed_Project_Layout_Types.RelayOut)
                rou.MultyLines = False

            if LouC == 0:
                rou = self.Add_Output(0,
                                  self.Height/2 - self.LayoutControl.Style.OutlineWidth,
                                  "")
                rou.SubconnectType = DKLed_Project_Layout_Types.RelayOut
                rou.ConnectableTypes.append(DKLed_Project_Layout_Types.Drive)
                rou.ConnectableTypes.append(DKLed_Project_Layout_Types.Relay)
                rou.MultyLines = True
            else:
                self.Update_Connection_Position_H(1, self.Width)

            self.Clear_Connectionshapes_Stoplists(True)

            Px = self.LayoutControl.Icons.RelayOutPx.scaled(self.LayoutControl.Style.ItemIconsize)
            PPx = self.LayoutControl.Scene.addPixmap(Px)
            PPx.setPos((self.Width - self.LayoutControl.Style.ItemIconsize.width()) / 2, self.LayoutControl.Style.ConnectionVOffset)
            PPx.setParentItem(self.Mainfield)
            self.PixIcons.append(PPx)
        elif self.RepresentedEntityType == DKLed_Project_Layout_Types.Servo:
            #self.InputCount = 0
            #self.OutputCount = 1
            #self.PortCount = 0
            self.IconVCount = 3.5
            #self.IconHCount = 1
            self.Adjust_size(0 - self.LayoutControl.Style.ConnectionHOffset/2,
                             0 - self.LayoutControl.Style.ConnectionVOffset)
            if LinC == 0:
                rou = self.Add_Input(0,
                                  self.LayoutControl.Style.ConnectionVOffset + 0.5 * self.LayoutControl.Style.ItemIconsize.height(),
                                  "")
                rou.SubconnectType = DKLed_Project_Layout_Types.Servo
                rou.ConnectableTypes.append(DKLed_Project_Layout_Types.OutputChannel)
                rou.ConnectableTypes.append(DKLed_Project_Layout_Types.ServoController)
                rou.MultyLines = False

            self.Clear_Connectionshapes_Stoplists()
            Px = self.LayoutControl.Icons.ServoInPx.scaled(self.LayoutControl.Style.ItemIconsize)
            PPx = self.LayoutControl.Scene.addPixmap(Px)
            PPx.setPos((self.Width - self.LayoutControl.Style.ItemIconsize.width()) / 2, self.LayoutControl.Style.ConnectionVOffset)
            PPx.setParentItem(self.Mainfield)
            self.PixIcons.append(PPx)
        elif self.RepresentedEntityType == DKLed_Project_Layout_Types.LedScreen:
            if InC1 == 0:
                self.InputCount = 1
            if OuC1 == 0:
                self.OutputCount = 1
            #self.PortCount = 0
            self.IconVCount = 1
            #self.IconHCount = 1
            self.Adjust_size(0, 0)
            for i in range(LinC, self.InputCount):
                rou = self.Add_Input(i,
                                     self.LayoutControl.Style.ConnectionVOffset + i * self.LayoutControl.Style.ConnectionPointRadius * 3,
                                     "I" + hex(i)[2])
                rou.SubconnectType = DKLed_Project_Layout_Types.LedScreen
                rou.ConnectableTypes.append(DKLed_Project_Layout_Types.OutputChannel)
                rou.MultyLines = False

            for i in range(LouC, self.OutputCount):
                rou = self.Add_Output(i,
                                      self.LayoutControl.Style.ConnectionVOffset + i * self.LayoutControl.Style.ConnectionPointRadius * 3,
                                      "O" + hex(i)[2])
                rou.SubconnectType = DKLed_Project_Layout_Types.OutputChannel
                rou.ConnectableTypes.append(DKLed_Project_Layout_Types.LedScreen)
                rou.ConnectableTypes.append(DKLed_Project_Layout_Types.WS2812B)
                rou.ConnectableTypes.append(DKLed_Project_Layout_Types.WS2815C)
                rou.ConnectableTypes.append(DKLed_Project_Layout_Types.SK6812)
                rou.ConnectableTypes.append(DKLed_Project_Layout_Types.WSSKMix)
                rou.MultyLines = False

            for i in range(len(self.ConnectionShapes)):
                if self.ConnectionShapes[i].ConnectType == DKLed_Project_Layout_Types.OutputChannel:
                    self.Update_Connection_Position_H(i, self.Width)

            self.Clear_Connectionshapes_Stoplists(True)
        else:
            self.Adjust_size(0, 0)

        self.Title.setPos((self.Width-self.Title.boundingRect().width())/2, 3)
        if len(self.SettingsBTN)>0:
            for i in range(len(self.SettingsBTN)):
                self.SettingsBTN[i].setPos((self.Width-self.SettingsBTN[i].boundingRect().width())/2 + self.BTNoffsetH, self.SettingsBTNdy[i])

    def Sync_with_Phisics_Device(self, i, slot, Type=DKLed_Project_Layout_Types.Nothing, Dev=None, ind=0):
        Rou = None
        for P_ in self.ConnectionShapes:
            if P_.No == i and P_.SubNo == slot and (P_.ConnectType == Type or Type == DKLed_Project_Layout_Types.EveryChannel):
                P_.Sync_with_Phisical_Device(Dev, ind)
                Rou = P_
        return Rou

    def Clear_Connectionshapes_Stoplists(self, Re=False):
        for cc in self.ConnectionShapes:
            cc.ConnectionStoplist.clear()
        if Re:
            for cc in self.ConnectionShapes:
                if cc.ConnectType == DKLed_Project_Layout_Types.InputChannel:
                    for c2 in self.ConnectionShapes:
                        if c2.ConnectType == DKLed_Project_Layout_Types.OutputChannel:
                            cc.ConnectionStoplist.append(c2)
                            c2.ConnectionStoplist.append(cc)

    def Delete_Connection(self, i=0): #deletes the connection marker itself
        if i < len(self.ConnectionShapes):
            for l_ in self.ConnectionShapes[i].Line:
                if l_:
                    l_.Destroy()
            if self.ConnectionShapes[i].ConnectType == DKLed_Project_Layout_Types.PortChanel:
                self.PortCount = self.PortCount-1
            if self.ConnectionShapes[i].ConnectType == DKLed_Project_Layout_Types.OutputChannel:
                self.OutputCount = self.OutputCount-1
            if self.ConnectionShapes[i].ConnectType == DKLed_Project_Layout_Types.InputChannel:
                self.InputCount = self.InputCount-1
            self.ConnectionShapes[i].Sync_with_Phisical_Device()
            self.RepresentedEntity.Remove_Connection_Point(self.ConnectionShapes[i])
            self.LayoutControl.Scene.removeItem(self.ConnectionLabels[i])
            self.LayoutControl.Scene.removeItem(self.ConnectionShapes[i])
            sip.delete(self.ConnectionLabels[i])
            sip.delete(self.ConnectionShapes[i])
            self.ConnectionShapes.pop(i)
            #self.RepresentedEntity.ConnectionPoints.pop(i)
            self.ConnectionLabels.pop(i)
            self.ConnectionPoints[i].clear()
            self.ConnectionPoints.pop(i)
            self.ConnectionKind[i].clear()
            self.ConnectionKind.pop(i)

    def Delete_Item(self, Kind=None):
        if self.RepresentedEntity.Destroy(Kind):
            self.LayoutControl.Scene.removeItem(self.Title)
            self.LayoutControl.Scene.removeItem(self.MainfieldOutline)
            self.RepresentedEntityType = DKLed_Project_Layout_Types.Nothing
            self.Connections.clear()
            self.ConnectionKind.clear()
            self.ConnectionPoints.clear()
            self.ConnectionShapes.clear()
            self.ConnectionLabels.clear()
            for p in self.PixIcons:
                self.LayoutControl.Scene.removeItem(p)
            self.PixIcons.clear()
            for p in self.SettingsBTN:
                self.LayoutControl.Scene.removeItem(p)
            self.SettingsBTN.clear()
            self.LayoutControl.Scene.removeItem(self.Mainfield)
            if self in self.LayoutControl.DrawnEntities:
                self.LayoutControl.DrawnEntities.remove(self)
            if not self.Paintershape.isEmpty():
                self.Paintershape.clear()
            del self
            gc.collect()

    def Empty_Item(self):
        for i in range(0,len(self.ConnectionShapes)):
            self.Delete_Connection()
