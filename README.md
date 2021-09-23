This project exposes a subset of the Fairy-Stockfish API into a C++
library. The intended use case is for language specific wrappers.

Building:

```
cmake -H. -Bbuild -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=1
cmake --build build
```
