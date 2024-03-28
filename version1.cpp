#include <SFML/Graphics.hpp>
#include <cmath>
#include <string>

const int   MAX_ITERATIONS = 256;
const int   WIDTH          = 800;
const int   HEIGHT         = 600;
const int   LIMIT          = 100;
const float ZOOM_FACTOR    = 1.1f;
const float MOVE_FACTOR    = 0.1f;
const float RADIUS         = 100.0f;

#define TIME_MEASURE
// #define CNT_FPS

void writeFPS(sf::RenderWindow* window, sf::Text* fpsText, sf::Clock* gameClock, int* frames, int* cntForFps, unsigned long long* all_fps) {
    (*frames)++;
    sf::Time elapsed = gameClock->getElapsedTime();
    if (elapsed.asSeconds() >= 1.0f) {
        float fps = (*frames) / gameClock->getElapsedTime().asSeconds();

        #ifndef CNT_FPS
        *all_fps += fps;
        #endif

        #ifdef TIME_MEASURE
        if (*cntForFps == LIMIT) {
            printf("FPS: %f\n", *all_fps / LIMIT);
        }
        #endif
        std::string fpsStr = "FPS: " + std::to_string(static_cast<int>(fps));

        fpsText->setString(fpsStr);
        (*frames) = 0;
        gameClock->restart();
    }

    window->draw(*fpsText);
}

inline int mandelbrot(float x0, float y0) {
    float x = 0.0f;
    float y = 0.0f;
    int iteration = 0;

    while (x*x + y*y <= RADIUS && iteration < MAX_ITERATIONS) {
        float xtemp = x * x - y * y + x0;
        y = 2 * x * y + y0;
        x = xtemp;
        iteration++;
    }

    if (iteration == MAX_ITERATIONS)
        return 0;
    else
        return iteration;
}

inline void handleKeyPress(sf::RenderWindow* window, float* xC, float* yC, float* zoom) {
    sf::Event event;
    while (window->pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            window->close();
        else if (event.type == sf::Event::KeyPressed) {
            switch (event.key.code) {
                case sf::Keyboard::Right:
                    (*xC) += MOVE_FACTOR; // Move image right
                    break;
                case sf::Keyboard::Left:
                    (*xC) -= MOVE_FACTOR; // Move image left
                    break;
                case sf::Keyboard::Up:
                    (*yC) -= MOVE_FACTOR; // Move image up
                    break;
                case sf::Keyboard::Down:
                    (*yC) += MOVE_FACTOR; // Move image down
                    break;
                case sf::Keyboard::Equal:
                    (*zoom) /= ZOOM_FACTOR; // Zoom in
                    break;
                case sf::Keyboard::Dash:
                    (*zoom) *= ZOOM_FACTOR; // Zoom out
                    break;
                default:
                    break;
            }
        }
    }
}

inline void initialize(sf::RenderWindow* window, sf::Image* image, sf::Texture* texture, sf::Sprite* sprite, sf::Text* fpsText, sf::Font* font) {
    window->create(sf::VideoMode(WIDTH, HEIGHT), "Mandelbrot Set");

    image->create(WIDTH, HEIGHT, sf::Color::Black);
    texture->create(WIDTH, HEIGHT);
    sprite->setTexture(*texture);

    font->loadFromFile("arial.ttf");
    fpsText->setFont(*font);
    fpsText->setCharacterSize(20);
    fpsText->setFillColor(sf::Color::Red);
    fpsText->setPosition(10, 10);
}

inline void processEvents(sf::RenderWindow* window, float* xC, float* yC, float* zoom, sf::Image* image, sf::Texture* texture, sf::Sprite* sprite, sf::Text* fpsText, sf::Clock* gameClock, int* frames) {
    int cntForTick = 0, cntForFps = 0;
    unsigned long long all_time = 0, all_fps = 0;

    while (window->isOpen()) {
        handleKeyPress(window, xC, yC, zoom);

        #ifdef TIME_MEASURE
        unsigned long long start = __rdtsc();
        #endif

        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                float xx = (float)x / WIDTH * 3.5f * (*zoom) - 2.5f + (*xC);
                float yy = (float)y / HEIGHT * 2.0f * (*zoom) - 1.0f + (*yC);
                volatile int color = mandelbrot(xx, yy);

                #ifndef TIME_MEASURE
                sf::Color sfColor((color * 6) % 256, 0, (color * 10) % 256);
                image->setPixel(x, y, sfColor);
                #endif
            }
        }

        #ifdef TIME_MEASURE
        unsigned long long end = __rdtsc();
        all_time += end - start;
        cntForTick++;
        #endif

        cntForFps++;

        #ifdef TIME_MEASURE
        if (cntForTick == LIMIT) {
            printf("Elapsed time: %llu cycles\n", all_time / LIMIT);
        }
        #endif

        texture->update(*image);
        window->clear();
        window->draw(*sprite);

        writeFPS(window, fpsText, gameClock, frames, &cntForFps, &all_fps);

        window->display();
    }
}

int main() {
    sf::RenderWindow window;
    sf::Image        image;
    sf::Texture      texture;
    sf::Sprite       sprite;
    sf::Text         fpsText;
    sf::Font         font;

    initialize(&window, &image, &texture, &sprite, &fpsText, &font);

    sf::Clock gameClock;
    int frames = 0;

    float xC = 0.f, yC = 0.f, zoom = 1.0f;

    processEvents(&window, &xC, &yC, &zoom, &image, &texture, &sprite, &fpsText, &gameClock, &frames);

    return 0;
}

