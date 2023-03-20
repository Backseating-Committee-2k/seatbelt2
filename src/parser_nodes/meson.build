## nearly the same as: 
# pymodule = import('python')
# prog_python = pymodule.find_installation('python3', required: true)
prog_python = find_program('python3', 'python', required: true)

# generate and regenerate the 'parser_nodes.(c|h)pp' files dynamically
src_files += custom_target(
    'parser_nodes_generator',
    output: ['parser_nodes.cpp', 'parser_nodes.hpp'],
    input: [
        meson.global_source_root() + '/tools/ast-node-generator-for-seatbelt2/main.py',
        meson.global_source_root() + '/tools/ast-node-generator-for-seatbelt2/parser_nodes.types',
    ],
    command: [
        prog_python,
        '@INPUT@',
        '@OUTDIR@/parser_nodes',
        'parser_nodes',
    ],
)


inc_dirs += include_directories('.')
