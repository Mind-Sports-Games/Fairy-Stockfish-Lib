#include "fairystockfish.h"

#include <iostream>

namespace SF = Stockfish;

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

    // Now print out some information
    std::cout << "[Fairy-Stockfish-Lib] Available Variants" << std::endl;
    for (auto const & name : availableVariants()) {
        std::cout << "\t- " << name << std::endl;
    }

    std::cout << "[Fairy-Stockfish-Lib] Available Pieces" << std::endl;
    for (auto const &[name, info] : availablePieces()) {
        std::cout << "\t- " << name << " " << info.betza << std::endl;
    }
}

std::vector<std::string> fairystockfish::availableVariants() {
    return SF::variants.get_keys();
}

std::map<std::string, fairystockfish::PieceInfo> fairystockfish::availablePieces() {
    std::map<std::string, PieceInfo> retVal;
    for (auto const &[id, info] : SF::pieceMap) {
        retVal[info->name] = {info->name, info->betza};
    }
    return retVal;
}
