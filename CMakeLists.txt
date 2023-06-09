cmake_minimum_required(VERSION 3.14.0)
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.13 CACHE STRING "")

set(vst3sdk_SOURCE_DIR "C:/Users/ericr/source/VST_SDK/vst3sdk")
if(NOT vst3sdk_SOURCE_DIR)
    message(FATAL_ERROR "Path to VST3 SDK is empty!")
endif()

project(Morsalyzer
    # This is your plug-in version number. Change it here only.
    # Version number symbols usable in C++ can be found in
    # source/version.h and ${PROJECT_BINARY_DIR}/projectversion.h.
    VERSION 0.0.0.0 
    DESCRIPTION "Morsalyzer VST 3 Plug-in"
)

set(SMTG_VSTGUI_ROOT "${vst3sdk_SOURCE_DIR}")

add_subdirectory(${vst3sdk_SOURCE_DIR} ${PROJECT_BINARY_DIR}/vst3sdk)
smtg_enable_vst3_sdk()

smtg_add_vst3plugin(Morsalyzer
    source/version.h
    source/myplugincids.h
    source/mypluginprocessor.h
    source/mypluginprocessor.cpp
    source/myplugincontroller.h
    source/myplugincontroller.cpp
    source/mypluginentry.cpp
    source/uimessagecontroller.h
)

#- VSTGUI Wanted ----
if(SMTG_ADD_VSTGUI)
    target_sources(Morsalyzer
        PRIVATE
            resource/myplugineditor.uidesc
    )
    target_link_libraries(Morsalyzer
        PRIVATE
            vstgui_support
    )
    smtg_target_add_plugin_resources(Morsalyzer
        RESOURCES
            "resource/myplugineditor.uidesc"
    )
endif(SMTG_ADD_VSTGUI)
# -------------------

smtg_target_add_plugin_snapshots (Morsalyzer
    RESOURCES
        resource/F6686E71FB6C5B4CBFA6CCDCD705EAE2_snapshot.png
        resource/F6686E71FB6C5B4CBFA6CCDCD705EAE2_snapshot_2.0x.png
)

target_link_libraries(Morsalyzer
    PRIVATE
        sdk
)

smtg_target_configure_version_file(Morsalyzer)

if(SMTG_MAC)
    smtg_target_set_bundle(Morsalyzer
        BUNDLE_IDENTIFIER com.baronvonwentz.morsalyzer
        COMPANY_NAME "Baron von Wenzelheimer"
    )
    smtg_target_set_debug_executable(Morsalyzer
        "/Applications/VST3PluginTestHost.app"
        "--pluginfolder;$(BUILT_PRODUCTS_DIR)"
    )
elseif(SMTG_WIN)
    target_sources(Morsalyzer PRIVATE 
        resource/win32resource.rc
    )
    if(MSVC)
        set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Morsalyzer)

        smtg_target_set_debug_executable(Morsalyzer
            "$(ProgramW6432)/Steinberg/VST3PluginTestHost/VST3PluginTestHost.exe"
            "--pluginfolder \"$(OutDir)/\""
        )
    endif()
endif(SMTG_MAC)
