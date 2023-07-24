import logger_def
import itertools
from datetime import datetime

def generate_logging_function_prototype(msg: logger_def.MessageDef):
  FUNCTION_PROTO_TEMPLATE = "bool {fun_name}({args})"
  FUNCTION_NAME_TEMPLATE = "MMR_BACK_LOG_{name}"

  return FUNCTION_PROTO_TEMPLATE.format(
    fun_name = FUNCTION_NAME_TEMPLATE.format(
      name = msg.name
    ),
    args = ", ".join("{} {}".format(logger_def.TYPE_TO_C_MAP[field.type_id], field.name) for field in msg.fields)
  )

def generate_logging_function_body(msg: logger_def.MessageDef):
  FUNCTION_BODY = """ {{
  uint16_t msg_id = {msg_id};
  WRITE_ASSERT(WRITE_BEGIN(msg_id));
{content}
  WRITE_ASSERT(WRITE_END());
  return true;
}}\n
"""

  values = (field.name for field in msg.fields)

  return FUNCTION_BODY.format(
    msg_id = msg.id,
    content = "\n".join(f"  WRITE_ASSERT(WRITE_APPEND({x}));" for x in values)
  )

if __name__ == '__main__':
  import argparse
  import os

  argparser = argparse.ArgumentParser()
  argparser.add_argument('def_file')
  argparser.add_argument('out_definitions')
  argparser.add_argument('out_prototypes')
  args = argparser.parse_args()
  

  PROLOGUE_TEMPLATE = """\
#ifndef {guard_name}
#define {guard_name}

/* Generated from file "{def_file}", last modified {last_modified} */
"""

  EPILOGUE_TEMPLATE = "\n#endif // !{guard_name}"

  def_file = args.def_file
  last_modified = datetime.fromtimestamp(os.path.getmtime(def_file))
  
  out_definitions_guard = args.out_definitions.replace(".", "_").upper()
  out_prototypes_guard = args.out_prototypes.replace(".", "_").upper()

  with open(args.out_prototypes, 'wt') as out_prototypes, open(args.out_definitions, 'wt') as out_definitions:
    out_prototypes.write(PROLOGUE_TEMPLATE.format(
      guard_name = out_prototypes_guard,
      def_file = def_file,
      last_modified = last_modified
    ))
    out_definitions.write(PROLOGUE_TEMPLATE.format(
      guard_name = out_definitions_guard,
      def_file = def_file,
      last_modified = last_modified
    ))

    for msg in logger_def.parse_from_file(args.def_file).messages.values():
      proto = generate_logging_function_prototype(msg)
      out_prototypes.write(proto + ";\n")
      out_definitions.write(proto)

      body = generate_logging_function_body(msg)
      out_definitions.write(body)

    out_prototypes.write(EPILOGUE_TEMPLATE.format(guard_name = out_prototypes_guard))
    out_definitions.write(EPILOGUE_TEMPLATE.format(guard_name = out_definitions_guard))