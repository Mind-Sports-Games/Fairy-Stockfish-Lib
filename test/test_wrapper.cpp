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
        auto position = fairystockfish::Position(variantName);
        SUBCASE("Initial FEN Must be valid") {
            REQUIRE(fairystockfish::validateFEN(variantName, initialFEN));
        }
        SUBCASE("Initial fen Insufficient material should be false at the start") {
            auto result = position.hasInsufficientMaterial();
            REQUIRE(!std::get<0>(result));
            REQUIRE(!std::get<1>(result));
        }

        SUBCASE("Initial fen There must be legal moves") {
            auto result = position.getLegalMoves();
            REQUIRE(result.size() > 0);
        }

        SUBCASE("Initial fen There must the appropriate amount of pieces ") {
            auto result = position.piecesOnBoard();
            if (variantName == "shogi") {
                REQUIRE(result.size() == 40);
            } else if (variantName == "xiangqi") {
                REQUIRE(result.size() == 32);
            }
        }

        SUBCASE("Initial fen Must not be game end") {
            auto result = position.isOptionalGameEnd();
            REQUIRE(std::get<0>(result) == false);
            // TODO: this triggers an assert right now
            //auto gameResult = fairystockfish::gameResult(variantName, initialFEN, {});
            //REQUIRE(gameResult != -Stockfish::VALUE_MATE);
        }

        SUBCASE("Initial fen Valid opening move must result in valid piecemap") {
            if (variantName == "xiangqi") {
                auto newPosition = position.makeMoves({"e1e2"});
                auto newFEN = newPosition.getFEN();
                auto result = newPosition.piecesOnBoard();
                REQUIRE(result.size() == 32);
            }
        }
    }
}

TEST_CASE("availablePieceChars") {
    fairystockfish::init();
    auto pieces = fairystockfish::availablePieceChars();
    REQUIRE(pieces.find('a') != std::string::npos);
    REQUIRE(pieces.find('A') != std::string::npos);
    REQUIRE(pieces.find('s') != std::string::npos);
    REQUIRE(pieces.find('S') != std::string::npos);
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
        auto position = fairystockfish::Position("chess", fen);
        auto result = position.gameResult();
        REQUIRE(result == -Stockfish::VALUE_MATE);
        REQUIRE(position.getLegalMoves().size() == 0);
    }
}

TEST_CASE("Chess Threefold") {
    fairystockfish::init();
    std::string fen{"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"};
    std::vector<std::string> moves {
        "b1c3", "g8f6", "c3b1", "f6g8", "b1c3", "g8f6", "c3b1", "f6g8"
    };
    auto position = fairystockfish::Position("chess", fen);
    position = position.makeMoves(moves);
    auto optionalGameEnd = position.isOptionalGameEnd();
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
        auto position = fairystockfish::Position("shogi", fen);
        auto result = position.gameResult();
        REQUIRE(result == -Stockfish::VALUE_MATE);
        REQUIRE(position.getLegalMoves().size() == 0);

        auto legalMoves = position.getLegalMoves();
        REQUIRE(legalMoves.size() == 0);
    }
}

TEST_CASE("Shogi FourFold") {
    fairystockfish::init();
    std::vector<std::string> moves {
        "h2i2", "b8a8", "i2h2", "a8b8", "h2i2", "b8a8", "i2h2", "a8b8", "h2i2", "b8a8", "i2h2", "a8b8"
    };
    auto position = fairystockfish::Position("shogi");
    position = position.makeMoves(moves);
    auto optionalGameEnd = position.isOptionalGameEnd();
    REQUIRE(std::get<0>(optionalGameEnd));

    // TODO: game result has an assert we need to deal with
    //auto result = fairystockfish::gameResult("shogi", fen, moves);
    //REQUIRE(result == -Stockfish::VALUE_MATE);

    auto legalMoves = position.getLegalMoves();
    REQUIRE(legalMoves.size() == 30);
}

TEST_CASE("Shogi checkmate FEN piecesInHand") {
    fairystockfish::init();
    std::string fen{"l2g1g1nl/5sk2/3p1p1p1/p3p1p1p/1n2n4/P4PP1P/1P1sPK1P1/5sR1+r/L4+p1N1[GPSBBglpp] w - - 4 38"};
    auto position = fairystockfish::Position("shogi", fen);
    auto result = position.piecesInHand();
    REQUIRE(result.size() == 9);
}

TEST_CASE("Shogi Fools mate") {
    fairystockfish::init();
    std::string foolsFEN = "lnsg1gsnl/5rkb1/ppppppp+Pp/9/9/9/PPPPPPP1P/1B5R1/LNSGKGSNL[P] b - - 0 4";

    auto position = fairystockfish::Position("shogi", foolsFEN);
    auto result = position.gameResult();
    REQUIRE(result == -Stockfish::VALUE_MATE);

    auto legalMoves = position.getLegalMoves();
    REQUIRE(legalMoves.size() == 0);
}


TEST_CASE("Chess givesCheck returns true after check") {
    fairystockfish::init();
    std::string checkFEN = "rnbqkbnr/pppp1ppp/8/4p3/5P2/5N2/PPPPP1PP/RNBQKB1R b KQkq - 1 2";
    auto position = fairystockfish::Position("chess", checkFEN);
    std::vector<std::string> moves = {"d8h4"};
    position = position.makeMoves(moves);
    REQUIRE(position.givesCheck());
}

TEST_CASE("Chess stalemate is a draw") {
    fairystockfish::init();
    std::string stalemateFEN = "5bnr/4p1pq/4Qpkr/7p/7P/4P3/PPPP1PP1/RNB1KBNR b KQ - 2 10";
    auto position = fairystockfish::Position("chess");
    std::vector<std::string> moves = {"e2e3", "a7a5", "d1h5", "a8a6", "h5a5", "h7h5", "a5c7", "a6h6", "h2h4", "f7f6", "c7d7", "e8f7", "d7b7", "d8d3", "b7b8", "d3h7", "b8c8", "f7g6", "c8e6"};
    position = position.makeMoves(moves);
    REQUIRE(stalemateFEN == position.getFEN());
    REQUIRE(position.getLegalMoves().size() == 0);
    REQUIRE(position.gameResult() == Stockfish::VALUE_DRAW);
}

TEST_CASE("Shogi stalemate is a win") {
    fairystockfish::init();
    std::string stalemateFEN = "8l/8k/9/8P/9/2P6/PP1PPPP2/1B5R1/LNSGKGSNL[] b - - 0 2";
    REQUIRE(fairystockfish::validateFEN("shogi", stalemateFEN));
    auto position = fairystockfish::Position("shogi", stalemateFEN);
    REQUIRE(position.getLegalMoves().size() == 0);
    REQUIRE(position.gameResult() == -Stockfish::VALUE_MATE);
}

TEST_CASE("Chess king only is insufficientMaterial") {
    fairystockfish::init();
    std::string insufficientMaterialFEN = "4k3/8/8/8/8/8/PPPPPPPP/RNBQKBNR w - - 0 1";
    REQUIRE(fairystockfish::validateFEN("chess", insufficientMaterialFEN));
    auto position = fairystockfish::Position("chess", insufficientMaterialFEN);
    auto result = position.hasInsufficientMaterial();
    REQUIRE(!std::get<0>(result));
    REQUIRE(std::get<1>(result));
}

TEST_CASE("Shogi king only is not insufficientMaterial") {
    fairystockfish::init();
    std::string insufficientMaterialFEN = "8k/9/9/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL[LNSGGSNLBRPPPPPPPPP] b - - 0 2";
    auto position = fairystockfish::Position("shogi", insufficientMaterialFEN);
    auto result = position.hasInsufficientMaterial();
    REQUIRE(!std::get<0>(result));
    REQUIRE(!std::get<1>(result));
}

TEST_CASE("Chess white king vs black king only should be insufficient material") {
    fairystockfish::init();
    std::string insufficientMaterialFEN = "4k3/8/8/8/8/8/8/3K4 w - - 0 1";
    REQUIRE(fairystockfish::validateFEN("chess", insufficientMaterialFEN));
    auto position = fairystockfish::Position("chess", insufficientMaterialFEN);
    auto result = position.hasInsufficientMaterial();
    REQUIRE(std::get<0>(result));
    REQUIRE(std::get<1>(result));

    // TODO: re-enable this once we figure out this silly assertion
    // REQUIRE(fairystockfish::gameResult("chess", insufficientMaterialFEN, {}) == Stockfish::VALUE_DRAW);
}

TEST_CASE("Chess autodraw") {
    fairystockfish::init();
    //https://lichess.org/BdvgPSMd#82
    //from src/test/scala/chess/AutodrawTest.scala
    std::vector<std::string> moves{
        "e2e4", "c7c5", "g1f3", "d7d6", "d2d4", "c5d4", "f3d4", "g8f6", "b1c3", "g7g6", "c1g5", "f8g7", "f2f4", "b8c6", "f1b5", "c8d7", "d4c6", "d7c6", "b5c6", "b7c6", "e1g1", "d8b6", "g1h1", "b6b2", "d1d3", "e8g8", "a1b1", "b2a3", "b1b3", "a3c5", "c3a4", "c5a5", "a4c3", "a8b8", "f4f5", "b8b3", "a2b3", "f6g4", "c3e2", "a5c5", "h2h3", "g4f2", "f1f2", "c5f2", "f5g6", "h7g6", "g5e7", "f8e8", "e7d6", "f2f1", "h1h2", "g7e5", "d6e5", "e8e5", "d3d8", "g8g7", "d8d4", "f7f6", "e2g3", "f1f4", "d4d7", "g7h6", "d7f7", "e5g5", "f7f8", "h6h7", "f8f7", "h7h8", "f7f8", "h8h7", "f8f7", "h7h6", "f7f8", "h6h7", "f8f7", "h7h8", "f7f8", "h8h7", "f8f7", "h7h6", "f7f8", "h6h7"
    };
    auto position = fairystockfish::Position("chess");
    position = position.makeMoves(moves);
    auto result = position.isOptionalGameEnd();
    REQUIRE(std::get<0>(result));

    // TODO: once again, this can't be called when there are moves available.
    /*REQUIRE(fairystockfish::gameResult(
        "chess",
        initialFEN,
        moves
    ) == Stockfish::VALUE_DRAW);*/
}


TEST_CASE("King of the hill variant win") {
    fairystockfish::init();
    std::vector<std::string> moves{"e2e4", "a7a6", "e1e2", "a6a5", "e2e3", "a5a4", "e3d4"};
    auto position = fairystockfish::Position("kingofthehill");
    position = position.makeMoves(moves);
    REQUIRE(position.gameResult() == -Stockfish::VALUE_MATE);
}

TEST_CASE("Racing Kings Draw") {
    fairystockfish::init();
    std::vector<std::string> moves{"h2h3", "a2a3", "h3h4", "a3a4", "h4h5", "a4a5", "h5h6", "a5a6", "h6g7", "a6b7", "g7g8", "b7b8"};
    auto position = fairystockfish::Position("racingkings");
    position = position.makeMoves(moves);
    REQUIRE(position.gameResult() == Stockfish::VALUE_DRAW);
}


TEST_CASE("Available Variants") {
    fairystockfish::init();
    auto variants = fairystockfish::availableVariants();
    REQUIRE(std::find(variants.begin(), variants.end(), "shogi") != variants.end());
    REQUIRE(std::find(variants.begin(), variants.end(), "xiangqi") != variants.end());
    REQUIRE(std::find(variants.begin(), variants.end(), "my little pony") == variants.end());
}

TEST_CASE("Promoted Pieces") {
    fairystockfish::init();
    std::string fen = "lnsgkgsnl/1r5b1/pppppppp1/P8/9/8p/1PPPPPPPP/1B5R1/LNSGKGSNL[-] w 0 1";
    auto position = fairystockfish::Position("shogi", fen);
    std::vector<std::string> moves{"a6a7+"};
    position = position.makeMoves(moves);
    std::string newFen = position.getFEN();
    std::map<std::string, fairystockfish::Piece> pieces = position.piecesOnBoard();

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
    fairystockfish::init();
    auto position = fairystockfish::Position("shogi");
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
        auto position2 = position.makeMoves(moves);
        //std::cout << "isOptionalGameEnd() -> " << std::boolalpha << std::get<0>(fairystockfish::isOptionalGameEnd(variant, initialFEN, moves)) << std::endl;
        //std::cout << "isDraw(0) -> " << std::boolalpha << fairystockfish::isDraw(variant, initialFEN, moves, 0) << std::endl;
        REQUIRE(!std::get<0>(position2.isOptionalGameEnd()));
        REQUIRE(!position2.isDraw(0));
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
        auto position2 = position.makeMoves(moves);
        //std::cout << "isOptionalGameEnd() -> " << std::boolalpha << std::get<0>(fairystockfish::isOptionalGameEnd(variant, initialFEN, moves)) << std::endl;
        //std::cout << "isDraw(0) -> " << std::boolalpha << fairystockfish::isDraw(variant, initialFEN, moves, 0) << std::endl;
        REQUIRE(!std::get<0>(position2.isOptionalGameEnd()));
        REQUIRE(!position2.isDraw(0));
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
        auto position2 = position.makeMoves(moves);
        //std::cout << "isOptionalGameEnd() -> " << std::boolalpha << std::get<0>(fairystockfish::isOptionalGameEnd(variant, initialFEN, moves)) << std::endl;
        //std::cout << "isDraw(0) -> " << std::boolalpha << fairystockfish::isDraw(variant, initialFEN, moves, 0) << std::endl;
        auto result = position2.isOptionalGameEnd();
        REQUIRE(std::get<0>(result));
        REQUIRE(std::get<1>(result) == Stockfish::VALUE_DRAW);
        REQUIRE(position2.isDraw(0));
    }
}

TEST_CASE("Shogi Forced checking repetition is a loss") {
    fairystockfish::init();
    auto position = fairystockfish::Position("shogi");
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
        auto position2 = position.makeMoves(moves);
        auto result = position2.isOptionalGameEnd();
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
        auto position2 = position.makeMoves(moves);
        auto result = position2.isOptionalGameEnd();
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
        auto position2 = position.makeMoves(moves);
        auto result = position2.isOptionalGameEnd();
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
        auto position2 = position.makeMoves(moves);
        auto result = position2.isOptionalGameEnd();
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
        {
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
        }
        for (size_t i = 0; i < 1000; ++i) {
            auto newPosition2 = shogiPos.makeMoves(
                {
                    "h2i2", "b8a8", "i2h2",
                    "a8b8", "h2i2", "b8a8",
                    "i2h2", "a8b8", "h2i2",
                    "b8a8", "i2h2", "a8b8"
                }
            );
            auto optGameEnd2 = newPosition2.isOptionalGameEnd();
            REQUIRE(std::get<0>(optGameEnd2));
        }
    }
}

TEST_CASE("fairystockfish variant makeMoves individually") {
    fairystockfish::init();

    SUBCASE("Must be able to make moves more than once from the same position") {
        fairystockfish::Position shogiPos("shogi");
        std::vector<std::string> moves = {
            "c3c4", "e7e6", "b2g7+", "e9d8",
            "g7f7", "e6e5", "e3e4", "e5e4",
            "h2e2", "e4e3+", "e2e3", "P@e4",
            "e3e4", "h9g7", "P@e7", "g7f5",
            "e7e8+", "d8c8", "i3i4", "b7b6",
            "h3h4", "b6b5", "h4h5", "b5b4",
            "h5h6", "a7a6", "h6h7+", "a6a5",
            "h7h8", "b8b7", "e4e7+", "b7b6",
            "P@e4", "c9b8", "g3g4", "b9a7",
            "d1e2", "c7c6", "e1d2", "d7d6",
            "f7g8", "b6b5", "g8f9", "b8b7",
            "i4i5", "i7i6"
        };
        for (auto const &m : moves) {
            shogiPos = shogiPos.makeMoves({m});
            REQUIRE(shogiPos.getLegalMoves().size() > 0);
        }
    }
}

TEST_CASE("makeMoves and shogi repetition") {
    fairystockfish::init();

    fairystockfish::Position startingPos("shogi");
    std::vector<std::string> moves = {
        "h2i2", "b8a8", "i2h2",
        "a8b8", "h2i2", "b8a8",
        "i2h2", "a8b8", "h2i2",
        "b8a8", "i2h2", "a8b8"
    };

    SUBCASE("Must be able to make the moves all at once") {
        auto shogiPos = startingPos.makeMoves(moves);
        auto result = shogiPos.isOptionalGameEnd();
        REQUIRE(std::get<0>(result));
    }

    SUBCASE("Must be able to make ") {
        auto shogiPos = startingPos;
        for (auto const &m : moves) {
            shogiPos = shogiPos.makeMoves({m});
        }
        auto result = shogiPos.isOptionalGameEnd();
        REQUIRE(std::get<0>(result));
    }
}

TEST_CASE("passing in othello") {
    fairystockfish::loadVariantConfig(R"variants(
[flipersi]
immobile = p
startFen = 8/8/8/8/8/8/8/8[PPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPpppppppppppppppppppppppppppppppp] w 0 1
pieceDrops = true
promotionPieceTypes = -
doubleStep = false
castling = false
stalemateValue = loss
stalematePieceCount = true
materialCounting = unweighted
enclosingDrop = reversi
enclosingDropStart = d4 e4 d5 e5
immobilityIllegal = false
flipEnclosedPieces = reversi
passOnStalemate = false

[flipello:flipersi]
startFen = 8/8/8/3pP3/3Pp3/8/8/8[PPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPpppppppppppppppppppppppppppppppp] w 0 1
passOnStalemate = true
    )variants"
    );
    fairystockfish::init();

    fairystockfish::Position startingPos("flipello");
    std::vector<std::string> moves = {
        "P@d6", "P@c4", "P@f3",
        "P@f4", "P@e3", "P@e6",
        "P@c6", "P@f6", "P@c5",
        "P@c3", "P@d3", "P@f2",
        "P@f5", "P@d2", "P@b4",
        "P@a5", "P@b3", "P@d7",
        "P@a4", "P@a3", "P@c2",
        "P@b5", "P@e2", "P@d1",
        "P@g4", "P@h5", "P@h4",
        "P@h3", "P@e1", "P@f1",
        "P@g3", "P@h2", "P@b1",
        "P@b2", "P@a6", "P@a7",
        "P@b6", "P@b7", "P@c7",
        "P@g2", "P@a8", "P@c8",
        "P@a2", "d1d1", "P@a1",
        "d1d1", "P@b8", "d1d1",
        "P@c1", "d1d1", "P@d8",
        "P@e7", "P@e8", "P@f8",
        "P@g1", "P@f7", "P@h1",
        "e2e2", "P@g5", "P@g6",
        // This is where  it previously
        // passed, and we can play it out.
        "P@h7", "P@h6", "P@g7",
        "P@g8"
    };

    SUBCASE("This is a valid game") {
        auto pos = startingPos;
        pos = pos.makeMoves(moves);

        auto legalMoves = pos.getLegalMoves();
        REQUIRE(legalMoves.size() == 1);
        REQUIRE(legalMoves[0] == "a1a1");

    }

}

TEST_CASE("5check game can continue after 3 checks") {
    fairystockfish::init();

    fairystockfish::Position startingPos("5check");
    std::vector<std::string> moves = {
        "e2e4", "c7c6", "d2d4", "d7d5",
        "e4d5", "c6c5", "d4c5", "c8g4",
        "f1b5", "g4d7", "b5d7", "d8d7",
        "c5c6", "b7c6", "d5c6", "d7d1",
        "e1d1", "b8c6", "b2b3", "e8c8",
        "c1d2", "d8d2",
    };

    SUBCASE("This is a valid game") {
        auto pos = startingPos;
        pos = pos.makeMoves(moves);
        auto legalMoves = pos.getLegalMoves();
        REQUIRE(legalMoves.size() > 0);
        bool foundb1b2 = false;
        for (auto const & m: legalMoves) {
            if (m == "b1d2") {
                foundb1b2 = true;
                break;
            }
        }
        REQUIRE(foundb1b2);

    }
}

TEST_CASE("Bug report https://github.com/Mind-Sports-Games/Fairy-Stockfish-Rust/issues/2") {
    fairystockfish::init();

    // black wins
    fairystockfish::Position pos1(
        "chess",
        "8/1Q2b1k1/2p3p1/p2p2P1/8/5PB1/PP3RK1/3r3q w - - 2 37",
        false
    );
    REQUIRE(pos1.gameResult() == -32000);

    // white wins
    fairystockfish::Position pos2(
        "chess",
        "2r3kr/p5Rp/1p3Qn1/4q3/8/2P5/P1P3PP/5R1K b - - 6 27",
        false
    );
    REQUIRE(pos2.gameResult() == -32000);

    // stalemate
    fairystockfish::Position pos3(
        "chess",
        "rn2k1nr/pp4pp/3p4/q1pP4/P1P2p1b/1b2pPRP/1P1NP1PQ/2B1KBNR w Kkq - 0 13",
        false
    );
    REQUIRE(pos3.gameResult() == 0);

    // Ongoing
    fairystockfish::Position pos4(
        "chess",
         "2r3kr/p4R1p/1p3Qn1/4q3/8/2P5/P1P3PP/5R1K w - - 5 27",
        false
    );
    REQUIRE(pos4.gameResult() == 0);
}

TEST_CASE("Convert to chess960") {
    fairystockfish::init();
    //https://lichess.org/BdvgPSMd#82
    //from src/test/scala/chess/AutodrawTest.scala
    std::vector<std::string> moves{
        "e2e4", "c7c5", "g1f3", "d7d6", "d2d4", "c5d4", "f3d4",
        "g8f6", "b1c3", "g7g6", "c1g5", "f8g7", "f2f4", "b8c6",
        "f1b5", "c8d7", "d4c6", "d7c6", "b5c6", "b7c6", "e1g1"
    };
    std::vector<std::string> _960Moves = fairystockfish::to960Uci("chess", moves);

    REQUIRE(moves.size()  == _960Moves.size());
    REQUIRE(_960Moves[0]  == "e2e4");
    REQUIRE(_960Moves[1]  == "c7c5");
    REQUIRE(_960Moves[2]  == "g1f3");
    REQUIRE(_960Moves[3]  == "d7d6");
    REQUIRE(_960Moves[4]  == "d2d4");
    REQUIRE(_960Moves[5]  == "c5d4");
    REQUIRE(_960Moves[6]  == "f3d4");
    REQUIRE(_960Moves[7]  == "g8f6");
    REQUIRE(_960Moves[8]  == "b1c3");
    REQUIRE(_960Moves[9]  == "g7g6");
    REQUIRE(_960Moves[10] == "c1g5");
    REQUIRE(_960Moves[11] == "f8g7");
    REQUIRE(_960Moves[12] == "f2f4");
    REQUIRE(_960Moves[13] == "b8c6");
    REQUIRE(_960Moves[14] == "f1b5");
    REQUIRE(_960Moves[15] == "c8d7");
    REQUIRE(_960Moves[16] == "d4c6");
    REQUIRE(_960Moves[17] == "d7c6");
    REQUIRE(_960Moves[18] == "b5c6");
    REQUIRE(_960Moves[19] == "b7c6");
    REQUIRE(_960Moves[20] == "e1h1");
}

TEST_CASE("Convert to chess960 #2") {
    fairystockfish::init();
    std::vector<std::string> moves{
        "e2e4", "b8c6", "b2b3", "e7e6", "c1b2", "d8h4", "b1c3",
        "h4e7", "d1f3", "c6d4", "f1b5", "d4f3", "g1f3", "a7a6",
        "e1g1"
    };
    std::vector<std::string> _960Moves = fairystockfish::to960Uci("5check", moves);

    REQUIRE(moves.size()  == _960Moves.size());
    REQUIRE(_960Moves[0]  == "e2e4");
    REQUIRE(_960Moves[1]  == "b8c6");
    REQUIRE(_960Moves[2]  == "b2b3");
    REQUIRE(_960Moves[3]  == "e7e6");
    REQUIRE(_960Moves[4]  == "c1b2");
    REQUIRE(_960Moves[5]  == "d8h4");
    REQUIRE(_960Moves[6]  == "b1c3");
    REQUIRE(_960Moves[7]  == "h4e7");
    REQUIRE(_960Moves[8]  == "d1f3");
    REQUIRE(_960Moves[9]  == "c6d4");
    REQUIRE(_960Moves[10] == "f1b5");
    REQUIRE(_960Moves[11] == "d4f3");
    REQUIRE(_960Moves[12] == "g1f3");
    REQUIRE(_960Moves[13] == "a7a6");
    REQUIRE(_960Moves[14] == "e1h1");
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
