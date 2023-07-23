import logger_def
import itertools
from datetime import datetime

FUNCTION_TEMPLATE = """\
void log{name}({args}) {{
  LOG({id_then_values});
}}\
"""


def generate_logging_function(msg: logger_def.MessageDef):
  args = ", ".join("{} {}".format(logger_def.TYPE_TO_C_MAP[field.type_id], field.name) for field in msg.fields)
  id_then_values = ", ".join(itertools.chain((str(msg.id), ), (field.name for field in msg.fields)))

  return FUNCTION_TEMPLATE.format(
    name = msg.name,
    id = msg.id,
    args = args,
    id_then_values = id_then_values
  )

if __name__ == '__main__':
  import argparse
  import os

  argparser = argparse.ArgumentParser()
  argparser.add_argument('def_file')
  args = argparser.parse_args()
 
  print(f'/* Generated at {datetime.now()} from file "{args.def_file}", last modified {datetime.fromtimestamp(os.path.getmtime(args.def_file))} */\n')

  for msg in logger_def.parse_from_file(args.def_file).messages.values():
    print(generate_logging_function(msg))