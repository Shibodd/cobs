import logger_def
import serial
from cobs import cobs
import crc
import time
import struct

def zcp_read(ser: serial.Serial, crc_calculator: crc.Calculator):
  while True:
    # Assume data is always shorter than 256 bytes
    received = ser.read_until(b'\0', 256)

    if len(received) <= 0:
      # Time out to allow CTRL+C termination.
      continue

    if received[-1] != 0:
      logging.warning(f"Skipped {len(received)} bytes! Missing terminator or frame longer than 256 bytes?")
      continue
    
    try:
      decoded = cobs.decode(received[:-1])
    except cobs.DecodeError as de:
      logging.warning(f"Skipped a frame due to failed COBS decode! {de}")
      continue

    crc_value = (decoded[-1] << 8) + decoded[-2]
    data = decoded[:-2]

    if not crc_calculator.verify(data, crc_value):
      logging.warning(f"Skipped a frame due to CRC mismatch!")
      continue

    return data

def parse_frame(log_def: logger_def.LoggerDef, frame: bytes):
  if len(frame) < 3:
    logging.warning(f"Skipped frame too short to be a logger frame")
    return
  
  header, data = frame[:3], frame[3:]
  
  seq_id, msg_id = struct.unpack('<BH', header)

  msg_def = log_def.messages.get(msg_id, None)
  if msg_def is None:
    logging.warning(f"Skipped message {seq_id} due to unknown id ({msg_id})")
    return

  logging.info(f"Handling message {seq_id} with id {msg_id}")




if __name__ == '__main__':
  import argparse

  argparser = argparse.ArgumentParser()
  argparser.add_argument('def_file')
  argparser.add_argument('serial_port')
  argparser.add_argument('baudrate')
  args = argparser.parse_args()

  import logging
  logging.basicConfig(format='%(asctime)s [%(levelname)s] %(funcName)s: %(message)s', level=logging.DEBUG)

  logging.info("Initializing dependencies...")

  # Parse the logger definition
  try:
    log_def = logger_def.parse_from_file(args.def_file)
  except Exception as e:
    logging.fatal("Failed to parse the logger definition file!", exc_info = True)
    exit(1)

  # Setup the CRC calculator
  crc_calculator = crc.Calculator(crc.Configuration(
    width = 16,
    polynomial = 0x1021,
    init_value = 0xFFFF,
    final_xor_value = 0,
    reverse_input = False,
    reverse_output = False
  ))

  # Open the serial port
  try:
    ser = serial.Serial(args.serial_port, baudrate = args.baudrate, timeout = 5)
  except serial.SerialException as e:
    logging.fatal(f"Failed to open serial port! {e}")
    exit(1)

  with ser:
    logging.info("Starting.")

    # Run the logger indefinitely.
    while True:
      try:
        frame = zcp_read(ser, crc_calculator)
        parse_frame(log_def, frame)
      
      except KeyboardInterrupt:
        logging.info("Exiting due to CTRL+C.")
        exit(0)

      except Exception:
        logging.error("Uncaught exception!", exc_info = True)
        time.sleep(1)