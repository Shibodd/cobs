from dataclasses import dataclass

@dataclass
class MessageFieldDef:
  name: str
  type_id: str

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

TYPES = [
  LoggerTypeDef('u8', 'B', 'uint8_t'),
  LoggerTypeDef('i8', 'b', 'int8_t'),
  LoggerTypeDef('bool', '?', 'bool'),
  LoggerTypeDef('u16', 'H', 'uint16_t'),
  LoggerTypeDef('i16', 'h', 'int16_t'),
  LoggerTypeDef('u32', 'L', 'uint32_t'),
  LoggerTypeDef('i32', 'l', 'int32_t'),
  LoggerTypeDef('f32', 'f', 'float')
]

TYPE_TO_STRUCT_MAP = dict((x.type_id, x.struct_type) for x in TYPES)
TYPE_TO_C_MAP = dict((x.type_id, x.c_type) for x in TYPES)



def __parse_field(f: dict):
  return MessageFieldDef(f["name"], f["type"])

def __parse_message_def(id: str, m: dict):
  fields = [__parse_field(f) for f in m.get('fields', [])]
  nId = int(id)
  struct_fmt = "<" + "".join(TYPE_TO_STRUCT_MAP[x.type_id] for x in fields)

  return (nId, MessageDef(nId, m["name"], m["fmt"], fields, struct_fmt))

def parse_from_dict(d: dict):
  messages = dict(__parse_message_def(*kvp) for kvp in d['messages'].items())
  return LoggerDef(messages)

import json
def parse_from_file(path):
  with open(path, "r") as f:
    return parse_from_dict(json.load(f))