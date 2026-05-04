from qfluentwidgets import ComboBox
from PySide6.QtCore import Qt, QPoint
from PySide6.QtGui import QAction, QPainter
from qfluentwidgets.components.widgets.menu import MenuAnimationType, MenuItemDelegate, RoundMenu
from qfluentwidgets.components.widgets.combo_box import ComboBoxMenu
from qfluentwidgets import themeColor, setFont

from config import cfg

class MultiComboBoxItemDelegate(MenuItemDelegate):
    def paint(self, painter: QPainter, option, index):
        super().paint(painter, option, index)
        
        action = index.data(Qt.UserRole)
        # if action is not checked, do not draw the indicator
        if not (isinstance(action, QAction) and action.isChecked()):
            return
            
        painter.save()
        painter.setRenderHints(QPainter.Antialiasing | QPainter.SmoothPixmapTransform | QPainter.TextAntialiasing)
            
        painter.setPen(Qt.NoPen)
        painter.setBrush(themeColor())
        # Draw the standard QFluentWidget ComboBox selected vertical line indicator
        painter.drawRoundedRect(6, 11 + option.rect.y(), 3, 15, 1.5, 1.5)
        
        painter.restore()

class MultiComboBoxMenu(ComboBoxMenu):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.view.setItemDelegate(MultiComboBoxItemDelegate())
        # Removing 'checkableListWidget' to avoid the text offset caused by Checkable style
        # and revert back to combobox native feel
        self.view.setObjectName('comboListWidget')
        
        original_adjustSize = self.view.adjustSize
        view = self.view
        
        def _view_adjustSize_patched(pos=None, aniType=MenuAnimationType.NONE):
            if pos is None and hasattr(view, '_exec_pos'):
                pos = view._exec_pos
                aniType = getattr(view, '_exec_aniType', MenuAnimationType.NONE)
            original_adjustSize(pos, aniType)
            
        self.view.adjustSize = _view_adjustSize_patched

    def _adjustItemText(self, item, action):
        return super()._adjustItemText(item, action)

    def exec(self, pos, ani=True, aniType=MenuAnimationType.DROP_DOWN):
        self.view._exec_pos = pos
        self.view._exec_aniType = aniType
        self.view.adjustSize(pos, aniType)
        self.adjustSize()
        return RoundMenu.exec(self, pos, ani, aniType)

    def _hideMenu(self, isHideBySystem=False):
        self.isHideBySystem = isHideBySystem
        if not isHideBySystem:
            return
        self.view.clearSelection()
        if self.isSubMenu:
            self.hide()
        else:
            self.close()

class MultiComboBox(ComboBox):
    def __init__(self, config_key: str, config_obj = None, parent = None):
        super().__init__(parent)
        self.config_key = config_key
        self.config_obj = cfg.config if config_obj is None else config_obj
        self.selected_texts = set()
        setFont(self, 12)
        self.updateText()

    def _onItemClicked(self, index):
        if not self.items[index].isEnabled:
            return
        item_text = self.items[index].text

        if item_text in self.selected_texts:
            self.selected_texts.remove(item_text)
        else:
            self.selected_texts.add(item_text)
        self.save_config()
        self.updateText()

    def load_config(self):
        if self.config_key is None:
            return
        item_texts = set(item.text for item in self.items)
        config_value = getattr(self.config_obj, self.config_key)
        self.selected_texts = set(config_value) & item_texts
        self.save_config()
        self.updateText()

    def save_config(self):
        if self.config_key is None:
            return
        setattr(self.config_obj, self.config_key, list(self.selected_texts))
        cfg.save()

    def updateText(self):
        count = len(self.selected_texts)
        if count == 0:
            super().setText("无限制")
        elif count == 1:
            super().setText(list(self.selected_texts)[0])
        else:
            super().setText(f"已选{count}项")

    def setCurrentIndex(self, index: int):
        pass

    def _createComboMenu(self):
        return MultiComboBoxMenu(self)

    def _showComboMenu(self):
        if not self.items:
            return

        menu = self._createComboMenu()
        for item in self.items:
            action = QAction(item.icon, item.text, self)
            action.setCheckable(True)
            action.setChecked(item.text in self.selected_texts)
            action.setEnabled(item.isEnabled)
            menu.addAction(action)

        menu.view.itemClicked.connect(lambda i: self._onItemClicked(self.findText(i.text().lstrip())))

        if menu.view.width() < self.width():
            menu.view.setMinimumWidth(self.width())
            menu.adjustSize()

        menu.setMaxVisibleItems(self.maxVisibleItems())
        menu.setAttribute(Qt.WidgetAttribute.WA_DeleteOnClose)
        menu.closedSignal.connect(self._onDropMenuClosed)
        self.dropMenu = menu

        # determine the animation type
        x = -menu.width()//2 + menu.layout().contentsMargins().left() + self.width()//2
        pd = self.mapToGlobal(QPoint(x, self.height()))
        hd = menu.view.heightForAnimation(pd, MenuAnimationType.DROP_DOWN)

        pu = self.mapToGlobal(QPoint(x, 0))
        hu = menu.view.heightForAnimation(pu, MenuAnimationType.PULL_UP)

        if hd >= hu:
            menu.view.adjustSize(pd, MenuAnimationType.DROP_DOWN)
            menu.exec(pd, ani=True, aniType=MenuAnimationType.DROP_DOWN)
        else:
            menu.view.adjustSize(pu, MenuAnimationType.PULL_UP)
            menu.exec(pu, ani=True, aniType=MenuAnimationType.PULL_UP)
