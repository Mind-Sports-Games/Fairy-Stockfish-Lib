#include "fairystockfish.h"

#include <iostream>
#include <climits>
#include <sstream>
#include <mutex>

#include "tabulate.hpp"
#include "types.h"

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

Stockfish::Notation fairystockfish::fromOurNotation(fairystockfish::Notation n) {
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


fairystockfish::Piece::Piece(int pt, int color, bool promoted)
    : _pieceInfo{pt}
    , _color{static_cast<SF::Color>(color)}
    , _promoted{promoted}
{
}

fairystockfish::PieceInfo fairystockfish::Piece::pieceInfo() const {
    return _pieceInfo;
}

int fairystockfish::Piece::color() const {
    return static_cast<int>(_color);
}

bool fairystockfish::Piece::promoted() const {
    return _promoted;
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
std::string fairystockfish::version() { return "v0.0.3"; }

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
    Position p(variantName, fen, uciMoves, isChess960);
    return p.getSANMoves(uciMoves, ourNotation);
}

std::vector<std::string> fairystockfish::getLegalMoves(
    std::string variantName,
    std::string fen,
    std::vector<std::string> uciMoves,
    bool isChess960
) {
    Position p(variantName, fen, uciMoves, isChess960);
    return p.getLegalMoves();
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
    Position p(variantName, fen, uciMoves, isChess960);
    return p.getFEN();
}

bool fairystockfish::givesCheck(
    std::string variantName,
    std::string fen,
    std::vector<std::string> uciMoves,
    bool isChess960
) {
    Position p(variantName, fen, uciMoves, isChess960);
    return p.givesCheck();
}

int fairystockfish::gameResult(
    std::string variantName,
    std::string fen,
    std::vector<std::string> uciMoves,
    bool isChess960
) {
    Position p(variantName, fen, uciMoves, isChess960);
    return p.gameResult();
}

std::tuple<bool, int> fairystockfish::isImmediateGameEnd(
    std::string variantName,
    std::string fen,
    std::vector<std::string> uciMoves,
    bool isChess960
) {
    Position p(variantName, fen, uciMoves, isChess960);
    return p.isImmediateGameEnd();
}

std::tuple<bool, int> fairystockfish::isOptionalGameEnd(
    std::string variantName,
    std::string fen,
    std::vector<std::string> uciMoves,
    bool isChess960,
    int countStarted
) {
    Position p(variantName, fen, uciMoves, isChess960);
    return p.isOptionalGameEnd(countStarted);
}

bool fairystockfish::isDraw(
    std::string variantName,
    std::string fen,
    std::vector<std::string> uciMoves,
    int ply,
    bool isChess960
) {
    Position p(variantName, fen, uciMoves, isChess960);
    return p.isDraw(ply);
}

std::tuple<bool, bool> fairystockfish::hasInsufficientMaterial(
    std::string variantName,
    std::string fen,
    std::vector<std::string> uciMoves,
    bool isChess960
) {
    Position p(variantName, fen, uciMoves, isChess960);
    return p.hasInsufficientMaterial();
}


bool fairystockfish::hasGameCycle(
    std::string variantName,
    std::string fen,
    std::vector<std::string> uciMoves,
    int ply,
    bool isChess960
) {
    Position p(variantName, fen, uciMoves, isChess960);
    return p.hasGameCycle(ply);
}

bool fairystockfish::hasRepeated(
    std::string variantName,
    std::string fen,
    std::vector<std::string> uciMoves,
    bool isChess960
) {
    Position p(variantName, fen, uciMoves, isChess960);
    return p.hasRepeated();
}

bool fairystockfish::validateFEN(
    std::string variantName,
    std::string fen,
    bool isChess960
) {

    return FenValidation::FEN_OK == SF::FEN::validate_fen(
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
    Position p(variantName, fen, {}, isChess960);
    return p.piecesOnBoard();
}

std::vector<fairystockfish::Piece> fairystockfish::piecesInHand(
    std::string variantName,
    std::string fen,
    bool isChess960
) {
    Position p(variantName, fen, {}, isChess960);
    return p.piecesInHand();
}
