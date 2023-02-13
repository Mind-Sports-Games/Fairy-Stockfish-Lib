# https://stackoverflow.com/questions/45688522/how-to-enable-c17-in-cmake
# Cmake 3.10 for std++17 flags properly
cmake_minimum_required(VERSION 3.10)
project(fairystockfish)

# https://cliutils.gitlab.io/modern-cmake/chapters/testing.html
# If we're the target project, then enable testing with an option
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)
endif()

# https://cmake.org/cmake/help/latest/prop_tgt/CXX_STANDARD.html
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


set(FSF_SOURCE_FILES
    vendor/Fairy-Stockfish/src/apiutil.h
    vendor/Fairy-Stockfish/src/benchmark.cpp
    vendor/Fairy-Stockfish/src/bitbase.cpp
    vendor/Fairy-Stockfish/src/bitboard.cpp
    vendor/Fairy-Stockfish/src/bitboard.h
    vendor/Fairy-Stockfish/src/endgame.cpp
    vendor/Fairy-Stockfish/src/endgame.h
    vendor/Fairy-Stockfish/src/evaluate.cpp
    vendor/Fairy-Stockfish/src/evaluate.h
    vendor/Fairy-Stockfish/src/incbin/incbin.h
    vendor/Fairy-Stockfish/src/magic.h
    vendor/Fairy-Stockfish/src/main.cpp
    vendor/Fairy-Stockfish/src/material.cpp
    vendor/Fairy-Stockfish/src/material.h
    vendor/Fairy-Stockfish/src/misc.cpp
    vendor/Fairy-Stockfish/src/misc.h
    vendor/Fairy-Stockfish/src/movegen.cpp
    vendor/Fairy-Stockfish/src/movegen.h
    vendor/Fairy-Stockfish/src/movepick.cpp
    vendor/Fairy-Stockfish/src/movepick.h
    vendor/Fairy-Stockfish/src/nnue/evaluate_nnue.cpp
    vendor/Fairy-Stockfish/src/nnue/evaluate_nnue.h
    vendor/Fairy-Stockfish/src/nnue/features/half_ka_v2.cpp
    vendor/Fairy-Stockfish/src/nnue/features/half_ka_v2.h
    vendor/Fairy-Stockfish/src/nnue/features/half_ka_v2_variants.cpp
    vendor/Fairy-Stockfish/src/nnue/features/half_ka_v2_variants.h
    vendor/Fairy-Stockfish/src/nnue/layers/affine_transform.h
    vendor/Fairy-Stockfish/src/nnue/layers/clipped_relu.h
    vendor/Fairy-Stockfish/src/nnue/layers/input_slice.h
    vendor/Fairy-Stockfish/src/nnue/nnue_accumulator.h
    vendor/Fairy-Stockfish/src/nnue/nnue_architecture.h
    vendor/Fairy-Stockfish/src/nnue/nnue_common.h
    vendor/Fairy-Stockfish/src/nnue/nnue_feature_transformer.h
    vendor/Fairy-Stockfish/src/parser.cpp
    vendor/Fairy-Stockfish/src/parser.h
    vendor/Fairy-Stockfish/src/partner.cpp
    vendor/Fairy-Stockfish/src/partner.h
    vendor/Fairy-Stockfish/src/pawns.cpp
    vendor/Fairy-Stockfish/src/pawns.h
    vendor/Fairy-Stockfish/src/piece.cpp
    vendor/Fairy-Stockfish/src/piece.h
    vendor/Fairy-Stockfish/src/position.cpp
    vendor/Fairy-Stockfish/src/position.h
    vendor/Fairy-Stockfish/src/psqt.cpp
    vendor/Fairy-Stockfish/src/psqt.h
    vendor/Fairy-Stockfish/src/search.cpp
    vendor/Fairy-Stockfish/src/search.h
    vendor/Fairy-Stockfish/src/syzygy/tbprobe.cpp
    vendor/Fairy-Stockfish/src/syzygy/tbprobe.h
    vendor/Fairy-Stockfish/src/thread.cpp
    vendor/Fairy-Stockfish/src/thread.h
    vendor/Fairy-Stockfish/src/thread_win32_osx.h
    vendor/Fairy-Stockfish/src/timeman.cpp
    vendor/Fairy-Stockfish/src/timeman.h
    vendor/Fairy-Stockfish/src/tt.cpp
    vendor/Fairy-Stockfish/src/tt.h
    vendor/Fairy-Stockfish/src/tune.cpp
    vendor/Fairy-Stockfish/src/tune.h
    vendor/Fairy-Stockfish/src/types.h
    vendor/Fairy-Stockfish/src/uci.cpp
    vendor/Fairy-Stockfish/src/uci.h
    vendor/Fairy-Stockfish/src/ucioption.cpp
    vendor/Fairy-Stockfish/src/variant.cpp
    vendor/Fairy-Stockfish/src/variant.h
    vendor/Fairy-Stockfish/src/xboard.cpp
    vendor/Fairy-Stockfish/src/xboard.h
)

#set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
#set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

set(SOURCE_FILES
    src/fairystockfish.h
    src/fairystockfish.cpp
)
add_library(fairystockfish SHARED ${FSF_SOURCE_FILES} ${SOURCE_FILES})
target_include_directories(fairystockfish PRIVATE
    vendor/Fairy-Stockfish/src
    vendor/doctest
)
target_compile_definitions(fairystockfish PRIVATE
    # TODO: long term we may want to enable NNUE, but for today, it's unimportant.
    NNUE_EMBEDDING_OFF
    LARGEBOARDS
    PRECOMPUTED_MAGICS
    ALLVARS
    IS_64BIT
)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
    add_subdirectory(test)
endif()

