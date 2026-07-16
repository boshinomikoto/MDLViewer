cmake_minimum_required(VERSION 3.14)
project(ModelsViewer)

# Путь к SDK — задаётся при вызове cmake -DURHO3D_HOME=...
# Если не задан, берём переменную окружения URHO3D_HOME
if(NOT DEFINED URHO3D_HOME)
    set(URHO3D_HOME "$ENV{URHO3D_HOME}" CACHE PATH "Path to Urho3D SDK")
endif()

list(APPEND CMAKE_MODULE_PATH "${URHO3D_HOME}/share/CMake/Modules")
find_package(Urho3D REQUIRED)

add_subdirectory(external/nfd)

add_executable(ModelsViewer WIN32
    main.cpp
)

set_target_properties(ModelsViewer PROPERTIES CXX_STANDARD 17)

target_include_directories(ModelsViewer PRIVATE
    ${URHO3D_INCLUDE_DIRS}
    "${URHO3D_HOME}/include/Urho3D/ThirdParty/SDL"
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/external/nfd/src/include"
)

target_link_libraries(ModelsViewer
    ${URHO3D_LIBRARIES}
    d3d11.lib
    d3dcompiler.lib
    dxgi.lib
    dxguid.lib
    imm32.lib
    version.lib
    hid.lib
    winmm.lib
    setupapi.lib
    nfd
)

# Копируем свои ресурсы в папку с exe после сборки
add_custom_command(TARGET ModelsViewer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/Data"
        "$<TARGET_FILE_DIR:ModelsViewer>/Data"
)