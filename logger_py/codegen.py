import logger_def
import itertools
from datetime import datetime

def generate_logging_function(msg: logger_def.MessageDef):
  args = ", ".join("{} {}".format(logger_def.TYPE_TO_C_MAP[field.type_id], field.name) for field in msg.fields)
  values = itertools.chain((str(msg.id), ), (field.name for field in msg.fields))
  
  
  FUNCTION_TEMPLATE = """\
bool {fun_name}({args}) {{
  WRITE_ASSERT(WRITE_BEGIN());
{content}
  WRITE_ASSERT(WRITE_END());
}}\
"""
  return FUNCTION_TEMPLATE.format(
    fun_name = f"MMR_BACK_LOG_{msg.name}",
    args = args,
    content = "\n".join(f"  WRITE_ASSERT(WRITE_APPEND({x}));" for x in values)
  )

if __name__ == '__main__':
  import argparse
  import os

  argparser = argparse.ArgumentParser()
  argparser.add_argument('def_file')
  args = argparser.parse_args()
 
  print(f'/* Generated from file "{args.def_file}", last modified {datetime.fromtimestamp(os.path.getmtime(args.def_file))} */\n')

  for msg in logger_def.parse_from_file(args.def_file).messages.values():
    print(generate_logging_function(msg))