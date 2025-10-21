"""
Monitor de Semáforos Inteligentes - PyQt6
Universidad Militar Nueva Granada

Aplicación de monitoreo en tiempo real para dos ESP32 con semáforos inteligentes.
Muestra estado de LEDs, distancias del sensor HC-SR04, logs serial y mensajes ESP-NOW.

Requisitos:
- PyQt6
- pyserial

Uso:
python monitor_semaforos.py
"""

import sys
import re
from datetime import datetime
from PyQt6.QtWidgets import (
    QApplication, QMainWindow, QWidget, QVBoxLayout, QHBoxLayout,
    QLabel, QPushButton, QTextEdit, QComboBox, QGroupBox, QGridLayout,
    QProgressBar, QTabWidget, QSplitter, QFrame
)
from PyQt6.QtCore import QThread, pyqtSignal, Qt, QTimer
from PyQt6.QtGui import QFont, QColor, QPalette, QTextCursor
import serial
import serial.tools.list_ports


class SerialReader(QThread):
    """Thread para leer datos del puerto serial sin bloquear la UI"""
    data_received = pyqtSignal(str, str)  # (port_id, data)
    connection_status = pyqtSignal(str, str)  # (port_id, status_message)
    
    def __init__(self, port, baudrate=115200, port_id="A"):
        super().__init__()
        self.port = port
        self.baudrate = baudrate
        self.port_id = port_id
        self.running = False
        self.serial_conn = None
    
    def run(self):
        try:
            # Intentar abrir puerto serial
            self.connection_status.emit(self.port_id, f"Conectando a {self.port}...")
            self.serial_conn = serial.Serial(
                port=self.port,
                baudrate=self.baudrate,
                timeout=1,
                write_timeout=1
            )
            
            # Limpiar buffer
            self.serial_conn.reset_input_buffer()
            self.serial_conn.reset_output_buffer()
            
            self.running = True
            self.connection_status.emit(self.port_id, f"✅ Conectado a {self.port} @ {self.baudrate} baud")
            self.data_received.emit(self.port_id, f"=== CONECTADO a {self.port} ({self.baudrate} baud) ===")
            
            buffer = ""
            
            while self.running:
                try:
                    if self.serial_conn.in_waiting > 0:
                        # Leer bytes disponibles
                        chunk = self.serial_conn.read(self.serial_conn.in_waiting)
                        decoded = chunk.decode('utf-8', errors='ignore')
                        buffer += decoded
                        
                        # Procesar líneas completas
                        while '\n' in buffer:
                            line, buffer = buffer.split('\n', 1)
                            line = line.strip()
                            if line:
                                self.data_received.emit(self.port_id, line)
                    else:
                        # Pequeña pausa si no hay datos
                        self.msleep(10)
                        
                except UnicodeDecodeError as e:
                    self.data_received.emit(self.port_id, f"ERROR decodificación: {e}")
                except serial.SerialException as e:
                    self.data_received.emit(self.port_id, f"ERROR serial: {e}")
                    self.running = False
                except Exception as e:
                    self.data_received.emit(self.port_id, f"ERROR inesperado: {e}")
                    
        except serial.SerialException as e:
            self.connection_status.emit(self.port_id, f"❌ Error: {e}")
            self.data_received.emit(self.port_id, f"ERROR: No se pudo abrir {self.port} - {e}")
        except Exception as e:
            self.connection_status.emit(self.port_id, f"❌ Error inesperado: {e}")
            self.data_received.emit(self.port_id, f"ERROR FATAL: {e}")
    
    def stop(self):
        self.running = False
        if self.serial_conn and self.serial_conn.is_open:
            try:
                self.serial_conn.close()
                self.connection_status.emit(self.port_id, f"Desconectado de {self.port}")
            except Exception as e:
                print(f"Error cerrando puerto: {e}")


class LEDIndicator(QWidget):
    """Widget personalizado para simular un LED con color"""
    def __init__(self, color_name, label=""):
        super().__init__()
        self.color_name = color_name
        self.is_on = False
        self.init_ui(label)
    
    def init_ui(self, label):
        layout = QVBoxLayout()
        
        # Círculo LED
        self.led_label = QLabel()
        self.led_label.setFixedSize(50, 50)
        self.led_label.setStyleSheet(self._get_stylesheet())
        self.led_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        
        # Etiqueta
        text_label = QLabel(label)
        text_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        font = QFont()
        font.setPointSize(9)
        font.setBold(True)
        text_label.setFont(font)
        
        layout.addWidget(self.led_label)
        layout.addWidget(text_label)
        layout.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.setLayout(layout)
    
    def _get_stylesheet(self):
        colors = {
            'red': ('#ff4444', '#330000'),
            'yellow': ('#ffff00', '#333300'),
            'green': ('#00ff00', '#003300')
        }
        
        on_color, off_color = colors.get(self.color_name, ('#888888', '#222222'))
        current_color = on_color if self.is_on else off_color
        
        return f"""
            QLabel {{
                background-color: {current_color};
                border: 3px solid #444;
                border-radius: 25px;
            }}
        """
    
    def set_state(self, is_on):
        self.is_on = is_on
        self.led_label.setStyleSheet(self._get_stylesheet())


class SemaforoPanel(QWidget):
    """Panel para un semáforo individual"""
    def __init__(self, semaforo_id="A"):
        super().__init__()
        self.semaforo_id = semaforo_id
        self.init_ui()
        # Error tracking for TX
        self.tx_error_count = 0
        self.tx_error_threshold = 3  # Show error only if 3 or more consecutive
    def reset_tx_error(self):
        self.tx_error_count = 0
        # Only clear error if currently showing error
        if self.sync_label.text().startswith("⚠️ ERROR TX"):
            self.sync_label.setText("✅ OK")
            self.sync_label.setStyleSheet("color: green; font-weight: bold;")

    def register_tx_error(self):
        self.tx_error_count += 1
        if self.tx_error_count >= self.tx_error_threshold:
            self.sync_label.setText("⚠️ ERROR TX (status=1)")
            self.sync_label.setStyleSheet("color: orange; font-weight: bold; background: #330000; border: 2px solid red;")
    
    def init_ui(self):
        main_layout = QVBoxLayout()
        
        # Título
        title = QLabel(f"SEMÁFORO {self.semaforo_id}")
        title.setAlignment(Qt.AlignmentFlag.AlignCenter)
        font = QFont()
        font.setPointSize(16)
        font.setBold(True)
        title.setFont(font)
        
        # LEDs
        led_group = QGroupBox("Estado de LEDs")
        led_layout = QHBoxLayout()
        
        self.led_rojo = LEDIndicator('red', 'ROJO')
        self.led_amarillo = LEDIndicator('yellow', 'AMARILLO')
        self.led_verde = LEDIndicator('green', 'VERDE')
        
        led_layout.addWidget(self.led_rojo)
        led_layout.addWidget(self.led_amarillo)
        led_layout.addWidget(self.led_verde)
        led_group.setLayout(led_layout)
        
        # Información de estado
        info_group = QGroupBox("Información del Sistema")
        info_layout = QGridLayout()
        
        # Estado actual
        info_layout.addWidget(QLabel("Estado:"), 0, 0)
        self.estado_label = QLabel("---")
        self.estado_label.setStyleSheet("font-weight: bold; font-size: 14px;")
        info_layout.addWidget(self.estado_label, 0, 1)
        
        # Tiempo restante
        info_layout.addWidget(QLabel("Tiempo restante:"), 1, 0)
        self.tiempo_label = QLabel("--- s")
        info_layout.addWidget(self.tiempo_label, 1, 1)
        
        # Distancia sensor (solo local)
        info_layout.addWidget(QLabel("Distancia (local):"), 2, 0)
        self.distancia_label = QLabel("--- cm")
        info_layout.addWidget(self.distancia_label, 2, 1)
        
        # Vehículo detectado
        info_layout.addWidget(QLabel("Vehículo:"), 3, 0)
        self.vehiculo_label = QLabel("NO")
        info_layout.addWidget(self.vehiculo_label, 3, 1)
        
        # Prioridad
        info_layout.addWidget(QLabel("Prioridad:"), 4, 0)
        self.prioridad_label = QLabel("---")
        info_layout.addWidget(self.prioridad_label, 4, 1)
        
        # Estado remoto
        info_layout.addWidget(QLabel("Vía remota:"), 5, 0)
        self.remoto_label = QLabel("---")
        info_layout.addWidget(self.remoto_label, 5, 1)
        
        # Sincronización
        info_layout.addWidget(QLabel("Sincronización:"), 6, 0)
        self.sync_label = QLabel("---")
        info_layout.addWidget(self.sync_label, 6, 1)
        
        info_group.setLayout(info_layout)
        
        # Barra de distancia visual
        dist_group = QGroupBox("Sensor Ultrasónico")
        dist_layout = QVBoxLayout()
        self.distancia_bar = QProgressBar()
        self.distancia_bar.setMaximum(400)  # 0-400 cm
        self.distancia_bar.setValue(400)
        self.distancia_bar.setTextVisible(True)
        self.distancia_bar.setFormat("%v cm")
        dist_layout.addWidget(self.distancia_bar)
        dist_group.setLayout(dist_layout)
        
        # Estadísticas ESP-NOW
        espnow_group = QGroupBox("ESP-NOW")
        espnow_layout = QGridLayout()
        
        espnow_layout.addWidget(QLabel("Mensajes TX:"), 0, 0)
        self.tx_count_label = QLabel("0")
        espnow_layout.addWidget(self.tx_count_label, 0, 1)
        
        espnow_layout.addWidget(QLabel("Mensajes RX:"), 1, 0)
        self.rx_count_label = QLabel("0")
        espnow_layout.addWidget(self.rx_count_label, 1, 1)
        
        espnow_layout.addWidget(QLabel("Último TX:"), 2, 0)
        self.last_tx_label = QLabel("---")
        espnow_layout.addWidget(self.last_tx_label, 2, 1)
        
        espnow_group.setLayout(espnow_layout)
        
        # Ensamblar layout
        main_layout.addWidget(title)
        main_layout.addWidget(led_group)
        main_layout.addWidget(info_group)
        main_layout.addWidget(dist_group)
        main_layout.addWidget(espnow_group)
        main_layout.addStretch()
        
        self.setLayout(main_layout)
        
        # Contadores
        self.tx_count = 0
        self.rx_count = 0
    
    def update_estado(self, estado):
        """Actualizar estado del semáforo y LEDs"""
        self.estado_label.setText(estado)
        # Actualizar LEDs
        self.led_rojo.set_state(estado in ['ROJO', 'ALL RED'])
        self.led_amarillo.set_state(estado == 'AMARILLO')
        self.led_verde.set_state(estado == 'VERDE')
        # Color del texto según estado
        colors = {
            'VERDE': 'green',
            'AMARILLO': 'orange',
            'ROJO': 'red',
            'ALL RED': 'darkred'
        }
        color = colors.get(estado, 'black')
        self.estado_label.setStyleSheet(f"font-weight: bold; font-size: 14px; color: {color};")

    def update_prioridad(self, local_request, remote_request):
        """Actualizar visualización de prioridad local/remota"""
        if local_request and remote_request:
            self.prioridad_label.setText("Conflicto (ambos)")
            self.prioridad_label.setStyleSheet("font-weight: bold; color: orange;")
        elif local_request:
            self.prioridad_label.setText("Solicitada (local)")
            self.prioridad_label.setStyleSheet("font-weight: bold; color: blue;")
        elif remote_request:
            self.prioridad_label.setText("Remota")
            self.prioridad_label.setStyleSheet("font-weight: bold; color: purple;")
        else:
            self.prioridad_label.setText("---")
            self.prioridad_label.setStyleSheet("")

    def update_estado_remoto(self, estado_remoto):
        """Actualizar visualización de estado remoto"""
        self.remoto_label.setText(estado_remoto)
        colors = {
            'VERDE': 'green',
            'AMARILLO': 'orange',
            'ROJO': 'red',
            'ALL RED': 'darkred',
            '---': 'gray'
        }
        color = colors.get(estado_remoto, 'black')
        self.remoto_label.setStyleSheet(f"font-weight: bold; color: {color};")
    
    def update_distancia(self, distancia_cm):
        """Actualizar distancia del sensor"""
        try:
            dist = int(distancia_cm)
            self.distancia_label.setText(f"{dist} cm")
            
            # Actualizar barra (invertida: menor distancia = más lleno)
            self.distancia_bar.setValue(400 - min(dist, 400))
            
            # Color según proximidad
            if dist < 50:
                self.distancia_bar.setStyleSheet("QProgressBar::chunk { background-color: red; }")
            elif dist < 100:
                self.distancia_bar.setStyleSheet("QProgressBar::chunk { background-color: orange; }")
            else:
                self.distancia_bar.setStyleSheet("QProgressBar::chunk { background-color: green; }")
        except ValueError:
            pass
    
    def update_vehiculo(self, detectado):
        """Actualizar detección de vehículo"""
        self.vehiculo_label.setText("SÍ" if detectado else "NO")
        color = "red" if detectado else "gray"
        self.vehiculo_label.setStyleSheet(f"font-weight: bold; color: {color};")
    
    def increment_tx(self):
        self.tx_count += 1
        self.tx_count_label.setText(str(self.tx_count))
    
    def increment_rx(self):
        self.rx_count += 1
        self.rx_count_label.setText(str(self.rx_count))


class MainWindow(QMainWindow):
    """Ventana principal de la aplicación"""
    def __init__(self):
        super().__init__()
        self.serial_readers = {}
        self.init_ui()
    
    def init_ui(self):
        self.setWindowTitle("Monitor de Semáforos Inteligentes - ESP32 ESP-NOW")
        self.setGeometry(100, 100, 1400, 900)
        
        # Widget central
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        
        main_layout = QVBoxLayout()
        
        # Barra de control superior
        control_layout = QHBoxLayout()
        
        # Selector puerto A
        control_layout.addWidget(QLabel("Puerto COM A:"))
        self.port_combo_a = QComboBox()
        control_layout.addWidget(self.port_combo_a)
        
        # Selector puerto B
        control_layout.addWidget(QLabel("Puerto COM B:"))
        self.port_combo_b = QComboBox()
        control_layout.addWidget(self.port_combo_b)
        
        # Llenar los combos DESPUÉS de crearlos
        self.refresh_ports()
        
        # Botones con mejor diseño UI/UX
        self.refresh_btn = QPushButton("🔄 Actualizar Puertos")
        self.refresh_btn.clicked.connect(self.refresh_ports)
        self.refresh_btn.setStyleSheet("""
            QPushButton {
                background-color: #2196F3;
                color: white;
                font-weight: bold;
                padding: 8px 16px;
                border-radius: 6px;
                border: none;
                font-size: 12px;
            }
            QPushButton:hover {
                background-color: #1976D2;
            }
            QPushButton:pressed {
                background-color: #0D47A1;
            }
        """)
        control_layout.addWidget(self.refresh_btn)
        
        self.connect_btn = QPushButton("▶ Conectar")
        self.connect_btn.clicked.connect(self.toggle_connection)
        self.connect_btn.setStyleSheet("""
            QPushButton {
                background-color: #4CAF50;
                color: white;
                font-weight: bold;
                padding: 10px 24px;
                border-radius: 6px;
                border: none;
                font-size: 14px;
            }
            QPushButton:hover {
                background-color: #45a049;
            }
            QPushButton:pressed {
                background-color: #388E3C;
            }
        """)
        control_layout.addWidget(self.connect_btn)
        
        self.clear_btn = QPushButton("🗑 Limpiar Logs")
        self.clear_btn.clicked.connect(self.clear_logs)
        self.clear_btn.setStyleSheet("""
            QPushButton {
                background-color: #FF9800;
                color: white;
                font-weight: bold;
                padding: 8px 16px;
                border-radius: 6px;
                border: none;
                font-size: 12px;
            }
            QPushButton:hover {
                background-color: #F57C00;
            }
            QPushButton:pressed {
                background-color: #E65100;
            }
        """)
        control_layout.addWidget(self.clear_btn)
        
        control_layout.addStretch()
        
        # Splitter principal (paneles de semáforos + logs)
        main_splitter = QSplitter(Qt.Orientation.Horizontal)
        
        # Panel izquierdo: Semáforos
        semaforos_widget = QWidget()
        semaforos_layout = QHBoxLayout()
        
        self.panel_a = SemaforoPanel("A")
        self.panel_b = SemaforoPanel("B")
        
        semaforos_layout.addWidget(self.panel_a)
        semaforos_layout.addWidget(self.panel_b)
        semaforos_widget.setLayout(semaforos_layout)
        
        # Panel derecho: Logs
        logs_widget = QWidget()
        logs_layout = QVBoxLayout()
        
        # Tabs para logs separados
        self.log_tabs = QTabWidget()
        
        self.log_a = QTextEdit()
        self.log_a.setReadOnly(True)
        self.log_a.setStyleSheet("background-color: #1e1e1e; color: #00ff00; font-family: 'Courier New';")
        
        self.log_b = QTextEdit()
        self.log_b.setReadOnly(True)
        self.log_b.setStyleSheet("background-color: #1e1e1e; color: #00ff00; font-family: 'Courier New';")
        
        self.log_combined = QTextEdit()
        self.log_combined.setReadOnly(True)
        self.log_combined.setStyleSheet("background-color: #1e1e1e; color: #00ff00; font-family: 'Courier New';")
        
        self.log_tabs.addTab(self.log_a, "Log Semáforo A")
        self.log_tabs.addTab(self.log_b, "Log Semáforo B")
        self.log_tabs.addTab(self.log_combined, "Log Combinado")
        
        logs_layout.addWidget(QLabel("Logs en Tiempo Real"))
        logs_layout.addWidget(self.log_tabs)
        logs_widget.setLayout(logs_layout)
        
        main_splitter.addWidget(semaforos_widget)
        main_splitter.addWidget(logs_widget)
        main_splitter.setSizes([700, 700])
        
        # Ensamblar layout principal
        main_layout.addLayout(control_layout)
        main_layout.addWidget(main_splitter)
        
        central_widget.setLayout(main_layout)
        
        # Estado de conexión
        self.connected = False
    
    def refresh_ports(self):
        """Actualizar lista de puertos COM disponibles"""
        ports = [port.device for port in serial.tools.list_ports.comports()]
        
        self.port_combo_a.clear()
        self.port_combo_b.clear()
        
        if ports:
            self.port_combo_a.addItems(ports)
            self.port_combo_b.addItems(ports)
            if len(ports) > 1:
                self.port_combo_b.setCurrentIndex(1)
        else:
            self.port_combo_a.addItem("No hay puertos disponibles")
            self.port_combo_b.addItem("No hay puertos disponibles")
    
    def toggle_connection(self):
        """Conectar o desconectar los puertos seriales"""
        if not self.connected:
            # Conectar
            port_a = self.port_combo_a.currentText()
            port_b = self.port_combo_b.currentText()
            
            if not port_a or not port_b or "No hay puertos" in port_a:
                self.append_log("combined", "ERROR: Selecciona puertos COM válidos")
                return
            
            if port_a == port_b:
                self.append_log("combined", "ERROR: No puedes usar el mismo puerto para A y B")
                return
            
            try:
                self.append_log("combined", f"=== Iniciando conexión... ===")
                
                # Iniciar thread para puerto A
                self.serial_readers['A'] = SerialReader(port_a, 115200, "A")
                self.serial_readers['A'].data_received.connect(self.on_data_received)
                self.serial_readers['A'].connection_status.connect(self.on_connection_status)
                self.serial_readers['A'].start()
                
                # Iniciar thread para puerto B
                self.serial_readers['B'] = SerialReader(port_b, 115200, "B")
                self.serial_readers['B'].data_received.connect(self.on_data_received)
                self.serial_readers['B'].connection_status.connect(self.on_connection_status)
                self.serial_readers['B'].start()
                
                self.connected = True
                self.connect_btn.setText("⏹ Desconectar")
                self.connect_btn.setStyleSheet("""
                    QPushButton {
                        background-color: #f44336;
                        color: white;
                        font-weight: bold;
                        padding: 10px 24px;
                        border-radius: 6px;
                        border: none;
                        font-size: 14px;
                    }
                    QPushButton:hover {
                        background-color: #da190b;
                    }
                    QPushButton:pressed {
                        background-color: #a30000;
                    }
                """)
                
                # Deshabilitar selectores mientras está conectado
                self.port_combo_a.setEnabled(False)
                self.port_combo_b.setEnabled(False)
                
            except Exception as e:
                self.append_log("combined", f"ERROR al iniciar conexión: {e}")
        else:
            # Desconectar
            self.append_log("combined", "=== Desconectando... ===")
            
            for reader in self.serial_readers.values():
                reader.stop()
                reader.wait(2000)  # Esperar máximo 2 segundos
            
            self.serial_readers.clear()
            self.connected = False
            self.connect_btn.setText("▶ Conectar")
            self.connect_btn.setStyleSheet("""
                QPushButton {
                    background-color: #4CAF50;
                    color: white;
                    font-weight: bold;
                    padding: 10px 24px;
                    border-radius: 6px;
                    border: none;
                    font-size: 14px;
                }
                QPushButton:hover {
                    background-color: #45a049;
                }
                QPushButton:pressed {
                    background-color: #388E3C;
                }
            """)
            
            # Rehabilitar selectores
            self.port_combo_a.setEnabled(True)
            self.port_combo_b.setEnabled(True)
            
            self.append_log("combined", "=== DESCONECTADO ===")
    
    def on_connection_status(self, port_id, message):
        """Manejar mensajes de estado de conexión"""
        self.append_log("combined", f"[{port_id}] {message}")
    
    def on_data_received(self, port_id, line):
        """Procesar datos recibidos del serial"""
        timestamp = datetime.now().strftime("%H:%M:%S.%f")[:-3]
        log_line = f"[{timestamp}] [{port_id}] {line}"
        
        # Agregar a logs
        if port_id == "A":
            self.append_log("A", log_line)
        else:
            self.append_log("B", log_line)
        
        self.append_log("combined", log_line)
        
        # Parsear y actualizar interfaz
        self.parse_serial_line(port_id, line)
    
    def parse_serial_line(self, port_id, line):
        """Parsear línea del serial y actualizar UI"""
        panel = self.panel_a if port_id == "A" else self.panel_b
        updated_dist = False

        # Formato CSV: seq,ts,state,dist,veh,auto
        csv_match = re.match(r'^\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*,\s*([01])\s*,\s*([01])\s*$', line.strip())
        if csv_match:
            seq, ts, state, dist, veh, auto = csv_match.groups()
            state_map = {
                '0': 'ALL RED',
                '1': 'VERDE',
                '2': 'AMARILLO',
                '3': 'ROJO'
            }
            if state in state_map:
                panel.update_estado(state_map[state])
            panel.update_distancia(dist)
            updated_dist = True
            panel.update_vehiculo(veh == '1')
            panel.update_prioridad(auto == '1', False)
            panel.update_estado_remoto('---')
            panel.reset_tx_error()
            return

        # Estado del semáforo (formato texto)
        if "-> VERDE" in line:
            panel.update_estado("VERDE")
            panel.reset_tx_error()
        elif "-> AMARILLO" in line:
            panel.update_estado("AMARILLO")
            panel.reset_tx_error()
        elif "-> ROJO" in line:
            panel.update_estado("ROJO")
            panel.reset_tx_error()
        elif "-> ALL_RED" in line:
            panel.update_estado("ALL RED")
            panel.reset_tx_error()

        # Mensajes TX ESP-NOW: solo actualiza distancia local
        tx_match = re.search(r'TX: estado=(\d+), request=(\d+), dist=(\d+)', line)
        if tx_match:
            panel.increment_tx()
            estado, request, dist = tx_match.groups()
            panel.last_tx_label.setText(f"estado={estado}, req={request}")
            # Actualizar distancia local (TX siempre es del propio ESP)
            panel.update_distancia(dist)
            updated_dist = True
            panel.update_prioridad(request == '1', False)
            state_map = {'0': 'ALL RED', '1': 'VERDE', '2': 'AMARILLO', '3': 'ROJO'}
            panel.update_estado(state_map.get(estado, '---'))
            panel.reset_tx_error()

        # Mensajes RX ESP-NOW (NO actualizar distancia local)
        rx_match = re.search(r'RX de ESP (\d+): estado=(\d+), request=(\d+), dist=(\d+)', line)
        if rx_match:
            panel.increment_rx()
            sender, estado, request, dist = rx_match.groups()
            panel.update_prioridad(False, request == '1')
            state_map = {'0': 'ALL RED', '1': 'VERDE', '2': 'AMARILLO', '3': 'ROJO'}
            panel.update_estado_remoto(state_map.get(estado, '---'))
            # NO actualizar distancia aquí (es distancia remota)
            panel.reset_tx_error()
            updated_dist = True  # Marcar para no procesar en el bloque genérico

        # Detección de vehículo (inferir de distancia solo si no vino en CSV ni TX ni RX)
        dist_match = re.search(r'dist[ancia]*[=:]\s*(\d+)', line, re.IGNORECASE)
        if dist_match and not updated_dist:
            dist = int(dist_match.group(1))
            panel.update_distancia(dist)
            if not csv_match:
                panel.update_vehiculo(dist < 100)

        # Sincronización y ESP-NOW
        if "SIN SYNC" in line or "Sin comunicación" in line:
            panel.sync_label.setText("❌ SIN SYNC")
            panel.sync_label.setStyleSheet("color: red; font-weight: bold;")
        elif "Peer añadido correctamente" in line:
            panel.sync_label.setText("✅ PEER OK")
            panel.sync_label.setStyleSheet("color: green; font-weight: bold;")
        elif "ESP-NOW inicializado OK" in line:
            panel.sync_label.setText("✅ INIT OK")
            panel.sync_label.setStyleSheet("color: yellow; font-weight: bold;")
        elif "Error TX ESP-NOW" in line or "Callback: Error" in line or "status=1" in line:
            panel.register_tx_error()
        elif "Peer MAC:" in line:
            mac_match = re.search(r'([0-9A-F]{2}:[0-9A-F]{2}:[0-9A-F]{2}:[0-9A-F]{2}:[0-9A-F]{2}:[0-9A-F]{2})', line)
            if mac_match:
                panel.sync_label.setText(f"✅ {mac_match.group(1)}")
                panel.sync_label.setStyleSheet("color: green; font-weight: bold;")
    
    def append_log(self, log_type, text):
        """Agregar línea al log correspondiente"""
        if log_type == "A":
            self.log_a.append(text)
            self.log_a.moveCursor(QTextCursor.MoveOperation.End)
        elif log_type == "B":
            self.log_b.append(text)
            self.log_b.moveCursor(QTextCursor.MoveOperation.End)
        elif log_type == "combined":
            self.log_combined.append(text)
            self.log_combined.moveCursor(QTextCursor.MoveOperation.End)
    
    def clear_logs(self):
        """Limpiar todos los logs"""
        self.log_a.clear()
        self.log_b.clear()
        self.log_combined.clear()
    
    def closeEvent(self, event):
        """Cerrar conexiones al salir"""
        if self.connected:
            for reader in self.serial_readers.values():
                reader.stop()
                reader.wait()
        event.accept()


def main():
    app = QApplication(sys.argv)
    
    # Estilo moderno
    app.setStyle('Fusion')
    
    # Paleta oscura
    palette = QPalette()
    palette.setColor(QPalette.ColorRole.Window, QColor(53, 53, 53))
    palette.setColor(QPalette.ColorRole.WindowText, Qt.GlobalColor.white)
    palette.setColor(QPalette.ColorRole.Base, QColor(25, 25, 25))
    palette.setColor(QPalette.ColorRole.AlternateBase, QColor(53, 53, 53))
    palette.setColor(QPalette.ColorRole.ToolTipBase, Qt.GlobalColor.white)
    palette.setColor(QPalette.ColorRole.ToolTipText, Qt.GlobalColor.white)
    palette.setColor(QPalette.ColorRole.Text, Qt.GlobalColor.white)
    palette.setColor(QPalette.ColorRole.Button, QColor(53, 53, 53))
    palette.setColor(QPalette.ColorRole.ButtonText, Qt.GlobalColor.white)
    palette.setColor(QPalette.ColorRole.BrightText, Qt.GlobalColor.red)
    palette.setColor(QPalette.ColorRole.Link, QColor(42, 130, 218))
    palette.setColor(QPalette.ColorRole.Highlight, QColor(42, 130, 218))
    palette.setColor(QPalette.ColorRole.HighlightedText, Qt.GlobalColor.black)
    
    app.setPalette(palette)
    
    window = MainWindow()
    window.show()
    
    sys.exit(app.exec())


if __name__ == '__main__':
    main()
