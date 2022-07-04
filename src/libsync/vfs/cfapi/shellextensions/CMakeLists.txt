if (WIN32)
    target_sources(nextcloudsync_vfs_cfapi PRIVATE contextmenus.cpp customstateprovider.cpp thumbnailprovider.cpp ShellServices.cpp)

    file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CustomStateProvider.g.h" MidlOutputPath)
    file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CustomStateProvider.tlb" MidlOutputPathTlb)
    file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CustomStateProvider.winmd" MidlOutputPathWinmd)
    file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Generated" GeneratedDirBackSlash)
    file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}" MidlOutputPathGeneral)

add_custom_target(CfApiCustomStateProviderImpl
   DEPENDS ${MidlOutputPath}
)

add_dependencies(nextcloudsync_vfs_cfapi CfApiCustomStateProviderImpl)

target_include_directories(nextcloudsync_vfs_cfapi PRIVATE ${GeneratedDirBackSlash})

target_compile_features(nextcloudsync_vfs_cfapi PRIVATE cxx_std_17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /await")

set(sdk_metadata_directory "D:\\Windows Kits\\10\\UnionMetadata\\10.0.22621.0")

add_custom_command(OUTPUT ${MidlOutputPath}
   COMMAND midl /winrt /h nul /tlb ${MidlOutputPathTlb} /winmd ${MidlOutputPathWinmd} /metadata_dir "D:\\Windows Kits\\10\\References\\10.0.22621.0\\Windows.Foundation.FoundationContract\\4.0.0.0" /nomidl /reference "D:\\Windows Kits\\10\\References\\10.0.22621.0\\Windows.Foundation.FoundationContract\\4.0.0.0\\Windows.Foundation.FoundationContract.winmd" /reference "D:\\Windows Kits\\10\\References\\10.0.22621.0\\Windows.Storage.Provider.CloudFilesContract\\7.0.0.0\\Windows.Storage.Provider.CloudFilesContract.winmd" /I ${MidlOutputPathGeneral} CustomStateProvider.idl
   COMMAND cppwinrt -in ${MidlOutputPathWinmd} -comp ${MidlOutputPathGeneral} -pch pch.h -ref ${sdk_metadata_directory} -out ${GeneratedDirBackSlash} -verbose
   COMMENT "Generating CustomStateProvider.g.h from ${MidlOutputPath}"
)
endif()
