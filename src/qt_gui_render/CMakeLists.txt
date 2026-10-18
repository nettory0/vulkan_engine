
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)  

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
#find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
#find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

#qt_standard_project_setup()

qt_add_executable(qt_gui_render
    GUI_Render.cpp
    GUI_Render.h
    main.cpp
    vulkan_window.cpp
	GUI_Render.ui
	)

set_target_properties(qt_gui_render PROPERTIES FOLDER "Project")

set_target_properties(qt_gui_render PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/bin/"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/"
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/lib/"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/lib/"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/lib/"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/lib/"
)

target_link_libraries(qt_gui_render 
	PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
    PRIVATE Qt${QT_VERSION_MAJOR}::Core
	PRIVATE volk
    PRIVATE ${VULKAN_LIBRARY} 
    PRIVATE vulkan_engine
    PRIVATE assimp
    PRIVATE freetype
	)


if(WIN32)
    get_filename_component(Qt6_BIN_DIR "${Qt6_DIR}/../../../bin" ABSOLUTE)

    add_custom_command(TARGET qt_gui_render POST_BUILD
        COMMAND "${Qt6_BIN_DIR}/windeployqt.exe" "$<TARGET_FILE:qt_gui_render>"
        COMMENT "Deploying Qt DLLs for Windows..."
    )
endif()

set_target_properties(qt_gui_render PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:qt_gui_render>"
)