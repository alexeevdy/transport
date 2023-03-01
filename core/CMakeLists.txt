file(GLOB source_files CONFIGURE_DEPENDS "*.cpp")

add_library(core ${source_files})

target_include_directories(core
        INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
        )