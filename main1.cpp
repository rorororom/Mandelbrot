#include <SFML/Graphics.hpp>
#include <cmath>
#include <string>

const int MAX_ITERATIONS = 256;

void writeFPS(sf::RenderWindow& window, sf::Text& fpsText, sf::Clock& gameClock, int& frames) {
    frames++;
    sf::Time elapsed = gameClock.getElapsedTime();
    if (elapsed.asSeconds() >= 1.0f) {
        float fps = frames / elapsed.asSeconds();
        std::string fpsStr = "FPS: " + std::to_string(static_cast<int>(fps));

        fpsText.setString(fpsStr);
        frames = 0;
        gameClock.restart();
    }

    window.draw(fpsText);
}

int mandelbrot(float x0, float y0) {
    float x = 0.0f;
    float y = 0.0f;
    int iteration = 0;

    while (x*x + y*y <= 100.0f && iteration < MAX_ITERATIONS) {
        float xtemp = x*x - y*y + x0;
        y = 2*x*y + y0;
        x = xtemp;
        iteration++;
    }

    if (iteration == MAX_ITERATIONS)
        return 0;
    else
        return iteration;
}

int main() {
    const int WIDTH = 800;
    const int HEIGHT = 600;

    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Mandelbrot Set");

    sf::Image image;
    image.create(WIDTH, HEIGHT, sf::Color::Black);

    sf::Texture texture;
    texture.create(WIDTH, HEIGHT);
    sf::Sprite sprite(texture);
    // sf::Font::loadFromFile();

    sf::Font font;
    text.setFont(font);
    sf::Text fpsText("", font, 20);
    fpsText.setColor(sf::Color::Red);
    fpsText.setPosition(10, 10);

    sf::Clock gameClock;
    int frames = 0;
    size_t fps_ct = 0;

    if (!font.loadFromFile("arial.ttf")) {
        fprintf(stderr, "Failed to load font!\n");
        return 1;
    }

    float xC = 0.f, yC = 0.f; // Переменные для смещения

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::KeyPressed) {
                // Обработка нажатий
                if (event.key.code == sf::Keyboard::Right) {
                    // Смещаем изображение вправо
                    xC += 0.1f;
                } else if (event.key.code == sf::Keyboard::Left) {
                    // Смещаем изображение влево
                    xC -= 0.1f;
                } else if (event.key.code == sf::Keyboard::Up) {
                    // Смещаем изображение вверх
                    yC -= 0.1f;
                } else if (event.key.code == sf::Keyboard::Down) {
                    // Смещаем изображение вниз
                    yC += 0.1f;
                }
            }
        }

        // Отрисовка Mandelbrot Set
        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                float xx = (float)x / WIDTH * 3.5f - 2.5f;
                float yy = (float)y / HEIGHT * 2.0f - 1.0f;
                int color = mandelbrot(xx, yy);
                sf::Color sfColor((color * 6) % 256, 0, (color * 10) % 256);
                image.setPixel(x, y, sfColor);
            }
        }

        texture.update(image);

        window.clear();
        window.draw(sprite);

        writeFPS(window, fpsText, gameClock, frames);

        window.display(); // Отображаем все нарисованное на экране
    }

    return 0;
}

