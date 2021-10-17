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
    using Value = Stockfish::Value;
    using Notation = Stockfish::Notation;
    using FenValidation = Stockfish::FEN::FenValidation;

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

    void init();
    void info();

    std::vector<std::string> availableVariants();
    // info about variants
    // Note that this is unsafe and does not check
    // to ensure the variant name exists.
    std::string initialFen(std::string variantName);

    std::map<std::string, PieceInfo> availablePieces();
}

#endif // FAIRYSTOCKFISH_H
