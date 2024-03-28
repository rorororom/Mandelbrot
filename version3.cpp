#include <SFML/Graphics.hpp> // Подключение заголовочного файла SFML для работы с графикой
#include <cmath> // Подключение заголовочного файла для математических функций
#include <string> // Подключение заголовочного файла для работы со строками

const int   MAX_ITERATIONS = 256;
const float RADIUS         = 100.0f;
const float ZOOM_FACTOR    = 1.1f;
const float MOVE_FACTOR    = 0.1f;
const int   WIDTH          = 800;
const int   HEIGHT         = 600;
const int   LIMIT          = 100;
// Вспомогательные функции для работы с SIMD (не используются в данной версии)

// Функция устанавливает значения в массиве mm[4]
inline void mm_set_ps(float mm[4], float val0, float val1, float val2, float val3) {
    mm[0] = val0;
    mm[1] = val1;
    mm[2] = val2;
    mm[3] = val3;
}

// Функция устанавливает одинаковые значения в массиве mm[4]
inline void mm_set_ps1(float mm[4], float val) {
    for (int i = 0; i < 4; i++)
        mm[i] = val;
}

// Функция копирует значения из массива mm2 в массив mm
inline void mm_cpy_ps(float mm[4], const float mm2[4]) {
    for (int i = 0; i < 4; i++)
        mm[i] = mm2[i];
}

// Функция складывает значения массивов mm1 и mm2 и записывает результат в массив mm
inline void mm_add_ps(float mm[4], const float mm1[4], const float mm2[4]) {
    for (int i = 0; i < 4; i++)
        mm[i] = mm1[i] + mm2[i];
}

// Функция вычитает значения массива mm2 из массива mm1 и записывает результат в массив mm
inline void mm_sub_ps(float mm[4], const float mm1[4], const float mm2[4]) {
    for (int i = 0; i < 4; i++)
        mm[i] = mm1[i] - mm2[i];
}

// Функция умножает значения массивов mm1 и mm2 и записывает результат в массив mm
inline void mm_mul_ps(float mm[4], const float mm1[4], const float mm2[4]) {
    for (int i = 0; i < 4; i++)
        mm[i] = mm1[i] * mm2[i];
}

// Функция складывает значения массива mm1 и массива mm2 (преобразованных в целые числа) и записывает результат в массив mm
inline void mm_add_epi32(int mm[4], const int mm1[4], const int mm2[4]) {
    for (int i = 0; i < 4; i++)
        mm[i] = (int)(mm1[i] - mm2[i]);
}

// Функция сравнивает значения массивов mm1 и mm2 и записывает результат (1 или 0) в массив cmp
inline void mm_cmple_ps(int cmp[4], const float mm1[4], const float mm2[4]) {
    for (int i = 0; i < 4; i++) {
        if (mm1[i] <= mm2[i])
            cmp[i] = 1;
        else
            cmp[i] = 0;
    }
}

// Функция определяет маску на основе значений массива cmp (используется для условных операций)
inline int mm_movemask_ps(const int cmp[4]) {
    int mask = 0;
    for (int i = 0; i < 4; i++) {
        mask |= (!!cmp[i] << i);
    }
    return mask;
}

void mandelbrot(float xC[], float yC[], int* color) {
    float X[4] = {}, Y[4] = {};

    mm_set_ps(X, xC[0], xC[1], xC[2], xC[3]);
    mm_set_ps(Y, yC[0], yC[1], yC[2], yC[3]);

    float x2[4] = {}, y2[4] = {}, xy[4] = {}, r2[4] = {};
    int cmp[4] = {};
    float radius[4] = {RADIUS, RADIUS, RADIUS, RADIUS};

    for (int n = 0; n < MAX_ITERATIONS; n++) {
        mm_mul_ps(x2, X, X);
        mm_mul_ps(y2, X, X);
        mm_mul_ps(xy, X, Y);

        mm_add_ps(r2, x2, y2);

        mm_cmple_ps(cmp, r2, radius);

        int mask = mm_movemask_ps(cmp);
        if (!mask) break;

        mm_add_epi32(color, color, cmp);

        float tempX[4] = {};
        mm_sub_ps(tempX, x2, y2);
        mm_add_ps(tempX, tempX, xC);

        float tempY[4] = {};
        mm_add_ps(tempY, xy, xy);
        mm_add_ps(tempY, tempY, yC);
    }
}

inline void handleKeyPress(sf::RenderWindow& window, float& xC, float& yC, float& zoom) {
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

inline void processEvents(sf::RenderWindow& window, float& xC, float& yC, float& zoom, sf::Image& image, sf::Texture& texture, sf::Sprite& sprite, sf::Text& fpsText, sf::Clock& gameClock, int& frames) {
    int cntForTick = 0, cntForFps = 0;
    unsigned long long all_time = 0, all_fps = 0;

    // Основной цикл приложения
    while (window.isOpen()) {
        handleKeyPress(window, xC, yC, zoom);

        #ifdef TIME_MEASURE
        unsigned long long start = __rdtsc();
        #endif

        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x += 4) {
                float X0[4] = {((float)(x + 0) / WIDTH * 3.5f * zoom - 2.5f + xC),
                                ((float)(x + 1) / WIDTH * 3.5f * zoom - 2.5f + xC),
                                ((float)(x + 2) / WIDTH * 3.5f * zoom - 2.5f + xC),
                                ((float)(x + 3) / WIDTH * 3.5f * zoom - 2.5f + xC)};
                float Y0[4] = {((float)y / HEIGHT * 2.0f * zoom - 1.0f + yC)};

                int color[4] = {0};
                mandelbrot(X0, Y0, color);

                #ifndef TIME_MEASURE
                for (int i = 0; i < 4; i++) {
                    sf::Color sfColor((color[i] * 6) % 256, 0, (color[i] * 10) % 256);
                    image.setPixel((x + i), y, sfColor);
                }
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

    return 0;
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
