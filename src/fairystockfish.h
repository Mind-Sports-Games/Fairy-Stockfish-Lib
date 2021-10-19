#ifndef FAIRYSTOCKFISH_H
#define FAIRYSTOCKFISH_H

#include <sstream>

#include "misc.h"
#include "types.h"
#include "bitboard.h"
#include "evaluate.h"
#include "position.h"
#include "search.h"
#include "syzygy/tbprobe.h"
#include "thread.h"
#include "tt.h"
#include "uci.h"
#include "piece.h"
#include "variant.h"
#include "apiutil.h"

#include <vector>
#include <map>

namespace fairystockfish {
    using FenValidation = Stockfish::FEN::FenValidation;


    // Copied from the apiutil.h
    enum Notation {
        NOTATION_DEFAULT,
        // https://en.wikipedia.org/wiki/Algebraic_notation_(chess)
        NOTATION_SAN,
        NOTATION_LAN,
        // https://en.wikipedia.org/wiki/Shogi_notation#Western_notation
        NOTATION_SHOGI_HOSKING, // Examples: P76, S’34
        NOTATION_SHOGI_HODGES, // Examples: P-7f, S*3d
        NOTATION_SHOGI_HODGES_NUMBER, // Examples: P-76, S*34
        // http://www.janggi.pl/janggi-notation/
        NOTATION_JANGGI,
        // https://en.wikipedia.org/wiki/Xiangqi#Notation
        NOTATION_XIANGQI_WXF,
    };
    extern const int VALUE_ZERO;
    extern const int VALUE_DRAW;
    extern const int VALUE_MATE;

    Stockfish::Notation fromOurNotation(Notation n);

    struct PieceInfo {
        private:
            Stockfish::PieceType pieceType;

        public:
            PieceInfo();
            PieceInfo(int pt);

            int id() const;

            std::string name() const;
            std::string betza() const;
    };

    extern bool _fairystockfish_is_initialized;
    ///------------------------------------------------------------------------------
    /// Initialize the fairystockfish library.
    ///------------------------------------------------------------------------------
    void init();

    ///------------------------------------------------------------------------------
    /// Return the version of the library.
    ///
    /// returns a string version number
    ///------------------------------------------------------------------------------
    std::string version();

    ///------------------------------------------------------------------------------
    /// Print to stdout useful information about the library and enabled variants
    ///------------------------------------------------------------------------------
    void info();

    // pyffish methods

    ///------------------------------------------------------------------------------
    /// Sets one of the UCI options that fairy stockfish supports.
    ///
    /// @param name The name of the parameter to set.
    /// @param value The value of the parameters (in string form)
    ///------------------------------------------------------------------------------
    void setUCIOption(std::string name, std::string value);

    ///------------------------------------------------------------------------------
    /// Given a string containing .ini style configuration of variants, load them into
    /// the supported variants for Fairy Stockfish.
    ///
    /// @param config A string containing the ini style variant configuration. Please
    ///               see https://github.com/ianfab/Fairy-Stockfish/blob/master/src/variants.ini
    ///               for example of syntax.
    ///------------------------------------------------------------------------------
    void loadVariantConfig(std::string config);

    ///------------------------------------------------------------------------------
    /// Returns the list of names of supported variants.
    ///
    /// @return A vector of variant names
    ///------------------------------------------------------------------------------
    std::vector<std::string> availableVariants();

    ///------------------------------------------------------------------------------
    /// Returns the initial FEN for a given variant name. Note that this method does
    /// not check that the variant you provided is within the set of supported
    /// variants.
    ///
    /// @param variantName The name of the supported variant.
    ///
    /// @return A string representing the starting FNE for this variant.
    ///------------------------------------------------------------------------------
    std::string initialFen(std::string variantName);

    ///------------------------------------------------------------------------------
    /// Returns a map from the name of a piece to information about that piece.
    ///
    /// @return The map
    ///------------------------------------------------------------------------------
    std::map<std::string, PieceInfo> availablePieces();

    ///------------------------------------------------------------------------------
    /// Converts a UCI move into a SAN notation move given the variant and fen and
    /// whether it's chess960 or not.
    ///
    /// @param variantName The variant for the fen
    /// @param fen The FEN of the current possition
    /// @param uciMove The move in UCI notation.
    /// @param isChess960 Whether the game is chess960 or not.
    /// @param notation The desired SAN notation.
    ///
    /// @return The move in SAN notation
    ///------------------------------------------------------------------------------
    std::string getSAN(
        std::string variantName,
        std::string fen,
        std::string uciMove,
        bool isChess960=false,
        Notation notation=Notation::NOTATION_DEFAULT
    );

    ///------------------------------------------------------------------------------
    /// Converts a set of UCI moves to SAN notation given the variant and fen and
    /// whether it's chess960 or not.
    ///
    /// @param variantName The variant for the fen
    /// @param fen The FEN of the current possition
    /// @param uciMoves A vector of moves in UCI notation
    /// @param isChess960 Whether the game is chess960 or not.
    /// @param notation The desired SAN notation.
    ///
    /// @return A vector of moves in SAN notation
    ///------------------------------------------------------------------------------
    std::vector<std::string> getSANMoves(
        std::string variantName,
        std::string fen,
        std::vector<std::string> uciMoves,
        bool isChess960=false,
        Notation notation=Notation::NOTATION_DEFAULT
    );

    ///------------------------------------------------------------------------------
    /// Get legal moves from a given FEN and move list.
    ///
    /// @param variantName The variant for the fen
    /// @param fen The FEN of the current possition
    /// @param uciMoves A vector of moves in UCI notation
    /// @param isChess960 Whether the game is chess960 or not.
    ///
    /// @return a vector of legal moves in UCI notation
    ///------------------------------------------------------------------------------
    std::vector<std::string> getLegalMoves(
        std::string variantName,
        std::string fen,
        std::vector<std::string> uciMoves,
        bool isChess960=false
    );

    ///------------------------------------------------------------------------------
    /// Get the resulting FEN from a given FEN and move list
    ///
    /// @param variantName The variant for the fen
    /// @param fen The FEN of the current possition
    /// @param uciMoves A vector of moves in UCI notation
    /// @param isChess960 Whether the game is chess960 or not.
    /// @param sFen Whether the output is in sFen notation? (I'm guessing here)
    /// @param showPromoted Whether the fen includes promoted pieces (I'm guessing here)
    /// @param countStarted No clue. (probably has to do with games that end by certain
    ///                     types of counts, like repetitions?)
    ///
    /// @return the FEN of the new position
    ///------------------------------------------------------------------------------
    std::string getFEN(
        std::string variantName,
        std::string fen,
        std::vector<std::string> uciMoves,
        bool isChess960=false,
        bool sFen=false,
        bool showPromoted=false,
        int countStarted=0
    );

    ///------------------------------------------------------------------------------
    /// Get check status from a given fen and movelist.
    ///
    /// @param variantName The variant for the fen
    /// @param fen The FEN of the current possition
    /// @param uciMoves A vector of moves in UCI notation
    /// @param isChess960 Whether the game is chess960 or not.
    ///
    /// @return the FEN of the new position
    ///------------------------------------------------------------------------------
    bool givesCheck(
        std::string variantName,
        std::string fen,
        std::vector<std::string> uciMoves,
        bool isChess960=false
    );

    ///------------------------------------------------------------------------------
    /// Gets result from a given FEN, considering variant end, checkmate and stalemate
    ///
    /// @param variantName The variant for the fen
    /// @param fen The FEN of the current possition
    /// @param uciMoves A vector of moves in UCI notation
    /// @param isChess960 Whether the game is chess960 or not.
    ///
    /// @return Returns an integer that represents the result (don't know what it
    ///         means yet
    ///------------------------------------------------------------------------------
    int gameResult(
        std::string variantName,
        std::string fen,
        std::vector<std::string> uciMoves,
        bool isChess960=false
    );

    ///------------------------------------------------------------------------------
    /// Figures out if variant rules immediately end the game
    ///
    /// @param variantName The variant for the fen
    /// @param fen The FEN of the current possition
    /// @param uciMoves A vector of moves in UCI notation
    /// @param isChess960 Whether the game is chess960 or not.
    ///
    /// @return Returns a boolean indicating if the game is an immediate end as well
    ///         as the integer result value.
    ///------------------------------------------------------------------------------
    std::tuple<bool, int> isImmediateGameEnd(
        std::string variantName,
        std::string fen,
        std::vector<std::string> uciMoves,
        bool isChess960=false
    );

    ///------------------------------------------------------------------------------
    /// Get result from given FEN if rules enable game end by player.
    ///
    /// @param variantName The variant for the fen
    /// @param fen The FEN of the current possition
    /// @param uciMoves A vector of moves in UCI notation
    /// @param isChess960 Whether the game is chess960 or not.
    /// @param countStarted No clue. (probably has to do with games that end by certain
    ///                     types of counts, like repetitions?)
    ///
    /// @return Returns a boolean indicating if the game is an immediate end as well
    ///         as the integer result value.
    ///------------------------------------------------------------------------------
    std::tuple<bool, int> isOptionalGameEnd(
        std::string variantName,
        std::string fen,
        std::vector<std::string> uciMoves,
        bool isChess960=false,
        int countStarted=0
    );

    ///------------------------------------------------------------------------------
    /// Checks for insufficient material on behalf of both players.
    ///
    /// @param variantName The variant for the fen
    /// @param fen The FEN of the current possition
    /// @param uciMoves A vector of moves in UCI notation
    /// @param isChess960 Whether the game is chess960 or not.
    ///
    /// @return Returns two booleans, one for each player which indicate if that player
    ///         has sufficient material
    ///------------------------------------------------------------------------------
    std::tuple<bool, bool> hasInsufficientMaterial(
        std::string variantName,
        std::string fen,
        std::vector<std::string> uciMoves,
        bool isChess960=false
    );

    ///------------------------------------------------------------------------------
    /// Validates an input FEN.
    ///
    /// @param variantName The variant for the fen
    /// @param fen The FEN of the current possition
    /// @param isChess960 Whether the game is chess960 or not.
    ///
    /// @return Whether the FEN is valid or not.
    ///------------------------------------------------------------------------------
    bool validateFEN(
        std::string variantName,
        std::string fen,
        bool isChess960=false
    );
}

#endif // FAIRYSTOCKFISH_H
