#include "fairystockfish.h"

#include <iostream>
#include <climits>
#include <sstream>
#include <set>
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
std::string fairystockfish::version() { return "v0.0.11"; }

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

enum class IsPromotable {
    PROMOTABLE,
    ALL
};

static std::string availablePieceChars__impl(IsPromotable t) {
    std::set<char> pieces;
    auto addPiece = [&](char p) {
        if (p != ' ') pieces.insert(p);
    };

    auto copy = [&](auto const &pieceTypes, SF::Variant const *variant) {
        for (const auto &pt : pieceTypes) {
            addPiece(variant->pieceToChar[make_piece(SF::WHITE, pt)]);
            addPiece(variant->pieceToChar[make_piece(SF::BLACK, pt)]);
            addPiece(variant->pieceToCharSynonyms[make_piece(SF::WHITE, pt)]);
            addPiece(variant->pieceToCharSynonyms[make_piece(SF::BLACK, pt)]);
        }
    };
    for (const auto &[name, variant] : SF::variants) {
        if (t == IsPromotable::PROMOTABLE) {
            copy(variant->promotionPieceTypes, variant);
        } else {
            copy(variant->pieceTypes, variant);
        }
    }
    std::string retVal;
    retVal.resize(pieces.size());
    std::copy(pieces.begin(), pieces.end(), retVal.begin());
    return retVal;
}

std::string fairystockfish::availablePieceChars() {
    return availablePieceChars__impl(IsPromotable::ALL);
}

std::string fairystockfish::availablePromotablePieceChars() {
    return availablePieceChars__impl(IsPromotable::PROMOTABLE);
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

void fairystockfish::Position::init(std::string startingFen, bool _isChess960) {
    const Stockfish::Variant* v = Stockfish::variants.find(std::string(variant))->second;

    ListOfImmutableStatesPtr inputStates = std::make_shared<std::list<StateInfoPtr>>();
    MutableStateInfoPtr firstState = std::make_shared<Stockfish::StateInfo>();
    inputStates->push_back(firstState);

    std::shared_ptr<Stockfish::Position> p =
        std::make_shared<Stockfish::Position>();
    p->set(v, startingFen, isChess960, firstState.get(), Stockfish::Threads.main());
    position = p;
    states = std::move(inputStates);
}

fairystockfish::Position::Position(
    std::string _variant,
    bool _isChess960
)
    : variant(_variant)
    , isChess960(_isChess960)
    , position{}
    , states{}
{
    // TODO: make this safer (throw an exception?)
    const Stockfish::Variant* v = Stockfish::variants.find(variant)->second;

    std::string fen = v->startFen;
    init(fen, _isChess960);
}

fairystockfish::Position::Position(
    std::string _variant, std::string startingFen, bool _isChess960
)
    : variant(_variant)
    , isChess960(_isChess960)
    , position{}
    , states{}
{
    init(startingFen, _isChess960);
}

fairystockfish::Position::SFPositionPtr fairystockfish::Position::copyPosition(
    fairystockfish::Position::SFPositionConstPtr const &ptr
) const  {
    // This depends on the idea that the only pointers that a position has are
    // the stateinfo (which we will update) and the thread pointer (which is always
    // the same for us), so we can safely bitwise copy the position.
    MutableStateInfoPtr firstState = std::make_shared<Stockfish::StateInfo>();
    states->push_back(firstState);
    std::shared_ptr<Stockfish::Position> p =
        std::make_shared<Stockfish::Position>();
    std::memcpy(
        p.get(),
        ptr.get(),
        sizeof(Stockfish::Position)
    );

    return p;
}

Stockfish::Notation fairystockfish::Position::fromOurNotation(fairystockfish::Notation n) const {
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

fairystockfish::Position fairystockfish::Position::makeMoves(MoveList const &uciMoves) const {
    SFPositionPtr p = copyPosition(position);

    // Make the moves in the given position
    for (auto moveStr : uciMoves) {
        Stockfish::Move m = Stockfish::UCI::to_move(*p, moveStr);
        if (m == Stockfish::MOVE_NONE) throw std::runtime_error("Invalid Move: '" + moveStr + "'");
        // do the move
        MutableStateInfoPtr newState = std::make_shared<Stockfish::StateInfo>();
        states->push_back(newState);
        p->do_move(m, *newState);
    }

    Position newPosition = *this;
    newPosition.position = p;
    return newPosition;
}

std::string fairystockfish::Position::getSAN(std::string uciMove, Notation notation) const {
    return this->getSANMoves({uciMove}, notation)[0];
}

std::vector<std::string> fairystockfish::Position::getSANMoves(
    std::vector<std::string> uciMoves,
    Notation ourNotation
) const {
    Stockfish::Notation notation = fromOurNotation(ourNotation);
    if (notation == Stockfish::NOTATION_DEFAULT)
        notation = Stockfish::default_notation(
            Stockfish::variants.find(variant)->second
        );

    // make a copy of the previous states
    // TODO: this copy may be pessimistic. I'd need to understand _why_
    //       move_to_san needs a non-const ref to Position
    SFPositionPtr p = copyPosition(position);

    ListOfImmutableStatesPtr tmpStates = std::make_shared<std::list<StateInfoPtr>>();
    std::vector<std::string> retVal;
    for (auto uciMove : uciMoves) {

        Stockfish::Move m = Stockfish::UCI::to_move(*p, uciMove);
        if (m == Stockfish::MOVE_NONE) throw std::runtime_error("Invalid Move: '" + uciMove + "'");

        retVal.push_back(Stockfish::SAN::move_to_san(*p, m, notation));
        MutableStateInfoPtr newState = std::make_shared<Stockfish::StateInfo>();
        tmpStates->push_back(newState);
        p->do_move(m, *newState);
    }
    return retVal;
}

std::vector<std::string> fairystockfish::Position::getLegalMoves() const {
    std::vector<std::string> legalMoves;
    for (auto const &m : Stockfish::MoveList<Stockfish::LEGAL>(*position)) {
        legalMoves.push_back(Stockfish::UCI::move(*position, m));
    }
    return legalMoves;
}

std::string fairystockfish::Position::getFEN(
    bool sFen,
    bool showPromoted,
    int countStarted
) const {
    countStarted = std::min<unsigned int>(countStarted, INT_MAX); // pseudo-unsigned
    return position->fen(sFen, showPromoted, countStarted);
}

bool fairystockfish::Position::givesCheck() const {
    return position->checkers() ? true : false;
}

int fairystockfish::Position::gameResult() const {
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

std::tuple<bool, int> fairystockfish::Position::isImmediateGameEnd() const {
    Stockfish::Value result = Stockfish::VALUE_ZERO;
    bool gameEnd = position->is_immediate_game_end(result);
    return std::make_tuple(gameEnd, int(result));
}

std::tuple<bool, int> fairystockfish::Position::isOptionalGameEnd(int countStarted) const {
    countStarted = std::min<unsigned int>(countStarted, INT_MAX); // pseudo-unsigned

    Stockfish::Value result;
    bool gameEnd = position->is_optional_game_end(result, 0, countStarted);
    return std::make_tuple(gameEnd, int(result));
}

bool fairystockfish::Position::isDraw(int ply) const {
    return position->is_draw(ply);
}

std::tuple<bool, bool> fairystockfish::Position::hasInsufficientMaterial() const {
    bool wInsufficient = Stockfish::has_insufficient_material(
        Stockfish::WHITE, *position);
    bool bInsufficient = Stockfish::has_insufficient_material(
        Stockfish::BLACK, *position);
    return std::make_tuple(wInsufficient, bInsufficient);
}

bool fairystockfish::Position::hasGameCycle(int ply) const {
    return position->has_game_cycle(ply);
}

bool fairystockfish::Position::hasRepeated() const {
    return position->has_repeated();
}

std::map<std::string, fairystockfish::Piece> fairystockfish::Position::piecesOnBoard() const {
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

std::vector<fairystockfish::Piece> fairystockfish::Position::piecesInHand() const {
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
