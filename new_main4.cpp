#include <SFML/Graphics.hpp>
#include <cmath>
#include <immintrin.h>

const int MAX_ITERATIONS = 256; // Максимальное количество итераций для определения цвета пикселя
const __m128 RADIUS = _mm_set_ps1(100.f);

// Функция для создания изображения множества Мандельброта
void mandelbrot(const __m128 X0, const __m128 Y0, __m128& color) {
    __m128 X =  _mm_set_ps1(0); // X = {0, 0, 0, 0}
    __m128 Y =  _mm_set_ps1(0); // Y = {0, 0, 0, 0}


    _mm_storeu_si128((__m128i*)&X, (__m128i)X0); // X = X0
    _mm_storeu_si128((__m128i*)&Y, (__m128i)Y0); // Y = Y0

    for (int n = 0; n < MAX_ITERATIONS; n++) {
        __m128 x2 = _mm_mul_ps(X, X);
        __m128 y2 = _mm_mul_ps(Y, Y);
        __m128 xy = _mm_mul_ps(X, Y);

        __m128 r2 = _mm_add_ps(x2, y2);
        __m128 cmp = _mm_cmple_ps(r2, RADIUS);

        int mask = _mm_movemask_ps(cmp);
        if (!mask) break;

        color = _mm_sub_epi32(color, _mm_castps_si128(cmp));

        X = _mm_add_ps(_mm_sub_ps(x2, y2), X0);
        Y = _mm_add_ps(_mm_add_ps(xy, xy), Y0);
    }
}

int main() {
    const int WIDTH = 800; // Ширина окна
    const int HEIGHT = 600; // Высота окна

    // Создаем окно с размерами WIDTH x HEIGHT и названием "Mandelbrot Set"
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Mandelbrot Set");

    // Создаем изображение для работы с пикселями
    sf::Image image;
    image.create(WIDTH, HEIGHT, sf::Color::Black);

    // Создаем текстуру для отображения изображения на экране
    sf::Texture texture;
    texture.create(WIDTH, HEIGHT);
    sf::Sprite sprite(texture);

    // Создаем массив scr для хранения цветов пикселей
    sf::Color scr[600][800];

    // Создаем часы для измерения времени и переменную для хранения количества кадров в секунду
    sf::Clock gameClock;
    int frames = 0;

    float xC = 0.f, yC = 0.f; // Переменные для смещения
    float zoom = 1.0f;

    // Основной цикл приложения
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
                } else if (event.key.code == sf::Keyboard::Equal) {
                    // Увеличиваем зум
                    zoom /= 1.1f;
                } else if (event.key.code == sf::Keyboard::Dash) {
                    // Уменьшаем зум
                    zoom *= 1.1f;
                }
            }
        }

        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x += 4) {
                __m128 X0 = _mm_set_ps((float)(x + 3) / WIDTH * 3.5f * zoom - 2.5f + xC,
                                        (float)(x + 2) / WIDTH * 3.5f * zoom - 2.5f + xC,
                                        (float)(x + 1) / WIDTH * 3.5f * zoom - 2.5f + xC,
                                        (float)(x + 0) / WIDTH * 3.5f * zoom - 2.5f + xC);
                __m128 Y0 = _mm_set_ps1((float)y / HEIGHT * 2.0f * zoom - 1.0f + yC);

                __m128 color = _mm_set_ps(0, 0, 0, 0);
                mandelbrot(X0, Y0, color);

                int* color_int = reinterpret_cast<int*>(&color);

                for (int i = 0; i < 4; i++) {
                    sf::Color sfColor((color_int[i] * 6) % 256, 0, (color_int[i] * 10) % 256);
                    scr[y][x + i] = sfColor;
                    image.setPixel(x + i, y, sfColor);
                }
            }
        }

        // Обновление текстуры изображения
        texture.update(image);

        // Отрисовка текстуры на экране
        window.clear();
        window.draw(sprite);

        // Вывод FPS
        frames++;
        if (gameClock.getElapsedTime().asSeconds() >= 1.0) {
            float fps = frames / gameClock.getElapsedTime().asSeconds();
            printf("FPS: %f\n", fps);
            gameClock.restart();
            frames = 0;
        }

        window.display();
    }

    return 0; // Возвращаем нулевой код в случае успешного завершения
}
