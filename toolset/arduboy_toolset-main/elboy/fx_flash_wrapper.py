# fx_flash_wrapper.py
import time
import logging
from elboy.bin_to_hex import binary_to_hex

# Константы протокола из elbear_uploader
TIMEOUT_DEFAULT = 0.1
COMMAND_PACKAGE_SIZE = b'\x30'
COMMAND_SEND_PACKAGE = b'\x60'
# COMMAND_FULL_ERASE = 0xBADC0FEE  # НЕ ИСПОЛЬЗУЕМ — стирает всю флешку!
ACK = 0x0F
NACK = 0xF0

# Адрес SPIFI в RISC-V приставке
SPIFI_BASE_ADDRESS = 0x80000000
START_IMAGE_OFFSET = 0x00020000   # Байтовое смещение (128 КБ) — НЕ номер страницы!
FX_PAGESIZE = 256


def _send_parsel(ser, data, max_attempts=10):
    """Отправка данных с проверкой ACK/NACK"""
    for attempt in range(max_attempts):
        ser.write(data)
        read_byte = ser.read(1)
        if not read_byte:
            continue
        response = int.from_bytes(read_byte, "big")
        if response == ACK:
            return True
        elif response == NACK:
            raise Exception(f"Device returned NACK on attempt {attempt + 1}")
    raise Exception("Device not responding")


def _cmd_package_size(ser, package_size):
    """Задать размер пакета"""
    _send_parsel(ser, COMMAND_PACKAGE_SIZE)
    _send_parsel(ser, (package_size - 1).to_bytes(1, "big"))


def _cmd_send_package(ser, data_package):
    """Отправить пакет данных"""
    _send_parsel(ser, COMMAND_SEND_PACKAGE)
    _send_parsel(ser, bytes(data_package))


def flash_fx(flashdata: bytearray, pagenumber: int, s_port, verify=True, report_progress=None):
    """
    Загрузка FX-картриджа в RISC-V приставку через SPIFI.
    Записывает данные начиная с адреса 0x80020000, НЕ стирая загрузчик по 0x80000000.

    :param flashdata: Бинарные данные FX-картриджа
    :param pagenumber: Начальная страница (относительно START_IMAGE_OFFSET)
    :param s_port: Открытый COM-порт (serial.Serial)
    :param verify: Игнорируется (в elbear верификация не реализована)
    :param report_progress: Функция отчёта о прогрессе (current, total)
    """
    if not len(flashdata):
        raise Exception("No flash data provided!")

    logging.info(f"Flashing {len(flashdata)} bytes to RISC-V device")

    # Переоткрываем порт на правильной скорости (230400)
    port_name = s_port.port
    s_port.close()
    time.sleep(0.1)

    import serial as ser_mod
    s_port = ser_mod.Serial(port_name, 230400, timeout=0.5)
    time.sleep(0.1)

    # Очищаем буферы
    s_port.reset_input_buffer()
    s_port.reset_output_buffer()

    # Пингуем устройство
    for attempt in range(5):
        try:
            _cmd_package_size(s_port, 15)
            logging.info("Device connected")
            break
        except Exception as e:
            logging.warning(f"Attempt {attempt + 1}/5 failed: {e}")
            time.sleep(0.3)
            s_port.reset_input_buffer()
            s_port.reset_output_buffer()
    else:
        s_port.close()
        raise Exception("Device not responding after 5 attempts.")

    # Вычисляем стартовый адрес в SPIFI
    if pagenumber < 0:
        start_address = SPIFI_BASE_ADDRESS + START_IMAGE_OFFSET
    else:
        start_address = SPIFI_BASE_ADDRESS + START_IMAGE_OFFSET + pagenumber * FX_PAGESIZE

    logging.info(f"Start address: 0x{start_address:08X}")

    # === ВАЖНО: ПОЛНОЕ СТИРАНИЕ УБРАНО! ===
    # Команда 0xBADC0FEE стирает ВЕСЬ чип, включая загрузчик по 0x80000000.
    # Загрузчик elbear сам стирает нужные сектора при получении данных.

    # Конвертируем бинарные данные в Intel HEX
    hex_text = binary_to_hex(flashdata, start_address)

    # Парсим HEX в строки данных
    data_lines = []
    for line in hex_text.split('\n'):
        line = line.strip()
        if not line.startswith(':'):
            continue
        line = line[1:]  # Убираем ':'
        if len(line) < 8:
            continue
        data = [int(line[i:i+2], 16) for i in range(0, len(line), 2)]
        data_lines.append(data)

    logging.info(f"Parsed {len(data_lines)} HEX records")

    # Загружаем строки HEX
    start_time = time.time()
    total = len(data_lines)

    for idx, line in enumerate(data_lines):
        _cmd_package_size(s_port, len(line))
        _cmd_send_package(s_port, bytes(line))
        if report_progress:
            report_progress(idx + 1, total)

    elapsed = time.time() - start_time
    logging.info(f"Flashed {total} records in {elapsed:.2f} seconds")

    s_port.close()
    return True