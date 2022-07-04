file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CustomStateProvider.g.h" MidlOutputPath)
file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CustomStateProvider.tlb" MidlOutputPathTlb)
file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CustomStateProvider.winmd" MidlOutputPathWinmd)
file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Generated" GeneratedDirBackSlash)
file(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR} MidlOutputPathGeneral)

add_custom_target(CustomStateProviderImpl
   DEPENDS ${MidlOutputPath}
)

set(WindowsSDK_IncludePath "D:\\Windows Kits\\10\\Include\\10.0.22621.0")
set(um_sdk_directory "${WindowsSDK_IncludePath}\\um")
set(shared_sdk_directory "${WindowsSDK_IncludePath}\\shared")
set(winrt_sdk_directory "${WindowsSDK_IncludePath}\\winrt")

set(sdk_metadata_directory "D:\\Windows Kits\\10\\UnionMetadata\\10.0.22621.0")
set(WindowsSDK_MetadataPath "D:\\Windows Kits\\10\\References")
set(WindowsSDK_MetadataPathVersioned "${WindowsSDK_MetadataPath}\\10.0.22621.0")
set(WindowsSDK_MetadataFoundationPath "${WindowsSDK_MetadataPathVersioned}\\Windows.Foundation.FoundationContract\\4.0.0.0")
set(WindowsSDK_MetadataCloudFilesPath "${WindowsSDK_MetadataPathVersioned}\\Windows.Storage.Provider.CloudFilesContract\\7.0.0.0")

add_custom_command(OUTPUT ${MidlOutputPath}
   COMMAND midl /winrt /h nul /tlb ${MidlOutputPathTlb} /winmd ${MidlOutputPathWinmd} /metadata_dir "D:\\Windows Kits\\10\\References\\10.0.22621.0\\Windows.Foundation.FoundationContract\\4.0.0.0" /nomidl /reference "D:\\Windows Kits\\10\\References\\10.0.22621.0\\Windows.Foundation.FoundationContract\\4.0.0.0\\Windows.Foundation.FoundationContract.winmd" /reference "D:\\Windows Kits\\10\\References\\10.0.22621.0\\Windows.Storage.Provider.CloudFilesContract\\7.0.0.0\\Windows.Storage.Provider.CloudFilesContract.winmd" /I ${MidlOutputPathGeneral} CustomStateProvider.idl
   COMMAND cppwinrt -in ${MidlOutputPathWinmd} -comp ${MidlOutputPathGeneral} -pch pch.h -ref ${sdk_metadata_directory} -out ${GeneratedDirBackSlash} -verbose
   COMMENT "Generating CustomStateProvider.g.h from ${MidlOutputPath}"
)

add_library(CfApiShellExtensions MODULE
    dllmain.cpp
    cfapishellintegrationclassfactory.cpp
    thumbnailprovider.cpp
    contextmenus.cpp
    customstateprovider.cpp
    logger.cpp
    CfApiShellIntegration.def
)

add_dependencies(CfApiShellExtensions CustomStateProviderImpl)

target_link_libraries(CfApiShellExtensions shlwapi)

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
