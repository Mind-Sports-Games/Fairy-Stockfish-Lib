
enable_testing()

add_executable(test_fairystockfish
    main.cpp
    test_wrapper.cpp
)
target_link_libraries(test_fairystockfish pthread fairystockfish)
target_include_directories(test_fairystockfish PRIVATE
    ../src/
    ../vendor/Fairy-Stockfish/src
    ../vendor/doctest
)
target_compile_definitions(test_fairystockfish PRIVATE
    NNUE_EMBEDDING_OFF
    LARGEBOARDS
    PRECOMPUTED_MAGICS
    DOCTEST_CONFIG_ASSERTION_PARAMETERS_BY_VALUE
    DOCTEST_CONFIG_SUPER_FAST_ASSERTS
)
add_test(all ./test_fairystockfish --force-colors)
