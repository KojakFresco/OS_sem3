#include "p_merge_sort.h"

#include <assert.h>
#include <stdlib.h>

static void assert_sorted_result(
    const int* input,
    const int* expected,
    size_t size,
    size_t thread_count
) {
    int* input_copy = malloc(size * sizeof(*input_copy));
    assert(input_copy != NULL);
    for (size_t i = 0; i < size; ++i) {
        input_copy[i] = input[i];
    }

    int* actual = p_merge_sort(input, size, thread_count);
    assert(actual != NULL);
    for (size_t i = 0; i < size; ++i) {
        assert(actual[i] == expected[i]);
    }

    for (size_t i = 0; i < size; ++i) {
        assert(input[i] == input_copy[i]);
    }

    free(actual);
    free(input_copy);
}

static void test_unsorted_array(void) {
    const int input[] = {8, 3, 7, 4, 9, 2, 6, 1, 5};
    const int expected[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    assert_sorted_result(input, expected, 9, 3);
}

static void test_duplicates_and_negative_values(void) {
    const int input[] = {4, -1, 4, 0, -7, 2, -1, 2};
    const int expected[] = {-7, -1, -1, 0, 2, 2, 4, 4};
    assert_sorted_result(input, expected, 8, 4);
}

static void test_already_sorted_array(void) {
    const int input[] = {-5, -2, 0, 3, 11};
    assert_sorted_result(input, input, 5, 1);
}

static void test_reverse_sorted_array(void) {
    const int input[] = {6, 5, 4, 3, 2, 1};
    const int expected[] = {1, 2, 3, 4, 5, 6};
    assert_sorted_result(input, expected, 6, 12);
}

static void test_single_element(void) {
    const int input[] = {42};
    assert_sorted_result(input, input, 1, 2);
}

int main(void) {
    test_unsorted_array();
    test_duplicates_and_negative_values();
    test_already_sorted_array();
    test_reverse_sorted_array();
    test_single_element();
    return EXIT_SUCCESS;
}
