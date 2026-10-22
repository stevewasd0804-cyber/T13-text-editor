#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cstdlib>
#include <termios.h>
#include <unistd.h>

size_t termRows = 24;
size_t termCols = 80;

char getch() {
    char buf = 0;
    struct termios old = {0};
    if (tcgetattr(0, &old) < 0) perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_iflag &= ~IXON; 
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0) perror("tcsetattr ~ICANON");
    if (read(0, &buf, 1) < 0) perror("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    old.c_iflag |= IXON;
    if (tcsetattr(0, TCSANOW, &old) < 0) perror("tcsetattr ICANON");
    return buf;
}

void updateTerminalSize() {
    termRows = 24;
    termCols = 80;
    
    if (const char* env_rows = std::getenv("LINES")) termRows = std::stoi(env_rows);
    if (const char* env_cols = std::getenv("COLUMNS")) termCols = std::stoi(env_cols);

    if (termRows == 24 && termCols == 80) {
        std::cout << "\033[s\033[999;999H\033[6n\033[u" << std::flush;
        char ch = getch();
        if (ch == 27) { 
            char n1 = getch(); if (n1 == 91) {
                std::string rStr = "", cStr = ""; char r;
                while ((r = getch()) != ';') rStr += r;
                while ((r = getch()) != 'R') cStr += r;
                try { termRows = std::stoi(rStr); termCols = std::stoi(cStr); } catch (...) {}
            }
        }
    }
}

class RealtimeEditor {
private:
    std::vector<std::string> lines;
    std::string currentFilename = "";
    size_t cursorX = 0;
    size_t cursorY = 0;
    size_t rowOffset = 0; 
    size_t colOffset = 0;
    bool isDirty = false; 
    const int headerLines = 5;
    void clearScreen() {
        std::system("clear");
    }
    void printColorizedLine(const std::string& line) {
        size_t i = 0;
        while (i < line.length()) {
            if (i + 1 < line.length() && line[i] == '/' && line[i+1] == '/') {
                std::cout << "\033[32m" << line.substr(i) << "\033[0m";
                break;
            }
            if (line[i] == '"') {
                std::cout << "\033[33m" << line[i]; i++;
                while (i < line.length() && line[i] != '"') { std::cout << line[i]; i++; }
                if (i < line.length()) std::cout << line[i];
                std::cout << "\033[0m"; i++; continue;
            }
            if (std::isdigit(line[i])) {
                std::cout << "\033[36m";
                while (i < line.length() && std::isdigit(line[i])) { std::cout << line[i]; i++; }
                std::cout << "\033[0m"; continue;
            }
            if (std::isalpha(line[i]) || line[i] == '_') {
                std::string word = "";
                while (i < line.length() && (std::isalnum(line[i]) || line[i] == '_')) { word += line[i]; i++; }
                if (word == "int" || word == "float" || word == "double" || word == "char" || 
                    word == "void" || word == "class" || word == "return" || word == "if" || 
                    word == "else" || word == "while" || word == "for" || word == "true" || 
                    word == "false" || word == "include") {
                    std::cout << "\033[31m" << word << "\033[0m";
                } else { std::cout << word; }
                continue;
            }
            std::cout << line[i];
            i++;
        }
    }
    void adjustScrollViewport(size_t maxViewRows, size_t maxViewCols) {
        // Vertical boundary adjustment
        if (cursorY < rowOffset) {
            rowOffset = cursorY; 
        }
        if (cursorY >= rowOffset + maxViewRows) {
            rowOffset = cursorY - maxViewRows + 1; 
        }

        if (cursorX < colOffset) {
            colOffset = cursorX;
        }
        if (cursorX >= colOffset + maxViewCols) {
            colOffset = cursorX - maxViewCols + 1;
        }
    }
public:
    RealtimeEditor() {
        lines.push_back("");
        isDirty = false; 
    }
    bool loadFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) return false;
        
        lines.clear();
        std::string line;
        while (std::getline(file, line)) {
            lines.push_back(line);
        }
        if (lines.empty()) lines.push_back("");

        currentFilename = filename;
        cursorX = 0;
        cursorY = 0;
        rowOffset = 0;
        colOffset = 0;
        isDirty = false; 
        return true;
    }
    void saveToFile() {
        clearScreen();
        std::cout << "=== SAVE FILE ===\n";
        if (currentFilename.empty()) {
            std::cout << "Enter filename to save (e.g., main.cpp): ";
            std::cin >> currentFilename;
        } else {
            std::cout << "Saving changes to existing file: " << currentFilename << "\n";
        }

        std::ofstream file(currentFilename);
        if (file.is_open()) {
            for (size_t i = 0; i < lines.size(); ++i) {
                file << lines[i];
                if (i < lines.size() - 1) file << "\n";
            }
            file.close();
            std::cout << "\nFile successfully saved as '" << currentFilename << "'!\n";
            isDirty = false; 
        }
        std::cout << "Press Enter to return to main menu...";
        std::cin.ignore();
        std::cin.get();
    }
    void render() {
        updateTerminalSize(); 

        size_t maxViewRows = (termRows > 8) ? (termRows - 8) : 5;
        size_t marginWidth = 6; 
        size_t maxViewCols = (termCols > marginWidth) ? (termCols - marginWidth) : 20;

        adjustScrollViewport(maxViewRows, maxViewCols); 
        
        std::cout << "\033[H"; // Move screen drawing index back to top-left
        
        std::cout << "=== T13 TERMINAL CODE EDITOR ===" << std::endl;
        std::cout << "File: " << (currentFilename.empty() ? "[New File]" : currentFilename) 
                  << (isDirty ? " *" : "") << std::endl;
        std::cout << "Instructions: [Arrow Keys] Move Window | Size: " << termRows << "x" << termCols << std::endl;
        std::cout << "Press [Ctrl + Q] to exit back to Main Menu." << std::endl;
        std::cout << "--------------------------------------------------" << std::endl;

        for (size_t i = 0; i < maxViewRows; ++i) {
            size_t absoluteLineIndex = rowOffset + i;

            if (absoluteLineIndex < lines.size()) {
                if (absoluteLineIndex + 1 < 10) std::cout << "\033[90m " << absoluteLineIndex + 1 << " | \033[0m";
                else std::cout << "\033[90m" << absoluteLineIndex + 1 << " | \033[0m";

                std::string currentLineContent = lines[absoluteLineIndex];
                if (colOffset < currentLineContent.length()) {
                    std::string viewableChunk = currentLineContent.substr(colOffset, maxViewCols);
                    
                    if (absoluteLineIndex == cursorY) {
                        size_t relativeCursorX = cursorX - colOffset;
                        if (relativeCursorX <= viewableChunk.length()) {
                            printColorizedLine(viewableChunk.substr(0, relativeCursorX));
                            std::cout << "\033[s"; 
                            printColorizedLine(viewableChunk.substr(relativeCursorX));
                        } else {
                            printColorizedLine(viewableChunk);
                            std::cout << "\033[s"; 
                        }
                    } else {
                        printColorizedLine(viewableChunk);
                    }
                } else if (absoluteLineIndex == cursorY) {
                    std::cout << "\033[s"; 
                }
                
                std::cout << "\033[K" << std::endl;
            } else {
                std::cout << "\033[90m~\033[0m\033[K" << std::endl;
            }
        }
        
        std::cout << "\033[J";
        std::cout << "--------------------------------------------------" << std::endl;
        std::cout << "Line: " << cursorY + 1 << " Col: " << cursorX + 1 << "\033[K" << std::endl;

        std::cout << "\033[u" << std::flush;
    }
    void handleKeyPress(char ch) {
        isDirty = true; 

        if (ch == '\n' || ch == '\r') {
            std::string currentLine = lines[cursorY];
            std::string textBeforeCursor = currentLine.substr(0, cursorX);
            std::string textAfterCursor = currentLine.substr(cursorX);

            lines[cursorY] = textBeforeCursor;
            lines.insert(lines.begin() + cursorY + 1, textAfterCursor);

            cursorY++;
            cursorX = 0;
        }
        else if (ch == 127 || ch == 8) {
            if (cursorX > 0) {
                if (cursorX >= 4 && lines[cursorY].substr(cursorX - 4, 4) == "    ") {
                    lines[cursorY].erase(cursorX - 4, 4);
                    cursorX -= 4;
                } else {
                    lines[cursorY].erase(cursorX - 1, 1);
                    cursorX--;
                }
            } else if (cursorY > 0) {
                size_t originalLength = lines[cursorY - 1].length();
                lines[cursorY - 1] += lines[cursorY];
                lines.erase(lines.begin() + cursorY);
                cursorY--;
                cursorX = originalLength;
            }
        }
        else if (ch == 9) { 
            lines[cursorY].insert(cursorX, "    ");
            cursorX += 4;
        }
        else if (ch >= 32 && ch <= 126) {
            lines[cursorY].insert(cursorX, 1, ch);
            cursorX++;
        }
    }

    void moveCursorUp() {
        if (cursorY > 0) {
            cursorY--;
            if (cursorX > lines[cursorY].length()) cursorX = lines[cursorY].length();
        }
    }
    void moveCursorDown() {
        if (cursorY < lines.size() - 1) {
            cursorY++;
            if (cursorX > lines[cursorY].length()) cursorX = lines[cursorY].length();
        }
    }
    void moveCursorLeft() {
        if (cursorX > 0) {
            cursorX--;
        } else if (cursorY > 0) {
            cursorY--;
            cursorX = lines[cursorY].length();
        }
    }
    void moveCursorRight() {
        if (cursorX < lines[cursorY].length()) {
            cursorX++;
        } else if (cursorY < lines.size() - 1) {
            cursorY++;
            cursorX = 0;
        }
    }
    void run() {
        clearScreen();
        while (true) {
            render();
            char ch = getch();
            if (ch == 27) {
                char next1 = getch();
                if (next1 == 91) {
                    char next2 = getch();
                    if (next2 == 65) moveCursorUp();
                    if (next2 == 66) moveCursorDown();
                    if (next2 == 67) moveCursorRight();
                    if (next2 == 68) moveCursorLeft();
                }
            continue;
            }
            if (ch == 17) break; // Ctrl+Q
                handleKeyPress(ch);
            }
            if (isDirty) {
                clearScreen();
                char saveChoice;
                std::cout << "You have unsaved changes. Save your work? (y/n): ";
                std::cin >> saveChoice;
                if (saveChoice == 'y' || saveChoice == 'Y') {
                saveToFile();
            }
        }
    }
};


int main() {
    while (true) {
        std::system("clear");
        std::cout << "=============================\n";
        std::cout << "   T13 TERMINAL CODE EDITOR  \n";
        std::cout << "=============================\n";
        std::cout << "1. Create New File\n";
        std::cout << "2. Open Existing File\n";
        std::cout << "3. Quit\n";
        std::cout << "Choose an option: ";
        int choice;
        std::cin >> choice;
        if (choice == 1) {
            RealtimeEditor editor;
            editor.run();
        }
        else if (choice == 2) {
            std::string filename;
            std::cout << "Enter the name of the file to open: ";
            std::cin >> filename;
            RealtimeEditor editor;
            if (editor.loadFromFile(filename)) {
                editor.run();
            } else {
                std::cout << "\nError: File not found! Press Enter to go back...";
                std::cin.ignore();
                std::cin.get();
            }
        }
        else if (choice == 3) {
            break;
        }
    }
    return 0;
}
