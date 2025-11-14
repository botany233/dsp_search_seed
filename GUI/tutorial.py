import os
from qfluentwidgets import TextBrowser, setCustomStyleSheet
from PySide6.QtWidgets import QFrame, QVBoxLayout, QWidget, QFileDialog
from PySide6.QtCore import QUrl
from PySide6.QtGui import QFont
import markdown
import re

class TutorialInterface(QFrame):
    def __init__(self):
        super().__init__()

        self.main_layout = QVBoxLayout(self)

        self.text_browser = TextBrowser()
        self.text_browser.setOpenExternalLinks(True)  # 允许打开外部链接
        self.text_browser.setFont(QFont("Microsoft YaHei", 12))
        self.text_browser.document().setDefaultStyleSheet("""
            img {
                max-width: 100% !important;
                height: auto !important;
                object-fit: contain;
            }
        """)
        qss = """TextBrowser{background:transparent}TextBrowser:hover{background:transparent}"""
        setCustomStyleSheet(self.text_browser,qss,qss)
        self.main_layout.addWidget(self.text_browser)
        self.load_markdown_file("assets/tutorial.md")

    def load_markdown_file(self, file_path):
        try:
            with open(file_path, 'r', encoding='utf-8') as file:
                content = file.read()
            base_dir = os.path.dirname(os.path.abspath(file_path))
            processed_content = self.process_image_paths(content, base_dir)
            html_content = markdown.markdown(
                processed_content,
                extensions=[
                    'markdown.extensions.extra',
                    'markdown.extensions.codehilite',
                    'markdown.extensions.toc',
                    'markdown.extensions.tables'
                ]
            )

            self.text_browser.setSearchPaths([base_dir])

            styled_html = f"""
            <style>
                img {{
                    max-width: 100% !important;
                    height: auto !important;
                    object-fit: contain;
                }}
            </style>
            {html_content}
            """

            self.text_browser.setHtml(styled_html)

        except Exception as e:
            error_html = f"""
            <h3>加载文件时出错</h3>
            <p>错误信息: {str(e)}</p>
            <p>请确保文件存在且可读。</p>
            """
            self.text_browser.setHtml(error_html)

    def process_image_paths(self, content, base_dir):
        pattern = r'!\[([^\]]*)\]\(([^)]+)\)'

        def replace_image_path(match):
            alt_text = match.group(1)
            img_path = match.group(2)

            if os.path.isabs(img_path):
                full_path = img_path
            else:
                full_path = os.path.join(base_dir, img_path)

            full_path = os.path.abspath(full_path)
            file_url = QUrl.fromLocalFile(full_path).toString()
            
            return f'![{alt_text}]({file_url})'
        
        processed_content = re.sub(pattern, replace_image_path, content)
        return processed_content
