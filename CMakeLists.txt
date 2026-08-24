cmake_minimum_required(VERSION 3.20)
project(NanoTekSpice VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build configuration" FORCE)
endif()
option(NTS_BONUS "Build graphical devices and computer architecture extensions" ON)
option(NTS_BUILD_GRAPHICS "Build the native SDL2 graph viewer" ON)
option(NTS_STRICT "Treat compiler warnings as errors" ON)
option(NTS_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(NTS_BENCHMARKS "Build deterministic performance workloads" ON)
include(CTest)
find_package(Threads REQUIRED)
add_library(nts_sdk INTERFACE)
target_include_directories(nts_sdk INTERFACE "${PROJECT_SOURCE_DIR}/include" "${PROJECT_SOURCE_DIR}/src")
target_compile_options(nts_sdk INTERFACE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
if(NTS_STRICT)
    target_compile_options(nts_sdk INTERFACE -Werror)
endif()
if(NTS_SANITIZERS)
    target_compile_options(nts_sdk INTERFACE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(nts_sdk INTERFACE -fsanitize=address,undefined)
endif()
if(NTS_BONUS)
    target_compile_definitions(nts_sdk INTERFACE NTS_BONUS)
    target_include_directories(nts_sdk INTERFACE "${PROJECT_SOURCE_DIR}/bonus")
endif()

set(component_sources
    src/components/advanced/Adder4008.cpp
    src/components/advanced/AdvancedFactory.cpp
    src/components/advanced/Counter4017.cpp
    src/components/advanced/Counter4040.cpp
    src/components/advanced/Decoder4514.cpp
    src/components/advanced/FlipFlop4013.cpp
    src/components/advanced/Logger.cpp
    src/components/advanced/Logic.cpp
    src/components/advanced/Ram4801.cpp
    src/components/advanced/Rom2716.cpp
    src/components/advanced/Selector4512.cpp
    src/components/advanced/Shift4094.cpp
    src/components/base/Component.cpp
    src/components/base/ComponentEvaluation.cpp
    src/components/base/ComponentLinks.cpp
    src/components/base/Error.cpp
    src/components/base/IComponent.cpp
    src/components/base/Tristate.cpp
    src/components/basic/BasicFactory.cpp
    src/components/basic/Gate.cpp
    src/components/basic/Source.cpp
    src/components/factory/ComponentFactory.cpp
)
set(core_sources src/Circuit.cpp src/Parser.cpp src/Text.cpp src/Shell.cpp src/Options.cpp ${component_sources})
if(NTS_BONUS)
    set(bonus_sources
        bonus/BonusFactory.cpp
        bonus/CircuitGraph.cpp
        bonus/components/Cpu.cpp
        bonus/components/Matrix.cpp
        bonus/components/SevenSegment.cpp
        bonus/cpu/Integer.cpp
        bonus/cpu/Program.cpp
        bonus/cpu/Subleq.cpp
        bonus/graphics/Export.cpp
        bonus/graphics/Glyphs.cpp
        bonus/graphics/Gui.cpp
    )
    list(APPEND core_sources ${bonus_sources})
endif()
add_library(nts_core STATIC ${core_sources})
target_link_libraries(nts_core PUBLIC nts_sdk Threads::Threads)
if(NTS_BONUS AND NTS_BUILD_GRAPHICS)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)
    target_link_libraries(nts_core PUBLIC PkgConfig::SDL2)
    target_compile_definitions(nts_sdk INTERFACE NTS_WITH_GUI)
endif()
add_executable(nanotekspice src/main.cpp)
target_link_libraries(nanotekspice PRIVATE nts_core)
set_target_properties(nanotekspice PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
add_custom_target(stage ALL COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:nanotekspice> "${PROJECT_SOURCE_DIR}/nanotekspice" DEPENDS nanotekspice)

if(BUILD_TESTING)
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    foreach(suite basic advanced parser)
        if(EXISTS "${PROJECT_SOURCE_DIR}/tests/${suite}.cpp")
            file(GLOB_RECURSE suite_sources CONFIGURE_DEPENDS "tests/${suite}/*.cpp")
            add_executable(test_${suite} "tests/${suite}.cpp" ${suite_sources})
            target_link_libraries(test_${suite} PRIVATE nts_core)
            add_test(NAME ${suite} COMMAND test_${suite})
            set_tests_properties(${suite} PROPERTIES TIMEOUT 90)
        endif()
    endforeach()
    if(EXISTS "${PROJECT_SOURCE_DIR}/tests/cli.py")
        add_test(NAME cli COMMAND ${Python3_EXECUTABLE} "${PROJECT_SOURCE_DIR}/tests/cli.py"
            $<TARGET_FILE:nanotekspice> "${PROJECT_SOURCE_DIR}" $<BOOL:${NTS_BONUS}>)
        set_tests_properties(cli PROPERTIES TIMEOUT 90)
    endif()
    if(EXISTS "${PROJECT_SOURCE_DIR}/tests/signals.py")
        add_test(NAME signals COMMAND ${Python3_EXECUTABLE} "${PROJECT_SOURCE_DIR}/tests/signals.py"
            $<TARGET_FILE:nanotekspice>)
        set_tests_properties(signals PROPERTIES TIMEOUT 30)
    endif()
    if(NTS_BONUS AND EXISTS "${PROJECT_SOURCE_DIR}/bonus/tests/Main.cpp")
        file(GLOB bonus_test_sources CONFIGURE_DEPENDS "bonus/tests/*.cpp")
        add_executable(test_bonus ${bonus_test_sources})
        target_link_libraries(test_bonus PRIVATE nts_core)
        add_test(NAME bonus COMMAND test_bonus)
        set_tests_properties(bonus PROPERTIES TIMEOUT 90)
    endif()
    if(NTS_BONUS AND NTS_BUILD_GRAPHICS AND NOT APPLE)
        find_program(XVFB_RUN_EXECUTABLE xvfb-run)
        if(XVFB_RUN_EXECUTABLE)
            add_test(NAME gui_lifecycle COMMAND ${CMAKE_COMMAND} -E env NTS_GUI_SMOKE_FRAMES=3
                ${XVFB_RUN_EXECUTABLE} -a $<TARGET_FILE:nanotekspice>
                --program cpu "${PROJECT_SOURCE_DIR}/bonus/examples/paint.subleq"
                --set clk=0 --ticks 42000 --gui "${PROJECT_SOURCE_DIR}/bonus/examples/computer.nts")
            set_tests_properties(gui_lifecycle PROPERTIES TIMEOUT 60 WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
        endif()
    endif()
    file(GLOB_RECURSE headers CONFIGURE_DEPENDS "include/*.hpp" "src/*.hpp" "tests/*.hpp")
    if(NTS_BONUS)
        file(GLOB_RECURSE bonus_headers CONFIGURE_DEPENDS "bonus/*.hpp")
        list(APPEND headers ${bonus_headers})
    endif()
    set(header_sources)
    foreach(header IN LISTS headers)
        string(MAKE_C_IDENTIFIER "${header}" key)
        set(source "${CMAKE_BINARY_DIR}/header-checks/${key}.cpp")
        file(GENERATE OUTPUT "${source}" CONTENT "#include \"${header}\"\n")
        list(APPEND header_sources "${source}")
    endforeach()
    add_library(header_checks OBJECT ${header_sources})
    target_link_libraries(header_checks PRIVATE nts_core)
    find_program(CLANG_FORMAT_EXECUTABLE NAMES clang-format clang-format-20 clang-format-19 clang-format-18)
    if(CLANG_FORMAT_EXECUTABLE AND EXISTS "${PROJECT_SOURCE_DIR}/tools/source_quality.py")
        add_test(NAME source_quality COMMAND ${Python3_EXECUTABLE}
            "${PROJECT_SOURCE_DIR}/tools/source_quality.py" "${PROJECT_SOURCE_DIR}" "${CLANG_FORMAT_EXECUTABLE}")
    endif()
endif()

if(NTS_BONUS AND NTS_BENCHMARKS)
    add_executable(nts_benchmark bonus/benchmarks/Main.cpp bonus/benchmarks/Workload.cpp)
    target_link_libraries(nts_benchmark PRIVATE nts_core)
    set_target_properties(nts_benchmark PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
endif()
