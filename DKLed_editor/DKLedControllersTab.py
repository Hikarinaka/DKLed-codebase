from PyQt5 import QtWidgets, uic, QtGui
from PyQt5.QtGui import QIntValidator, QRegularExpressionValidator, QCloseEvent, QBrush, QPen, QPainter, QColor, \
    QFont, QIcon, QPainterPath, QPixmap, QMouseEvent
from PyQt5.QtCore import QRegularExpression, Qt, QSize, QEvent, QPointF
from PyQt5.QtWidgets import QInputDialog, QGraphicsView, QGraphicsItem, QGraphicsScene, \
    QStyle, QStyleOptionGraphicsItem, QGraphicsEllipseItem, QGraphicsPathItem, QGraphicsRectItem, QTabWidget
from PyQt5.QtSerialPort import QSerialPort, QSerialPortInfo
from PyQt5.QtCore import QIODevice
#from DKLedProjecLayout import DKLed_Layout_Icons
from enum import Enum
import sip
import sys
import gc
from DKLedInterfaceNames import Interface_Names, Interface_Style, Interface_Icons

class DKLed_Controller_Controls:
    def __init__(self, Layoutinto):
        self.TabWidget = QtWidgets.QTabWidget()
        self.TabWidget.setElideMode(3)  # Qt::ElideNone
        self.TabWidget.setUsesScrollButtons(True)
        self.TabWidget.setTabPosition(QTabWidget.West)
        self.TabWidget.setTabShape(QTabWidget.Triangular)

        self.ControllerTab = self.TabWidget
        self.DeviceTab = self.TabWidget

        self.Style = Interface_Style
        self.Icons = Interface_Icons

        LayV = QtWidgets.QHBoxLayout()

        self.ToolBar = QtWidgets.QToolBar()
        self.ToolBar.setOrientation(Qt.Horizontal)
        self.ToolBar.setIconSize(self.Style.ToolIconsize)
        self.ActionAddController = self.ToolBar.addAction(self.Icons.AddControllerIcon, Interface_Names.MainWindow_LayoutTab_AddControllerTool)
        self.ActionAddMP3 = self.ToolBar.addAction(self.Icons.AddMP3Icon, Interface_Names.MainWindow_LayoutTab_AddMP3Tool)
        self.ActionAddDevice = self.ToolBar.addAction(self.Icons.AddDeviceIcon, Interface_Names.MainWindow_LayoutTab_AddDeviceTool)

        LayV.addWidget(self.ToolBar)
        LayV.addStretch()
        Layoutinto.addLayout(LayV)
        Layoutinto.addWidget(self.TabWidget)


        #self.View.show()

class DKLed_Screens_Controls:
    def __init__(self, Layoutinto):
        self.TabWidget = QtWidgets.QTabWidget()
        self.TabWidget.setElideMode(3)  # Qt::ElideNone
        self.TabWidget.setUsesScrollButtons(True)
        self.TabWidget.setTabPosition(QTabWidget.West)
        self.TabWidget.setTabShape(QTabWidget.Triangular)

        self.ControllerTab = self.TabWidget
        self.DeviceTab = self.TabWidget

        self.Style = Interface_Style
        self.Icons = Interface_Icons

        LayV = QtWidgets.QVBoxLayout()

        self.ToolBar = QtWidgets.QToolBar()
        self.ToolBar.setOrientation(Qt.Vertical)
        self.ToolBar.setIconSize(self.Style.ToolIconsize)
        self.ActionAddScreen = self.ToolBar.addAction(self.Icons.AddScreenIcon, Interface_Names.MainWindow_LayoutTab_AddScreenTool)


        LayV.addWidget(self.ToolBar)
        LayV.addStretch()
        Layoutinto.addLayout(LayV)
        Layoutinto.addWidget(self.TabWidget)

