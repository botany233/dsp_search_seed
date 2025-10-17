from qfluentwidgets import MessageBox

from qfluentwidgets import BodyLabel


class SettingMessageBox(MessageBox):
    def __init__(self, parent = None):
        super().__init__("矿物数量设置", "", parent)

        self.textLayout.removeWidget(self.contentLabel)

        self.label1 = BodyLabel("设置已更改，请重启软件以应用更改。")
        self.textLayout.addWidget(self.label1)
        print(self.textLayout)






if __name__ == "__main__":
    import sys
    from qfluentwidgets import setTheme, Theme
    from PySide6.QtWidgets import QApplication

    app = QApplication(sys.argv)

    dlg = SettingMessageBox("设置提示", "设置已更改，请重启软件以应用更改。")
    dlg.yesButton.setText("重启软件")
    dlg.cancelButton.setText("稍后重启")
    dlg.show()
    sys.exit(app.exec())
