import sys
from PyQt6.QtWidgets import (
    QApplication, QWidget, QVBoxLayout, QHBoxLayout, QLabel, QLineEdit, QPushButton, QComboBox, QSpinBox, QMessageBox
)
from PyQt6.QtGui import QFont, QPalette, QColor
from PyQt6.QtCore import Qt
import serial
import serial.tools.list_ports

PRODUCTOS = [
    ("Hamburguesas", "HBRGR", 50),
    ("Pan de alga", "SWBRD", 50),
    ("Salsa camaron", "SHRMP", 15),
    ("Pepinillos", "PICKL", 20),
    ("Tomate", "TMATO", 20),
    ("Lechuga", "LETCE", 20),
]

class CalamardoGUI(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Calamardo - Envío de Insumos (ESP-NOW)")
        self.setFixedSize(420, 260)
        self.init_ui()
        self.serial_port = None

    def init_ui(self):
        # Dark mode palette
        palette = QPalette()
        palette.setColor(QPalette.ColorRole.Window, QColor(30, 30, 30))
        palette.setColor(QPalette.ColorRole.WindowText, Qt.GlobalColor.white)
        palette.setColor(QPalette.ColorRole.Base, QColor(40, 40, 40))
        palette.setColor(QPalette.ColorRole.Text, Qt.GlobalColor.white)
        palette.setColor(QPalette.ColorRole.Button, QColor(50, 50, 50))
        palette.setColor(QPalette.ColorRole.ButtonText, Qt.GlobalColor.white)
        palette.setColor(QPalette.ColorRole.Highlight, QColor(0, 120, 215))
        self.setPalette(palette)

        font = QFont("Segoe UI", 11)
        self.setFont(font)

        layout = QVBoxLayout()
        layout.setSpacing(18)

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
        self.send_btn = QPushButton("Enviar")
        self.send_btn.clicked.connect(self.enviar_comando)
        layout.addWidget(self.send_btn)

        # Estado
        self.status_label = QLabel("")
        self.status_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        layout.addWidget(self.status_label)

        self.setLayout(layout)
        self.prod_combo.currentIndexChanged.connect(self.update_cantidad)

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
            self.status_label.setText(f"✅ Enviado: {codigo} {cantidad}")
        except Exception as e:
            self.status_label.setText(f"❌ Error: {e}")

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = CalamardoGUI()
    window.show()
    sys.exit(app.exec())
