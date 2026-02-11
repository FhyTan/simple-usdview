find_package(Qt6 REQUIRED COMPONENTS Core Gui OpenGL Widgets)
find_package(pxr REQUIRED)
find_package(OpenGL REQUIRED)

# Setup Qt
qt_standard_project_setup()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
add_definitions(-DQT_NO_KEYWORDS)

qt_add_executable(simple_usdview
    main.cpp
    MainWindow.h MainWindow.cpp
    StageViewWidget.h StageViewWidget.cpp
    FreeCamera.h FreeCamera.cpp
    Outliner.h Outliner.cpp
    Settings.h
    resources.qrc
)

set_target_properties(simple_usdview PROPERTIES
    WIN32_EXECUTABLE FALSE
)

target_link_libraries(simple_usdview PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::OpenGL
    Qt6::Widgets
)

# Setup USD
target_include_directories(simple_usdview PRIVATE
    ${PXR_INCLUDE_DIRS}
)

target_link_libraries(simple_usdview PRIVATE
    OpenGL::GL
    usdImagingGL
    # ${PXR_LIBRARIES}
)

# Copy the USD file alongside the executable
add_custom_command(
    TARGET  simple_usdview POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_SOURCE_DIR}/display.usda"
        $<TARGET_FILE_DIR:simple_usdview>
)

