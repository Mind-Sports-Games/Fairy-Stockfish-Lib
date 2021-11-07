#include <doctest.h>

#include "types.h"
#include "fairystockfish.h"

static std::vector<std::string> variants = {
    "shogi",
    "xiangqi"
};

TEST_CASE("Calling init a bazillion times shouldn't do much") {
    for (int i = 0; i < 10000; ++i) {
        fairystockfish::init();
    }
}


TEST_CASE("fairystockfish variant setup stuff") {
    fairystockfish::init();

    for (auto const &variantName : variants) {
        std::string shogInitialFen = fairystockfish::initialFen(variantName);
        SUBCASE("Initial FEN Must be valid") {
            CHECK(fairystockfish::validateFEN(variantName, shogInitialFen));
        }
        SUBCASE("Insufficient material should be false at the start") {
            auto result = fairystockfish::hasInsufficientMaterial(variantName, shogInitialFen, {});
            CHECK(!std::get<0>(result));
            CHECK(!std::get<1>(result));
        }

        SUBCASE("There must be legal moves") {
            auto result = fairystockfish::getLegalMoves(variantName, shogInitialFen, {});
            CHECK(result.size() > 0);
        }

        SUBCASE("There must the appropriate amount of pieces ") {
            auto result = fairystockfish::piecesOnBoard(variantName, shogInitialFen, {});
            if (variantName == "shogi") {
                CHECK(result.size() == 40);
            } else if (variantName == "xiangqi") {
                CHECK(result.size() == 32);
            }
        }

        SUBCASE("Must not be game end") {
            auto result = fairystockfish::isOptionalGameEnd(variantName, shogInitialFen, {});
            if (variantName == "shogi") {
                CHECK(std::get<0>(result) == false);
            }
        }

        SUBCASE("Valid opening move must result in valid piecemap") {
            if (variantName == "xiangqi") {
                auto newFEN = fairystockfish::getFEN(variantName, shogInitialFen, {"e1e2"});
                auto result = fairystockfish::piecesOnBoard(variantName, shogInitialFen, {});
                CHECK(result.size() == 32);
            }
        }
    }
}

TEST_CASE("fairystockfish invalid fens") {
    fairystockfish::init();
    std::string invalidFen{"lnsgkgsnl/1r5b1/ppppppppp/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL[-] w 0 1"};
    CHECK(!fairystockfish::validateFEN("shogi", invalidFen));
    CHECK(!fairystockfish::validateFEN("shogi", "I'm a Shogi FEN! (not)"));
}

TEST_CASE("Chess checkmate FEN") {
    fairystockfish::init();
    std::vector<std::string> mateFENs = {
        "rnb1kbnr/pppp1ppp/8/4p3/5PPq/8/PPPPP2P/RNBQKBNR w KQkq - 1 3",
        "r1bqkbnr/1ppppQ1p/p1n3p1/8/2B1P3/8/PPPP1PPP/RNB1K1NR b KQkq - 0 4"
    };
    for (auto const &fen : mateFENs) {
        auto result = fairystockfish::gameResult("chess", fen, {});
        CHECK(result == -Stockfish::VALUE_MATE);
        CHECK(fairystockfish::getLegalMoves("chess", fen, {}).size() == 0);
    }
}

TEST_CASE("Chess Threefold") {
    std::string fen{"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"};
    std::vector<std::string> moves {
        "b1c3", "g8f6", "c3b1", "f6g8", "b1c3", "g8f6", "c3b1", "f6g8"
    };
    auto optionalGameEnd = fairystockfish::isOptionalGameEnd("chess", fen, moves);
    CHECK(std::get<0>(optionalGameEnd));

    auto result = fairystockfish::gameResult("chess", fen, moves);
    CHECK(result == Stockfish::VALUE_ZERO);

}


TEST_CASE("Shogi checkmate FEN") {
    fairystockfish::init();
    std::vector<std::string> mateFENs = {
        "l2g1g1nl/5sk2/3p1p1p1/p3p1p1p/1n2n4/P4PP1P/1P1sPK1P1/5sR1+r/L4+p1N1[GPSBBglpp] w - - 4 38"
    };
    for (auto const &fen : mateFENs) {
        auto result = fairystockfish::gameResult("shogi", fen, {});
        CHECK(result == -Stockfish::VALUE_MATE);
        CHECK(fairystockfish::getLegalMoves("shogi", fen, {}).size() == 0);
    }
}

TEST_CASE("Shogi FourFold") {
    std::string fen = fairystockfish::initialFen("shogi");
    std::vector<std::string> moves {
        "h2i2", "b8a8", "i2h2", "a8b8", "h2i2", "b8a8", "i2h2", "a8b8", "h2i2", "b8a8", "i2h2", "a8b8"
    };
    auto optionalGameEnd = fairystockfish::isOptionalGameEnd("shogi", fen, moves);
    CHECK(std::get<0>(optionalGameEnd));

    auto result = fairystockfish::gameResult("shogi", fen, moves);
    CHECK(result == -Stockfish::VALUE_MATE);

    auto legalMoves = fairystockfish::getLegalMoves("shogi", fen, {});
    CHECK(legalMoves.size() == 30);
}

TEST_CASE("Shogi checkmate FEN piecesInHand") {
    fairystockfish::init();
    std::string fen{"l2g1g1nl/5sk2/3p1p1p1/p3p1p1p/1n2n4/P4PP1P/1P1sPK1P1/5sR1+r/L4+p1N1[GPSBBglpp] w - - 4 38"};
    auto result = fairystockfish::piecesInHand("shogi", fen, {});
    CHECK(result.size() == 9);
}

TEST_CASE("Shogi Fools mate") {
    std::string foolsFEN = "lnsg1gsnl/5rkb1/ppppppp+Pp/9/9/9/PPPPPPP1P/1B5R1/LNSGKGSNL[P] b - - 0 4";
    std::vector<std::string> moves {};

    auto result = fairystockfish::gameResult("shogi", foolsFEN, moves);
    CHECK(result == -Stockfish::VALUE_MATE);

    auto legalMoves = fairystockfish::getLegalMoves("shogi", foolsFEN, {});
    CHECK(legalMoves.size() == 0);
}


TEST_CASE("Chess givesCheck returns true after check") {
    std::string checkFEN = "rnbqkbnr/pppp1ppp/8/4p3/5P2/5N2/PPPPP1PP/RNBQKB1R b KQkq - 1 2";
    std::vector<std::string> moves = {"d8h4"};
    CHECK(fairystockfish::givesCheck("chess", checkFEN, moves));
}
