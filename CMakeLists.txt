cmake_minimum_required(VERSION 3.20)
project(Updated_GUI VERSION 0.1 LANGUAGES CXX)
add_compile_definitions(NOMINMAX)
# Global compiler requirements
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
# 1. Path Registries (Global Toolchain Configuration)
set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake")
set(CMAKE_PREFIX_PATH "C:/Qt/6.11.1/msvc2022_64")
list(APPEND CMAKE_PREFIX_PATH "C:/vcpkg/installed/x64-windows")
# Force CMake to prioritize global classic mode instead of searching for local manifests
set(VCPKG_MANIFEST_MODE OFF CACHE BOOL "Disable manifest mode" FORCE)
# 2. Package discovery for Qt6 and OpenCV
find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia MultimediaWidgets)
set(OpenCV_DIR "C:/vcpkg/installed/x64-windows/share/opencv4")
find_package(OpenCV REQUIRED)
# 3. Targeted Linking to your vcpkg ONNX Runtime
# We point directly to the directory containing onnxruntime_cxx_api.h
set(ONNXRUNTIME_INCLUDE_DIR "C:/vcpkg/installed/x64-windows/include/onnxruntime")
set(ONNXRUNTIME_LIBRARY "C:/vcpkg/installed/x64-windows/lib/onnxruntime.lib")
# 4. Define target executable and source compilation tree
qt_add_executable(Updated_GUI
    MANUAL_FINALIZATION
    main.cpp
    MainWindow.h       MainWindow.cpp
    TitleBar.h         TitleBar.cpp
    CameraFeed.h       CameraFeed.cpp
    LiveTextBar.h      LiveTextBar.cpp
    SignLibraryPanel.h SignLibraryPanel.cpp
    ASLBackend.h       ASLBackend.cpp
)
# 5. Header inclusions (Injects the ONNX headers directly into your files)
target_include_directories(Updated_GUI PRIVATE ${ONNXRUNTIME_INCLUDE_DIR})
# 6. Comprehensive library mappings
target_link_libraries(Updated_GUI PRIVATE
    Qt6::Widgets
    Qt6::Multimedia
    Qt6::MultimediaWidgets
    ${OpenCV_LIBS}
    ${ONNXRUNTIME_LIBRARY}  # Explicitly linked to the vcpkg .lib file
    ole32
    oleaut32
    sapi
)
# Auto-copy all vcpkg DLLs after every build
add_custom_command(TARGET Updated_GUI POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "C:/vcpkg/installed/x64-windows/bin/onnxruntime.dll"
        "C:/vcpkg/installed/x64-windows/bin/re2.dll"
        "C:/vcpkg/installed/x64-windows/bin/abseil_dll.dll"
        "C:/vcpkg/installed/x64-windows/bin/libprotobuf.dll"
        "C:/vcpkg/installed/x64-windows/bin/libprotobuf-lite.dll"
        $<TARGET_FILE_DIR:Updated_GUI>
)
# Auto-copy Qt DLLs
add_custom_command(TARGET Updated_GUI POST_BUILD
    COMMAND "C:/Qt/6.11.1/msvc2022_64/bin/windeployqt.exe"
         $<TARGET_FILE:Updated_GUI>
)
# 7. Post-build asset replication rules
add_custom_command(TARGET Updated_GUI POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/signs
    $<TARGET_FILE_DIR:Updated_GUI>/signs
)
qt_finalize_executable(Updated_GUI)