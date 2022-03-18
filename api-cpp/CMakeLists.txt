# Use an absolute reference here so that doxygen can locate in the doc context by target
set(API_CPP_PUBLIC_HEADERS
    ${CMAKE_BINARY_DIR}/include/dfx/api/CloudAPI_Export.hpp
    ${CMAKE_SOURCE_DIR}/api-cpp/include/dfx/api/CloudAPI.hpp
    ${CMAKE_SOURCE_DIR}/api-cpp/include/dfx/api/CloudConfig.hpp
    ${CMAKE_SOURCE_DIR}/api-cpp/include/dfx/api/CloudLog.hpp
    ${CMAKE_SOURCE_DIR}/api-cpp/include/dfx/api/CloudStatus.hpp
    ${CMAKE_SOURCE_DIR}/api-cpp/include/dfx/api/CloudTypes.hpp
    ${CMAKE_SOURCE_DIR}/api-cpp/include/dfx/api/DeviceAPI.hpp
    ${CMAKE_SOURCE_DIR}/api-cpp/include/dfx/api/LicenseAPI.hpp
    ${CMAKE_SOURCE_DIR}/api-cpp/include/dfx/api/MeasurementAPI.hpp
    ${CMAKE_SOURCE_DIR}/api-cpp/include/dfx/api/MeasurementStreamAPI.hpp
    ${CMAKE_SOURCE_DIR}/api-cpp/include/dfx/api/OrganizationAPI.hpp
    ${CMAKE_SOURCE_DIR}/api-cpp/include/dfx/api/ProfileAPI.hpp
    ${CMAKE_SOURCE_DIR}/api-cpp/include/dfx/api/SignalAPI.hpp
    ${CMAKE_SOURCE_DIR}/api-cpp/include/dfx/api/StudyAPI.hpp
    ${CMAKE_SOURCE_DIR}/api-cpp/include/dfx/api/UserAPI.hpp
    ${CMAKE_SOURCE_DIR}/api-cpp/include/dfx/api/types/DeviceTypes.hpp
    ${CMAKE_SOURCE_DIR}/api-cpp/include/dfx/api/types/LicenseTypes.hpp
    ${CMAKE_SOURCE_DIR}/api-cpp/include/dfx/api/types/MeasurementTypes.hpp
    ${CMAKE_SOURCE_DIR}/api-cpp/include/dfx/api/types/OrganizationTypes.hpp
    ${CMAKE_SOURCE_DIR}/api-cpp/include/dfx/api/types/ProfileTypes.hpp
    ${CMAKE_SOURCE_DIR}/api-cpp/include/dfx/api/types/SignalTypes.hpp
    ${CMAKE_SOURCE_DIR}/api-cpp/include/dfx/api/types/StudyTypes.hpp
    ${CMAKE_SOURCE_DIR}/api-cpp/include/dfx/api/types/UserTypes.hpp)

add_library(
  api-cpp OBJECT
  src/CloudAPI.cpp
  src/CloudConfig.cpp
  src/CloudLog.cpp
  src/CloudStatus.cpp
  src/CloudTypes.cpp
  src/DeviceAPI.cpp
  src/LicenseAPI.cpp
  src/MeasurementAPI.cpp
  src/MeasurementStreamAPI.cpp
  src/OrganizationAPI.cpp
  src/ProfileAPI.cpp
  src/SignalAPI.cpp
  src/StudyAPI.cpp
  src/UserAPI.cpp
  src/DeviceTypes.cpp
  src/LicenseTypes.cpp
  src/MeasurementTypes.cpp
  src/OrganizationTypes.cpp
  src/ProfileTypes.cpp
  src/SignalTypes.cpp
  src/StudyTypes.cpp
  src/UserTypes.cpp
  $<$<BOOL:${EMBED_CA_CERTS}>:cacert.c>
  ${CMAKE_BINARY_DIR}/version_info.cpp
  ${API_CPP_PUBLIC_HEADERS})

if(MSVC)
  # Silence the Microsoft compiler warnings C4996/STL4015, for generated gRPC code when building with C++17
  if(CMAKE_CXX_STANDARD GREATER_EQUAL 17)
    target_compile_definitions(api-cpp PUBLIC "-D_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING")
  endif()
  if(BUILD_SHARED_LIBS)
    # Warning C4251 complaining about std::string on a DLL boundary
    target_compile_options(api-cpp PUBLIC /wd4251)
  endif()
  target_compile_options(api-cpp PUBLIC /utf-8)
endif()

target_compile_definitions(api-cpp PRIVATE -Ddfxapi_EXPORTS)

target_include_directories(
  api-cpp
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include> # For CloudAPI_Export.hpp
    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/> # For version_info.hpp
    $<BUILD_INTERFACE:$<TARGET_PROPERTY:api-cpp-validator,INTERFACE_INCLUDE_DIRECTORIES>> # stub if not
                                                                                          # WITH_VALIDATORS
    $<$<BOOL:${WITH_GRPC}>:$<BUILD_INTERFACE:$<TARGET_PROPERTY:api-cpp-grpc,INTERFACE_INCLUDE_DIRECTORIES>>>
    $<$<BOOL:${WITH_REST}>:$<BUILD_INTERFACE:$<TARGET_PROPERTY:api-cpp-rest,INTERFACE_INCLUDE_DIRECTORIES>>>
    $<$<BOOL:${WITH_WEBSOCKET}>:$<BUILD_INTERFACE:$<TARGET_PROPERTY:api-cpp-websocket,INTERFACE_INCLUDE_DIRECTORIES>>>
)

set_target_properties(api-cpp PROPERTIES PUBLIC_HEADER "${API_CPP_PUBLIC_HEADERS}")

target_link_libraries(api-cpp PRIVATE $<$<BOOL:${WITH_YAML}>:yaml-cpp::yaml-cpp> nlohmann_json::nlohmann_json)

if(EMBED_CA_CERTS)
  embed_file("cacert" "${CMAKE_SOURCE_DIR}/resources/cacert.pem" cacert.c)

  # embed_file("cacert" "${CMAKE_BINARY_DIR}/res/cacert.pem" cacert.c)
  target_compile_definitions(api-cpp PRIVATE EMBED_CA_CERTS)
endif(EMBED_CA_CERTS)

if(WITH_CURL)
  target_compile_definitions(api-cpp PRIVATE WITH_CURL)
endif(WITH_CURL)

if(WITH_GRPC)
  target_compile_definitions(api-cpp PRIVATE WITH_GRPC)
endif(WITH_GRPC)

if(WITH_REST)
  target_compile_definitions(api-cpp PRIVATE WITH_REST)
endif(WITH_REST)

if(WITH_WEBSOCKET)
  target_compile_definitions(api-cpp PRIVATE WITH_WEBSOCKET)
endif(WITH_WEBSOCKET)

if(WITH_YAML)
  target_compile_definitions(api-cpp PRIVATE WITH_YAML)
endif(WITH_YAML)

install(TARGETS api-cpp PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dfx/api)