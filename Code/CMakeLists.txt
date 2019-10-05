cmake_minimum_required(VERSION 3.4)
project(ApoapseCore LANGUAGES CXX)
set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lstdc++fs")
	
include_directories("../ThirdParty/openssl/include/")
include_directories("../ThirdParty/cryptopp/")
include_directories("../ThirdParty/boost_1_71_0_LINUX/")
include_directories("../ThirdParty/magic_enum/include/")

set(OUTPUT_RELEASE ../../BinLinux/)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_RELEASE}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_RELEASE}")
set(CMAKE_EXECUTABLE_OUTPUT_DIRECTORY "${OUTPUT_RELEASE}")

# Source Files
set(SRC_FILES
    ./ApoapseData.cpp
    ./ApoapseError.cpp
    ./ApoapseOpenSSL.cpp
    ./ApoapseOperation.cpp
    ./CommandsManagerV2.cpp
    ./CommandV2.cpp
    ./CustomDataType.cpp
    ./DatabaseIntegrityPatcher.cpp
    ./DatabaseSettings.cpp
    ./DataStructure.cpp
    ./FileStreamConnection.cpp
    ./FileUtils.cpp
    ./ProtectedByteContainer.cpp
    ./StringExtensions.cpp
    ./TCPConnectionNoTLS.cpp
    ./TCPServerNoTLS.cpp
    ./ThreadUtils.cpp
    ./UsergroupManager.cpp
    ./Username.cpp
    ./UT_Cryptography.cpp
    ./UT_MessagePack.cpp
    ./UT_ProtectedByteContainer.cpp
    ./UT_Range.cpp
    ./UT_Uuid.cpp
    ./Uuid.cpp
    ./SQLQuery.cpp
    ./TCPServer.cpp
    ./UT_ByteUtils.cpp
    ./UT_DateTimeUtils.cpp
    ./UT_Maths.cpp
    ./UT_NetworkPayload.cpp
    ./UT_StringExtensions.cpp
    ./Common.cpp
    ./DateTimeUtils.cpp
    ./GenericConnection.cpp
    ./Log.cpp
    ./Logger.cpp
    ./NetworkPayload.cpp
    ./stdafx.cpp
    ./TCPConnection.cpp
    ./ThreadPool.cpp
    ./UnitTestsManager.cpp
)
source_group("Sources" FILES ${SRC_FILES})

# Header Files
set(HEADERS_FILES
    ApoapseError.h
    ApoapseErrors.hpp
    ApoapseOpenSSL.h
    DatabaseSettings.h
    FileStreamConnection.h
    FileUtils.h
    IUser.h
    AES.hpp
    ApoapseData.h
    ApoapseOperation.h
    CommandsDef.hpp
    CommandsManagerV2.h
    CommandV2.h
    CustomDataType.h
    ICustomDataType.hpp
    DatabaseIntegrityPatcher.h
    DataStructure.h
    DataStructures.hpp
    Hash.hpp
    CryptographyTypes.hpp
    HMAC.hpp
    ICommandManager.hpp
    IHtmlUI.hpp
    Json.hpp
    ProtectedByteContainer.h
    GlobalVarDefines.hpp
    Random.hpp
    RSA.hpp
    ByteUtils.hpp
    Common.h
    Database.hpp
    DateTimeUtils.h
    Diagnostic.hpp
    eLogSeverity.hpp
    DllExportSymbolApi.hpp
    GenericConnection.h
    ILogger.hpp
    INetworkSender.h
    LibraryLoader.hpp
    Log.h
    Logger.h
    MacroUtils.hpp
    Maths.hpp
    MemoryUtils.hpp
    NetworkPayload.h
    ObjectsFactory.hpp
    Range.hpp
    ReadBufferSize.hpp
    SQLPackagedResult.hpp
    SQLQuery.h
    SQLUtils.hpp
    SQLValue.hpp
    SQLValueType.hpp
    stdafx.h
    StringExtensions.h
    TCPConnection.h
    TCPConnectionNoTLS.h
    TCPServer.h
    TCPServerNoTLS.h
    ThreadPool.h
    ThreadUtils.h
    TypeDefs.hpp
    MessagePack.hpp
    UnitTest.hpp
    UnitTestsManager.h
    UsergroupManager.h
    Username.h
    Uuid.h
)
source_group("Headers" FILES ${HEADERS_FILES})

ADD_LIBRARY(${PROJECT_NAME} ${SRC_FILES} ${HEADERS_FILES})

set_property(TARGET ApoapseCore PROPERTY CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")