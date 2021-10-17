#include "fairystockfish.h"

#include <iostream>
#include "tabulate.hpp"

namespace SF = Stockfish;

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

void fairystockfish::init() {

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
}

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
