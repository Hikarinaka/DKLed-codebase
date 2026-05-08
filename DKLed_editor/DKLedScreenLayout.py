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

class DKLed_Screen_Layout_Actions(Enum):
    NoAction = 0
    NewMatrix = 1
    AppendNewLed = 2
    InsertNewLed = 3
    DelLed = 4

class DKLed_Screen_Layout_Controls:
    def __init__(self, Layoutinto, Scr):
        self.Screen = Scr

        self.Scene = QGraphicsSceneCallable()
        self.View = QtWidgets.QGraphicsView(self.Scene)
        #self.View.setDragMode(QGraphicsView.ScrollHandDrag)
        self.View.setTransformationAnchor(QGraphicsView.AnchorUnderMouse)

        self.Style = Interface_Style
        self.Icons = Interface_Icons
        self.Colorer = QColor()

        self.ChannelCount = 8

        Layoutinto.addWidget(self.View)

        self.DrawnEntities = []
        self.DrawnEntities.clear()
        self.LineConnectionsPurgeInProgress = False

        self.Hovered = None
        self.PathInProgress = None
        self.ConnectionInProgress = None

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
        self.BackGroundBrush = QBrush(self.Icons.ScreenBackgroundScaledPx)
        #self.BackGroundBrush.setStyle(Qt.TexturePattern)
        self.NoBrush = QBrush(Qt.NoBrush)
        self.MainFont = QFont()

        self.Hoverboard = QGraphicsRectHoverBoard(0, 0, 20, 20)
        self.Scene.addItem(self.Hoverboard)
        self.Hoverboard.S = self.Scene
        self.Hoverboard.Lay = self
        self.Hoverboard.setZValue(-1)
        self.Hoverboard.setPen(self.PenConnect)
        self.Hoverboard.setBrush(self.NoBrush)
        self.Hoverboard.MousePressFailHandler = self.New_Connection_fail_handler
        #self.Hoverboard.setFlag(QGraphicsItem.ItemIsSelectable, False)

        self.Marker = self.Scene.addRect(0, 0, self.Style.ScreenLayoutMarkerStep, self.Style.ScreenLayoutMarkerStep, self.PenConnectConstructed, self.NoBrush)
        self.Marker.setZValue(self.Style.MaximumScreenChannelCount + 16)
        self.View.setBackgroundBrush(self.BackGroundBrush)
        self.MarkerAncorX = 0
        self.MarkerAncorY = 0
        #self.View.scale(0.5,0.5)

        self.MainPainter = QPainter()
        self.MainPainter.setBrush(self.FillBrush)
        self.MainPainter.setPen(self.PenOutline)

        self.Scene.changed.connect(self.Rect_to_front)
        #self.View.show()

    def Rect_to_front(self):

        MkX = (self.Scene.MX//self.Style.ScreenLayoutMarkerStep) * self.Style.ScreenLayoutMarkerStep
        MkY = (self.Scene.MY//self.Style.ScreenLayoutMarkerStep) * self.Style.ScreenLayoutMarkerStep
        self.Marker.setPos(MkX, MkY)

        W = self.View.maximumViewportSize().width()-2
        H = self.View.maximumViewportSize().height() - 2
        P = self.View.mapToScene(0, 0)
        X0 = P.x()+1
        Y0 = P.y()+1
        self.Hoverboard.setRect(X0, Y0, W, H)

        self.View.update()

    def New_Connection_fail_handler(self):
        pass

    def Update_ChannelCount(self, Ck):
        if Ck < 1:
            Ck = 1
        if Ck > self.Style.MaximumScreenChannelCount:
            Ck = self.Style.MaximumScreenChannelCount
        self.ChannelCount = Ck

    def button_react(self):
        print("BB is pressed")