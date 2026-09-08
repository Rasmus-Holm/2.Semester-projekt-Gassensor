cmake_minimum_required(VERSION 3.10)
project(MQTT_Project)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


include(FetchContent)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz)
FetchContent_MakeAvailable(json)

FetchContent_Declare(
    c_questdb_client_proj
    GIT_REPOSITORY https://github.com/questdb/c-questdb-client.git
    GIT_TAG 5.1.0)   # CHANGE ME!

FetchContent_MakeAvailable(c_questdb_client_proj)

# 1. Find Paho pakken på systemet
# Dette erstatter både find_library og find_path
find_package(PahoMqttCpp REQUIRED)


# Find biblioteket manuelt
#find_library(MARIADBCPP_LIB NAMES mariadbcpp)
# Find header-stien
#find_path(MARIADBCPP_INCLUDE_DIR NAMES mariadb/conncpp.hpp)

#if(NOT MARIADBCPP_LIB OR NOT MARIADBCPP_INCLUDE_DIR)
 #   message(FATAL_ERROR "Kunne ikke finde MariaDB Connector C++. Tjek installationen!")
#endif()


# --- APP 2: Subscriber ---
add_executable(mqtt_sub ./src/main.cpp)
#target_include_directories(mqtt_sub PUBLIC src ${MARIADBCPP_INCLUDE_DIR})
target_link_libraries(mqtt_sub PRIVATE PahoMqttCpp::paho-mqttpp3 PahoMqttCpp::paho-mqttpp3 questdb_client)