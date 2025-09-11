#include "scenes/MainMenuScene.h"
#include <iostream>

int main() {
    try {
        MainMenuScene mainMenu;
        mainMenu.run();
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
        return -1;
    }
    
    return 0;
}