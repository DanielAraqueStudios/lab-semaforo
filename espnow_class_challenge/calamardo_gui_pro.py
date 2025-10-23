import sys
from PyQt6.QtWidgets import (
    QApplication, QWidget, QVBoxLayout, QHBoxLayout, QLabel, QLineEdit, QPushButton, QComboBox, QSpinBox, QTextEdit, QMessageBox, QFrame
)
from PyQt6.QtGui import QFont, QPalette, QColor, QPixmap
from PyQt6.QtCore import Qt
import serial
import serial.tools.list_ports
import datetime

PRODUCTOS = [
    ("Hamburguesas", "HBRGR", 50),
    ("Pan de alga", "SWBRD", 50),
    ("Salsa camaron", "SHRMP", 15),
    ("Pepinillos", "PICKL", 20),
    ("Tomate", "TMATO", 20),
    ("Lechuga", "LETCE", 20),
]

SPONGE_COLORS = [QColor(255, 255, 102), QColor(255, 230, 128), QColor(255, 255, 153)]

class CalamardoGUI(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Calamardo - Envío de Insumos (ESP-NOW)")
        self.setFixedSize(500, 420)
        self.init_ui()
        self.serial_port = None

    def set_spongebob_palette(self):
        palette = QPalette()
        # Fondo tipo "esponja"
        palette.setColor(QPalette.ColorRole.Window, SPONGE_COLORS[0])
        palette.setColor(QPalette.ColorRole.Base, SPONGE_COLORS[1])
        palette.setColor(QPalette.ColorRole.AlternateBase, SPONGE_COLORS[2])
        palette.setColor(QPalette.ColorRole.WindowText, QColor(40, 40, 40))
        palette.setColor(QPalette.ColorRole.Text, QColor(40, 40, 40))
        palette.setColor(QPalette.ColorRole.Button, QColor(255, 230, 128))
        palette.setColor(QPalette.ColorRole.ButtonText, QColor(40, 40, 40))
        palette.setColor(QPalette.ColorRole.Highlight, QColor(0, 120, 215))
        self.setPalette(palette)

    def init_ui(self):
        self.set_spongebob_palette()
        font = QFont("Comic Sans MS", 12)
        self.setFont(font)

        layout = QVBoxLayout()
        layout.setSpacing(10)

        # Logo
        logo = QLabel()
        logo.setAlignment(Qt.AlignmentFlag.AlignCenter)
        logo.setPixmap(QPixmap().fromImage(self.spongebob_logo_img()).scaledToHeight(60))
        layout.addWidget(logo)

        # Título
        title = QLabel("Calamardo - Reto ESP-NOWClass")
        title.setAlignment(Qt.AlignmentFlag.AlignCenter)
        title.setStyleSheet("font-size: 22px; font-weight: bold; color: #2d2d2d; text-shadow: 1px 1px 0 #fff;")
        layout.addWidget(title)

        # Serial port selection
        port_layout = QHBoxLayout()
        port_label = QLabel("Puerto Serial:")
        self.port_combo = QComboBox()
        self.refresh_ports()
        refresh_btn = QPushButton("⟳")
        refresh_btn.setFixedWidth(32)
        refresh_btn.clicked.connect(self.refresh_ports)
        port_layout.addWidget(port_label)
        port_layout.addWidget(self.port_combo)
        port_layout.addWidget(refresh_btn)
        layout.addLayout(port_layout)

        # Producto selection
        prod_layout = QHBoxLayout()
        prod_label = QLabel("Producto:")
        self.prod_combo = QComboBox()
        for nombre, codigo, cantidad in PRODUCTOS:
            self.prod_combo.addItem(f"{nombre} ({codigo})", (codigo, cantidad))
        prod_layout.addWidget(prod_label)
        prod_layout.addWidget(self.prod_combo)
        layout.addLayout(prod_layout)

        # Cantidad
        cant_layout = QHBoxLayout()
        cant_label = QLabel("Cantidad:")
        self.cant_spin = QSpinBox()
        self.cant_spin.setRange(1, 255)
        self.cant_spin.setValue(PRODUCTOS[0][2])
        cant_layout.addWidget(cant_label)
        cant_layout.addWidget(self.cant_spin)
        layout.addLayout(cant_layout)

        # Enviar button
        self.send_btn = QPushButton("Enviar a Don Cangrejo 🦀")
        self.send_btn.setStyleSheet("background-color: #ffb300; color: #2d2d2d; font-weight: bold; border-radius: 8px; padding: 8px 0;")
        self.send_btn.clicked.connect(self.enviar_comando)
        layout.addWidget(self.send_btn)

        # Línea separadora
        sep = QFrame()
        sep.setFrameShape(QFrame.Shape.HLine)
        sep.setFrameShadow(QFrame.Shadow.Sunken)
        layout.addWidget(sep)

        # Logs
        logs_label = QLabel("Registro de envíos:")
        logs_label.setStyleSheet("font-weight: bold;")
        layout.addWidget(logs_label)
        self.logs = QTextEdit()
        self.logs.setReadOnly(True)
        self.logs.setStyleSheet("background: #fffde7; color: #2d2d2d; border-radius: 6px;")
        layout.addWidget(self.logs, stretch=1)

        self.setLayout(layout)
        self.prod_combo.currentIndexChanged.connect(self.update_cantidad)

    def spongebob_logo_img(self):
        # Logo simple: texto tipo Bob Esponja (puedes reemplazar por imagen real si tienes una)
        from PyQt6.QtGui import QImage, QPainter
        img = QImage(320, 60, QImage.Format.Format_ARGB32)
        img.fill(QColor(255, 255, 102))
        painter = QPainter(img)
        painter.setFont(QFont("Comic Sans MS", 32, QFont.Weight.Bold))
        painter.setPen(QColor(0, 0, 0))
        painter.drawText(10, 45, "BOB ESPONJA")
        painter.end()
        return img

    def refresh_ports(self):
        self.port_combo.clear()
        ports = serial.tools.list_ports.comports()
        for port in ports:
            self.port_combo.addItem(port.device)

    def update_cantidad(self):
        _, cantidad = self.prod_combo.currentData()
        self.cant_spin.setValue(cantidad)

    def enviar_comando(self):
        port_name = self.port_combo.currentText()
        if not port_name:
            QMessageBox.warning(self, "Error", "Selecciona un puerto serial.")
            return
        codigo, _ = self.prod_combo.currentData()
        cantidad = self.cant_spin.value()
        comando = f"{codigo}{cantidad}\n"
        try:
            with serial.Serial(port_name, 115200, timeout=1) as ser:
                ser.write(comando.encode())
            self.log(f"✅ [{self.now()}] Enviado: {codigo} {cantidad}")
        except Exception as e:
            self.log(f"❌ [{self.now()}] Error: {e}")

    def log(self, msg):
        self.logs.append(msg)
        self.logs.moveCursor(self.logs.textCursor().End)

    def now(self):
        return datetime.datetime.now().strftime("%H:%M:%S")

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = CalamardoGUI()
    window.show()
    sys.exit(app.exec())
