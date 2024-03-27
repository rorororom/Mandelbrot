#include <SFML/Graphics.hpp> // Включаем заголовочный файл SFML для работы с графикой
#include <cmath> // Включаем заголовочный файл для математических функций
#include <string> // Включаем заголовочный файл для работы со строками

const int MAX_ITERATIONS = 256; // Максимальное количество итераций для определения цвета пикселя
const float RADIUS = 10.0f;

// Функция для создания изображения множества Мандельброта
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
                }
            }
        }

        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x += 4) {

                float X0[4] = {0};
                float Y0[4] = {0};

                for (int i = 0; i < 4; i++) {
                    X0[i] = (float)(x + i) / WIDTH  * 3.5f - 2.5f + xC;
                    Y0[i] = (float)y       / HEIGHT * 2.0f - 1.0f + yC;
                }

                int color[4] = {0};
                mandelbrot(X0, Y0, color);

                for (int i = 0; i < 4; i++) {
                    sf::Color sfColor((color[i] * 6) % 256, 0, (color[i] * 10) % 256);
                    scr[y][x + i] = sfColor;
                    image.setPixel((x + i), y, sfColor );
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
