#include "fairystockfish.h"

#include <iostream>
#include <climits>
#include <sstream>
#include <mutex>

#include "tabulate.hpp"

namespace SF = Stockfish;

static bool _fairystockfish_is_initialized = false;
static std::mutex _canInitialize;
const int fairystockfish::VALUE_ZERO = 0;
const int fairystockfish::VALUE_DRAW = 0;
const int fairystockfish::VALUE_MATE = 32000;


//------------------------------------------------------------------------------
// This struct is an internal API intended to build a position from variant,
// fen, a set of moves and whether or not it's chess960.
//------------------------------------------------------------------------------
struct PositionAndStates {
    using MoveList = std::vector<std::string>;
    std::unique_ptr<SF::Position> pos;
    SF::StateListPtr states;

    PositionAndStates(
        std::string variant,
        std::string fen,
        MoveList const &moveList,
        const bool isChess960
    )
        : pos{std::make_unique<SF::Position>()}
        , states{SF::StateListPtr(new std::deque<SF::StateInfo>(1))}
    {
        const SF::Variant* v = SF::variants.find(std::string(variant))->second;

        // Figure out starting fen.
        if (fen == "startpos") fen = v->startFen;

        // Setup the position
        pos->set(v, fen, isChess960, &states->back(), SF::Threads.main());

        // Make the moves in the given position
        for (auto moveStr : moveList) {
            SF::Move m = SF::UCI::to_move(*pos, moveStr);
            if (m == SF::MOVE_NONE) throw std::runtime_error("Invalid Move: '" + moveStr + "'");
            // do the move
            states->emplace_back();
            pos->do_move(m, states->back());
        }
    }
};

Stockfish::Notation fromOurNotation(fairystockfish::Notation n) {
    switch (n) {
        case fairystockfish::NOTATION_DEFAULT: return SF::NOTATION_DEFAULT;
        case fairystockfish::NOTATION_SAN: return SF::NOTATION_SAN;
        case fairystockfish::NOTATION_LAN: return SF::NOTATION_LAN;
        case fairystockfish::NOTATION_SHOGI_HOSKING: return SF::NOTATION_SHOGI_HOSKING;
        case fairystockfish::NOTATION_SHOGI_HODGES: return SF::NOTATION_SHOGI_HODGES;
        case fairystockfish::NOTATION_SHOGI_HODGES_NUMBER: return SF::NOTATION_SHOGI_HODGES_NUMBER;
        case fairystockfish::NOTATION_JANGGI: return SF::NOTATION_JANGGI;
        case fairystockfish::NOTATION_XIANGQI_WXF: return SF::NOTATION_XIANGQI_WXF;
    }
    return SF::NOTATION_DEFAULT;
}

fairystockfish::PieceInfo::PieceInfo()
    : pieceType{static_cast<SF::PieceType>(0)}
{}

fairystockfish::PieceInfo::PieceInfo(int pt)
    : pieceType{static_cast<SF::PieceType>(pt)}
{}

std::string fairystockfish::PieceInfo::name() const {
    return SF::pieceMap[pieceType]->name;
}

std::string fairystockfish::PieceInfo::betza() const {
    return SF::pieceMap[pieceType]->betza;
}

int fairystockfish::PieceInfo::id() const {
    return static_cast<int>(pieceType);
}


fairystockfish::Piece::Piece(int pt, int color)
    : _pieceInfo{pt}
    , _color{static_cast<SF::Color>(color)}
{
}

fairystockfish::PieceInfo fairystockfish::Piece::pieceInfo() const {
    return _pieceInfo;
}

int fairystockfish::Piece::color() const {
    return static_cast<int>(_color);
}

void fairystockfish::init() {
    std::lock_guard<std::mutex> guard(_canInitialize);
    if (_fairystockfish_is_initialized) {
        return;
    }
    _fairystockfish_is_initialized = true;

    // initialize stockfish
    SF::pieceMap.init();
    SF::variants.init();
    SF::UCI::init(SF::Options);
    SF::PSQT::init(SF::variants.find(SF::Options["UCI_Variant"])->second);
    SF::Bitboards::init();
    SF::Position::init();
    SF::Bitbases::init();
    SF::Search::init();
    SF::Threads.set(SF::Options["Threads"]);
    SF::Search::clear(); // After threads are up

    // Initialize all variants.
    for (const auto &[name, variant] : SF::variants) {
        // Initialize the variant.
        SF::UCI::init_variant(variant);
    }
}

// TODO: make it so that the version number comes from compile time settings.
std::string fairystockfish::version() { return "v0.0.1"; }

void fairystockfish::info() {
    // Now print out some information
    using namespace tabulate;
    Table variantTable;
    std::cout << "[Fairy-Stockfish-Lib] Available Variants" << std::endl;
    variantTable.add_row({"Variant Name", "Initial FEN"});
    for (auto const & name : availableVariants()) {
        variantTable.add_row({name, initialFen(name)});
    }
    std::cout << variantTable << std::endl;

    Table pieceTable;
    std::cout << "[Fairy-Stockfish-Lib] Available Pieces" << std::endl;
    for (auto const &[name, info] : availablePieces()) {
        std::cout << "val " << info.name() << " = " << info.id() << std::endl;
    }
}

void fairystockfish::setUCIOption(std::string name, std::string value) {
    if (SF::Options.count(name)) SF::Options[name] = value;
    else throw std::runtime_error("Unrecognized option");
}

void fairystockfish::loadVariantConfig(std::string config) {
    std::stringstream ss(config);
    SF::variants.parse_istream<false>(ss);
    SF::Options["UCI_Variant"].set_combo(SF::variants.get_keys());
}

std::vector<std::string> fairystockfish::availableVariants() {
    return SF::variants.get_keys();
}

std::string fairystockfish::initialFen(std::string variantName) {
    return SF::variants[variantName]->startFen;
}

std::map<std::string, fairystockfish::PieceInfo> fairystockfish::availablePieces() {
    std::map<std::string, PieceInfo> retVal;
    for (auto const &[id, info] : SF::pieceMap) {
        retVal[info->name] = PieceInfo(id);
    }
    return retVal;
}

std::string fairystockfish::getSAN(
    std::string variantName,
    std::string fen,
    std::string uciMove,
    bool isChess960,
    Notation ourNotation
) {
    // Can reuse the method below with a single move in the move list.
    return getSANMoves(variantName, fen, {uciMove}, isChess960, ourNotation)[0];
}

std::vector<std::string> fairystockfish::getSANMoves(
    std::string variantName,
    std::string fen,
    std::vector<std::string> uciMoves,
    bool isChess960,
    Notation ourNotation
) {
    SF::Notation notation = fromOurNotation(ourNotation);
    if (notation == SF::NOTATION_DEFAULT)
        notation = SF::default_notation(SF::variants.find(variantName)->second);

    PositionAndStates posAndStates(variantName, fen, {}, isChess960);

    std::vector<std::string> retVal;
    for (auto uciMove : uciMoves) {

        SF::Move m = SF::UCI::to_move(*posAndStates.pos, uciMove);
        if (m == SF::MOVE_NONE) throw std::runtime_error("Invalid Move: '" + uciMove + "'");

        retVal.push_back(SF::SAN::move_to_san(*posAndStates.pos, m, notation));

        posAndStates.states->emplace_back();
        posAndStates.pos->do_move(m, posAndStates.states->back());

    }
    return retVal;
}

std::vector<std::string> fairystockfish::getLegalMoves(
    std::string variantName,
    std::string fen,
    std::vector<std::string> uciMoves,
    bool isChess960
) {
    PositionAndStates posAndStates(variantName, fen, uciMoves, isChess960);

    std::vector<std::string> legalMoves;
    for (auto const &m : SF::MoveList<SF::LEGAL>(*posAndStates.pos)) {
        legalMoves.push_back(SF::UCI::move(*posAndStates.pos, m));
    }

    return legalMoves;
}

std::string fairystockfish::getFEN(
    std::string variantName,
    std::string fen,
    std::vector<std::string> uciMoves,
    bool isChess960,
    bool sFen,
    bool showPromoted,
    int countStarted
) {
    countStarted = std::min<unsigned int>(countStarted, INT_MAX); // pseudo-unsigned

    PositionAndStates posAndStates(variantName, fen, uciMoves, isChess960);
    return posAndStates.pos->fen(sFen, showPromoted, countStarted);
}

bool fairystockfish::givesCheck(
    std::string variantName,
    std::string fen,
    std::vector<std::string> uciMoves,
    bool isChess960
) {
    PositionAndStates posAndStates(variantName, fen, uciMoves, isChess960);
    return posAndStates.pos->checkers() ? true : false;
}

int gameResult(
    std::string variantName,
    std::string fen,
    std::vector<std::string> uciMoves,
    bool isChess960=false
) {
    PositionAndStates posAndStates(variantName, fen, uciMoves, isChess960);
    assert(!SF::MoveList<SF::LEGAL>(*posAndStates.pos).size());
    SF::Value result;
    bool gameEnd = posAndStates.pos->is_immediate_game_end(result);
    if (!gameEnd)
        result =
            posAndStates.pos->checkers()
            ? posAndStates.pos->checkmate_value()
            : posAndStates.pos->stalemate_value();
    return int(result);
}

std::tuple<bool, int> fairystockfish::isImmediateGameEnd(
    std::string variantName,
    std::string fen,
    std::vector<std::string> uciMoves,
    bool isChess960
) {
    PositionAndStates posAndStates(variantName, fen, uciMoves, isChess960);

    SF::Value result;
    bool gameEnd = posAndStates.pos->is_immediate_game_end(result);
    return std::make_tuple(gameEnd, int(result));
}

std::tuple<bool, int> fairystockfish::isOptionalGameEnd(
    std::string variantName,
    std::string fen,
    std::vector<std::string> uciMoves,
    bool isChess960,
    int countStarted
) {
    countStarted = std::min<unsigned int>(countStarted, INT_MAX); // pseudo-unsigned

    PositionAndStates posAndStates(variantName, fen, uciMoves, isChess960);

    SF::Value result;
    bool gameEnd = posAndStates.pos->is_optional_game_end(result, 0, countStarted);

    gameEnd = posAndStates.pos->is_optional_game_end(result, 0, countStarted);
    return std::make_tuple(gameEnd, int(result));
}

std::tuple<bool, bool> fairystockfish::hasInsufficientMaterial(
    std::string variantName,
    std::string fen,
    std::vector<std::string> uciMoves,
    bool isChess960
) {
    PositionAndStates posAndStates(variantName, fen, uciMoves, isChess960);

    bool wInsufficient = SF::has_insufficient_material(SF::WHITE, *posAndStates.pos);
    bool bInsufficient = SF::has_insufficient_material(SF::BLACK, *posAndStates.pos);
    return std::make_tuple(wInsufficient, bInsufficient);
}

bool fairystockfish::validateFEN(
    std::string variantName,
    std::string fen,
    bool isChess960
) {

    return SF::FEN::validate_fen(
        fen,
        SF::variants.find(variantName)->second,
        isChess960
    );
}

std::map<std::string, fairystockfish::Piece>
fairystockfish::piecesOnBoard(
    std::string variantName,
    std::string fen,
    bool isChess960
) {
    std::map<std::string, Piece> retVal;
    PositionAndStates posAndStates(variantName, fen, {}, isChess960);

    for(SF::Square s = SF::Square::SQ_A1; s <= SF::Square::SQ_L10; ++s) {
        SF::Piece p = posAndStates.pos->piece_on(s);
        if (p == SF::Piece::NO_PIECE) continue;
        SF::PieceType pt = type_of(p);
        SF::Color c = color_of(p);

        std::string uciSquare = SF::UCI::square(
            *posAndStates.pos,
            s
        );
        retVal.insert({uciSquare, fairystockfish::Piece(pt, c)});
    }
    return retVal;
}

std::vector<fairystockfish::Piece> fairystockfish::piecesInHand(
    std::string variantName,
    std::string fen,
    bool isChess960
) {
    std::vector<Piece> retVal;
    PositionAndStates posAndStates(variantName, fen, {}, isChess960);
    for (int _c = static_cast<int>(SF::Color::WHITE);
            _c <= static_cast<int>(SF::Color::BLACK); ++_c) {
        SF::Color c = static_cast<SF::Color>(_c);
        for (auto const &[id, info] : SF::pieceMap) {
            auto numInHand = posAndStates.pos->count_in_hand(c, id);
            for(size_t i = 0; i < numInHand; ++i) {
                retVal.push_back(Piece(id, c));
            }
        }
    }
    return retVal;
}
