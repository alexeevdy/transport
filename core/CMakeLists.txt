add_library(core svg.cpp json.cpp descriptions.cpp transport_router.cpp transport_render.cpp transport_guide.cpp request.cpp)

target_include_directories(core
        INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
        )