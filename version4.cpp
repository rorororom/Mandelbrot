#include <SFML/Graphics.hpp>
#include <cmath>
#include <immintrin.h>
#include <string.h>

const int    WIDTH          = 800;
const int    HEIGHT         = 600;
const float  ZOOM_FACTOR    = 1.1f;
const float  MOVE_FACTOR    = 0.1f;
const int    MAX_ITERATIONS = 256;
const float  RADIUS         = 100.0f;
const int    LIMIT          = 100.0;

#define TIME_MEASURE

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

// Функция для создания изображения множества Мандельброта
inline void mandelbrot(const __m128 X0, const __m128 Y0, volatile __m128& color) {
    __m128 X =  _mm_set_ps1(0); // X = {0, 0, 0, 0}
    __m128 Y =  _mm_set_ps1(0); // Y = {0, 0, 0, 0}
    __m128 radius         = _mm_set_ps1(RADIUS);


    _mm_storeu_si128((__m128i*)&X, (__m128i)X0); // X = X0
    _mm_storeu_si128((__m128i*)&Y, (__m128i)Y0); // Y = Y0

    for (int n = 0; n < MAX_ITERATIONS; n++) {
        __m128 x2 = _mm_mul_ps(X, X);
        __m128 y2 = _mm_mul_ps(Y, Y);
        __m128 xy = _mm_mul_ps(X, Y);

        __m128 r2 = _mm_add_ps(x2, y2);
        __m128 cmp = _mm_cmple_ps(r2, radius);

        int mask = _mm_movemask_ps(cmp);
        if (!mask) break;

        color = _mm_sub_epi32(color, _mm_castps_si128(cmp));

        X = _mm_add_ps(_mm_sub_ps(x2, y2), X0);
        Y = _mm_add_ps(_mm_add_ps(xy, xy), Y0);
    }
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
            for (int x = 0; x < WIDTH; x += 4) {
                __m128 X0 = _mm_set_ps((float) (x + 3) / WIDTH * 3.5f * (*zoom) - 2.5f + (*xC),
                                        (float)(x + 2) / WIDTH * 3.5f * (*zoom) - 2.5f + (*xC),
                                        (float)(x + 1) / WIDTH * 3.5f * (*zoom) - 2.5f + (*xC),
                                        (float)(x + 0) / WIDTH * 3.5f * (*zoom) - 2.5f + (*xC));
                __m128 Y0 = _mm_set_ps1((float)y / HEIGHT * 2.0f * (*zoom) - 1.0f + (*yC));

                volatile __m128 color = _mm_set_ps(0, 0, 0, 0);
                mandelbrot(X0, Y0, color);

                #ifndef TIME_MEASURE
                int* color_int = (int*)(&color);
                for (int i = 0; i < 4; i++) {
                    sf::Color sfColor((color_int[i] * 6) % 256, 0, (color_int[i] * 10) % 256);
                    image->setPixel(x + i, y, sfColor);
                }
                #endif
            }
        }

        #ifdef TIME_MEASURE
        unsigned long long end = __rdtsc();
        unsigned long long elapsedTime = end - start;
        cntForTick++;
        all_time += elapsedTime;
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
