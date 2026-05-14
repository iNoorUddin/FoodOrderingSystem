cmake_minimum_required(VERSION 3.16)
project(FoodOrderingSystem VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Fix MySQL 'byte' conflict with Qt on Windows
add_compile_definitions(
    WIN32_LEAN_AND_MEAN
    NOGDI
    NOMINMAX
    _WINSOCKAPI_
)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# MySQL Connector/C — Windows paths
set(MYSQL_INCLUDE_DIR "C:/mysql-connector-c/mysql-connector-c-6.1.11-winx64")
set(MYSQL_LIBRARY "C:/mysql-connector-c/mysql-connector-c-6.1.11-winx64/lib/libmysql.lib")

set(SOURCES
    src/main.cpp
    src/Person.cpp
    src/Customer.cpp
    src/Admin.cpp
    src/MenuItem.cpp
    src/OrderItem.cpp
    src/Order.cpp
    src/DatabaseManager.cpp
    src/FoodService.cpp
    src/FileManager.cpp
    src/MainWindow.cpp
)

set(HEADERS
    include/Person.h
    include/Customer.h
    include/Admin.h
    include/MenuItem.h
    include/OrderItem.h
    include/Order.h
    include/DatabaseManager.h
    include/FoodService.h
    include/FileManager.h
    include/MainWindow.h
)

add_executable(FoodOrderingSystem
    ${SOURCES}
    ${HEADERS}
)

target_include_directories(FoodOrderingSystem PRIVATE
    include
    ${MYSQL_INCLUDE_DIR}/include
)

target_link_libraries(FoodOrderingSystem PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    ${MYSQL_LIBRARY}
)
