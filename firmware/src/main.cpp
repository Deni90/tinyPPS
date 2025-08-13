#include "tiny_pps.h"

int main() {
    TinyPPS tiny_pps;

    // FIXME handle return value
    tiny_pps.initialize();

    while (true) {
        tiny_pps.handle();
    }
}
