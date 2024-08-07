#ifndef FAIRYSTOCKFISH_H
#define FAIRYSTOCKFISH_H

#include "apiutil.h"
#include "bitboard.h"
#include "evaluate.h"
#include "misc.h"
#include "piece.h"
#include "position.h"
#include "search.h"
#include "syzygy/tbprobe.h"
#include "thread.h"
#include "tt.h"
#include "types.h"
#include "uci.h"
#include "variant.h"

#include <climits>
#include <list>
#include <map>
#include <memory>
#include <sstream>
#include <vector>

namespace fairystockfish {
using FenValidation = Stockfish::FEN::FenValidation;

// Copied from the types.h
enum Square : std::uint8_t {
    SQ_A1,
    SQ_B1,
    SQ_C1,
    SQ_D1,
    SQ_E1,
    SQ_F1,
    SQ_G1,
    SQ_H1,
    SQ_I1,
    SQ_J1,
    SQ_K1,
    SQ_L1,
    SQ_A2,
    SQ_B2,
    SQ_C2,
    SQ_D2,
    SQ_E2,
    SQ_F2,
    SQ_G2,
    SQ_H2,
    SQ_I2,
    SQ_J2,
    SQ_K2,
    SQ_L2,
    SQ_A3,
    SQ_B3,
    SQ_C3,
    SQ_D3,
    SQ_E3,
    SQ_F3,
    SQ_G3,
    SQ_H3,
    SQ_I3,
    SQ_J3,
    SQ_K3,
    SQ_L3,
    SQ_A4,
    SQ_B4,
    SQ_C4,
    SQ_D4,
    SQ_E4,
    SQ_F4,
    SQ_G4,
    SQ_H4,
    SQ_I4,
    SQ_J4,
    SQ_K4,
    SQ_L4,
    SQ_A5,
    SQ_B5,
    SQ_C5,
    SQ_D5,
    SQ_E5,
    SQ_F5,
    SQ_G5,
    SQ_H5,
    SQ_I5,
    SQ_J5,
    SQ_K5,
    SQ_L5,
    SQ_A6,
    SQ_B6,
    SQ_C6,
    SQ_D6,
    SQ_E6,
    SQ_F6,
    SQ_G6,
    SQ_H6,
    SQ_I6,
    SQ_J6,
    SQ_K6,
    SQ_L6,
    SQ_A7,
    SQ_B7,
    SQ_C7,
    SQ_D7,
    SQ_E7,
    SQ_F7,
    SQ_G7,
    SQ_H7,
    SQ_I7,
    SQ_J7,
    SQ_K7,
    SQ_L7,
    SQ_A8,
    SQ_B8,
    SQ_C8,
    SQ_D8,
    SQ_E8,
    SQ_F8,
    SQ_G8,
    SQ_H8,
    SQ_I8,
    SQ_J8,
    SQ_K8,
    SQ_L8,
    SQ_A9,
    SQ_B9,
    SQ_C9,
    SQ_D9,
    SQ_E9,
    SQ_F9,
    SQ_G9,
    SQ_H9,
    SQ_I9,
    SQ_J9,
    SQ_K9,
    SQ_L9,
    SQ_A10,
    SQ_B10,
    SQ_C10,
    SQ_D10,
    SQ_E10,
    SQ_F10,
    SQ_G10,
    SQ_H10,
    SQ_I10,
    SQ_J10,
    SQ_K10,
    SQ_L10,
    SQ_NONE,

    SQUARE_ZERO     = 0,
    SQUARE_NB       = 120,
    SQUARE_BIT_MASK = 127,
    SQ_MAX = 119,  // SQUARE_NB - 1 NOTE: this has to be hard coded due to javacpp (or my lack of
                   // knowledge of javacpp)
    SQUARE_NB_CHESS = 64,
    SQUARE_NB_SHOGI = 81,
};

// Copied from the apiutil.h
enum Notation : std::uint8_t {
    NOTATION_DEFAULT,
    // https://en.wikipedia.org/wiki/Algebraic_notation_(chess)
    NOTATION_SAN,
    NOTATION_LAN,
    // https://en.wikipedia.org/wiki/Shogi_notation#Western_notation
    NOTATION_SHOGI_HOSKING,  // Examples: P76, S’34
    NOTATION_SHOGI_HODGES,  // Examples: P-7f, S*3d
    NOTATION_SHOGI_HODGES_NUMBER,  // Examples: P-76, S*34
    // http://www.janggi.pl/janggi-notation/
    NOTATION_JANGGI,
    // https://en.wikipedia.org/wiki/Xiangqi#Notation
    NOTATION_XIANGQI_WXF,
};
extern int const VALUE_ZERO;
extern int const VALUE_DRAW;
extern int const VALUE_MATE;

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

struct Piece {
  private:
    PieceInfo _pieceInfo;
    Stockfish::Color _color;
    bool _promoted;

  public:
    Piece();
    Piece(int pt, int color, bool promoted = false);

    PieceInfo pieceInfo() const;
    int color() const;
    bool promoted() const;
    bool isWhite() const { return _color == Stockfish::Color::WHITE; }
    bool isBlack() const { return _color == Stockfish::Color::BLACK; }
    int id() const { return _pieceInfo.id(); };
};

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
/// All of the available piece chars for all variants in a string.
/// NOTE: These are both upper and lower case.
///
/// @return The string
///------------------------------------------------------------------------------
std::string availablePieceChars();

///------------------------------------------------------------------------------
/// All of the available promotable piece chars for all variants as a string.
/// NOTE: These are both upper and lower case.
///
/// @return The string
///------------------------------------------------------------------------------
std::string availablePromotablePieceChars();

///------------------------------------------------------------------------------
/// Validates an input FEN.
///
/// @param variantName The variant for the fen
/// @param fen The FEN of the current possition
/// @param isChess960 Whether the game is chess960 or not.
///
/// @return Whether the FEN is valid or not.
///------------------------------------------------------------------------------
bool validateFEN(std::string variantName, std::string fen, bool isChess960 = false);

///------------------------------------------------------------------------------
/// Converts uci moves into chess960 notation
///
/// @param variantName The variant for the fen
/// @param moves The list of moves to convert
///
/// @return the moves in chess960 UCI notation
///------------------------------------------------------------------------------
std::vector<std::string> to960Uci(std::string variantName, std::vector<std::string> moves);

///------------------------------------------------------------------------------
/// A position with a specific game variant.
///------------------------------------------------------------------------------
class Position {
  public:
    using MoveList                 = std::vector<std::string>;
    using SFPositionPtr            = std::shared_ptr<Stockfish::Position>;
    using SFPositionConstPtr       = std::shared_ptr<Stockfish::Position const>;
    using StateInfoPtr             = std::shared_ptr<Stockfish::StateInfo const>;
    using MutableStateInfoPtr      = std::shared_ptr<Stockfish::StateInfo>;
    using ListOfImmutableStatesPtr = std::shared_ptr<std::list<StateInfoPtr>>;

    std::string variant;
    bool isChess960;

  private:
    std::shared_ptr<Stockfish::Position const> position;
    // We never give this deque to FairyStockfish proper,
    // instead, we only ever give it the address of one of the state infos.
    //
    // We'll be pretending that we have value semantics, to achieve this
    // we're going to use a linked_list of shared_ptrs<const T> of StateInfo
    // This list is
    struct StateNode {
        std::shared_ptr<StateNode> previous = nullptr;
        Stockfish::StateInfo stateInfo;
    };
    mutable std::shared_ptr<StateNode> state = nullptr;

    // Copy the position
    // NOTE: This depends some things that FairyStockfish may break
    //       later. So when we upgrade FairyStockfish we will want to
    //       be certain it's still ok.
    SFPositionPtr copyPosition(SFPositionConstPtr const &ptr) const;

    Stockfish::Notation fromOurNotation(fairystockfish::Notation n) const;

    void init(std::string startingFen, bool _isChess960 = false);

  public:
    Position(std::string _variant, bool _isChess960 = false);
    Position(std::string _variant, std::string startingFen, bool _isChess960 = false);

    Position(Position const &p)            = default;
    Position(Position &&)                  = default;
    Position &operator=(Position const &p) = default;
    Position &operator=(Position &&)       = default;
    virtual ~Position()                    = default;

    ///------------------------------------------------------------------------------
    /// Returns a new, updated positions with the given moves
    ///------------------------------------------------------------------------------
    Position makeMoves(MoveList const &uciMoves) const;

    ///------------------------------------------------------------------------------
    /// Converts a UCI move into a SAN notation move given the variant and fen and
    /// whether it's chess960 or not.
    ///
    /// @param uciMove The move in UCI notation.
    /// @param notation The desired SAN notation.
    ///
    /// @return The move in SAN notation
    ///------------------------------------------------------------------------------
    std::string getSAN(std::string uciMove, Notation notation = Notation::NOTATION_DEFAULT) const;

    ///------------------------------------------------------------------------------
    /// Converts a set of UCI moves to SAN notation given the variant and fen and
    /// whether it's chess960 or not.
    ///
    /// @param uciMoves A vector of moves in UCI notation
    /// @param notation The desired SAN notation.
    ///
    /// @return A vector of moves in SAN notation
    ///------------------------------------------------------------------------------
    std::vector<std::string> getSANMoves(
        std::vector<std::string> uciMoves,
        Notation ourNotation = Notation::NOTATION_DEFAULT
    ) const;

    ///------------------------------------------------------------------------------
    /// Get legal moves from a given FEN and move list.
    ///
    /// @return a vector of legal moves in UCI notation
    ///------------------------------------------------------------------------------
    std::vector<std::string> getLegalMoves() const;

    ///------------------------------------------------------------------------------
    /// Get the resulting FEN from a given FEN and move list
    ///
    /// @param sFen Whether the output is in sFen notation? (I'm guessing here)
    /// @param showPromoted Whether the fen includes promoted pieces (I'm guessing here)
    /// @param countStarted No clue. (probably has to do with games that end by certain
    ///                     types of counts, like repetitions?)
    ///
    /// @return the FEN of the new position
    ///------------------------------------------------------------------------------
    std::string getFEN(bool sFen = false, bool showPromoted = false, int countStarted = 0) const;

    ///------------------------------------------------------------------------------
    /// Get check status from a given fen and movelist.
    ///
    /// @return the FEN of the new position
    ///------------------------------------------------------------------------------
    bool givesCheck() const;

    ///------------------------------------------------------------------------------
    /// Gets result from a given FEN, considering variant end, checkmate and stalemate
    /// Note that this function will assert there are no legal moves. So only call it
    /// when legalMoves are zero
    ///
    /// @return Returns an integer that represents the result (don't know what it
    ///         means yet
    ///------------------------------------------------------------------------------
    int gameResult() const;

    ///------------------------------------------------------------------------------
    /// Figures out if variant rules immediately end the game.
    /// Note that this does not mean checkmate, checkmates are somewhere else.
    ///
    /// @return Returns a boolean indicating if the game is an immediate end as well
    ///         as the integer result value.
    ///------------------------------------------------------------------------------
    std::tuple<bool, int> isImmediateGameEnd() const;

    ///------------------------------------------------------------------------------
    /// Get result from given FEN if rules enable game end by player.
    ///
    /// @param countStarted No clue. (probably has to do with games that end by certain
    ///                     types of counts, like repetitions?)
    ///
    /// @return Returns a boolean indicating if the game is an immediate end as well
    ///         as the integer result value.
    ///------------------------------------------------------------------------------
    std::tuple<bool, int> isOptionalGameEnd(int countStarted = 0) const;

    ///------------------------------------------------------------------------------
    /// Returns whether it's a draw or not.
    ///
    /// @param ply the number of ply to consider.
    ///
    /// @return Returns a boolean indicating if the game is an immediate end as well
    ///         as the integer result value.
    ///------------------------------------------------------------------------------
    bool isDraw(int ply) const;

    ///------------------------------------------------------------------------------
    /// Checks for insufficient material on behalf of both players.
    ///
    /// @return Returns two booleans, one for each player which indicate if that player
    ///         has sufficient material
    ///------------------------------------------------------------------------------
    std::tuple<bool, bool> hasInsufficientMaterial() const;

    ///------------------------------------------------------------------------------
    /// Tests if the position has a mvoe which draws by repetition,or an earlier
    /// position has a move that directly reaches the current position.
    ///
    /// @param ply The number of ply to search (Don't really know?)
    ///
    /// @return Whether the game has a cycle or not
    ///------------------------------------------------------------------------------
    bool hasGameCycle(int ply) const;

    ///------------------------------------------------------------------------------
    /// Tests whether there has been at least one repetition of positions since the
    /// last capture or pawn move.
    ///
    /// @return Whether the game has repeated or not
    ///------------------------------------------------------------------------------
    bool hasRepeated() const;

    ///------------------------------------------------------------------------------
    /// Returns a piece map for a given position and variant.
    /// @return The map from UCI square notation to piece id integers.
    ///
    ///------------------------------------------------------------------------------
    std::map<std::string, Piece> piecesOnUciBoard() const;

    ///------------------------------------------------------------------------------
    /// Returns a piece map for a given position and variant.
    /// @return The map from square integer values to piece values.
    ///------------------------------------------------------------------------------
    std::map<Square, Piece> piecesOnBoard() const;

    ///------------------------------------------------------------------------------
    /// Returns a map for a given position that maps squares to walls
    /// @return The map from square integer values to a boolean indicating if
    ///         there is a wall
    ///------------------------------------------------------------------------------
    std::map<Square, bool> wallsOnBoard() const;

    ///------------------------------------------------------------------------------
    /// Returns pieces in hand. It returns a single vector where pieces can be of
    /// either color. So it's up to the caller to filter them
    ///
    /// @return A vectors of pieces that are "in hand"
    ///------------------------------------------------------------------------------
    std::vector<Piece> piecesInHand() const;
};
}  // namespace fairystockfish

#endif  // FAIRYSTOCKFISH_H
