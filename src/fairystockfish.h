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


namespace fairystockfish {
    using Value = Stockfish::Value;
    using Notation = Stockfish::Notation;
    using FenValidation = Stockfish::FEN::FenValidation;

    void init ();
}

#endif // FAIRYSTOCKFISH_H
