cmake_minimum_required(VERSION 3.12)
project(EZ4Connect)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

option(NIGHTLY_BUILD    "Set if built nightly.")

if(NIGHTLY_BUILD)
    set(PROJ_VER "${GIT_HASH}-nightly")
elseif(DEFINED REL_VER)
    set(PROJ_VER "${REL_VER}")
else()
    set(PROJ_VER "dev")
endif()

add_compile_definitions(PROJ_VER="${PROJ_VER}")
message("PROJ_VER=${PROJ_VER}")

find_package(Qt6 COMPONENTS
    Core
    Concurrent
    Gui
    Widgets
    Network
    Svg
    Core5Compat
    WebEngineWidgets
    REQUIRED
)


set(SOURCE_FILES
    main.cpp
    mainwindow.cpp
    zjuconnectmode.cpp
    application/applicationlogger.cpp
    application/connectionsession.cpp
    application/systemproxysession.cpp
    core/corecommandbuilder.cpp
    core/coreoutputbuffer.cpp
    core/coreoutputparser.cpp
    core/connectionsessionstate.cpp
    infrastructure/corelogfile.cpp
    infrastructure/settingsprofileloader.cpp
    infrastructure/platformsystemproxybackend.cpp
    zjuconnectcontroller/zjuconnectcontroller.cpp
    extrasettingwindow/extrasettingwindow.cpp
    settingwindow/settingwindow.cpp
    loginwindow/loginwindow.cpp
    sudowindow/sudowindow.cpp
    ssologinwebview/ssologinwebview.cpp
    authinfowindow/authinfowindow.cpp
    graphcaptchawindow/graphcaptchawindow.cpp
    graphcaptchawindow/captchacanvas.cpp
    utils/utils.cpp
    utils/profilemanager.cpp
    utils/proxysettings.cpp
    utils/coreversion.cpp
    utils/trustdevice.cpp
    resource.qrc
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if (WIN32)
    message("Building for Windows")

    enable_language("RC")

    add_executable(${PROJECT_NAME} WIN32
        ${SOURCE_FILES}
        ${CMAKE_CURRENT_SOURCE_DIR}/resource/icon.rc
    )

    set(PLATFORM_LIBS wininet rasapi32)
elseif(APPLE)
    message("Building for MacOS")
    
    set(MACOSX_BUNDLE_ICON_FILE icon.icns)
    
    set(app_icon_macos ${CMAKE_CURRENT_SOURCE_DIR}/resource/icon.icns)
    set_source_files_properties(${app_icon_macos} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

    add_executable(${PROJECT_NAME} MACOSX_BUNDLE
        ${SOURCE_FILES}
        ${app_icon_macos}
    )

    set_target_properties(${PROJECT_NAME}
        PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/resource/Info.plist")
else()
    add_executable(${PROJECT_NAME}
        ${SOURCE_FILES}
    )

    install(
        TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION bin
    )
endif()

set(QAPPLICATION_CLASS QApplication CACHE STRING "Inheritance class for SingleApplication")

add_subdirectory(singleapplication)

target_link_libraries(${PROJECT_NAME}
        Qt::Core
        Qt::Concurrent
        Qt::Gui
        Qt::Widgets
        Qt::Network
        Qt::Svg
        Qt::Core5Compat
        Qt::WebEngineWidgets
        SingleApplication::SingleApplication
        ${PLATFORM_LIBS}
)

include(CTest)

if(BUILD_TESTING)
    add_executable(applicationlogger_test
        tests/applicationlogger_test.cpp
        application/applicationlogger.cpp
    )
    target_include_directories(applicationlogger_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(applicationlogger_test Qt::Core)
    add_test(NAME applicationlogger_test COMMAND applicationlogger_test)

    add_executable(corelogfile_test
        tests/corelogfile_test.cpp
        infrastructure/corelogfile.cpp
    )
    target_include_directories(corelogfile_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(corelogfile_test Qt::Core)
    add_test(NAME corelogfile_test COMMAND corelogfile_test)

    add_executable(corecommandbuilder_test
        tests/corecommandbuilder_test.cpp
        core/corecommandbuilder.cpp
    )
    target_include_directories(corecommandbuilder_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(corecommandbuilder_test Qt::Core)
    add_test(NAME corecommandbuilder_test COMMAND corecommandbuilder_test)

    add_executable(coreoutputparser_test
        tests/coreoutputparser_test.cpp
        core/coreoutputparser.cpp
    )
    target_include_directories(coreoutputparser_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(coreoutputparser_test Qt::Core)
    add_test(NAME coreoutputparser_test COMMAND coreoutputparser_test)

    add_executable(coreoutputbuffer_test
        tests/coreoutputbuffer_test.cpp
        core/coreoutputbuffer.cpp
    )
    target_include_directories(coreoutputbuffer_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(coreoutputbuffer_test Qt::Core)
    add_test(NAME coreoutputbuffer_test COMMAND coreoutputbuffer_test)

    add_executable(settingsprofileloader_test
        tests/settingsprofileloader_test.cpp
        infrastructure/settingsprofileloader.cpp
    )
    target_include_directories(settingsprofileloader_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(settingsprofileloader_test Qt::Core)
    add_test(NAME settingsprofileloader_test COMMAND settingsprofileloader_test)

    add_executable(connectionsessionstate_test
        tests/connectionsessionstate_test.cpp
        core/connectionsessionstate.cpp
    )
    target_include_directories(connectionsessionstate_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(connectionsessionstate_test Qt::Core)
    add_test(NAME connectionsessionstate_test COMMAND connectionsessionstate_test)

    add_executable(systemproxysession_test
        tests/systemproxysession_test.cpp
        application/systemproxysession.cpp
        infrastructure/platformsystemproxybackend.cpp
        utils/proxysettings.cpp
    )
    target_include_directories(systemproxysession_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(systemproxysession_test Qt::Core Qt::Concurrent Qt::Network Qt::Widgets ${PLATFORM_LIBS})
    add_test(NAME systemproxysession_test COMMAND systemproxysession_test)
endif()
