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
    add_project_arguments('-std=c++latest', language: 'cpp')
elif cpp.get_id() == 'gcc'
    add_project_arguments('-std=c++23', language: 'cpp')
else
    add_project_arguments('-std=c++20', language: 'cpp')
endif


vcpkg_prog = find_program(
    'vcpkg',
    required: false,
    dirs: [meson.current_source_dir() + '/vcpkg/'],
)

if not vcpkg_prog.found()

    prog_git = find_program('git', required: true)

    message('Now getting vcpkg and bootstrapping it')

    # git clone vcpkg
    run_command(
        prog_git,
        'clone',
        'https://github.com/Microsoft/vcpkg.git',
        meson.current_source_dir() + '/vcpkg',
        check: true,
    )

    vcpkg_script_extension = 'sh'

    if build_machine.system() == 'windows'
        vcpkg_script_extension = 'bat'
    elif build_machine.system() == 'linux'
        vcpkg_script_extension = 'sh'
    else
        error('Not supported system: ' + build_machine.system())
    endif


    run_command(
        meson.current_source_dir() + '/vcpkg/bootstrap-vcpkg.' + vcpkg_script_extension,
        check: true,
    )

    vcpkg_prog = find_program(
        'vcpkg',
        required: true,
        dirs: [meson.current_source_dir() + '/vcpkg/'],
    )
endif

arch = build_machine.cpu_family()


if arch == 'x86_64'
    arch = 'x64'
elif arch == 'x86'
    arch = 'x86'
else
    error('Not supported cpu_family: ' + arch)
endif


vcpkg_buildtype = 'release'


if (
    get_option('buildtype') == 'debug'
    or get_option('buildtype') == 'debugoptimized'
)
    vcpkg_buildtype = 'debug'
endif

vcpkg_triplet = arch + '-' + build_machine.system() + '-' + vcpkg_buildtype

run_command(
    vcpkg_prog,
    'install',
    '--x-wait-for-lock',
    '--x-manifest-root=' + meson.current_source_dir(),
    '--triplet=' + vcpkg_triplet,
    '--host-triplet=' + vcpkg_triplet,
    check: true,
)


library_extension = 'a'

if build_machine.system() == 'windows'
    library_extension = 'dll'
elif build_machine.system() == 'linux'
    library_extension = 'a'
else
    error('Not supported system: ' + build_machine.system())
endif

prog_python = find_program('python3', 'python', required: true)
installed_libs = run_command(
    prog_python,
    'tools/grabber.py',
    './vcpkg_installed/' + vcpkg_triplet + '/lib',
    library_extension,
    check: true,
).stdout().strip().split('\n')


vcpkg_libs = []

cpp = meson.get_compiler('cpp')

foreach dep : installed_libs
    vcpkg_libs += cpp.find_library(
        dep,
        dirs: meson.current_source_dir() + '/vcpkg_installed/' + vcpkg_triplet +
        '/lib',
        required: true,
    )
endforeach


deps = declare_dependency(
    include_directories: include_directories(
        './vcpkg_installed/' + vcpkg_triplet + '/include',
    ),
    dependencies: vcpkg_libs,
)

inc_dirs = []

subdir('src')

executable(
    'Seatbelt2',
    src_files,
    link_with: [],
    include_directories: inc_dirs,
    dependencies: deps,
    override_options: [
        'warning_level=3',
        'werror=true',
    ],
)


