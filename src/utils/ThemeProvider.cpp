#include <utils/ThemeProvider.hpp>

using namespace tulip::editor;
using namespace geode::prelude;

// main: https://coolors.co/palette/f3eaf9-f1e3fc-ecd6fc-e9ccfc-e6c1ff-e1b7ff-dbaaff-d399ff-ca85ff-bb5cff
// accent: https://coolors.co/palette/ff0a54-ff477e-ff5c8a-ff7096-ff85a1-ff99ac-fbb1bd-f9bec7-f7cad0-fae0e4

ThemeProvider::ThemeProvider() {
    // main900 = ccColor3B(0xF3, 0xEA, 0xF9);
    // main800 = ccColor3B(0xF1, 0xE3, 0xFC);
    // main700 = ccColor3B(0xEC, 0xD6, 0xFC);
    // main600 = ccColor3B(0xE9, 0xCC, 0xFC);
    // main500 = ccColor3B(0xE6, 0xC1, 0xFF);
    // main400 = ccColor3B(0xE1, 0xB7, 0xFF);
    // main300 = ccColor3B(0xDB, 0xAA, 0xFF);
    // main200 = ccColor3B(0xD3, 0x99, 0xFF);
    // main100 = ccColor3B(0xCA, 0x85, 0xFF);
    // main50 = ccColor3B(0xBB, 0x5C, 0xFF);

    // accent900 = ccColor3B(0xFF, 0x0A, 0x54);
    // accent800 = ccColor3B(0xFF, 0x47, 0x7E);
    // accent700 = ccColor3B(0xFF, 0x5C, 0x8A);
    // accent600 = ccColor3B(0xFF, 0x70, 0x96);
    // accent500 = ccColor3B(0xFF, 0x85, 0xA1);
    // accent400 = ccColor3B(0xFF, 0x99, 0xAC);
    // accent300 = ccColor3B(0xFB, 0xB1, 0xBD);
    // accent200 = ccColor3B(0xF9, 0xBE, 0xC7);
    // accent100 = ccColor3B(0xF7, 0xCA, 0xD0);
    // accent50 = ccColor3B(0xFA, 0xE0, 0xE4);
}

ThemeProvider* ThemeProvider::get() {
    static ThemeProvider instance;
    return &instance;
}

