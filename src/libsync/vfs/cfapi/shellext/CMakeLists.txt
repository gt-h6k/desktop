# most Windows SDK tools require native pathes (with backslashes)
file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CustomStateProvider.g.h" MidlOutputPath)
file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CustomStateProvider.tlb" MidlOutputPathTlb)
file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CustomStateProvider.winmd" MidlOutputPathWinmd)
file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Generated" GeneratedDirBackSlash)
file(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR} MidlOutputPathGeneral)

add_custom_target(CustomStateProviderImpl
   DEPENDS ${MidlOutputPath}
)

if(NOT DEFINED ENV{WindowsSdkDir1})
    message("Getting WindowsSdkDir from Registry")
    get_filename_component(WindowsSdkDir "[HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows Kits\\Installed Roots;KitsRoot10]" ABSOLUTE)
    # get_filename_component always strips trailing slash, but, we need it present
    string(APPEND WindowsSdkDir "/")
else()
    message("Setting WindowsSdkDir from ENV{WindowsSdkDir}")
    set(WindowsSdkDir $ENV{WindowsSdkDir})
endif()
file(TO_NATIVE_PATH ${WindowsSdkDir} WindowsSdkDir)
message("WindowsSdkDir has been set to: ${WindowsSdkDir}")

set(TargetPlatform "x64")
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(TargetPlatform "x64")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(TargetPlatform "x86")
endif()

set(UCRTVersion "10.0.18362.0")
set(UCRTVersionForTools "10.0.22000.0")
set(WindowsFoundationContractVersion "3.0.0.0")
set(WindowsStorageProviderCloudFilesContractVersion "3.0.0.0")
set(WindowsSDKReferencesPath "${WindowsSdkDir}References\\${UCRTVersion}")
set(WindowsSDKBinPathForTools "${WindowsSdkDir}bin\\${UCRTVersionForTools}\\${TargetPlatform}")
set(WindowsSDKMetadataDirectory "${WindowsSdkDir}UnionMetadata\\${UCRTVersion}")

IF(NOT EXISTS "${WindowsSDKReferencesPath}" OR NOT IS_DIRECTORY "${WindowsSDKReferencesPath}")
    message( FATAL_ERROR "Please install Windows SDK ${UCRTVersion}")
ENDIF()
IF(NOT EXISTS "${WindowsSDKMetadataDirectory}" OR NOT IS_DIRECTORY "${WindowsSDKMetadataDirectory}")
    message( FATAL_ERROR "Please install Windows SDK ${UCRTVersion}")
ENDIF()
set(midlExe "${WindowsSDKBinPathForTools}\\midl.exe")
set(cppWinRtExe "${WindowsSDKBinPathForTools}\\cppwinrt.exe")

message("cppWinRtExe: ${cppWinRtExe}")
message("midlExe: ${midlExe}")

add_custom_command(OUTPUT ${MidlOutputPath}
   COMMAND ${midlExe} /winrt /h nul /tlb ${MidlOutputPathTlb} /winmd ${MidlOutputPathWinmd} /metadata_dir "${WindowsSDKReferencesPath}\\Windows.Foundation.FoundationContract\\${WindowsFoundationContractVersion}" /nomidl /reference "${WindowsSDKReferencesPath}\\Windows.Foundation.FoundationContract\\${WindowsFoundationContractVersion}\\Windows.Foundation.FoundationContract.winmd" /reference "${WindowsSDKReferencesPath}\\Windows.Storage.Provider.CloudFilesContract\\${WindowsStorageProviderCloudFilesContractVersion}\\Windows.Storage.Provider.CloudFilesContract.winmd" /I ${MidlOutputPathGeneral} CustomStateProvider.idl
   COMMAND ${cppWinRtExe} -in ${MidlOutputPathWinmd} -comp ${MidlOutputPathGeneral} -pch pch.h -ref ${WindowsSDKMetadataDirectory} -out ${GeneratedDirBackSlash} -verbose
   COMMENT "Generating CustomStateProvider.g.h from ${MidlOutputPath}"
)

add_library(CfApiShellExtensions MODULE
    dllmain.cpp
    cfapishellintegrationclassfactory.cpp
    thumbnailprovider.cpp
    contextmenus.cpp
    customstateprovider.cpp
    CfApiShellIntegration.def
)

add_dependencies(CfApiShellExtensions CustomStateProviderImpl)

target_link_libraries(CfApiShellExtensions shlwapi onecore onecoreuap Qt5::Core Qt5::Network Qt5::Gui)

target_include_directories(CfApiShellExtensions PRIVATE ${GeneratedDirBackSlash})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /await")

target_compile_features(CfApiShellExtensions PRIVATE cxx_std_17)



set_target_properties(CfApiShellExtensions
    PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY
            ${BIN_OUTPUT_DIRECTORY}
        RUNTIME_OUTPUT_DIRECTORY
            ${BIN_OUTPUT_DIRECTORY}
)

install(TARGETS CfApiShellExtensions 
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_BINDIR}
)
