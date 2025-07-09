#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <string>

using namespace std;

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int BLOCK_SIZE = 20;

enum Direction { UP, DOWN, LEFT, RIGHT };

class Snake {
public:
    Snake() {
        reset();
    }

    void reset() {
        snakeBody.clear();
        snakeBody.push_back(sf::Vector2i(5, 5));
        dir = RIGHT;
        growNext = false;
    }

    void move() {
        sf::Vector2i newHead = snakeBody[0];
        switch (dir) {
            case UP: newHead.y -= 1; break;
            case DOWN: newHead.y += 1; break;
            case LEFT: newHead.x -= 1; break;
            case RIGHT: newHead.x += 1; break;
        }
        snakeBody.insert(snakeBody.begin(), newHead);
        if (!growNext)
            snakeBody.pop_back();
        else
            growNext = false;
    }

    void grow() {
        growNext = true;
    }

    void setDirection(Direction newDir) {
        if ((dir == UP && newDir != DOWN) ||
            (dir == DOWN && newDir != UP) ||
            (dir == LEFT && newDir != RIGHT) ||
            (dir == RIGHT && newDir != LEFT)) {
            dir = newDir;
        }
    }

    bool checkCollision() {
        const auto& head = snakeBody[0];
        if (head.x < 0 || head.x >= WINDOW_WIDTH / BLOCK_SIZE ||
            head.y < 0 || head.y >= WINDOW_HEIGHT / BLOCK_SIZE)
            return true;
        for (int i = 1; i < snakeBody.size(); ++i) {
            if (head == snakeBody[i])
                return true;
        }
        return false;
    }

    const vector<sf::Vector2i>& getBody() const { return snakeBody; }

private:
    vector<sf::Vector2i> snakeBody;
    Direction dir;
    bool growNext;
};

class Food {
public:
    Food() {
        spawn({});
    }

    void spawn(const vector<sf::Vector2i>& snakeBody) {
        do {
            foodPosition.x = rand() % (WINDOW_WIDTH / BLOCK_SIZE);
            foodPosition.y = rand() % (WINDOW_HEIGHT / BLOCK_SIZE);
        } while (isOnSnake(snakeBody));
    }

    sf::Vector2i getPosition() const { return foodPosition; }

private:
    sf::Vector2i foodPosition;

    bool isOnSnake(const vector<sf::Vector2i>& snakeBody) {
        for (const auto& segment : snakeBody) {
            if (segment == foodPosition)
                return true;
        }
        return false;
    }
};

int main() {
    srand(static_cast<unsigned int>(time(0)));
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Snake Game");

    Snake snake;
    Food food;
    int score = 0;
    bool isGameOver = false;

    sf::RectangleShape snakeBlock(sf::Vector2f(BLOCK_SIZE, BLOCK_SIZE));
    snakeBlock.setFillColor(sf::Color::Green);

    sf::RectangleShape foodBlock(sf::Vector2f(BLOCK_SIZE, BLOCK_SIZE));
    foodBlock.setFillColor(sf::Color::Red);

    // Font for score and game over
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        return -1;
    }

    sf::Text scoreText;
    scoreText.setFont(font);
    scoreText.setCharacterSize(24);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition(10, 10);

    sf::Text gameOverText;
    gameOverText.setFont(font);
    gameOverText.setCharacterSize(40);
    gameOverText.setFillColor(sf::Color::Red);
    gameOverText.setStyle(sf::Text::Bold);
    gameOverText.setString("GAME OVER\nPress R to Restart");
    gameOverText.setPosition(WINDOW_WIDTH / 2 - 180, WINDOW_HEIGHT / 2 - 80);

    // Load Sound Buffers
    sf::SoundBuffer eatBuffer, gameOverBuffer;
    if (!eatBuffer.loadFromFile("eat.wav") || !gameOverBuffer.loadFromFile("gameover.wav")) {
        return -2;
    }

    sf::Sound eatSound, gameOverSound;
    eatSound.setBuffer(eatBuffer);
    gameOverSound.setBuffer(gameOverBuffer);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::KeyPressed) {
                if (!isGameOver) {
                    if (event.key.code == sf::Keyboard::Up)
                        snake.setDirection(UP);
                    else if (event.key.code == sf::Keyboard::Down)
                        snake.setDirection(DOWN);
                    else if (event.key.code == sf::Keyboard::Left)
                        snake.setDirection(LEFT);
                    else if (event.key.code == sf::Keyboard::Right)
                        snake.setDirection(RIGHT);
                } else {
                    if (event.key.code == sf::Keyboard::R) {
                        snake.reset();
                        food.spawn(snake.getBody());
                        score = 0;
                        isGameOver = false;
                    }
                }
            }
        }

        if (!isGameOver) {
            snake.move();

            if (snake.getBody()[0] == food.getPosition()) {
                snake.grow();
                food.spawn(snake.getBody());
                score++;
                eatSound.play();
            }

            if (snake.checkCollision()) {
                isGameOver = true;
                gameOverSound.play();
            }
        }

        window.clear(sf::Color::Black);

        // Draw snake
        for (const auto& segment : snake.getBody()) {
            snakeBlock.setPosition(segment.x * BLOCK_SIZE, segment.y * BLOCK_SIZE);
            window.draw(snakeBlock);
        }

        // Draw food
        foodBlock.setPosition(food.getPosition().x * BLOCK_SIZE, food.getPosition().y * BLOCK_SIZE);
        window.draw(foodBlock);

        // Draw score
        scoreText.setString("Score: " + std::to_string(score));
        window.draw(scoreText);

        // Game Over text
        if (isGameOver)
            window.draw(gameOverText);

        window.display();

        sf::sleep(sf::milliseconds(100));
    }

    return 0;
}
