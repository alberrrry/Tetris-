#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <math.h>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iostream>
using namespace std;
using namespace sf;

const int COLUMNS = 17;
const int ROWS = 20;
const int CELL_SIZE = 40;

// Define game state
enum GameState
{
    Home,
    Game,
    Exit
};

// Define grid
int grid[ROWS][COLUMNS] = {0};
int score = 0;
bool clear = false;

// Define Shape
int shapes[7][4][4] =
    {
        1, 0, 0, 0,
        1, 0, 0, 0,
        1, 0, 0, 0,
        1, 0, 0, 0, // I

        1, 0, 0, 0,
        1, 0, 0, 0,
        1, 1, 0, 0,
        0, 0, 0, 0, // L

        1, 0, 0, 0,
        1, 1, 0, 0,
        0, 1, 0, 0,
        0, 0, 0, 0, // Z

        1, 1, 0, 0,
        1, 1, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0, // O

        1, 0, 0, 0,
        1, 1, 0, 0,
        1, 0, 0, 0,
        0, 0, 0, 0, // T

        0, 1, 0, 0,
        1, 1, 0, 0,
        1, 0, 0, 0,
        0, 0, 0, 0, // S

        0, 1, 0, 0,
        0, 1, 0, 0,
        1, 1, 0, 0,
        0, 0, 0, 0 // J
};

// Define colors
const Color colors[] =
    {
        Color::Green,
        Color::Blue,
        Color::Red,
        Color::Yellow,
        Color::White,
        Color::Magenta,
        Color::Cyan};

int main()
{
    RenderWindow window(VideoMode(1200, 800), "Game Space");
    Color gridColors[COLUMNS][ROWS] = {sf::Color::Black};

    // Load home screen texture
    Texture home;
    // Create sprites for home screen
    home.loadFromFile("resources/background.png");
    Sprite homeSprite(home);
    Vector2u size = home.getSize();
    homeSprite.setOrigin(size.x / 50, size.y / 50);

    // Load sound buffers
    sf::SoundBuffer buffer1;
    if (!buffer1.loadFromFile("resources/game_start.wav"))
        return -1;
    sf::Sound StartSound;
    StartSound.setBuffer(buffer1);

    sf::SoundBuffer buffer2;
    if (!buffer2.loadFromFile("resources/game_music.wav"))
        return -1;
    sf::Sound GameSound;
    GameSound.setBuffer(buffer2);

    sf::SoundBuffer buffer3;
    if (!buffer3.loadFromFile("resources/ending.wav"))
        return -1;
    sf::Sound EndSound;
    EndSound.setBuffer(buffer3);

    sf::SoundBuffer buffer4;
    if (!buffer4.loadFromFile("resources/line_clear.wav"))
        return -1;
    sf::Sound ClearSound;
    ClearSound.setBuffer(buffer4);

    sf::SoundBuffer buffer5;
    if (!buffer5.loadFromFile("resources/rotate.wav"))
        return -1;
    sf::Sound RotateSound;
    RotateSound.setBuffer(buffer5);

    sf::SoundBuffer buffer6;
    if (!buffer6.loadFromFile("resources/fall.wav"))
        return -1;
    sf::Sound FallSound;
    FallSound.setBuffer(buffer6);
    GameSound.play();
    GameSound.setLoop(true);

    // Create a font
    Font font;
    if (!font.loadFromFile("resources/BlockHead_bold.ttf"))
    {
        // Handle the case where the font cannot be loaded
        return -1;
    }

    // Create a text object
    Text text;
    text.setFont(font);                                // Set the font for the text
    text.setString("\t\tTETRIS\nEnter space to play"); // Set the string to be displayed
    text.setCharacterSize(80);                         // Set the character size
    text.setFillColor(Color::White);                   // Set the fill color
    text.setStyle(Text::Bold);                         // Set the text style (optional)
    text.setPosition(400.0f, 300.0f);                  // Set the position of the text

    Text score_display;
    score_display.setFont(font);
    score_display.setCharacterSize(80);
    score_display.setFillColor(Color::White);
    score_display.setPosition(850.0f, 550.0f);

    // draw a cell in the grid
    RectangleShape cell(Vector2f(CELL_SIZE, CELL_SIZE));
    RectangleShape preview_cell(Vector2f(CELL_SIZE, CELL_SIZE));

    // populate a block
    int block;
    int b_x;
    int b_y;
    int pre;

    block = rand() % 7;
    auto preview_block = [&]()
    {
        if (pre == block)
        {
            if (pre == 6)
                pre = 0;
            else
                pre = block + 1;
        }
        else
            pre = rand() % 7;
    };

    auto new_block = [&]()
    {
        b_x = COLUMNS / 2;
        b_y = 0;
    };
    preview_block();
    new_block();

    // boundary check for a block
    auto check_block_boundary = [&]()
    {
        for (int y = 0; y < 4; y++)
        {
            for (int x = 0; x < 4; x++)
            {
                if (shapes[block][y][x] == 0)
                    continue;

                // hit world boundary
                if (x + b_x < 0 || x + b_x >= COLUMNS || y + b_y >= ROWS)
                    return false;

                // collision with world blocks
                if (grid[y + b_y][x + b_x])
                    return false;
            }
        }
        return true;
    };

    // clear lines when grid width is full-filled
    auto clear_lines = [&]()
    {
        int deleted_line = ROWS - 1;

        // from bottom line to top line...
        for (int undeleted_line = ROWS - 1; undeleted_line >= 0; undeleted_line--)
        {
            int count_width = 0;
            for (int x = 0; x < COLUMNS; x++)
            {
                if (grid[undeleted_line][x])
                    count_width++;
            }

            // if current line is not full, copy lines
            // else, the line will be deleted
            if (count_width < COLUMNS)
            {
                for (int x = 0; x < COLUMNS; x++)
                    grid[deleted_line][x] = grid[undeleted_line][x];

                deleted_line--;
            }
            else if (count_width == COLUMNS)
            {
                clear == true;
                ClearSound.play();
                score += 20;
                {
                    score_display.setString("Score: " + std::to_string(score));
                }
            }
        }
    };

    if (clear == true)
    {
        ClearSound.play();
        clear == false;
    }

    // fall down
    auto fall_down = [&]()
    {
        b_y++;

        // hit bottom
        if (check_block_boundary() == false)
        {
            b_y--;
            for (int y = 0; y < 4; y++)
            {
                for (int x = 0; x < 4; x++)
                {
                    if (shapes[block][y][x])
                    {
                        // +1 for avoiding 0
                        grid[b_y + y][b_x + x] = block + 1;
                    }
                }
            }
            // delete lines
            clear_lines();

            // start next block
            block = pre;
            new_block();
            preview_block();
            return false;
        }
        return true;
    };

    // rotate shape
    auto rotate = [&]()
    {
        // check rotation block size
        int len = 0;
        for (int y = 0; y < 4; y++)
        {
            for (int x = 0; x < 4; x++)
            {
                if (shapes[block][y][x])
                {
                    len = max(max(x, y) + 1, len);
                }
            }
        }

        int rotated_block[4][4] = {0};

        // rotate conter-clock-wise (90 degree)
        for (int y = 0; y < len; y++)
        {
            for (int x = 0; x < len; x++)
            {
                if (shapes[block][y][x])
                {
                    rotated_block[len - 1 - x][y] = 1;
                }
            }
        }

        for (int y = 0; y < 4; y++)
        {
            for (int x = 0; x < 4; x++)
            {
                shapes[block][y][x] = rotated_block[y][x];
            }
        }
    };

    Clock clock;
    // Set the initial game state
    GameState gameState = Home;

    // Main loop
    while (window.isOpen())
    {
        sf::Event e1;
        while (window.pollEvent(e1))
        {
            if (e1.type == sf::Event::Closed)
            {
                gameState = Home;
                window.close(); // Close the window when the close button is pressed
            }
            // Handle other events as needed
        }

        auto HomeScreen = [&]()
        {
            window.clear();
            window.draw(homeSprite);
            window.draw(text);
            window.display();
            Event event;
            while (window.pollEvent(event))
            {
                if (event.type == Event::Closed)
                {
                    window.close();
                }
                else if (event.type == Event::KeyPressed)
                {
                    // Switch to the game state when the spacebar is pressed
                    if (event.key.code == Keyboard::Space && gameState == Home)
                    {
                        gameState = Game;
                    }
                }
                if (gameState = Game)
                    return;
            }
            if (gameState == Game)
                return;
        };

        auto GameScreen = [&]()
        {
            StartSound.play();
            GameSound.play();
            score_display.setString("Score: " + std::to_string(score));

            while (window.isOpen())
            {
                // start clock
                static float prev = clock.getElapsedTime().asSeconds();
                if (clock.getElapsedTime().asSeconds() - prev >= 0.5)
                {
                    prev = clock.getElapsedTime().asSeconds();
                    fall_down();
                }

                // Define system event
                Event e;

                // polling event (eg. key pressed)
                while (window.pollEvent(e))
                {
                    // close window
                    if (e.type == Event::Closed)
                        window.close();

                    // keyboard interrupt
                    if (e.type == Event::KeyPressed)
                    {
                        if (e.key.code == Keyboard::Left)
                        {
                            b_x--;
                            if (check_block_boundary() == false)
                                b_x++;
                        }
                        else if (e.key.code == Keyboard::Right)
                        {
                            b_x++;
                            if (check_block_boundary() == false)
                                b_x--;
                        }
                        else if (e.key.code == Keyboard::Down)
                        {
                            fall_down();
                        }
                        else if (e.key.code == Keyboard::Space)
                        {
                            // fall down until reaches the bottom
                            FallSound.play();
                            while (fall_down() == true)
                                ;
                        }
                        else if (e.key.code == Keyboard::Up)
                        {
                            RotateSound.play();

                            // RotateSound.play();
                            rotate();
                            // if rotation hits boundary, do not allow to rotate
                            if (check_block_boundary() == false)
                            {
                                rotate(), rotate(), rotate();
                            }
                        }
                    }
                }

                window.clear();
                window.draw(score_display);
                RectangleShape preview_border(sf::Vector2f(5 * CELL_SIZE, 5 * CELL_SIZE));
                preview_border.setFillColor(sf::Color(0, 0, 0));
                preview_border.setOutlineThickness(-1);
                preview_border.setPosition(850, 200);

                // Draw the preview border
                window.draw(preview_border);

                auto draw_preview = [&]()
                {
                    for (int y = 0; y < 4; y++)
                    {
                        for (int x = 0; x < 4; x++)
                        {
                            if (shapes[pre][y][x])
                            {
                                preview_cell.setPosition(Vector2f(900 + (x * CELL_SIZE), 220 + (y * CELL_SIZE)));
                                preview_cell.setFillColor(colors[pre]);
                                window.draw(preview_cell);
                            }
                        }
                    }
                };
                draw_preview();

                RectangleShape frame(sf::Vector2f(5 * CELL_SIZE, 5 * CELL_SIZE));
                frame.setFillColor(Color(47, 79, 79));
                frame.setOutlineThickness(2);
                frame.setPosition(0, 0);
                frame.setSize(sf::Vector2f(680, 800));
                // Draw the preview border
                window.draw(frame);

                // draw grid
                auto draw_grid = [&]()
                {
                    for (int y = 0; y < ROWS; y++)
                    {
                        for (int x = 0; x < COLUMNS; x++)
                        {
                            if (grid[y][x])
                            {
                                cell.setFillColor(colors[grid[y][x] - 1]);
                                cell.setPosition(Vector2f(x * CELL_SIZE, y * CELL_SIZE));
                                window.draw(cell);
                            }
                        }
                    }
                };
                draw_grid();

                // define C++11 lambda function
                // this function can use all the outside variables, such as block

                auto draw_block = [&]()
                {
                    cell.setFillColor(colors[block]);
                    for (int y = 0; y < 4; y++)
                    {
                        for (int x = 0; x < 4; x++)
                        {
                            if (shapes[block][y][x])
                            {
                                cell.setPosition(Vector2f((b_x + x) * CELL_SIZE, (b_y + y) * CELL_SIZE));
                                window.draw(cell);
                            }
                        }
                    }
                };

                // call the above lambda function
                draw_block();

                if (gameState == Exit)
                    return;

                for (int y = 0; y < 4; y++)
                {
                    for (int x = 0; x < 4; x++)
                    {
                        if (grid[b_y][b_x])
                        {
                            gameState = Exit;
                        }
                    }
                }

                // display rendered object on screen
                window.display();
                if (gameState == Exit)
                    break;
            }
            if (gameState == Exit)
                return;
            window.display();
        };

        Text highScore;
        highScore.setFont(font);
        highScore.setString("TOP SCORES\n");
        highScore.setCharacterSize(55);
        highScore.setFillColor(sf::Color::White);
        highScore.setPosition(480.0f, 450.0f);

        Text highScores;
        highScores.setFont(font); // Set the font for the text
        highScores.setCharacterSize(35);
        highScores.setFillColor(sf::Color::White);
        highScores.setPosition(350.0f, 550.0f);

        string str;
        auto storeScore = [&]()
        {
            ofstream scoreFile("scores.txt", std::ios::app);
            // Check if the file is open
            if (scoreFile.is_open())
            {
                // Write content to the file
                scoreFile << score << endl;
                // Close the file
                scoreFile.close();
                cout << "Game score written to the file successfully." << endl;
            }

            // Read numbers from the external file
            ifstream inputFile("scores.txt");
            if (!inputFile.is_open())
            {
                cerr << "Unable to open the file for reading." << endl;
                return EXIT_FAILURE;
            }
            vector<int> numbers;
            int num;

            while (inputFile >> num)
            {
                numbers.push_back(num);
            }
            inputFile.close();
            sort(numbers.begin(), numbers.end(), greater<int>());
            for (size_t i = 0; i < 5; ++i)
            {
                str += "\t\t\t" + to_string(numbers[i]);
            }
            highScores.setString("\t" + str);
        };

        auto GameOverScren = [&]()
        {
            EndSound.play();
            storeScore();

            Text score_display_final;
            score_display_final.setFont(font);
            score_display_final.setString("Score: " + to_string(score));
            score_display_final.setCharacterSize(80);
            score_display_final.setFillColor(Color::White);
            score_display_final.setPosition(490.0f, 150.0f);

            Text gameover_text;
            gameover_text.setFont(font);
            gameover_text.setString("GAME OVER");
            gameover_text.setCharacterSize(90);
            gameover_text.setFillColor(Color::White);
            gameover_text.setPosition(440.0f, 270.0f);

            Text play_again;
            play_again.setFont(font);
            play_again.setString("PRESS  SPACE  TO  PLAY  AGAIN");
            play_again.setCharacterSize(35);
            play_again.setFillColor(Color::White);
            play_again.setPosition(430.0f, 700.0f);

            RectangleShape border(Vector2f(450, 180));
            border.setOutlineThickness(2);
            border.setPosition(380, 450);
            border.setFillColor(Color(0, 0, 0));

            while (window.isOpen())
            {
                Event event1;
                while (window.pollEvent(event1))
                {
                    if (event1.type == sf::Event::Closed)
                    {
                        window.close();
                    }

                    else if (event1.type == Event::KeyPressed)
                    {
                        // Switch to the game state when the spacebar is pressed
                        if (event1.key.code == Keyboard::Space)
                        {
                            gameState = Game;
                            // reset game parameters
                            score = 0;
                            for (int y = 0; y < ROWS; y++)
                            {
                                for (int x = 0; x < COLUMNS; x++)
                                {
                                    grid[y][x] = 0;
                                }
                            }
                        }
                    }
                    if (gameState == Game)
                        break;
                }
                GameSound.pause();
                RotateSound.pause();
                FallSound.pause();
                window.clear();
                window.draw(gameover_text);
                window.draw(score_display_final);
                window.draw(play_again);
                window.draw(border);
                window.draw(highScore);
                window.draw(highScores);
                window.display();
                if (gameState == Game)
                    break;
            }

            if (gameState == Game)
                return;
        };

        switch (gameState)
        {
        case Home:
            HomeScreen();
            break;

        case Game:
            GameScreen();
            break;

        case Exit:
            GameOverScren();
            break;
        }
    }
    return 0;
}