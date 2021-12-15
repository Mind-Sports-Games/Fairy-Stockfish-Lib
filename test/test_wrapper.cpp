#include <doctest.h>

#include <iostream>

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
        std::string initialFEN = fairystockfish::initialFen(variantName);
        SUBCASE("Initial FEN Must be valid") {
            REQUIRE(fairystockfish::validateFEN(variantName, initialFEN));
        }
        SUBCASE("Initial fen Insufficient material should be false at the start") {
            auto result = fairystockfish::hasInsufficientMaterial(variantName, initialFEN, {});
            REQUIRE(!std::get<0>(result));
            REQUIRE(!std::get<1>(result));
        }

        SUBCASE("Initial fen There must be legal moves") {
            auto result = fairystockfish::getLegalMoves(variantName, initialFEN, {});
            REQUIRE(result.size() > 0);
        }

        SUBCASE("Initial fen There must the appropriate amount of pieces ") {
            auto result = fairystockfish::piecesOnBoard(variantName, initialFEN, {});
            if (variantName == "shogi") {
                REQUIRE(result.size() == 40);
            } else if (variantName == "xiangqi") {
                REQUIRE(result.size() == 32);
            }
        }

        SUBCASE("Initial fen Must not be game end") {
            auto result = fairystockfish::isOptionalGameEnd(variantName, initialFEN, {});
            REQUIRE(std::get<0>(result) == false);
            // TODO: this triggers an assert right now
            //auto gameResult = fairystockfish::gameResult(variantName, initialFEN, {});
            //REQUIRE(gameResult != -Stockfish::VALUE_MATE);
        }

        SUBCASE("Initial fen Valid opening move must result in valid piecemap") {
            if (variantName == "xiangqi") {
                auto newFEN = fairystockfish::getFEN(variantName, initialFEN, {"e1e2"});
                auto result = fairystockfish::piecesOnBoard(variantName, initialFEN, {});
                REQUIRE(result.size() == 32);
            }
        }
    }
}

TEST_CASE("fairystockfish invalid fens") {
    fairystockfish::init();

    // missing middle rank
    std::string invalidFen{"lnsgkgsnl/1r5b1/ppppppppp/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL[-] w 0 1"};
    REQUIRE(!fairystockfish::validateFEN("shogi", invalidFen));

    // Obviously invalid
    REQUIRE(!fairystockfish::validateFEN("shogi", "I'm a Shogi FEN! (not)"));
}

TEST_CASE("Chess checkmate FEN") {
    fairystockfish::init();
    std::vector<std::string> mateFENs = {
        "rnb1kbnr/pppp1ppp/8/4p3/5PPq/8/PPPPP2P/RNBQKBNR w KQkq - 1 3",
        "r1bqkbnr/1ppppQ1p/p1n3p1/8/2B1P3/8/PPPP1PPP/RNB1K1NR b KQkq - 0 4"
    };
    for (auto const &fen : mateFENs) {
        auto result = fairystockfish::gameResult("chess", fen, {});
        REQUIRE(result == -Stockfish::VALUE_MATE);
        REQUIRE(fairystockfish::getLegalMoves("chess", fen, {}).size() == 0);
    }
}

TEST_CASE("Chess Threefold") {
    fairystockfish::init();
    std::string fen{"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"};
    std::vector<std::string> moves {
        "b1c3", "g8f6", "c3b1", "f6g8", "b1c3", "g8f6", "c3b1", "f6g8"
    };
    auto optionalGameEnd = fairystockfish::isOptionalGameEnd("chess", fen, moves);
    REQUIRE(std::get<0>(optionalGameEnd));

    // TODO: reenable once we have the assert figured out
    //auto result = fairystockfish::gameResult("chess", fen, moves);
    //REQUIRE(result == Stockfish::VALUE_ZERO);

}

TEST_CASE("Shogi checkmate FEN") {
    fairystockfish::init();
    std::vector<std::string> mateFENs = {
        "l2g1g1nl/5sk2/3p1p1p1/p3p1p1p/1n2n4/P4PP1P/1P1sPK1P1/5sR1+r/L4+p1N1[GPSBBglpp] w - - 4 38"
    };
    for (auto const &fen : mateFENs) {
        auto result = fairystockfish::gameResult("shogi", fen, {});
        REQUIRE(result == -Stockfish::VALUE_MATE);
        REQUIRE(fairystockfish::getLegalMoves("shogi", fen, {}).size() == 0);

        auto legalMoves = fairystockfish::getLegalMoves("shogi", fen, {});
        REQUIRE(legalMoves.size() == 0);
    }
}

TEST_CASE("Shogi FourFold") {
    fairystockfish::init();
    std::string fen = fairystockfish::initialFen("shogi");
    std::vector<std::string> moves {
        "h2i2", "b8a8", "i2h2", "a8b8", "h2i2", "b8a8", "i2h2", "a8b8", "h2i2", "b8a8", "i2h2", "a8b8"
    };
    auto optionalGameEnd = fairystockfish::isOptionalGameEnd("shogi", fen, moves);
    REQUIRE(std::get<0>(optionalGameEnd));

    // TODO: game result has an assert we need to deal with
    //auto result = fairystockfish::gameResult("shogi", fen, moves);
    //REQUIRE(result == -Stockfish::VALUE_MATE);

    auto legalMoves = fairystockfish::getLegalMoves("shogi", fen, {});
    REQUIRE(legalMoves.size() == 30);
}

TEST_CASE("Shogi checkmate FEN piecesInHand") {
    fairystockfish::init();
    std::string fen{"l2g1g1nl/5sk2/3p1p1p1/p3p1p1p/1n2n4/P4PP1P/1P1sPK1P1/5sR1+r/L4+p1N1[GPSBBglpp] w - - 4 38"};
    auto result = fairystockfish::piecesInHand("shogi", fen, {});
    REQUIRE(result.size() == 9);
}

TEST_CASE("Shogi Fools mate") {
    fairystockfish::init();
    std::string foolsFEN = "lnsg1gsnl/5rkb1/ppppppp+Pp/9/9/9/PPPPPPP1P/1B5R1/LNSGKGSNL[P] b - - 0 4";
    std::vector<std::string> moves {};

    auto result = fairystockfish::gameResult("shogi", foolsFEN, moves);
    REQUIRE(result == -Stockfish::VALUE_MATE);

    auto legalMoves = fairystockfish::getLegalMoves("shogi", foolsFEN, {});
    REQUIRE(legalMoves.size() == 0);
}


TEST_CASE("Chess givesCheck returns true after check") {
    fairystockfish::init();
    std::string checkFEN = "rnbqkbnr/pppp1ppp/8/4p3/5P2/5N2/PPPPP1PP/RNBQKB1R b KQkq - 1 2";
    std::vector<std::string> moves = {"d8h4"};
    REQUIRE(fairystockfish::givesCheck("chess", checkFEN, moves));
}

TEST_CASE("Chess stalemate is a draw") {
    fairystockfish::init();
    std::string stalemateFEN = "5bnr/4p1pq/4Qpkr/7p/7P/4P3/PPPP1PP1/RNB1KBNR b KQ - 2 10";
    REQUIRE(
        stalemateFEN == fairystockfish::getFEN(
            "chess",
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
            {"e2e3", "a7a5", "d1h5", "a8a6", "h5a5", "h7h5", "a5c7", "a6h6", "h2h4", "f7f6", "c7d7", "e8f7", "d7b7", "d8d3", "b7b8", "d3h7", "b8c8", "f7g6", "c8e6"}
        )
    );
    REQUIRE(fairystockfish::getLegalMoves("chess", stalemateFEN, {}).size() == 0);
    REQUIRE(fairystockfish::gameResult("chess", stalemateFEN, {}) == Stockfish::VALUE_DRAW);
}

TEST_CASE("Shogi stalemate is a win") {
    fairystockfish::init();
    std::string stalemateFEN = "8l/8k/9/8P/9/2P6/PP1PPPP2/1B5R1/LNSGKGSNL[] b - - 0 2";
    REQUIRE(fairystockfish::validateFEN("shogi", stalemateFEN, {}));
    REQUIRE(fairystockfish::getLegalMoves("shogi", stalemateFEN, {}).size() == 0);
    REQUIRE(fairystockfish::gameResult("shogi", stalemateFEN, {}) == -Stockfish::VALUE_MATE);
}

TEST_CASE("Chess king only is insufficientMaterial") {
    fairystockfish::init();
    std::string insufficientMaterialFEN = "4k3/8/8/8/8/8/PPPPPPPP/RNBQKBNR w - - 0 1";
    REQUIRE(fairystockfish::validateFEN("chess", insufficientMaterialFEN));
    auto result = fairystockfish::hasInsufficientMaterial("chess", insufficientMaterialFEN, {});
    REQUIRE(!std::get<0>(result));
    REQUIRE(std::get<1>(result));
}

TEST_CASE("Shogi king only is not insufficientMaterial") {
    fairystockfish::init();
    std::string insufficientMaterialFEN = "8k/9/9/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL[LNSGGSNLBRPPPPPPPPP] b - - 0 2";
    auto result = fairystockfish::hasInsufficientMaterial("shogi", insufficientMaterialFEN, {});
    REQUIRE(!std::get<0>(result));
    REQUIRE(!std::get<1>(result));
}

TEST_CASE("Chess white king vs black king only should be insufficient material") {
    fairystockfish::init();
    std::string insufficientMaterialFEN = "4k3/8/8/8/8/8/8/3K4 w - - 0 1";
    REQUIRE(fairystockfish::validateFEN("chess", insufficientMaterialFEN));
    auto result = fairystockfish::hasInsufficientMaterial("chess", insufficientMaterialFEN, {});
    REQUIRE(std::get<0>(result));
    REQUIRE(std::get<1>(result));

    // TODO: re-enable this once we figure out this silly assertion
    // REQUIRE(fairystockfish::gameResult("chess", insufficientMaterialFEN, {}) == Stockfish::VALUE_DRAW);
}

TEST_CASE("Chess autodraw") {
    //https://lichess.org/BdvgPSMd#82
    //from src/test/scala/chess/AutodrawTest.scala
    std::vector<std::string> moves{
        "e2e4", "c7c5", "g1f3", "d7d6", "d2d4", "c5d4", "f3d4", "g8f6", "b1c3", "g7g6", "c1g5", "f8g7", "f2f4", "b8c6", "f1b5", "c8d7", "d4c6", "d7c6", "b5c6", "b7c6", "e1g1", "d8b6", "g1h1", "b6b2", "d1d3", "e8g8", "a1b1", "b2a3", "b1b3", "a3c5", "c3a4", "c5a5", "a4c3", "a8b8", "f4f5", "b8b3", "a2b3", "f6g4", "c3e2", "a5c5", "h2h3", "g4f2", "f1f2", "c5f2", "f5g6", "h7g6", "g5e7", "f8e8", "e7d6", "f2f1", "h1h2", "g7e5", "d6e5", "e8e5", "d3d8", "g8g7", "d8d4", "f7f6", "e2g3", "f1f4", "d4d7", "g7h6", "d7f7", "e5g5", "f7f8", "h6h7", "f8f7", "h7h8", "f7f8", "h8h7", "f8f7", "h7h6", "f7f8", "h6h7", "f8f7", "h7h8", "f7f8", "h8h7", "f8f7", "h7h6", "f7f8", "h6h7"
    };
    std::string initialFEN = fairystockfish::initialFen("chess");
    auto result = fairystockfish::isOptionalGameEnd(
        "chess",
        initialFEN,
        moves
    );
    REQUIRE(std::get<0>(result));

    // TODO: once again, this can't be called when there are moves available.
    /*REQUIRE(fairystockfish::gameResult(
        "chess",
        initialFEN,
        moves
    ) == Stockfish::VALUE_DRAW);*/
}


TEST_CASE("King of the hill variant win") {
    std::vector<std::string> moves{"e2e4", "a7a6", "e1e2", "a6a5", "e2e3", "a5a4", "e3d4"};
    std::string initialFen = fairystockfish::initialFen("kingofthehill");
    REQUIRE(
        fairystockfish::gameResult("kingofthehill", initialFen, moves)
        == -Stockfish::VALUE_MATE
    );
}

TEST_CASE("Racing Kings Draw") {
    std::vector<std::string> moves{"h2h3", "a2a3", "h3h4", "a3a4", "h4h5", "a4a5", "h5h6", "a5a6", "h6g7", "a6b7", "g7g8", "b7b8"};
    std::string initialFen = fairystockfish::initialFen("racingkings");
    REQUIRE(
        fairystockfish::gameResult("racingkings", initialFen, moves)
        == Stockfish::VALUE_DRAW
    );
}


TEST_CASE("Available Variants") {
    auto variants = fairystockfish::availableVariants();
    REQUIRE(std::find(variants.begin(), variants.end(), "shogi") != variants.end());
    REQUIRE(std::find(variants.begin(), variants.end(), "xiangqi") != variants.end());
    REQUIRE(std::find(variants.begin(), variants.end(), "my little pony") == variants.end());
}

TEST_CASE("Promoted Pieces") {
    auto variant = "shogi";
    auto fen = "lnsgkgsnl/1r5b1/pppppppp1/P8/9/8p/1PPPPPPPP/1B5R1/LNSGKGSNL[-] w 0 1";
    std::vector<std::string> moves{"a6a7+"};
    std::string newFen = fairystockfish::getFEN(variant, fen, moves);
    std::map<std::string, fairystockfish::Piece> pieces
        = fairystockfish::piecesOnBoard(variant, newFen);

    auto p = pieces.find("a7");
    if (p == pieces.end()) {
        REQUIRE(false);
    } else {
        REQUIRE(p->second.promoted());
        REQUIRE(p->second.pieceInfo().name() == "shogiPawn");
    }
/*
    for (const auto &[square, piece] : pieces) {
        std::cout
            << "square: " << square
            << " piece.color: " << piece.color()
            << " piece.pieceInfo(): " << piece.pieceInfo().name()
            << " piece.promoted(): " << std::boolalpha << piece.promoted()
            << std::endl;
    }
*/

}

TEST_CASE("Shogi Unforced repetition is a draw") {
    auto variant = "shogi";
    std::string initialFEN = fairystockfish::initialFen(variant);
    // 1 move before optional draw
    std::vector<std::vector<std::string>> notDrawnSituations{
        {
            "c3c4", "a7a6",
            "b2g7+", "e9d8", "g7f6", "d8e9",
            "f6g7", "e9d8", "g7f6", "d8e9",
            "f6g7", "e9d8", "g7f6"
        },
        {
            "h2i2", "b8a8", "i2h2", "a8b8",
            "h2i2", "b8a8", "i2h2", "a8b8",
            "h2i2", "b8a8", "i2h2"
        },
    };
    for (auto const &moves : notDrawnSituations) {
        //std::cout << "isOptionalGameEnd() -> " << std::boolalpha << std::get<0>(fairystockfish::isOptionalGameEnd(variant, initialFEN, moves)) << std::endl;
        //std::cout << "isDraw(0) -> " << std::boolalpha << fairystockfish::isDraw(variant, initialFEN, moves, 0) << std::endl;
        REQUIRE(!std::get<0>(fairystockfish::isOptionalGameEnd(variant, initialFEN, moves)));
        REQUIRE(!fairystockfish::isDraw(variant, initialFEN, moves, 0));
    }

    // 1 move before forced draw
    std::vector<std::vector<std::string>> soonDrawnSituations{
        {
            "c3c4", "a7a6",
            "b2g7+", "e9d8", "g7f6", "d8e9",
            "f6g7", "e9d8", "g7f6", "d8e9",
            "f6g7", "e9d8", "g7f6", "d8e9"
        },
        {
            "h2i2", "b8a8", "i2h2", "a8b8",
            "h2i2", "b8a8", "i2h2", "a8b8",
            "h2i2", "b8a8", "i2h2"
        },
    };
    for (auto const &moves : soonDrawnSituations) {
        //std::cout << "isOptionalGameEnd() -> " << std::boolalpha << std::get<0>(fairystockfish::isOptionalGameEnd(variant, initialFEN, moves)) << std::endl;
        //std::cout << "isDraw(0) -> " << std::boolalpha << fairystockfish::isDraw(variant, initialFEN, moves, 0) << std::endl;
        REQUIRE(!std::get<0>(fairystockfish::isOptionalGameEnd(variant, initialFEN, moves)));
        REQUIRE(!fairystockfish::isDraw(variant, initialFEN, moves, 0));
    }

    // Forced draw.
    std::vector<std::vector<std::string>> drawnSituations{
        {
            "c3c4", "a7a6",
            "b2g7+", "e9d8", "g7f6", "d8e9",
            "f6g7", "e9d8", "g7f6", "d8e9",
            "f6g7", "e9d8", "g7f6", "d8e9",
            "f6g7"
        },
        {
            "h2i2", "b8a8", "i2h2", "a8b8",
            "h2i2", "b8a8", "i2h2", "a8b8",
            "h2i2", "b8a8", "i2h2", "a8b8",
        },
    };
    for (auto const &moves : drawnSituations) {
        //std::cout << "isOptionalGameEnd() -> " << std::boolalpha << std::get<0>(fairystockfish::isOptionalGameEnd(variant, initialFEN, moves)) << std::endl;
        //std::cout << "isDraw(0) -> " << std::boolalpha << fairystockfish::isDraw(variant, initialFEN, moves, 0) << std::endl;
        auto result = fairystockfish::isOptionalGameEnd(variant, initialFEN, moves);
        REQUIRE(std::get<0>(result));
        REQUIRE(std::get<1>(result) == Stockfish::VALUE_DRAW);
        REQUIRE(fairystockfish::isDraw(variant, initialFEN, moves, 0));
    }
}

TEST_CASE("Shogi Forced checking repetition is a loss") {
    auto variant = "shogi";
    std::string initialFEN = fairystockfish::initialFen(variant);
    // 1 move before optional draw
    std::vector<std::vector<std::string>> notLossSituations{
        {
            "h3h4", "e9d8", "h4h5", "d7d6", "h2h4", "d8d7", "h4f4", "d7e6", "g3g4", "c9c8", "c3c4", "c8d7", "b1c3", "g7g6", "g1f2", "f9g8", "d1d2", "g8g7",
            "f4e4", "e6f6", "e4f4", "f6e6",
            "f4e4", "e6f6", "e4f4", "f6e6",
        },
        {
            "h3h4", "e9d8", "h4h5", "d7d6", "h2h4", "d8d7", "h4f4", "d7e6", "g3g4", "c9c8", "c3c4", "c8d7", "b1c3", "g7g6", "g1f2", "f9g8", "d1d2", "g8g7",
            "f4e4", "e6f6", "e4f4", "f6e6",
            "f4e4", "e6f6", "e4f4", "f6e6",
            "f4e4",
        },
    };
    for (auto const &moves : notLossSituations) {
        auto result = fairystockfish::isOptionalGameEnd(variant, initialFEN, moves);
        REQUIRE(!std::get<0>(result));
    }

    // 1 move before forced loss
    // NOTE: dunno why my opponents move is the one that causes the loss.
    std::vector<std::vector<std::string>> soonLostSituations{
        {
            "h3h4", "e9d8", "h4h5", "d7d6", "h2h4", "d8d7", "h4f4", "d7e6", "g3g4", "c9c8", "c3c4", "c8d7", "b1c3", "g7g6", "g1f2", "f9g8", "d1d2", "g8g7",
            "f4e4", "e6f6", "e4f4", "f6e6",
            "f4e4", "e6f6", "e4f4", "f6e6",
            "f4e4", "e6f6", "e4f4"
        },
    };
    for (auto const &moves : soonLostSituations) {
        auto result = fairystockfish::isOptionalGameEnd(variant, initialFEN, moves);
        REQUIRE(!std::get<0>(result));
    }

    // When my opponent moves out of check it's now a loss
    std::vector<std::vector<std::string>> lossSituations{
        {
            "h3h4", "e9d8", "h4h5", "d7d6", "h2h4", "d8d7", "h4f4", "d7e6", "g3g4", "c9c8", "c3c4", "c8d7", "b1c3", "g7g6", "g1f2", "f9g8", "d1d2", "g8g7",
            "f4e4", "e6f6", "e4f4", "f6e6",
            "f4e4", "e6f6", "e4f4", "f6e6",
            "f4e4", "e6f6", "e4f4", "f6e6",
        },
    };
    for (auto const &moves : lossSituations) {
        auto result = fairystockfish::isOptionalGameEnd(variant, initialFEN, moves);
        REQUIRE(std::get<0>(result));
        REQUIRE(std::get<1>(result) == -Stockfish::VALUE_MATE);
    }

    // Still a loss when I put them into check
    std::vector<std::vector<std::string>> lossSituations2{
        {
            "c3c4", "e7e6",
            "b2g7+", "e9d8", "g7f6", "d8e9",
            "f6g7", "e9d8", "g7f6", "d8e9",
            "f6g7", "e9d8", "g7f6", "d8e9",
            "f6g7"
        },
        {
            "h3h4", "e9d8", "h4h5", "d7d6", "h2h4", "d8d7", "h4f4", "d7e6", "g3g4", "c9c8", "c3c4", "c8d7", "b1c3", "g7g6", "g1f2", "f9g8", "d1d2", "g8g7",
            "f4e4", "e6f6", "e4f4", "f6e6",
            "f4e4", "e6f6", "e4f4", "f6e6",
            "f4e4", "e6f6", "e4f4", "f6e6",
            "f4e4"
        },
    };
    for (auto const &moves : lossSituations2) {
        auto result = fairystockfish::isOptionalGameEnd(variant, initialFEN, moves);
        REQUIRE(std::get<0>(result));
        REQUIRE(std::get<1>(result) == Stockfish::VALUE_MATE);
    }
}

TEST_CASE("fairystockfish variant setup stuff v3") {
    fairystockfish::init();

    SUBCASE("Initial fen There must the appropriate amount of pieces ") {
        fairystockfish::Position shogiPos("shogi");
        auto result = shogiPos.piecesOnBoard();
        REQUIRE(result.size() == 40);
    }
}

TEST_CASE("fairystockfish variant makeMoves v3") {
    fairystockfish::init();

    SUBCASE("Must be able to make moves more than once from the same position") {
        fairystockfish::Position shogiPos("shogi");
        auto newPosition = shogiPos.makeMoves(
            {
                "h2i2", "b8a8", "i2h2",
                "a8b8", "h2i2", "b8a8",
                "i2h2", "a8b8", "h2i2",
                "b8a8", "i2h2", "a8b8"
            }
        );
        auto optGameEnd = newPosition.isOptionalGameEnd();
        REQUIRE(std::get<0>(optGameEnd));
        auto newPosition2 = shogiPos.makeMoves(
            {
                "h2i2", "b8a8", "i2h2",
                "a8b8", "h2i2", "b8a8",
                "i2h2", "a8b8", "h2i2",
                "b8a8", "i2h2", "a8b8"
            }
        );
        auto optGameEnd2 = newPosition.isOptionalGameEnd();
        REQUIRE(std::get<0>(optGameEnd2));
    }
}

/*
// TODO: this test is failing, but we're just trying to figure it out anyways.
TEST_CASE("Shogi Repetition") {
    auto variant = "shogi";
    std::string initialFEN = fairystockfish::initialFen(variant);
    std::vector<std::vector<std::string>> situations{
        {
            "h3h4", "e9d8", "h4h5", "d7d6", "h2h4", "d8d7", "h4f4", "d7e6", "g3g4", "c9c8", "c3c4", "c8d7", "b1c3", "g7g6", "g1f2", "f9g8", "d1d2", "g8g7",
            "f4e4", "e6f6", "e4f4", "f6e6",
            "f4e4", "e6f6", "e4f4", "f6e6",
        },
        {
            "h3h4", "e9d8", "h4h5", "d7d6", "h2h4", "d8d7", "h4f4", "d7e6", "g3g4", "c9c8", "c3c4", "c8d7", "b1c3", "g7g6", "g1f2", "f9g8", "d1d2", "g8g7",
            "f4e4", "e6f6", "e4f4", "f6e6",
            "f4e4", "e6f6", "e4f4", "f6e6",
            "f4e4",
        },
        //{"c3c4", "a7a6", "b2g7+", "e9d8", "g7f6", "d8e9", "f6g7", "e9d8", "g7f6", "d8e9", "f6g7", "e9d8", "g7f6", "d8e9"},
        //{"h2i2", "b8a8", "i2h2", "a8b8", "h2i2", "b8a8", "i2h2", "a8b8"},
        //{"c3c4", "a7a6", "b2g7+", "e9d8", "g7f6", "d8e9", "f6g7", "e9d8", "g7f6", "d8e9", "f6g7", "e9d8", "g7f6", "d8e9", "f6g7"},
        //{"h2i2", "b8a8", "i2h2", "a8b8", "h2i2", "b8a8", "i2h2", "a8b8", "h2i2", "b8a8", "i2h2", "a8b8", "h2i2", "b8a8", "i2h2", "a8b8"}
    };
    for (auto const &moves : situations) {
        std::cout << "hasGameCycle(0) -> " << std::boolalpha << fairystockfish::hasGameCycle(variant, initialFEN, moves, 0) << std::endl;
        std::cout << "hasGameCycle(15) -> " << std::boolalpha << fairystockfish::hasGameCycle(variant, initialFEN, moves, 15) << std::endl;
        std::cout << "hasRepeated() -> " << std::boolalpha << fairystockfish::hasRepeated(variant, initialFEN, moves) << std::endl;
        std::cout << "isDraw(0) -> " << std::boolalpha << fairystockfish::isDraw(variant, initialFEN, moves, 0) << std::endl;
        std::cout << "isDraw(15) -> " << std::boolalpha << fairystockfish::isDraw(variant, initialFEN, moves, 15) << std::endl;
        std::cout << "isOptionalGameEnd() -> " << std::boolalpha << std::get<0>(fairystockfish::isOptionalGameEnd(variant, initialFEN, moves)) << std::endl;
        std::cout << "isImmediateGameEnd() -> " << std::boolalpha << std::get<0>(fairystockfish::isImmediateGameEnd(variant, initialFEN, moves)) << std::endl;
        std::cout << "isImmediateGameEnd() -> " << std::boolalpha << std::get<1>(fairystockfish::isImmediateGameEnd(variant, initialFEN, moves)) << std::endl;
        //std::cout << "gameResult() -> " << std::boolalpha << fairystockfish::gameResult(variant, initialFEN, moves) << std::endl;
        REQUIRE(fairystockfish::hasGameCycle(variant, initialFEN, moves, 15));
        REQUIRE(fairystockfish::hasRepeated(variant, initialFEN, moves));
        REQUIRE(fairystockfish::hasGameCycle(variant, initialFEN, moves, 0));
        REQUIRE(fairystockfish::hasGameCycle(variant, initialFEN, moves, 15));
        REQUIRE(fairystockfish::hasRepeated(variant, initialFEN, moves));
    }

}*/
