from PyQt6.QtWidgets import QApplication, QLabel
from PyQt6.QtCore import Qt, QTimer, QPoint
from PyQt6.QtGui import QFont, QColor, QPainter, QFontMetrics
import sys
from utils import get_system_info_text
from config import UPDATE_INTERVAL_MS, FONT_FAMILY, FONT_SIZE, COLOR_DEFAULT, COLOR_HIGHLIGHT, GAP_X, GAP_Y


class TransparentLabel(QLabel):
    def __init__(self):
        super().__init__()
        self.setAttribute(Qt.WidgetAttribute.WA_TranslucentBackground)
        self.setWindowFlags(Qt.WindowType.FramelessWindowHint | Qt.WindowType.Tool | Qt.WindowType.WindowStaysOnTopHint
        )
        self.setAlignment(Qt.AlignmentFlag.AlignTop | Qt.AlignmentFlag.AlignRight)

        # Fontes
        self.font_main = QFont(FONT_FAMILY, FONT_SIZE)
        self.font_title = QFont(FONT_FAMILY, 16, QFont.Weight.Bold)

        # Cores
        self.color_default = QColor(COLOR_DEFAULT)
        self.color_highlight = QColor(COLOR_HIGHLIGHT)

        self.text_data = ""

    def set_info_text(self, info_text: str):
        self.text_data = info_text
        self.update()

    def paintEvent(self, event):
        if not self.text_data:
            return

        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.TextAntialiasing)

        lines = self.text_data.splitlines()

        # Margens
        margin_x = 15
        y = 15  # Margem superior
        line_spacing = 1  # Espaçamento reduzido entre linhas

        for i, line in enumerate(lines):
            if i == 0:
                painter.setFont(self.font_title)
                color = self.color_highlight
            elif line.strip("- ") == "":
                painter.setFont(self.font_main)
                color = self.color_highlight
            else:
                painter.setFont(self.font_main)
                color = self.color_default

            # Desenhar contorno sutil
            pen_outline = QColor(0, 0, 0)
            painter.setPen(pen_outline)
            for dx, dy in [(-1, 0), (1, 0), (0, -1), (0, 1)]:
                painter.drawText(margin_x + dx, y + dy, line)

            painter.setPen(color)
            painter.drawText(margin_x, y, line)
            y += QFontMetrics(painter.font()).height() + line_spacing

        painter.end()


def create_gui():
    app = QApplication(sys.argv)
    label = TransparentLabel()
    label.setWindowTitle("Desk Info")

    def update_info():
        text = get_system_info_text()
        label.set_info_text(text)

        # Ajusta dinamicamente o tamanho da janela ao texto
        fm = QFontMetrics(label.font_main)
        width = max(fm.horizontalAdvance(line) for line in text.splitlines()) + 40
        height = len(text.splitlines()) * (fm.height() + 2) + 30
        label.resize(width, height)

        # Posição — canto superior direito
        screen = app.primaryScreen().availableGeometry()
        x = screen.width() - width - GAP_X
        y = GAP_Y
        label.move(QPoint(x, y))

    update_info()
    timer = QTimer()
    timer.timeout.connect(update_info)
    timer.start(UPDATE_INTERVAL_MS)

    label.show()
    sys.exit(app.exec())


if __name__ == "__main__":
    create_gui()
