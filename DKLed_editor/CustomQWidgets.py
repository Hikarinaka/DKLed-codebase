from PyQt5 import QtWidgets, uic
from PyQt5.QtWidgets import QInputDialog, QGraphicsView, QGraphicsItem, QGraphicsScene, \
    QStyle, QStyleOptionGraphicsItem, QGraphicsEllipseItem, QGraphicsPathItem, QGraphicsRectItem


class QtComboBoxUnScroll(QtWidgets.QComboBox):
    def __init__(self, *args, **kwargs):
        super(QtComboBoxUnScroll, self).__init__(*args, **kwargs)

    def wheelEvent(self, event):
        event.ignore()

class QtScrollDrawArea(QtWidgets.QScrollArea):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def paintEvent(self, event):
        pass

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
        Program_Windows.append(self)
        self.show()

    def closeEvent(self, event):
        val = True
        print("closing window")
        if self.ParentAction:
            val = self.ParentAction()
        if val:
            #print(Program_Windows)
            Program_Windows.remove(self)
            event.accept()
        else:
            event.ignore()

class QGraphicsRectHoverBoard(QGraphicsRectItem):
    def __init__(self,*args, **kwargs):
        super().__init__(*args, **kwargs)
        self.S = None
        self.Lay = None
        self.setAcceptHoverEvents(True)
        self.MousePressHandler = None
        self.MouseReleaseHandler = None

    def hoverMoveEvent(self, e):
        p = e.scenePos()
        self.S.MX = p.x()
        self.S.MY = p.y()
        self.S.update(self.rect())
        pass

    def mousePressEvent(self, e):
        if self.MousePressHandler:
            self.MousePressHandler()
            #self.Lay.New_Connection_fail_handler()
        pass

    def mouseReleaseEvent(self, e):
        if self.MouseReleaseHandler:
            self.MouseReleaseHandler()
            #self.Lay.New_Connection_fail_handler()
        pass

class QGraphicsSceneCallable(QtWidgets.QGraphicsScene):
    def __init__(self,*args, **kwargs):
        super().__init__(*args, **kwargs)
        self.MX = 0
        self.MY = 0
        self.Scale = 1.0

    #def mouseMoveEvent(self, e):
        #p = e.scenePos()
        #self.MX = p.x()
        #self.MY = p.y()
        #pass


def Close_All_Windows():
    for W in Program_Windows:
        if W:
            if isinstance(W, Functional_Window):
                W.close()
    return True


Program_Windows = []