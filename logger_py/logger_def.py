from dataclasses import dataclass
import logging
@dataclass
class MessageFieldDef:
  name: str
  type_id: str
  fmt_map: dict

@dataclass
class MessageDef:
  id: int
  name: str
  fmt: str
  fields: list[MessageFieldDef]
  struct_fmt: str

@dataclass
class LoggerDef:
  messages: dict[int, MessageDef]

@dataclass
class LoggerTypeDef:
  type_id: str
  struct_type: str
  c_type: str
  klass: type

TYPES = [
  LoggerTypeDef('u8', 'B', 'uint8_t', int),
  LoggerTypeDef('i8', 'b', 'int8_t', int),
  LoggerTypeDef('bool', '?', 'bool', bool),
  LoggerTypeDef('u16', 'H', 'uint16_t', int),
  LoggerTypeDef('i16', 'h', 'int16_t', int),
  LoggerTypeDef('u32', 'L', 'uint32_t', int),
  LoggerTypeDef('i32', 'l', 'int32_t', int),
  LoggerTypeDef('f32', 'f', 'float', float)
]

TYPE_TO_STRUCT_MAP = dict((x.type_id, x.struct_type) for x in TYPES)
TYPE_TO_C_MAP = dict((x.type_id, x.c_type) for x in TYPES)
TYPE_TO_PYTHON_MAP = dict((x.type_id, x.klass) for x in TYPES)

def __parse_fmtmap(f: dict):
  fmt_map = f['fmt_map']
  pytype = TYPE_TO_PYTHON_MAP[f['type']]
  return dict((pytype(value), dst) for (value, dst) in fmt_map.items())

def __parse_field(f: dict):
  return MessageFieldDef(
    name = f["name"],
    type_id = f["type"],
    fmt_map = __parse_fmtmap(f) if 'fmt_map' in f else None
  )

def __parse_message_def(id: str, m: dict):
  fields = [__parse_field(f) for f in m.get('fields', tuple())]
  nId = int(id)
  struct_fmt = None if len(fields) <= 0 else ("<" + "".join(TYPE_TO_STRUCT_MAP[x.type_id] for x in fields))

  return (nId, MessageDef(nId, m["name"], m["fmt"], fields, struct_fmt))

def parse_from_dict(d: dict):
  messages = dict(__parse_message_def(*kvp) for kvp in d['messages'].items())
  return LoggerDef(messages)

import json
def parse_from_file(path):
  with open(path, "r") as f:
    return parse_from_dict(json.load(f))