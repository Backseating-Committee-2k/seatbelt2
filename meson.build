project(
    'Seatbelt2',
    'cpp',
    default_options: [
        'buildtype=release',
        'optimization=3',
        'c_std=c2x',
        'cpp_std=c++20',
        'strip=true',
        'b_ndebug=if-release',
    ],
    version: '2.0.0-alpha',
)

cpp = meson.get_compiler('cpp')
if cpp.get_id() == 'msvc'
    add_project_arguments('/std:c++latest', language: 'cpp')
elif cpp.get_id() == 'gcc'
    add_project_arguments('-std=c++23', language: 'cpp')
else
    add_project_arguments('-std=c++20', language: 'cpp')
endif

deps = []
inc_dirs = []
link_deps = []
src_files = []

subdir('tools')
subdir('src')

executable(
    'Seatbelt2',
    src_files,
    link_with: link_deps,
    include_directories: inc_dirs,
    dependencies: deps,
    override_options: [
        'warning_level=3',
        'werror=true',
    ],
)


