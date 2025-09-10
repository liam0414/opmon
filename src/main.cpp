#include <SFML/Graphics.hpp>

int main() {
    // Create a window (1280x720 with titlebar + close button)
    sf::RenderWindow window(sf::VideoMode(1280, 720), "SFML Test Window");

    // Limit framerate
    window.setFramerateLimit(60);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // Fill background
        window.clear(sf::Color::Black);

        // (optional: draw stuff here)

        window.display();
    }

    return 0;
}
