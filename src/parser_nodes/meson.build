## nearly the same as: 
# pymodule = import('python')
# prog_python = pymodule.find_installation('python3', required: true)
prog_python = find_program('python3', 'python', required: true)

# generate and regenerate the 'parser_nodes.(c|h)pp' files dynamically
# this generates the files into the built dir, where they are needed to built the project
parser_nodes = custom_target(
    'parser_nodes',
    output: [
        'parser_nodes.cpp',
        'parser_nodes.hpp',
    ],
    input: [
        ## this is an input and not as normal string below in the command list, since changes in here should trigger an automatic rebuilt!
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

src_files += parser_nodes

# this generates the file into the source tree, where they are needed for autocompletion and manual lookover (to invoke run "meson compile generate_parser_nodes" (add -C <builddir> if necessary!)
run_target(
    'generate_parser_nodes',
    depends: [parser_nodes],
    command: [
        prog_python,
        meson.global_source_root() + '/tools/ast-node-generator-for-seatbelt2/main.py',
        meson.global_source_root() + '/tools/ast-node-generator-for-seatbelt2/parser_nodes.types',
        meson.current_source_dir() + '/parser_nodes',
        'parser_nodes',
    ],
)


inc_dirs += include_directories('.')
