#ifndef FAIRYSTOCKFISH_H
#define FAIRYSTOCKFISH_H

#include <sstream>
#include <climits>

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
            Piece(int pt, int color, bool promoted=false);

            PieceInfo pieceInfo() const;
            int color() const;
            bool promoted() const;
            bool isWhite() const {
                return _color == Stockfish::Color::WHITE;
            }
            bool isBlack() const {
                return _color == Stockfish::Color::WHITE;
            }
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
    /// Note that this function will assert there are no legal moves. So only call it
    /// when legalMoves are zero
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
    /// Figures out if variant rules immediately end the game.
    /// Note that this does not mean checkmate, checkmates are somewhere else.
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
    /// Returns whether it's a draw or not.
    ///
    /// @param variantName The variant for the fen
    /// @param fen The FEN of the current possition
    /// @param uciMoves A vector of moves in UCI notation
    /// @param ply the number of ply to consider.
    /// @param isChess960 Whether the game is chess960 or not.
    ///
    /// @return Returns a boolean indicating if the game is an immediate end as well
    ///         as the integer result value.
    ///------------------------------------------------------------------------------
    bool isDraw(
        std::string variantName,
        std::string fen,
        std::vector<std::string> uciMoves,
        int ply,
        bool isChess960=false
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
    /// Tests if the position has a mvoe which draws by repetition,or an earlier
    /// position has a move that directly reaches the current position.
    ///
    /// @param variantName The variant for the fen
    /// @param fen The FEN of the current possition
    /// @param ply The number of ply to search (Don't really know?)
    /// @param isChess960 Whether the game is chess960 or not.
    ///
    /// @return A vectors of pieces that are "in hand"
    ///------------------------------------------------------------------------------
    bool hasGameCycle(
        std::string variantName,
        std::string fen,
        std::vector<std::string> uciMoves,
        int ply,
        bool isChess960=false
    );

    ///------------------------------------------------------------------------------
    /// Tests whether there has been at least one repetition of positions since the
    /// last capture or pawn move.
    ///
    /// @param variantName The variant for the fen
    /// @param fen The FEN of the current possition
    /// @param isChess960 Whether the game is chess960 or not.
    ///
    /// @return A vectors of pieces that are "in hand"
    ///------------------------------------------------------------------------------
    bool hasRepeated(
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

    ///------------------------------------------------------------------------------
    /// Returns a piece map for a given position and variant.
    ///
    /// @param variantName The variant for the fen
    /// @param fen The FEN of the current possition
    /// @param isChess960 Whether the game is chess960 or not.
    ///
    /// @return The map from UCI square notation to piece id integers.
    ///------------------------------------------------------------------------------
    std::map<std::string, Piece> piecesOnBoard(
        std::string variantName,
        std::string fen,
        bool isChess960=false
    );

    ///------------------------------------------------------------------------------
    /// Returns pieces in hand. It returns a single vector where pieces can be of
    /// either color. So it's up to the caller to filter them
    ///
    /// @param variantName The variant for the fen
    /// @param fen The FEN of the current possition
    /// @param isChess960 Whether the game is chess960 or not.
    ///
    /// @return A vectors of pieces that are "in hand"
    ///------------------------------------------------------------------------------
    std::vector<Piece> piecesInHand(
        std::string variantName,
        std::string fen,
        bool isChess960=false
    );

    ///------------------------------------------------------------------------------
    /// A position with a specific game variant.
    ///------------------------------------------------------------------------------
    class Position {
        public:
            using MoveList = std::vector<std::string>;
            using SFPositionPtr = std::unique_ptr<Stockfish::Position>;

            std::string variant;
            bool isChess960;


        private:
            std::shared_ptr<const Stockfish::Position> position;
            Stockfish::StateListPtr states;

            Stockfish::StateListPtr copy(Stockfish::StateListPtr const &slp) const {
                return std::make_unique<std::deque<Stockfish::StateInfo>>(*slp);
            }
            SFPositionPtr copyPosition(Stockfish::StateListPtr &newStates) const  {
                const Stockfish::Variant* v = Stockfish::variants.find(std::string(variant))->second;
                SFPositionPtr newPos = std::make_unique<Stockfish::Position>();
                // TODO: there are options related to the FEN that we may need to use here?
                newPos->set(v, getFEN(), isChess960, &newStates->back(), Stockfish::Threads.main());
                return newPos;
            }

            // This constructor is private
            Position(
                std::string _variant,
                bool _isChess960,
                std::shared_ptr<const Stockfish::Position> position,
                Stockfish::StateListPtr _states
            )
                : variant(_variant)
                , isChess960(_isChess960)
                , position{position}
                , states{copy(_states)}
            {
            }

            Stockfish::Notation fromOurNotation(fairystockfish::Notation n) const {
                switch (n) {
                    case fairystockfish::NOTATION_DEFAULT: return Stockfish::NOTATION_DEFAULT;
                    case fairystockfish::NOTATION_SAN: return Stockfish::NOTATION_SAN;
                    case fairystockfish::NOTATION_LAN: return Stockfish::NOTATION_LAN;
                    case fairystockfish::NOTATION_SHOGI_HOSKING: return Stockfish::NOTATION_SHOGI_HOSKING;
                    case fairystockfish::NOTATION_SHOGI_HODGES: return Stockfish::NOTATION_SHOGI_HODGES;
                    case fairystockfish::NOTATION_SHOGI_HODGES_NUMBER: return Stockfish::NOTATION_SHOGI_HODGES_NUMBER;
                    case fairystockfish::NOTATION_JANGGI: return Stockfish::NOTATION_JANGGI;
                    case fairystockfish::NOTATION_XIANGQI_WXF: return Stockfish::NOTATION_XIANGQI_WXF;
                }
                return Stockfish::NOTATION_DEFAULT;
            }

        public:

            Position(
                std::string _variant,
                std::string startingFen,
                MoveList const &moveList,
                bool _isChess960
            )
                : variant(_variant)
                , isChess960(_isChess960)
                , position{}
                , states{Stockfish::StateListPtr(new std::deque<Stockfish::StateInfo>(1))}
            {
                const Stockfish::Variant* v = Stockfish::variants.find(std::string(variant))->second;
                std::shared_ptr<Stockfish::Position> p =
                    std::make_shared<Stockfish::Position>();
                p->set(v, startingFen, isChess960, &states->back(), Stockfish::Threads.main());

                // Make the moves in the given position
                for (auto moveStr : moveList) {
                    Stockfish::Move m = Stockfish::UCI::to_move(*p, moveStr);
                    if (m == Stockfish::MOVE_NONE) throw std::runtime_error("Invalid Move: '" + moveStr + "'");
                    // do the move
                    states->emplace_back();
                    p->do_move(m, states->back());
                }
                position = p;
            }

            Position(Position const &p)
                : variant(p.variant)
                , isChess960(p.isChess960)
                , position(p.position)
                , states(copy(p.states))
            {
            }
            Position(Position&&) = default;
            Position& operator=(Position const &p) {
                variant = p.variant;
                isChess960 = p.isChess960;
                position = p.position;
                states = copy(p.states);
                return *this;
            }
            Position& operator=(Position&&) = default;
            virtual ~Position() = default;

            ///------------------------------------------------------------------------------
            /// Returns a new, updated positions with the given moves
            ///------------------------------------------------------------------------------
            Position makeMoves(MoveList const &uciMoves) const {
                Stockfish::StateListPtr newStates = copy(states);
                SFPositionPtr p = copyPosition(newStates);
                // Make the moves in the given position
                for (auto moveStr : uciMoves) {
                    Stockfish::Move m = Stockfish::UCI::to_move(*p, moveStr);
                    if (m == Stockfish::MOVE_NONE) throw std::runtime_error("Invalid Move: '" + moveStr + "'");
                    // do the move
                    states->emplace_back();
                    p->do_move(m, states->back());
                }

                return Position(variant, isChess960, std::move(p), std::move(newStates));
            }

            ///------------------------------------------------------------------------------
            /// Converts a UCI move into a SAN notation move given the variant and fen and
            /// whether it's chess960 or not.
            ///
            /// @param uciMove The move in UCI notation.
            /// @param notation The desired SAN notation.
            ///
            /// @return The move in SAN notation
            ///------------------------------------------------------------------------------
            std::string getSAN(
                std::string uciMove,
                Notation notation=Notation::NOTATION_DEFAULT
            ) const {
                return this->getSANMoves({uciMove}, notation)[0];
            }

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
                Notation ourNotation=Notation::NOTATION_DEFAULT
            ) const {
                Stockfish::Notation notation = fromOurNotation(ourNotation);
                if (notation == Stockfish::NOTATION_DEFAULT)
                    notation = Stockfish::default_notation(
                        Stockfish::variants.find(variant)->second
                    );

                // make a copy of the previous states
                // TODO: this copy may be pessimistic. I'd need to understand _why_
                //       move_to_san needs a non-const ref to Position
                Stockfish::StateListPtr newStates = copy(states);
                SFPositionPtr p = copyPosition(newStates);

                std::vector<std::string> retVal;
                for (auto uciMove : uciMoves) {

                    Stockfish::Move m = Stockfish::UCI::to_move(*p, uciMove);
                    if (m == Stockfish::MOVE_NONE) throw std::runtime_error("Invalid Move: '" + uciMove + "'");

                    retVal.push_back(Stockfish::SAN::move_to_san(*p, m, notation));

                    states->emplace_back();
                    p->do_move(m, newStates->back());

                }
                return retVal;
            }

            ///------------------------------------------------------------------------------
            /// Get legal moves from a given FEN and move list.
            ///
            /// @return a vector of legal moves in UCI notation
            ///------------------------------------------------------------------------------
            std::vector<std::string> getLegalMoves() const {
                std::vector<std::string> legalMoves;
                for (auto const &m : Stockfish::MoveList<Stockfish::LEGAL>(*position)) {
                    legalMoves.push_back(Stockfish::UCI::move(*position, m));
                }
                return legalMoves;
            }

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
            std::string getFEN(
                bool sFen=false,
                bool showPromoted=false,
                int countStarted=0
            ) const {
                countStarted = std::min<unsigned int>(countStarted, INT_MAX); // pseudo-unsigned
                return position->fen(sFen, showPromoted, countStarted);
            }

            ///------------------------------------------------------------------------------
            /// Get check status from a given fen and movelist.
            ///
            /// @return the FEN of the new position
            ///------------------------------------------------------------------------------
            bool givesCheck() const {
                return position->checkers() ? true : false;
            }

            ///------------------------------------------------------------------------------
            /// Gets result from a given FEN, considering variant end, checkmate and stalemate
            /// Note that this function will assert there are no legal moves. So only call it
            /// when legalMoves are zero
            ///
            /// @return Returns an integer that represents the result (don't know what it
            ///         means yet
            ///------------------------------------------------------------------------------
            int gameResult() const {
                assert(!Stockfish::MoveList<Stockfish::LEGAL>(*position).size());
                Stockfish::Value result;
                bool gameEnd = position->is_immediate_game_end(result);
                if (!gameEnd)
                    result =
                        position->checkers()
                        ? position->checkmate_value()
                        : position->stalemate_value();
                return int(result);
            }

            ///------------------------------------------------------------------------------
            /// Figures out if variant rules immediately end the game.
            /// Note that this does not mean checkmate, checkmates are somewhere else.
            ///
            /// @return Returns a boolean indicating if the game is an immediate end as well
            ///         as the integer result value.
            ///------------------------------------------------------------------------------
            std::tuple<bool, int> isImmediateGameEnd() const {
                Stockfish::Value result = Stockfish::VALUE_ZERO;
                bool gameEnd = position->is_immediate_game_end(result);
                return std::make_tuple(gameEnd, int(result));
            }

            ///------------------------------------------------------------------------------
            /// Get result from given FEN if rules enable game end by player.
            ///
            /// @param countStarted No clue. (probably has to do with games that end by certain
            ///                     types of counts, like repetitions?)
            ///
            /// @return Returns a boolean indicating if the game is an immediate end as well
            ///         as the integer result value.
            ///------------------------------------------------------------------------------
            std::tuple<bool, int> isOptionalGameEnd(int countStarted=0) const {
                countStarted = std::min<unsigned int>(countStarted, INT_MAX); // pseudo-unsigned

                Stockfish::Value result;
                bool gameEnd = position->is_optional_game_end(result, 0, countStarted);
                return std::make_tuple(gameEnd, int(result));
            }

            ///------------------------------------------------------------------------------
            /// Returns whether it's a draw or not.
            ///
            /// @param ply the number of ply to consider.
            ///
            /// @return Returns a boolean indicating if the game is an immediate end as well
            ///         as the integer result value.
            ///------------------------------------------------------------------------------
            bool isDraw(int ply) const {
                return position->is_draw(ply);
            }

            ///------------------------------------------------------------------------------
            /// Checks for insufficient material on behalf of both players.
            ///
            /// @return Returns two booleans, one for each player which indicate if that player
            ///         has sufficient material
            ///------------------------------------------------------------------------------
            std::tuple<bool, bool> hasInsufficientMaterial() const {
                bool wInsufficient = Stockfish::has_insufficient_material(
                    Stockfish::WHITE, *position);
                bool bInsufficient = Stockfish::has_insufficient_material(
                    Stockfish::BLACK, *position);
                return std::make_tuple(wInsufficient, bInsufficient);
            }

            ///------------------------------------------------------------------------------
            /// Tests if the position has a mvoe which draws by repetition,or an earlier
            /// position has a move that directly reaches the current position.
            ///
            /// @param ply The number of ply to search (Don't really know?)
            ///
            /// @return A vectors of pieces that are "in hand"
            ///------------------------------------------------------------------------------
            bool hasGameCycle(int ply) const {
                return position->has_game_cycle(ply);
            }

            ///------------------------------------------------------------------------------
            /// Tests whether there has been at least one repetition of positions since the
            /// last capture or pawn move.
            ///
            /// @return A vectors of pieces that are "in hand"
            ///------------------------------------------------------------------------------
            bool hasRepeated() const  {
                return position->has_repeated();
            }

            ///------------------------------------------------------------------------------
            /// Returns a piece map for a given position and variant.
            ///
            /// @return The map from UCI square notation to piece id integers.
            ///------------------------------------------------------------------------------
            std::map<std::string, Piece> piecesOnBoard() const {
                std::map<std::string, Piece> retVal;
                const Stockfish::Variant *v = Stockfish::variants[variant];

                for(Stockfish::File f = Stockfish::File::FILE_A; f <= v->maxFile; ++f) {
                    for(Stockfish::Rank r = Stockfish::Rank::RANK_1; r <= v->maxRank; ++r) {
                        Stockfish::Square s = make_square(f, r);
                        Stockfish::Piece unpromotedPiece = position->unpromoted_piece_on(s);
                        Stockfish::Piece p = position->piece_on(s);
                        bool promoted = false;
                        if (unpromotedPiece) {
                            p = unpromotedPiece;
                            promoted = true;
                        }
                        if (p == Stockfish::Piece::NO_PIECE) continue;
                        Stockfish::PieceType pt = type_of(p);
                        Stockfish::Color c = color_of(p);

                        std::string uciSquare = Stockfish::UCI::square(
                            *position,
                            s
                        );
                        retVal.insert({uciSquare, fairystockfish::Piece(pt, c, promoted)});
                    }
                }
                return retVal;
            }

            ///------------------------------------------------------------------------------
            /// Returns pieces in hand. It returns a single vector where pieces can be of
            /// either color. So it's up to the caller to filter them
            ///
            /// @return A vectors of pieces that are "in hand"
            ///------------------------------------------------------------------------------
            std::vector<Piece> piecesInHand() const {
                std::vector<Piece> retVal;
                for (int _c = static_cast<int>(Stockfish::Color::WHITE);
                        _c <= static_cast<int>(Stockfish::Color::BLACK); ++_c) {
                    Stockfish::Color c = static_cast<Stockfish::Color>(_c);
                    for (auto const &[id, info] : Stockfish::pieceMap) {
                        size_t numInHand = size_t(position->count_in_hand(c, id));
                        for(size_t i = 0; i < numInHand; ++i) {
                            retVal.push_back(Piece(id, c));
                        }
                    }
                }
                return retVal;
            }
    };


}

#endif // FAIRYSTOCKFISH_H
