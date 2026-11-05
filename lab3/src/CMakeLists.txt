find_package(Threads REQUIRED)

add_executable(lab3 main.c)

add_library(PMergeSort p_merge_sort.c)
target_sources(PMergeSort
        PUBLIC
        FILE_SET HEADERS
        FILES p_merge_sort.h)
target_link_libraries(PMergeSort PUBLIC Threads::Threads)

target_link_libraries(lab3 PRIVATE PMergeSort)

add_executable(benchmark benchmark/benchmark.c)
target_link_libraries(benchmark PRIVATE PMergeSort)

enable_testing()
add_executable(test_p_merge_sort test_p_merge_sort.c)
target_link_libraries(test_p_merge_sort PRIVATE PMergeSort)
add_test(NAME p_merge_sort COMMAND test_p_merge_sort)
