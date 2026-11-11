from PySide6.QtCore import QSize, Qt
from PySide6.QtWidgets import QWidget, QVBoxLayout, QHBoxLayout, QTreeWidgetItem, QHeaderView
from qfluentwidgets import BodyLabel, ComboBox, FluentIcon, RoundMenu, Action, SegmentedWidget, setFont

from config import cfg
from config.cfg_dict_tying import (
    AdvancedLogicNode, AdvancedStarNode, AdvancedPlanetNode, AdvancedBondNode,
    AdvancedMineralNode, VeinsName,
)
from language import tr
from ...Compoents.Widgets.line_edit import LimitLineEdit
from .tree import SortTree, StarTreeWidgetItem, PlanetTreeWidgetItem, MoonTreeWidgetItem
from .fields import LeaveBase, TreeWidgetLeave, SettingsTreeLeave
from ..config_to_condition import config_to_advanced_condition


class AdvancedStarItem(StarTreeWidgetItem):
    def _on_del_button_clicked(self):
        self.remove_node()


class AdvancedPlanetItem(PlanetTreeWidgetItem):
    def _on_del_button_clicked(self):
        self.remove_node()


class AdvancedTree(SortTree):
    def __init__(self, editor):
        super().__init__(editor)
        self.editor = editor
        self.setUniformRowHeights(False)
        self.header().setSectionResizeMode(0, QHeaderView.Fixed)
        self.setColumnWidth(0, 200)
        self.header().setSectionResizeMode(1, QHeaderView.Stretch)
        self.header().setSectionResizeMode(2, QHeaderView.Fixed)
        self.setColumnWidth(2, 90)
        self.itemChanged.connect(self._on_item_changed)

    def _on_item_changed(self, item, column):
        node = getattr(item, "node", None)
        if column != 0 or node is None or item.data(0, Qt.CheckStateRole) is None:
            return
        checked = item.checkState(0) == Qt.Checked
        if node.checked != checked:
            node.checked = checked
            self.editor._changed()
        elif isinstance(node, (AdvancedLogicNode, AdvancedMineralNode)):
            name = node.custom_name if node.custom_name is not None else tr(f"search.advanced.{node.kind}")
            if item.text(0) != name:
                node.custom_name = item.text(0)
                cfg.save()

    def on_custom_context_menu_requested(self, pos):
        item = self.itemAt(pos)
        if item is not None:
            self.editor.show_menu(item, self.viewport().mapToGlobal(pos))


class AdvancedConditionEditor(QWidget):
    """独立高级树；普通恒星/行星的固定层级仍复用现有编辑控件。"""

    def __init__(self, config_obj, parent=None):
        super().__init__(parent)
        self.config_obj = config_obj
        layout = QVBoxLayout(self)
        self.tree = AdvancedTree(self)
        self.tree.setBorderVisible(True)
        self.tree.setBorderRadius(8)
        self.warningLabel = BodyLabel()
        self.warningLabel.setTextColor("#c42b1c", "#ff99a4")
        self.warningLabel.setWordWrap(True)
        layout.addWidget(self.warningLabel)
        layout.addWidget(self.tree)
        # 与基础编辑器保持一致的底部布局间距。
        layout.addLayout(QHBoxLayout())
        self.rebuild()

    def validate_condition(self):
        try:
            config_to_advanced_condition(self.config_obj.condition)
        except ValueError as error:
            self.warningLabel.setText(str(error))
            return False
        self.warningLabel.clear()
        return True

    def rebuild(self):
        self.tree.clear()
        self._build_node(self.config_obj.condition, None)
        self.tree.expandAll()
        self.validate_condition()

    def _changed(self):
        cfg.save()
        self.validate_condition()

    def _structure_changed(self):
        cfg.save()
        self.rebuild()

    def add_node(self, parent_node, kind):
        if parent_node.operation == "not" and parent_node.children:
            self.warningLabel.setText(tr("search.advanced.not_one"))
            return
        constructors = {"logic": AdvancedLogicNode, "star": AdvancedStarNode,
                        "planet": AdvancedPlanetNode, "mineral": AdvancedMineralNode,
                        "bond": AdvancedBondNode}
        parent_node.children.append(constructors[kind]())
        self._structure_changed()

    def remove_node(self, parent_node, node):
        # 按身份删除，避免内容相同的两个节点互相误删。
        for index, child in enumerate(parent_node.children):
            if child is node:
                parent_node.children.pop(index)
                break
        self._structure_changed()

    def _row(self, item):
        # 与恒星、行星行一致：40px 行高，其中上下留白共 12px。
        item.setSizeHint(0, QSize(0, 40))
        widget = LeaveBase()
        layout = QHBoxLayout(widget)
        layout.setContentsMargins(0, 0, 0, 0)
        self.tree.setItemWidget(item, 1, widget)
        return layout

    def _manage(self, item, removable):
        widget = TreeWidgetLeave()
        item.manageButtons = widget
        widget.addButton.hide()
        add = widget.addPlanetButton
        add.setIcon(FluentIcon.ADD)
        add.setToolTip(tr("search.advanced.add"))
        add.clicked.connect(lambda: self.show_menu(
            item, add.mapToGlobal(add.rect().bottomLeft()), allow_delete=False))
        for button in (add, widget.delButton):
            policy = button.sizePolicy()
            policy.setRetainSizeWhenHidden(True)
            button.setSizePolicy(policy)
        widget.delButton.setVisible(removable)
        widget.delButton.clicked.connect(item.remove_node)
        self.tree.setItemWidget(item, 2, widget)
        return add

    def _build_node(self, node, parent, parent_node=None):
        if node.kind in ("star", "planet"):
            item = self._build_core(node.kind, node.condition, parent)
            item.remove_node = lambda: self.remove_node(parent_node, node)
            return item
        item = QTreeWidgetItem(parent or self.tree)
        name = tr(f"search.advanced.{node.kind}")
        if isinstance(node, (AdvancedLogicNode, AdvancedMineralNode)):
            item.setFlags(item.flags() | Qt.ItemIsEditable)
            if node.custom_name is not None:
                name = node.custom_name
        item.setText(0, name)
        item.setCheckState(0, Qt.Checked if node.checked else Qt.Unchecked)
        item.node = node
        item.remove_node = lambda: self.remove_node(parent_node, node)
        layout = self._row(item)
        add = self._manage(item, parent_node is not None)
        if node.kind == "logic":
            operations = ("all", "any", "at_least", "not")
            combo = ComboBox()
            setFont(combo, 12)
            combo.addItems([tr(f"search.advanced.{key}") for key in operations])
            combo.setCurrentIndex(operations.index(node.operation))
            layout.addWidget(combo)
            minimum = LimitLineEdit("minimum", node, min_value=1, max_value=65535,
                                    default_value=1, empty_invisible=False)
            minimum.setMaximumWidth(96)
            minimum.setFixedHeight(28)
            layout.addWidget(minimum)
            minimum.setVisible(node.operation == "at_least")
            add.setEnabled(node.operation != "not" or not node.children)

            def change_operation(index):
                operation = operations[index]
                if operation == "not" and len(node.children) > 1:
                    combo.blockSignals(True)
                    combo.setCurrentIndex(operations.index(node.operation))
                    combo.blockSignals(False)
                    self.warningLabel.setText(tr("search.advanced.not_one"))
                    return
                node.operation = operation
                minimum.setVisible(operation == "at_least")
                add.setEnabled(operation != "not" or not node.children)
                self._changed()

            combo.currentIndexChanged.connect(change_operation)
            minimum.editingFinished.connect(self.validate_condition)
            layout.addWidget(BodyLabel(tr("search.advanced.children_count").format(count=len(node.children))))
            layout.addStretch()
            for child in node.children:
                self._build_node(child, item, node)
        elif node.kind == "mineral":
            add.hide()
            for key in ("veins_point", "veins_amount"):
                layout.addWidget(SettingsTreeLeave(
                    buttonText=tr(f"search.condition_tree.buttons.{key}"),
                    config_obj=node,
                    config_key=f"{key}_condition",
                    items=VeinsName().model_dump(),
                    display_title=tr(f"search.condition_tree.settings.galaxy_{key}_title"),
                    description=tr(f"search.condition_tree.settings.galaxy_{key}_description"),
                ))
            layout.addStretch()
        else:
            add.setHidden(True)
            layout.addWidget(BodyLabel(tr("search.advanced.distance")))
            distance = LimitLineEdit("distance", node, "float", min_value=0, max_value=1e9,
                                     default_value=1000.0, empty_invisible=False)
            distance.setMaximumWidth(96)
            distance.setFixedHeight(28)
            layout.addWidget(distance)
            layout.addWidget(BodyLabel(tr("search.advanced.bond_count")))
            count = LimitLineEdit("satisfy_num", node, min_value=1, max_value=2147483647,
                                  default_value=1, empty_invisible=False)
            count.setMaximumWidth(96)
            count.setFixedHeight(28)
            layout.addWidget(count)
            for key in ("con1", "con2"):
                endpoint = getattr(node, key)
                layout.addWidget(BodyLabel(tr(f"search.advanced.{key}")))
                segment = SegmentedWidget()
                for kind in ("star", "planet"):
                    button = segment.addItem(kind, tr(f"search.advanced.{kind}"),
                                             onClick=lambda checked=False, ep=endpoint, k=kind:
                                             self._change_endpoint(ep, k))
                    setFont(button, 12)
                segment.setCurrentItem(endpoint.kind)
                layout.addWidget(segment)
                core = self._build_core(endpoint.kind, getattr(endpoint, endpoint.kind), item)
                core.leaf.hitStarNumLabel.setText(tr("search.advanced.max_connections"))
                core.remove_node = lambda: None
                policy = core.manageButtons.delButton.sizePolicy()
                policy.setRetainSizeWhenHidden(True)
                core.manageButtons.delButton.setSizePolicy(policy)
                core.manageButtons.delButton.hide()
                core.endpoint = True
            layout.addStretch()
        return item

    def _change_endpoint(self, endpoint, kind):
        if endpoint.kind == kind:
            return
        endpoint.kind = kind
        self._structure_changed()

    def _build_core(self, kind, condition, parent):
        item_class = AdvancedStarItem if kind == "star" else AdvancedPlanetItem
        item = item_class(self.tree, condition)
        (parent.addChild(item) if parent else self.tree.addTopLevelItem(item))
        item.add_widgets()
        # 类型保留在提示中，条件列只显示实际字段。
        item.setToolTip(0, tr(f"search.advanced.{kind}"))
        planets = condition.planet_condition if kind == "star" else [condition]
        for planet in planets:
            planet_item = item.addPlanetLeaf(planet) if kind == "star" else item
            for moon in planet.moon_conditions:
                planet_item.addMoonLeaf(moon)
        return item

    def show_menu(self, item, position, allow_delete=True):
        menu = RoundMenu(parent=self)

        def action(text, callback):
            entry = Action(text)
            entry.triggered.connect(lambda checked=False: callback())
            menu.addAction(entry)

        node = getattr(item, "node", None)
        if isinstance(node, AdvancedLogicNode):
            if node.operation != "not" or not node.children:
                for kind in ("logic", "star", "planet", "mineral", "bond"):
                    action(tr("search.advanced.add_type").format(kind=tr(f"search.advanced.{kind}")),
                           lambda checked=False, k=kind: self.add_node(node, k))
        elif isinstance(item, StarTreeWidgetItem):
            action(tr("search.condition_tree.menu.add_planet"), item.addPlanetLeaf)
        elif isinstance(item, PlanetTreeWidgetItem):
            action(tr("search.condition_tree.menu.add_moon"), item.addMoonLeaf)
        if allow_delete and ((node is not None and item.parent() is not None)
                or (isinstance(item, (StarTreeWidgetItem, PlanetTreeWidgetItem, MoonTreeWidgetItem))
                    and not getattr(item, "endpoint", False))):
            action(tr("search.condition_tree.menu.delete"),
                   item.remove_node if node is not None else item._on_del_button_clicked)
        if menu.menuActions():
            menu.closedSignal.connect(menu.deleteLater)
            menu.exec(position)
        else:
            menu.deleteLater()
