cmake_minimum_required(VERSION 3.19)
project(codeIntegrity LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ==================== Qt6 ====================

set(QT_VERSION_CANDIDATES
    "$ENV{QTDIR}"
    "$ENV{Qt6_DIR}"
    "C:/Qt/6.10.1/mingw_64/lib/cmake/Qt6"
    "C:/Qt/6.9.0/mingw_64/lib/cmake/Qt6"
    "C:/Qt/6.8.0/mingw_64/lib/cmake/Qt6"
    "C:/Qt/6.7.0/mingw_64/lib/cmake/Qt6"
    "C:/Qt/6.6.0/mingw_64/lib/cmake/Qt6"
    "C:/Qt/6.5.0/mingw_64/lib/cmake/Qt6"
    "D:/Qt/6.10.1/mingw_64/lib/cmake/Qt6"
    "D:/Qt/6.9.0/mingw_64/lib/cmake/Qt6"
    "D:/Qt/6.8.0/mingw_64/lib/cmake/Qt6"
)

if(NOT Qt6_DIR)
    foreach(candidate ${QT_VERSION_CANDIDATES})
        if(EXISTS "${candidate}/Qt6Config.cmake")
            set(Qt6_DIR "${candidate}")
            break()
        endif()
    endforeach()
endif()

if(Qt6_DIR)
    message(STATUS "Qt6_DIR: ${Qt6_DIR}")
else()
    message(WARNING "Qt6_DIR no encontrado automáticamente. "
                    "Defínelo manualmente: -DQt6_DIR=<ruta>")
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql)
qt_standard_project_setup()

# ==================== PostgreSQL ====================

set(PG_SEARCH_PATHS
    "$ENV{PGROOT}"
    "$ENV{PostgreSQL_ROOT}"
    "C:/Program Files/PostgreSQL/17"
    "C:/Program Files/PostgreSQL/16"
    "C:/Program Files/PostgreSQL/15"
    "C:/Program Files (x86)/PostgreSQL/17"
    "D:/PostgreSQL/17"
    "D:/Program Files/PostgreSQL/17"
)

if(NOT PostgreSQL_ROOT)
    foreach(path ${PG_SEARCH_PATHS})
        if(EXISTS "${path}/include/libpq-fe.h")
            set(PostgreSQL_ROOT "${path}")
            break()
        endif()
    endforeach()
endif()

if(NOT PostgreSQL_ROOT)
    message(FATAL_ERROR
        "PostgreSQL no encontrado.\n"
        "Defínelo manualmente: -DPostgreSQL_ROOT=<ruta>\n"
        "O establece la variable de entorno PGROOT."
    )
endif()

message(STATUS "PostgreSQL: ${PostgreSQL_ROOT}")

set(PostgreSQL_INCLUDE_DIRS "${PostgreSQL_ROOT}/include")
set(PostgreSQL_LIBRARIES    "${PostgreSQL_ROOT}/lib/libpq.lib")

if(NOT EXISTS "${PostgreSQL_INCLUDE_DIRS}/libpq-fe.h")
    message(FATAL_ERROR "libpq-fe.h no encontrado en: ${PostgreSQL_INCLUDE_DIRS}")
endif()

if(NOT EXISTS "${PostgreSQL_LIBRARIES}")
    message(FATAL_ERROR "libpq.lib no encontrado en: ${PostgreSQL_ROOT}/lib")
endif()

# ==================== OpenSSL ====================

set(OPENSSL_SEARCH_PATHS
    "$ENV{OPENSSL_ROOT_DIR}"
    "${PostgreSQL_ROOT}"
    "C:/Program Files/OpenSSL-Win64"
    "C:/Program Files/OpenSSL"
    "C:/OpenSSL-Win64"
    "D:/OpenSSL-Win64"
)

if(NOT OPENSSL_ROOT_DIR)
    foreach(path ${OPENSSL_SEARCH_PATHS})
        if(EXISTS "${path}/include/openssl/ssl.h")
            set(OPENSSL_ROOT_DIR "${path}")
            break()
        endif()
    endforeach()
endif()

if(OPENSSL_ROOT_DIR)
    message(STATUS "OpenSSL: ${OPENSSL_ROOT_DIR}")
    find_package(OpenSSL QUIET)
endif()

if(NOT OPENSSL_FOUND)
    message(WARNING "OpenSSL no encontrado. El hashing de contraseñas puede no funcionar.")
endif()

# ==================== Fuentes ====================

file(GLOB_RECURSE PROJECT_SOURCES
    "${CMAKE_SOURCE_DIR}/src/*.hpp"
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/*.ui"
)

if(NOT PROJECT_SOURCES)
    message(FATAL_ERROR "No se encontraron archivos fuente en src/")
endif()

list(LENGTH PROJECT_SOURCES NUM_FILES)
message(STATUS "Archivos fuente: ${NUM_FILES}")

# ==================== Ejecutable ====================

qt_add_executable(codeIntegrity WIN32 ${PROJECT_SOURCES})

target_include_directories(codeIntegrity PRIVATE
    ${PostgreSQL_INCLUDE_DIRS}
)

set(LINK_LIBS Qt6::Core Qt6::Widgets Qt6::Sql ${PostgreSQL_LIBRARIES})
if(OPENSSL_FOUND)
    list(APPEND LINK_LIBS OpenSSL::Crypto OpenSSL::SSL)
endif()

target_link_libraries(codeIntegrity PRIVATE ${LINK_LIBS})

# ==================== Deploy ====================

if(WIN32)
    set(PG_DLL "${PostgreSQL_ROOT}/bin/libpq.dll")
    if(EXISTS "${PG_DLL}")
        add_custom_command(TARGET codeIntegrity POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${PG_DLL}"
                $<TARGET_FILE_DIR:codeIntegrity>
            COMMENT "Copiando libpq.dll"
        )
    else()
        message(WARNING "libpq.dll no encontrado en: ${PG_DLL}")
    endif()

    qt_generate_deploy_app_script(
        TARGET codeIntegrity
        OUTPUT_SCRIPT deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR
    )
endif()

# ==================== Instalación ====================

include(GNUInstallDirs)
install(TARGETS codeIntegrity RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

if(WIN32 AND DEFINED deploy_script AND deploy_script)
    install(SCRIPT ${deploy_script})
endif()
