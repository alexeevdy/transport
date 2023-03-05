file(GLOB source_files CONFIGURE_DEPENDS "*.cpp")

add_library(transport ${source_files})

target_include_directories(transport
        INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
        )