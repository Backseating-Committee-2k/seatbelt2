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

subdir('tools')

deps = []

deps += dependency('cxxopts', required: true) ## this is a header only lib
deps += dependency('fmt', required: true, fallback: ['libfmt', 'fmt_dep'])
deps += dependency('range-v3', required: true) ## this is a header only lib
deps += dependency('tl-expected', required: true) ## this is a header only lib
deps += dependency('tl-optional', required: true) ## this is a header only lib
deps += dependency('magic-enum', required: true) ## this is a header only lib
deps += dependency('ctre', required: true) ## this is a header only lib
deps += dependency('ms-gsl', required: true) ## this is a header only lib
deps += dependency(
    'utf8proc',
    required: true,
    fallback: ['libutf8proc', 'utf8proc_dep'],
)



inc_dirs = []
link_deps = []
src_files = []


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


