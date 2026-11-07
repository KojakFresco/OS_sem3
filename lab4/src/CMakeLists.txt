add_executable(lab4 main.c)

set_target_properties(lab4 PROPERTIES OUTPUT_NAME "main")

add_library(child STATIC child.c)
target_include_directories(child PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(parent STATIC parent.c)
target_include_directories(parent PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(lab4 PRIVATE child parent)