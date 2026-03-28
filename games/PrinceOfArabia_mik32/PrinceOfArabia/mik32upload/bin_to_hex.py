# bin-to-hex.py
import sys
import os
import argparse

VERSION = '1.1'

def print(s):
    sys.stdout.write(s + '\n')
    sys.stdout.flush()

def calculate_checksum(data):
    """
    Вычисляет контрольную сумму для строки Intel HEX.
    :param data: Список байт данных.
    :return: Контрольная сумма (один байт).
    """
    checksum = (256 - sum(data) % 256) % 256
    return checksum

def binary_to_hex(data, start_address=0x80000000):
    """
    Преобразует бинарные данные в формат Intel HEX с поддержкой расширенных адресов.
    :param data: Бинарные данные (bytearray).
    :param start_address: Начальный адрес загрузки данных.
    :return: Текст в формате Intel HEX.
    """
    hex_lines = []
    address = start_address
    current_extended_address = (address >> 16) & 0xFFFF  # Текущие старшие 16 бит адреса

    # Добавляем начальную запись расширенного адреса
    extended_address_record = [
        0x02,  # Количество байт данных (2 байта)
        0x00,  # Адрес всегда 0x0000 для записи типа 04
        0x00,
        0x04,  # Тип записи - расширенный линейный адрес
        (current_extended_address >> 8) & 0xFF,  # Старший байт старших 16 бит адреса
        current_extended_address & 0xFF         # Младший байт старших 16 бит адреса
    ]
    checksum = calculate_checksum(extended_address_record)
    hex_lines.append(":02000004{:04X}{:02X}".format(current_extended_address, checksum))

    for i in range(0, len(data), 16):  # Разбиваем данные по 16 байт
        chunk = data[i:i + 16]

        # Проверяем, нужно ли обновить расширенный адрес
        new_extended_address = (address >> 16) & 0xFFFF
        if new_extended_address != current_extended_address:
            current_extended_address = new_extended_address
            extended_address_record = [
                0x02,  # Количество байт данных (2 байта)
                0x00,  # Адрес всегда 0x0000 для записи типа 04
                0x00,
                0x04,  # Тип записи - расширенный линейный адрес
                (current_extended_address >> 8) & 0xFF,  # Старший байт старших 16 бит адреса
                current_extended_address & 0xFF         # Младший байт старших 16 бит адреса
            ]
            checksum = calculate_checksum(extended_address_record)
            hex_lines.append(":02000004{:04X}{:02X}".format(current_extended_address, checksum))

        # Формируем строку данных
        line = [
            len(chunk),                  # Количество байт данных
            (address >> 8) & 0xFF,       # Старший байт адреса (младшие 16 бит)
            address & 0xFF,              # Младший байт адреса (младшие 16 бит)
            0x00                          # Тип записи - данные
        ]
        line.extend(chunk)
        crc = calculate_checksum(line)
        line.append(crc)

        hex_line = ":" + "".join(f"{b:02X}" for b in line)
        hex_lines.append(hex_line)

        # Обновляем адрес
        address += len(chunk)

    # Добавляем запись конца файла
    hex_lines.append(":00000001FF\n")
    return "\n".join(hex_lines)

def main():
    print(f"Binary to HEX converter version {VERSION}")
    
    parser = argparse.ArgumentParser(
        description="Convert a binary file to Intel HEX format."
    )
    parser.add_argument(
        "binfile",
        help="Path to the input binary file (e.g., fxdata.bin)."
    )
    parser.add_argument(
        "start_address",
        type=lambda x: int(x, 16),
        help="Start address in hexadecimal format (e.g., 0x80000000)."
    )
    args = parser.parse_args()

    binfile = args.binfile
    start_address = args.start_address

    if not os.path.isfile(binfile):
        print(f"Error: File '{binfile}' not found.")
        sys.exit(-1)

    # Чтение бинарного файла
    with open(binfile, "rb") as file:
        binary_data = bytearray(file.read())

    # Преобразование в HEX
    hex_data = binary_to_hex(binary_data, start_address)

    # Сохранение HEX файла
    hexfile = os.path.splitext(binfile)[0] + ".hex"
    with open(hexfile, "w") as file:
        file.write(hex_data)

    print(f"Converted {len(binary_data)} bytes to HEX format.")
    print(f"Saved HEX data to {hexfile}")

if __name__ == "__main__":
    main()