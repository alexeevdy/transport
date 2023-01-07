add_library(core json.cpp descriptions.cpp request.cpp transport_router.cpp transport_guide.cpp)

target_include_directories(core
        INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
        )