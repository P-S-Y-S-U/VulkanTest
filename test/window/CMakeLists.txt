add_executable(WindowTest window_test.cpp)
target_link_libraries(WindowTest PUBLIC $<BUILD_INTERFACE:vulkanrenderer>)
