#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <thread>
#include <algorithm>
#include <sstream>
#include <random>
#include <map>
#include <ncurses.h>
#include <cassert>

#define NUMBER_WIDTH 4

// SPACE_SPEED is the time taken to type a space, treated differently because
// whitespace is boring to watch and who manages to typo a space
// default = 20
#define SPACE_SPEED 10
// Max time taken to type a character, actual time taken is X ~ U[0, <this>];
// Default = 80
#define MAX_CHAR_SPEED 20

std::vector<std::string> getLines(const std::string& filename) {
    std::vector<std::string> tmp;

    std::ifstream file(filename);
    while(!file.eof()) {
        std::string tm;
        std::getline(file, tm);
        tmp.push_back(tm);
    }

    return tmp;
}

// Print a std::string to a window
void wprints(WINDOW* window, const std::string& str) {
    wprintw(window, str.c_str());
}

// Print a std::string to a window and refresh it
void rwprints(WINDOW* window, const std::string& str) {
    wprints(window, str);
    wrefresh(window);
}

// Move to a location on screen, draw a std::string there and refresh the screen
void rmvwprints(WINDOW* win, int y, int x, const std::string& str) {
    mvwprintw(win, y, x, str.c_str());
    wrefresh(win);
}

void drawStatus(WINDOW* status, int lineNumber, int charNumber) {
    mvwprintw(status, 0, 0, "-- INSERT --");
    mvwprintw(status, 0, COLS - 4, lineNumber >= LINES ? "Bot" : "All");

    if(lineNumber == 1 && charNumber == 0) {
        lineNumber = 0;
    }

    mvwprintw(status, 0, COLS - 20, "       ");

    rmvwprints(status, 0, COLS - 20, std::to_string(lineNumber) +
                                   "," + std::to_string(charNumber));
}

void drawCharacter(WINDOW* window, WINDOW* numbers,
                   char character, int charNumber) {
    if((charNumber % COLS) == 0 && charNumber > 0) {
        // Enter a newline in the numbers collumn to not break the order
        rwprints(numbers, "\n");
    }

    rwprints(window, std::string(1, character));
}

void sleepMS(int time) {
    std::this_thread::sleep_for(std::chrono::milliseconds(time));
}

void printLines(std::vector<std::string> lines, WINDOW* window,
                WINDOW* numbers, WINDOW* status) {
    int lineNumber = 1;
    int charNumber = 0;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> rand(0, 1);

    rwprints(numbers, std::to_string(lineNumber) + "\n");

    for(auto line : lines) {
        for(char character : line) {
            drawCharacter(window, numbers, character, charNumber);

            if(character == ' ') {
                sleepMS(SPACE_SPEED);
            } else {
                sleepMS(static_cast<int>(MAX_CHAR_SPEED * rand(gen)));
            }

            charNumber++;

            drawStatus(status, lineNumber, charNumber);
        }

        rwprints(window, "\n");

        lineNumber++;
        charNumber = 0;

        rwprints(numbers, std::to_string(lineNumber) + "\n");

        drawStatus(status, lineNumber, charNumber);
    }
}

void addNumberPlaceholders(WINDOW* numbers) {
    wattron(numbers, COLOR_PAIR(2));

    std::string temp = "~";

    while(temp.length() < NUMBER_WIDTH) {
        temp += " ";
    }

    for(int x = 0; x < LINES; x++) {
        rwprints(numbers, temp);
    }
    wmove(numbers, 0, 0);

    wattron(numbers, COLOR_PAIR(1));
}

void setTitle(const std::string& titleMsg)
{
#ifdef WIN32
    string title = "title ";
	title += titleMsg.c_str();
	system(title.c_str());
#else
    char esc_start[] = { 0x1b, ']', '0', ';', 0 };
    char esc_end[] = { 0x07, 0 };
    std::cout << esc_start << titleMsg << esc_end;
#endif
}

void clearStatus(WINDOW* status) {
    rmvwprints(status, 0, COLS - 20, "            ");
    rmvwprints(status, 0, COLS - 4, "            ");
    rmvwprints(status, 0, 0, "            ");
}

void printSaveCommand(WINDOW* status) {
    std::string saveStr = ":wq";

    wmove(status, 0, 0);
    for(char character : saveStr) {
        rwprints(status, std::string(1, character));
        sleepMS(MAX_CHAR_SPEED);
    }
}

int main(int argc, char** argv) {
    setTitle("vim");

    initscr();
    start_color();

    printf("Size = %dx%d\n", LINES, COLS);

    if(COLORS == 8) {
        // Have to fallback to innacurate ones
        printf("Unable to use accurate colours"
               "due to bad terminal colour support\n");
    } else {
        // ncurses uses 0-1000, hex is 0-255
        assert(init_color(COLOR_YELLOW, 250 * 4, 233 * 4, 79 * 4) == OK);
        assert(init_color(COLOR_WHITE, 1000, 1000, 1000) == OK);
    }

    init_pair(1, COLOR_YELLOW, COLOR_BLACK);
    init_pair(2, COLOR_BLUE, COLOR_BLACK);


    WINDOW* content = newwin(LINES - 1, COLS - NUMBER_WIDTH, 0, NUMBER_WIDTH);
    WINDOW* numbers = newwin(LINES, NUMBER_WIDTH, 0, 0);
    WINDOW* status = newwin(1, COLS, LINES - 1, 0);

    scrollok(content, TRUE);
    scrollok(numbers, TRUE);

    addNumberPlaceholders(numbers);

    std::vector<std::string> lines = getLines(argv[1]);

    sleepMS(500);

    printLines(lines, content, numbers, status);

    sleepMS(500);

    clearStatus(status);
    printSaveCommand(status);

    sleepMS(3000);

    endwin();

    return 0;
}