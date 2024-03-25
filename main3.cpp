#include <SFML/Graphics.hpp> // Подключение заголовочного файла SFML для работы с графикой
#include <cmath> // Подключение заголовочного файла для математических функций
#include <string> // Подключение заголовочного файла для работы со строками

const int MAX_ITERATIONS = 256; // Определение максимального количества итераций
const float RADIUS = 10.0f;
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
inline void mm_add_epi32(int mm[4], const int mm1[4], const float mm2[4]) {
    for (int i = 0; i < 4; i++)
        mm[i] = (int)(mm1[i] + mm2[i]);
}

// Функция сравнивает значения массивов mm1 и mm2 и записывает результат (1 или 0) в массив cmp
inline void mm_cmple_ps(float cmp[4], const float mm1[4], const float mm2[4]) {
    for (int i = 0; i < 4; i++) {
        if (mm1[i] <= mm2[i])
            cmp[i] = 1.0f;
        else
            cmp[i] = 0.0f;
    }
}

// Функция определяет маску на основе значений массива cmp (используется для условных операций)
inline int mm_movemask_ps(const float cmp[4]) {
    int mask = 0;
    for (int i = 0; i < 4; i++) {
        mask |= (!!cmp[i] << i);
    }
    return mask;
}

// Функция для генерации фрактала Мандельброта
void mandelbrot(float xC[], float yC[], int color[]) {
    float X0[4] = {}, Y0[4] = {};
    // mm_set_ps1(X, 0);
    // mm_set_ps1(Y, 0);

    float X[4] = {xC[0], xC[1], xC[2], xC[3]};
    float Y[4] = {yC[0], yC[1], yC[2], yC[3]};

    mm_set_ps(X0, xC[0], xC[1], xC[2], xC[3]);
    mm_set_ps(Y0, yC[0], yC[1], yC[2], yC[3]);

    float x2[4] = {}, y2[4] = {}, xy[4] = {}, r2[4] = {}, cmp[4] = {};
    float radius[4] = {RADIUS, RADIUS, RADIUS, RADIUS};

    for (int n = 0; n < MAX_ITERATIONS; n++) {
        mm_mul_ps(x2, X, X);
        mm_mul_ps(y2, X, X);
        mm_mul_ps(xy, X, Y);

        mm_add_ps(r2, x2, y2);

        mm_cmple_ps(cmp, r2, &RADIUS);

        int mask = mm_movemask_ps(cmp);
        if (!mask) break;

        mm_add_epi32(color, color, cmp);

        float tempX[4] = {};
        mm_sub_ps(tempX, x2, y2);
        mm_add_ps(tempX, tempX, X0);

        float tempY[4] = {};
        mm_add_ps(tempY, xy, xy);
        mm_sub_ps(tempY, tempY, Y0);
    }

}

// Основная функция main
int main() {
    const int WIDTH = 800; // Ширина окна
    const int HEIGHT = 600; // Высота окна

    // Создание окна
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Mandelbrot Set");

    // Создание изображения для работы с пикселями
    sf::Image image;
    image.create(WIDTH, HEIGHT, sf::Color::Black);

    // Создание текстуры
    sf::Texture texture;
    texture.create(WIDTH, HEIGHT);
    sf::Sprite sprite(texture);

    // Массив для хранения цветов пикселей
    sf::Color scr[600][800];

    // Часы для подсчета FPS
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
                float X0[4] = {((float)(x + 0) / WIDTH * 3.5f * zoom - 2.5f + xC),
                                ((float)(x + 1) / WIDTH * 3.5f * zoom - 2.5f + xC),
                                ((float)(x + 2) / WIDTH * 3.5f * zoom - 2.5f + xC),
                                ((float)(x + 3) / WIDTH * 3.5f * zoom - 2.5f + xC)};
                float Y0[4] = {((float)y / HEIGHT * 2.0f * zoom - 1.0f + yC)};

                int color[4] = {0};
                mandelbrot(X0, Y0, color);

                for (int i = 0; i < 4; i++) {
                    sf::Color sfColor((color[i] * 6) % 256, 0, (color[i] * 10) % 256);
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

    return 0;
}
