cmake_minimum_required(VERSION 3.21)
project(Seatbelt2)
include(FetchContent)

set(CMAKE_CXX_STANDARD 23)

if (MSVC)
    # enable code analysis
    set_property(GLOBAL PROPERTY VS_GLOBAL_EnableCppCoreCheck true)
    set_property(GLOBAL PROPERTY VS_GLOBAL_CodeAnalysisRuleSet CppCoreCheckRules.ruleset)
    set_property(GLOBAL PROPERTY VS_GLOBAL_RunCodeAnalysis true)

    # remove /W3 from defaults
    string(REGEX REPLACE "/W3" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    string(REGEX REPLACE "-W3" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
endif ()

find_package(ctre CONFIG REQUIRED)
find_package(magic_enum CONFIG REQUIRED)
find_package(Microsoft.GSL CONFIG REQUIRED)
find_package(tl-expected CONFIG REQUIRED)
find_package(tl-optional CONFIG REQUIRED)
find_package(unofficial-utf8proc CONFIG REQUIRED)
find_package(cxxopts CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(range-v3 CONFIG REQUIRED)

set(TARGET_LIST Seatbelt2)

add_executable(Seatbelt2
        src/main.cpp
        src/tokens.hpp src/types.hpp src/lexer.cpp src/lexer.hpp src/source_location.hpp src/error_codes.hpp src/utils.cpp src/utils.hpp)

foreach (target ${TARGET_LIST})
    target_link_libraries(${target} PRIVATE cxxopts::cxxopts)
    target_link_libraries(${target} PRIVATE fmt::fmt)
    target_link_libraries(${target} PRIVATE range-v3 range-v3-meta range-v3::meta range-v3-concepts)
    target_link_libraries(${target} PRIVATE ctre::ctre)
    target_link_libraries(${target} PRIVATE magic_enum::magic_enum)
    target_link_libraries(${target} PRIVATE Microsoft.GSL::GSL)
    target_link_libraries(${target} PRIVATE tl::expected)
    target_link_libraries(${target} PRIVATE tl::optional)
    target_link_libraries(${target} PRIVATE utf8proc)

    # set warning levels
    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        message("MSVC build")
        if (CMAKE_BUILD_TYPE STREQUAL "Release")
            target_compile_options(${target} PUBLIC /W4 /WX /permissive-)
        else ()
            target_compile_options(${target} PUBLIC /W4 /WX /permissive- /fsanitize=address)
        endif ()
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        message("GCC build")
        if (CMAKE_BUILD_TYPE STREQUAL "Release")
            target_compile_options(${target} PUBLIC -Wall -Wextra -Werror -pedantic -Wconversion -pthread)
        else ()
            target_compile_options(${target} PUBLIC -Wall -Wextra -Werror -pedantic -Wconversion -pthread -fsanitize=address,undefined)
        endif ()
        target_link_options(${target} PUBLIC -fsanitize=address,undefined)
    endif ()

    # define DEBUG_BUILD
    target_compile_definitions(${target} PUBLIC "$<$<CONFIG:DEBUG>:DEBUG_BUILD>")

    # static runtime library
    set_property(TARGET ${target} PROPERTY
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

    if (CMAKE_BUILD_TYPE STREQUAL "Release")
        message("Enabling LTO for target ${target}")
        set_property(TARGET ${target} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    else ()
        message("Not enabling LTO for target ${target} (not a release build)")
    endif ()
endforeach ()
