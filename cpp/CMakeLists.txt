cmake_minimum_required(VERSION 3.9)

project(arrow-dotnet-c-interop VERSION 1.0.0 DESCRIPTION "Example shared library exposing Arrow data")

find_package(Arrow REQUIRED)
find_package(Parquet REQUIRED)
find_package(ArrowDataset REQUIRED)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE Release)

message(STATUS "Arrow version: ${ARROW_VERSION}")
message(STATUS "Arrow SO version: ${ARROW_FULL_SO_VERSION}")

add_library(arrow-dotnet-c-interop SHARED
    src/lib.cpp
)

set_target_properties(arrow-dotnet-c-interop PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(arrow-dotnet-c-interop PROPERTIES SOVERSION 1)
set_target_properties(arrow-dotnet-c-interop PROPERTIES PUBLIC_HEADER include/api.h)

target_include_directories(arrow-dotnet-c-interop PRIVATE include)

target_link_libraries(arrow-dotnet-c-interop PRIVATE arrow_shared)
target_link_libraries(arrow-dotnet-c-interop PRIVATE parquet_shared)
target_link_libraries(arrow-dotnet-c-interop PRIVATE arrow_dataset_shared)

include(GNUInstallDirs)

install(TARGETS arrow-dotnet-c-interop
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
