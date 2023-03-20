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

    run_command(
        meson.current_source_dir() + '/vcpkg/bootstrap-vcpkg.sh',
        check: true,
    )

    vcpkg_prog = find_program(
        'vcpkg',
        required: true,
        dirs: [meson.current_source_dir() + '/vcpkg/'],
    )
endif

run_command(
    vcpkg_prog,
    'install',
    '--x-wait-for-lock',
    '--x-manifest-root=' + meson.current_source_dir(),
    check: true,
)

vkpkg_includes = include_directories('./vcpkg_installed/x64-linux/include')

declare_dependency(
    include_directories: './vcpkg_installed/x64-linux',
)

deps = []

ctre_dep = dependency('ctre', required: false)
if not ctre_dep.found()
    # deps += dependency('ctre', required: true, method: 'pkg-config')
else
    deps += ctre_dep
endif

utf8proc_dep = dependency('utf8proc', required: false)
if not utf8proc_dep.found()
    deps += dependency(
        'libutf8proc',
        required: true,
        fallback: 'utf8proc',
    )
else
    deps += utf8proc_dep
endif

deps += dependency('magic_enum', required: true, fallback: 'magic_enum')

microsoft_gsl_dep = dependency('Microsoft.GSL', required: false)
if not microsoft_gsl_dep.found()
    # deps += dependency(
    #     'Microsoft.GSL',
    #     required: true,
    #     fallback: 'Microsoft.GSL',
    # )
else
    deps += microsoft_gsl_dep
endif

deps += dependency('tl-expected', required: true, fallback: 'tl-expected')
deps += dependency('tl-optional', required: true, fallback: 'tl-optional')
deps += dependency('cxxopts', required: true, fallback: 'cxxopts')
deps += dependency('fmt', required: true, fallback: 'fmt')
deps += dependency('range-v3', required: true, fallback: 'range-v3')


inc_dirs = []


subdir('src')

executable(
    'Seatbelt2',
    src_files,
    link_with: [],
    include_directories: inc_dirs + vkpkg_includes,
    dependencies: deps,
    override_options: [
        'warning_level=3',
        'werror=true',
    ],
)


