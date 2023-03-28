## THIS is a vcpkg helper, it is a helpfull tool to install and use vcpkg packages via "vcpkg.json" manifest
# it doesn't support everyithing and isn't guanteed to work, but it works for this project atm.


#it adds its dependecies to deps, so pleaxse define it (desp = []) before using subdir() to use this file!


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
    '--x-manifest-root=' + meson.project_source_root(),
    '--triplet=' + vcpkg_triplet,
    '--host-triplet=' + vcpkg_triplet,
    '--x-install-root=' + meson.current_source_dir() + '/vcpkg_installed/',
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
    meson.current_source_dir() + '/grabber.py',
    meson.current_source_dir() + '/vcpkg_installed/' + vcpkg_triplet + '/lib',
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


deps += declare_dependency(
    include_directories: include_directories(
        './vcpkg_installed/' + vcpkg_triplet + '/include',
    ),
    dependencies: vcpkg_libs,
)

message(
    'Found ' + installed_libs.length().to_string() + ' libraries via vcpkg manifest mode and added them to the project',
)
