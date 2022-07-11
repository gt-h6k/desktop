# most Windows SDK tools require native pathes (with backslashes)
file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CustomStateProvider.g.h" MidlOutputPath)
file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CustomStateProvider.tlb" MidlOutputPathTlb)
file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CustomStateProvider.winmd" MidlOutputPathWinmd)
file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Generated" GeneratedDirBackSlash)
file(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR} MidlOutputPathGeneral)

add_custom_target(CustomStateProviderImpl
   DEPENDS ${MidlOutputPath}
)

function(GetSubList resultVar)
  set(result)
  foreach(ITR ${ARGN})  # ARGN holds all arguments to function after last named one
    if(ITR MATCHES "10\.0\.\d\d\d\d\d\.\d")
      list(APPEND result ${ITR})
    endif()
  endforeach()
  set(${resultVar} ${result} PARENT_SCOPE)
endfunction()

if(NOT DEFINED ENV{WindowsSdkDir1})
    message("Getting WindowsSdkDir from Registry")
    get_filename_component(WindowsSdkDir "[HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows Kits\\Installed Roots;KitsRoot10]" ABSOLUTE)
    # get_filename_component always strips trailing slash, but, we need it present
    string(APPEND WindowsSdkDir "/")
else()
    message("Setting WindowsSdkDir from ENV{WindowsSdkDir}")
    set(WindowsSdkDir $ENV{WindowsSdkDir})
endif()

# list all subdirs under WindowsSdkDir
MACRO(SUBDIRLIST result curdir)
  FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
  SET(dirlist "")
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${curdir}/${child})
      LIST(APPEND dirlist ${child})
    ENDIF()
  ENDFOREACH()
  SET(${result} ${dirlist})
ENDMACRO()

SUBDIRLIST(SdkList "${WindowsSdkDir}/bin")

# pick only dirs that start with 10.0
list(FILTER SdkList INCLUDE REGEX "10.0.")
# sort the list of subdirs and choose the latest
list(SORT SdkList COMPARE NATURAL)
list(GET SdkList -1 WindowsSdkLatest)

message("WindowsSdkLatest has been set to: ${WindowsSdkLatest}")

if(NOT WindowsSdkLatest)
    message( FATAL_ERROR "Windows SDK not found")
endif()

SUBDIRLIST(listFoundationContracts "${WindowsSdkDir}/References/${WindowsSdkLatest}/Windows.Foundation.FoundationContract")
list(FILTER listFoundationContracts INCLUDE REGEX "[0-9]+\.")
list(SORT listFoundationContracts COMPARE NATURAL)
list(GET listFoundationContracts -1 WindowsFoundationContractVersion)
message("WindowsFoundationContractVersion has been set to: ${WindowsFoundationContractVersion}")

if(NOT WindowsFoundationContractVersion)
    message( FATAL_ERROR "Windows Foundation Contract is not found in ${WindowsSdkLatest} SDK.")
endif()

SUBDIRLIST(listCloudFilesContracts "${WindowsSdkDir}/References/${WindowsSdkLatest}/Windows.Storage.Provider.CloudFilesContract")
list(FILTER listCloudFilesContracts INCLUDE REGEX "[0-9]+\.")
list(SORT listCloudFilesContracts COMPARE NATURAL)
list(GET listCloudFilesContracts -1 WindowsStorageProviderCloudFilesContractVersion)
message("WindowsStorageProviderCloudFilesContractVersion has been set to: ${WindowsStorageProviderCloudFilesContractVersion}")

if(NOT WindowsStorageProviderCloudFilesContractVersion)
    message( FATAL_ERROR "Windows Storage Provider Cloud Files Contract is not found in ${WindowsSdkLatest} SDK.")
endif()

file(TO_NATIVE_PATH ${WindowsSdkDir} WindowsSdkDir)

message("WindowsSdkDir has been set to: ${WindowsSdkDir}")
message("SdkList has been set to: ${SdkList}")
message("WindowsSdkLatest has been set to: ${WindowsSdkLatest}")

set(TargetPlatform "x64")
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(TargetPlatform "x64")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(TargetPlatform "x86")
endif()

set(WindowsSDKReferencesPath "${WindowsSdkDir}References\\${WindowsSdkLatest}")

set(WindowsSDKBinPathForTools "${WindowsSdkDir}bin\\${WindowsSdkLatest}\\${TargetPlatform}")

set(WindowsSDKMetadataDirectory "${WindowsSdkDir}UnionMetadata\\${WindowsSdkLatest}")

IF(NOT EXISTS "${WindowsSDKReferencesPath}" OR NOT IS_DIRECTORY "${WindowsSDKReferencesPath}")
    message( FATAL_ERROR "Please install Windows SDK ${WindowsSdkLatest}")
ENDIF()
IF(NOT EXISTS "${WindowsSDKMetadataDirectory}" OR NOT IS_DIRECTORY "${WindowsSDKMetadataDirectory}")
    message( FATAL_ERROR "Please install Windows SDK ${WindowsSdkLatest}")
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
