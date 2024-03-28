#include <SFML/Graphics.hpp>
#include <cmath>
#include <string>

const int   MAX_ITERATIONS = 256;
const int   WIDTH          = 800;
const int   HEIGHT         = 600;
const float ZOOM_FACTOR    = 1.1f;
const float MOVE_FACTOR    = 0.1f;
const int   LIMIT          = 100;
const float RADIUS         = 100.0f;

inline void writeFPS(sf::RenderWindow& window, sf::Text& fpsText, sf::Clock& gameClock, int& frames, int& cntForFps, unsigned long long& all_fps);
inline void handleKeyPress(sf::RenderWindow& window, float& xC, float& yC, float& zoom);
inline void initialize(sf::RenderWindow& window, sf::Image& image, sf::Texture& texture, sf::Sprite& sprite, sf::Text& fpsText, sf::Font& font);
inline void processEvents(sf::RenderWindow& window, sf::Image& image, sf::Texture& texture, sf::Sprite& sprite, sf::Text& fpsText);
inline void mandelbrot(float X0[4], float Y0[4], int* color);

int main() {
    sf::RenderWindow window;
    sf::Image        image;
    sf::Texture      texture;
    sf::Sprite       sprite;
    sf::Text         fpsText;
    sf::Font         font;

    initialize(window, image, texture, sprite, fpsText, font);

    processEvents(window, image, texture, sprite, fpsText);

    return 0;
}

void writeFPS(sf::RenderWindow& window, sf::Text& fpsText, sf::Clock& gameClock, int& frames, int& cntForFps, unsigned long long& all_fps) {
    frames++;
    sf::Time elapsed = gameClock.getElapsedTime();
    if (elapsed.asSeconds() >= 1.0f) {
        float fps = frames / gameClock.getElapsedTime().asSeconds();
        all_fps += fps;
        printf("FPS: %f\n", fps);
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

// Function to handle key presses for movement and zoom
void handleKeyPress(sf::RenderWindow& window, float& xC, float& yC, float& zoom) {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            window.close();
        else if (event.type == sf::Event::KeyPressed) {
            switch (event.key.code) {
                case sf::Keyboard::Right:
                    xC += MOVE_FACTOR; // Move image right
                    break;
                case sf::Keyboard::Left:
                    xC -= MOVE_FACTOR; // Move image left
                    break;
                case sf::Keyboard::Up:
                    yC -= MOVE_FACTOR; // Move image up
                    break;
                case sf::Keyboard::Down:
                    yC += MOVE_FACTOR; // Move image down
                    break;
                case sf::Keyboard::Equal:
                    zoom /= ZOOM_FACTOR; // Zoom in
                    break;
                case sf::Keyboard::Dash:
                    zoom *= ZOOM_FACTOR; // Zoom out
                    break;
                default:
                    break;
            }
        }
    }
}

// Function to initialize window, image, texture, sprite, and font
void initialize(sf::RenderWindow& window, sf::Image& image, sf::Texture& texture, sf::Sprite& sprite, sf::Text& fpsText, sf::Font& font) {
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

// Function to draw the Mandelbrot set and handle events
inline void processEvents(sf::RenderWindow& window, sf::Image& image, sf::Texture& texture, sf::Sprite& sprite, sf::Text& fpsText) {
    int cntForTick = 0, cntForFps = 0, frames = 0;
    unsigned long long all_time = 0, all_fps = 0;

    sf::Clock gameClock;
    sf::Color scr[600][800];

    float xC = 0.f, yC = 0.f, zoom = 1.0f;

    while (window.isOpen()) {
        handleKeyPress(window, xC, yC, zoom);

        sf::Time elapsed = gameClock.getElapsedTime();
        frames++;
        if (elapsed.asSeconds() >= 1.0f) {
            float fps = frames / elapsed.asSeconds();
            all_fps += fps;
            if (cntForFps == LIMIT) {
                printf("FPS: %llu\n", all_fps / LIMIT);
            }
            std::string fpsStr = "FPS: " + std::to_string(static_cast<int>(fps));
            fpsText.setString(fpsStr);
            frames = 0;
            gameClock.restart();
        }

        #ifdef TIME_MEASURE
        unsigned long long start = __rdtsc();
        #endif

        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x += 4) {

                float X0[4] = {0};
                float Y0[4] = {0};

                for (int i = 0; i < 4; i++) {
                    X0[i] = (float)(x + i) / WIDTH  * 3.5f * zoom - 2.5f + xC;
                    Y0[i] = (float)y       / HEIGHT * 2.0f * zoom - 1.0f + yC;
                }

                int color[4] = {0};
                mandelbrot(X0, Y0, color);

                #ifndef TIME_MEASURE
                for (int i = 0; i < 4; i++) {
                    sf::Color sfColor((color[i] * 6) % 256, 0, (color[i] * 10) % 256);
                    scr[y][x + i] = sfColor;
                    image.setPixel((x + i), y, sfColor );
                }
                #endif
            }
        }

        #ifdef TIME_MEASURE
        unsigned long long end = __rdtsc();
        unsigned long long elapsedTime = end - start;
        all_time += elapsedTime;
        cntForTick++;
        #endif

        cntForFps++;

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

inline void mandelbrot(float X0[4], float Y0[4], int* color) {
    float X [4] = {0};
    float Y [4] = {0};

    for (int i = 0; i < 4; i++) {
        X[i] = X0[i];
        Y[i] = Y0[i];
    }

    for (int n = 0; n < MAX_ITERATIONS; n++) {
        float x2[4] = {}, y2[4] = {}, xy[4] = {}, r2[4] = {};
        for (int i = 0; i < 4; i++) {
            x2[i] = X[i] * X[i];
            y2[i] = Y[i] * Y[i];
            xy[i] = X[i] * Y[i];
            r2[i] = x2[i] + y2[i];
        }

        int cmp[4] = {};
        for (int i = 0; i < 4; i++) {
            if (r2[i] <= RADIUS)
                cmp[i] = 1;
        }

        int mask = 0;
        for (int i = 0; i < 4; i++) {
            mask |= (cmp[i] << i);
        }

        if (!mask) break;

        for (int i = 0; i < 4; i++) {
           color[i] += cmp[i];
           X[i] = x2[i] - y2[i] + X0[i];
           Y[i] = xy[i] + xy[i] + Y0[i];
        }

    }
}

