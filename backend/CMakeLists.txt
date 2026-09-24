cmake_minimum_required(VERSION 3.10)
project(StudentResultSystem)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Fetch nlohmann_json
include(FetchContent)

FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
)

FetchContent_MakeAvailable(nlohmann_json)

# Fetch Asio
include(FetchContent)

FetchContent_Declare(
  asio
  URL https://github.com/chriskohlhoff/asio/archive/refs/tags/asio-1-36-0.zip
)

FetchContent_MakeAvailable(asio)

set(ASIO_INCLUDE_DIR "${asio_SOURCE_DIR}/asio/include" CACHE PATH "Asio include directory" FORCE)

# Fetch Crow
FetchContent_Declare(
  crow
  GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
  GIT_TAG v1.3.3
)
FetchContent_MakeAvailable(crow)

# MySQL C client. Override MYSQL_DIR when MySQL is installed elsewhere.
set(MYSQL_DIR "C:/Program Files/MySQL/MySQL Server 8.0" CACHE PATH "MySQL installation directory")
find_path(MYSQL_INCLUDE_DIR mysql.h
  HINTS "${MYSQL_DIR}/include")
find_library(MYSQL_LIBRARY NAMES mysqlclient libmysql
  HINTS "${MYSQL_DIR}/lib")
if(NOT MYSQL_INCLUDE_DIR OR NOT MYSQL_LIBRARY)
  message(FATAL_ERROR "MySQL client not found. Install MySQL Server/Connector and set MYSQL_DIR to its installation directory.")
endif()

# Shared source files
set(COMMON_SOURCES
  src/database.cpp
  src/auth.cpp
  src/student.cpp
  src/faculty.cpp
  src/marks.cpp
  src/results.cpp
)

# The normal executable is the terminal application. The existing Crow API
# remains available as a separate web_server executable.
add_executable(server src/terminal.cpp ${COMMON_SOURCES})
add_executable(web_server src/main.cpp ${COMMON_SOURCES})

foreach(target server web_server)
  target_include_directories(${target} PRIVATE ${MYSQL_INCLUDE_DIR})
  target_link_libraries(${target}
    PRIVATE
    Crow::Crow
    nlohmann_json::nlohmann_json
    ${MYSQL_LIBRARY}
  )

  if(WIN32)
    target_link_libraries(${target} PRIVATE ws2_32)
  endif()

  set_target_properties(${target} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
  )
endforeach()
