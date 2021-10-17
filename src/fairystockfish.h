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
      std::string name = "";
      std::string betza = "";
    };

    void init ();

    std::vector<std::string> availableVariants();
    std::map<std::string, PieceInfo> availablePieces();
}

#endif // FAIRYSTOCKFISH_H
