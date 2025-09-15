#pragma once
#include <Geode/Geode.hpp>

namespace tulip::editor {
    class ThemeProvider {
        ThemeProvider();
    public:
        static ThemeProvider* get();

        cocos2d::ccColor3B main50;
        cocos2d::ccColor3B main100;
        cocos2d::ccColor3B main200;
        cocos2d::ccColor3B main300;
        cocos2d::ccColor3B main400;
        cocos2d::ccColor3B main500;
        cocos2d::ccColor3B main600;
        cocos2d::ccColor3B main700;
        cocos2d::ccColor3B main800;
        cocos2d::ccColor3B main900;

        cocos2d::ccColor3B accent50;
        cocos2d::ccColor3B accent100;
        cocos2d::ccColor3B accent200;
        cocos2d::ccColor3B accent300;
        cocos2d::ccColor3B accent400;
        cocos2d::ccColor3B accent500;
        cocos2d::ccColor3B accent600;
        cocos2d::ccColor3B accent700;
        cocos2d::ccColor3B accent800;
        cocos2d::ccColor3B accent900;

    };
}