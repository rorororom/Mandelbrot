#include <SFML/Graphics.hpp>
#include <cmath>
#include <string>

const int   MAX_ITERATIONS = 256;
const int   WIDTH          = 800;
const int   HEIGHT         = 600;
const int   LIMIT          = 100.0;

// #define TIME_MEASURE

void writeFPS(sf::RenderWindow& window, sf::Text& fpsText, sf::Clock& gameClock, int& frames, int& cntForFps, unsigned long long& all_fps) {
    frames++;
    sf::Time elapsed = gameClock.getElapsedTime();
    if (elapsed.asSeconds() >= 1.0f) {
        float fps = frames / gameClock.getElapsedTime().asSeconds();
        all_fps += fps;
        if (cntForFps == LIMIT) {
            printf("FPS: %llu\n", all_fps / LIMIT);
        }
        std::string fpsStr = "FPS: " + std::to_string(static_cast<int>(fps));

        fpsText.setString(fpsStr);
        frames = 0;
        gameClock.restart();
    }

    window.draw(fpsText);
}

inline int mandelbrot(float x0, float y0) {
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

inline void handleKeyPress(sf::RenderWindow& window, float& xC, float& yC, float& zoom) {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            window.close();
        else if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Right) {
                xC += 0.1f; // Смещаем изображение вправо
            } else if (event.key.code == sf::Keyboard::Left) {
                xC -= 0.1f; // Смещаем изображение влево
            } else if (event.key.code == sf::Keyboard::Up) {
                yC -= 0.1f; // Смещаем изображение вверх
            } else if (event.key.code == sf::Keyboard::Down) {
                yC += 0.1f; // Смещаем изображение вниз
            } else if (event.key.code == sf::Keyboard::Equal) {
                zoom /= 1.1f; // Увеличиваем зум
            } else if (event.key.code == sf::Keyboard::Dash) {
                zoom *= 1.1f; // Уменьшаем зум
            }
        }
    }
}

inline void initialize(sf::RenderWindow& window, sf::Image& image, sf::Texture& texture, sf::Sprite& sprite, sf::Text& fpsText, sf::Font& font) {
    window.create(sf::VideoMode(WIDTH, HEIGHT), "Mandelbrot Set");

    image.create(WIDTH, HEIGHT, sf::Color::Black);
    texture.create(WIDTH, HEIGHT);
    sprite.setTexture(texture);

    font.loadFromFile("arial.ttf");
    fpsText.setFont(font);
    fpsText.setCharacterSize(20);
    fpsText.setFillColor(sf::Color::Red);
    fpsText.setPosition(10, 10);
}

inline void processEvents(sf::RenderWindow& window, float& xC, float& yC, float& zoom, sf::Image& image, sf::Texture& texture, sf::Sprite& sprite, sf::Text& fpsText, sf::Clock& gameClock, int& frames) {
    int cntForTick = 0, cntForFps = 0;
    unsigned long long all_time = 0, all_fps = 0;

    while (window.isOpen()) {
        handleKeyPress(window, xC, yC, zoom);

        #ifdef TIME_MEASURE
        unsigned long long start = __rdtsc();
        #endif

        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                float xx = (float)x / WIDTH * 3.5f * zoom - 2.5f + xC;
                float yy = (float)y / HEIGHT * 2.0f * zoom - 1.0f + yC;
                volatile int color = mandelbrot(xx, yy);

                #ifndef TIME_MEASURE
                sf::Color sfColor((color * 6) % 256, 0, (color * 10) % 256);
                image.setPixel(x, y, sfColor);
                #endif
            }
        }

        #ifdef TIME_MEASURE
        unsigned long long end = __rdtsc();
        unsigned long long elapsedTime = end - start;
        cntForTick++;
        #endif

        cntForFps++;
        all_time += elapsedTime;

        #ifdef TIME_MEASURE
        if (cntForTick == LIMIT) {
            printf("Elapsed time: %llu cycles\n", all_time / LIMIT);
        }
        #endif

        texture.update(image);
        window.clear();
        window.draw(sprite);

        writeFPS(window, fpsText, gameClock, frames, cntForFps, all_fps);

        window.display();
    }
}

int main() {
    sf::RenderWindow window;
    sf::Image        image;
    sf::Texture      texture;
    sf::Sprite       sprite;
    sf::Text         fpsText;
    sf::Font         font;

    initialize(window, image, texture, sprite, fpsText, font);

    sf::Clock gameClock;
    int frames = 0;

    float xC = 0.f, yC = 0.f, zoom = 1.0f;

    processEvents(window, xC, yC, zoom, image, texture, sprite, fpsText, gameClock, frames);

    return 0;
}
