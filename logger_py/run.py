import logger_def
import serial
from cobs import cobs
import crc
import time
import struct
import logging
from dataclasses import dataclass
import contextlib
import typing

def read_until(source, terminator: bytes, max_size: int):
  ans = bytearray()
  term_len = len(terminator)

  while len(ans) < max_size and ans[-term_len:] != terminator:
    c = source.read(1)
    if c:
      ans += c
    else:
      break

  return ans

def zcp_read(source, crc_calculator: crc.Calculator, dump_file: typing.BinaryIO = None):
  while True:
    # Assume data is always shorter than 256 bytes
    received = read_until(source, b'\0', 256)

    if dump_file is not None:
      dump_file.write(received)

    if received[-1] != 0:
      logging.warning(f"Skipped {len(received)} bytes! Missing terminator or frame longer than 256 bytes?")
      continue
    
    # Decode COBS
    try:
      decoded = cobs.decode(received[:-1])
    except cobs.DecodeError as de:
      logging.warning(f"Skipped a frame due to failed COBS decode! {de}")
      continue

    # Verify CRC
    crc_value = (decoded[-1] << 8) + decoded[-2]
    data = decoded[:-2]

    if not crc_calculator.verify(data, crc_value):
      logging.warning(f"Skipped a frame due to CRC mismatch!")
      continue

    return data

@dataclass
class LogMsg:
  seq_id: int
  tick: int
  msg_def: logger_def.MessageDef
  values: tuple

def parse_frame(log_def: logger_def.LoggerDef, frame: bytes):
  HEADER_FMT = '<BLH'
  HEADER_SIZE = struct.calcsize(HEADER_FMT)

  if len(frame) < HEADER_SIZE:
    logging.warning(f"Skipped frame too short to be a logger message")
    return
  
  header, data = frame[:HEADER_SIZE], frame[HEADER_SIZE:]

  # Parse the header
  seq_id, tick, msg_id = struct.unpack(HEADER_FMT, header)

  msg_def = log_def.messages.get(msg_id, None)

  if msg_def is None:
    logging.warning(f"Message {seq_id} has unknown id {msg_id}")
    values = None

  elif len(msg_def.fields) > 0:
    # Parse the data/payload
    try:
      values = struct.unpack(msg_def.struct_fmt, data)
    except struct.error as e:
      logging.warning(f"Message {seq_id} has malformed payload ({len(data)} bytes long): {e}")
      values = None
  else:
    values = tuple()

  return LogMsg(seq_id, tick, msg_def, values)

from datetime import timedelta
def log_msg(log: logging.Logger, msg: LogMsg):
  if msg.values is not None:
    values_dict = dict(zip((field.name for field in msg.msg_def.fields), msg.values))
    formatted = msg.msg_def.fmt.format(**values_dict)
  else:
    formatted = "MALFORMED"

  log.info(f"[{msg.seq_id:03}] @{timedelta(milliseconds=msg.tick).total_seconds():.03f}s : {formatted}")


if __name__ == '__main__':
  import argparse
  import sys

  def parse_args():
    EPILOG = """\
EXAMPLES:
  %(prog)s my_definition.json serial /dev/ttyS0 9600
  %(prog)s my_definition.json -d comms_dump.bin serial /dev/ttyS0 9600
  %(prog)s my_definition.json replay comms_dump.bin\
"""
    p = argparse.ArgumentParser(epilog = EPILOG, formatter_class = argparse.RawDescriptionHelpFormatter)
    p.add_argument('def_file', help = 'The logger definition json file.')
    p.add_argument('-d', '--dump', help = 'Dump all raw data read to a file (see replay).')
    
    subp = p.add_subparsers(title = 'source', required = True, help = 'Where to read data from.', dest = 'source')

    ser_p = subp.add_parser('serial', help = 'Read from a serial port.')
    ser_p.add_argument('serial_port')
    ser_p.add_argument('baudrate')

    file_p = subp.add_parser('replay', help = 'Read from a binary file.')
    file_p.add_argument('filename')

    return p.parse_args()
  
  args = parse_args()

  # Configure logging
  logging.basicConfig(format='%(asctime)s [%(levelname)s] %(funcName)s: %(message)s', level=logging.DEBUG, stream = sys.stderr)

  msg_logger = logging.getLogger('log-msg')
  msg_logger_handler = logging.StreamHandler(sys.stdout)
  msg_logger_handler.setLevel(logging.INFO)
  msg_logger_handler.setFormatter(logging.Formatter(fmt = '%(asctime)s %(message)s', datefmt='%H:%M:%S'))
  msg_logger.addHandler(msg_logger_handler)

  # Configure dependencies
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

  with contextlib.ExitStack() as ctx:
    # Initialize the source
    if args.source == 'serial':
      try:
        source = serial.Serial(args.serial_port, baudrate = args.baudrate)
      except serial.SerialException as e:
        logging.fatal(f"Failed to open serial port! {e}")
        exit(1)

    else:
      try:
        source = open(args.filename, 'rb')
      except Exception as e:
        logging.fatal(f"Failed to open file to replay! {e}")
        exit(1)
    
    ctx.enter_context(source)

    if args.dump is not None:
      try:
        dump_file = open(args.dump, 'wb')
        ctx.enter_context(dump_file)
      except Exception as e:
        logging.fatal(f'Failed to open dump file! {e}')
        exit(1)
    else:
      dump_file = None

    logging.info("Starting.")


    last_seq_id = None

    # Run the logger indefinitely.
    while True:
      try:
        frame = zcp_read(source, crc_calculator, dump_file)
        msg = parse_frame(log_def, frame)
        if msg is not None:
          if last_seq_id is not None and (not (last_seq_id == 255 and msg.seq_id == 0)) and last_seq_id != msg.seq_id - 1:
            msg_logger.warning("!! SEQUENCE ID GAP !!")
          last_seq_id = msg.seq_id

          log_msg(msg_logger, msg)
      
      except KeyboardInterrupt:
        logging.info("Exiting due to CTRL+C.")
        exit(0)

      except Exception:
        logging.error("Uncaught exception!", exc_info = True)
        time.sleep(1)