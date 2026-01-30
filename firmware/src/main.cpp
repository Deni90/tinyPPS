#include "tiny_pps.h"

int main() {
    stdio_init_all();

    TinyPPS tiny_pps;
    // FIXME handle return value
    tiny_pps.initialize();

    while (true) {
        tiny_pps.handle();
    }
}
