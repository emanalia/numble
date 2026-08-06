// ============================================================
//  NUMBER GUESSING ARENA
//  Course  : Programming Fundamentals (PF)
//  Language: C++ — Visual Studio / Code::Blocks / MinGW
//
//  FIXES APPLIED:
//  [1] SetConsoleOutputCP(65001) — fixes garbled arrow symbols
//  [2] Replaced Unicode arrows (up/down) with ASCII [^^^] [vvv]
//  [3] cin.ignore() added after every cin>> to prevent input skip
//  [4] cin.fail() guard stops letters from crashing the game
//  [5] Input range validation — out-of-range doesn't waste guess
//  [6] #ifdef _WIN32 guards for Windows-only headers
// ============================================================

#include <iostream>
#include <string>
#include <ctime>
#include <cstdlib>

#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
#endif

using namespace std;

// ================= ANSI COLOR CODES =================
#define RESET   "\033[0m"
#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE    "\033[1;34m"
#define MAGENTA "\033[1;35m"
#define CYAN    "\033[1;36m"
#define WHITE   "\033[1;37m"

// ================= STRUCTURES =================
struct Player {
    string name;
    int    score;
    int    gamesPlayed;
    int    bestScore;
    int    streak;
};

struct HighScore {
    string name;
    int    score;
};

// ================= FUNCTION PROTOTYPES =================
void clearScreen();
void playSound(int freq, int duration);
void printLine(char ch, int len, const string& color);
void displayWelcome();
void showMenu(Player& p);
void playGame(Player& player);
void displayStats(Player& p);
void displayHighScores(HighScore scores[], int count);
void updateHighScores(HighScore scores[], int& count, Player& player);
int  getMaxAttempts(int level);
void showCloseness(int guess, int secret, int range);

// ================= MAIN =================
int main() {

#ifdef _WIN32
    // FIX [1]: Force UTF-8 so special characters don't show as garbage
    SetConsoleOutputCP(65001);

    // Enable ANSI color codes on Windows 10+
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD  dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
#endif

    srand((unsigned int)time(0));

    // Initialize player struct
    Player player;
    player.score       = 0;
    player.gamesPlayed = 0;
    player.bestScore   = 0;
    player.streak      = 0;

    // High score array — max 5 entries
    HighScore highScores[5];
    int highCount = 0;

    displayWelcome();

    cout << CYAN << "  Enter your name: " << RESET;
    getline(cin, player.name);
    if (player.name.empty()) player.name = "Player";

    int choice;

    do {
        showMenu(player);

        cout << YELLOW << "  Enter choice: " << RESET;

        // FIX [4]: guard against non-integer input
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(1000, '\n');
            cout << RED << "  Invalid input! Enter a number 1-4.\n" << RESET;
            continue;
        }
        cin.ignore(); // FIX [3]: clear leftover newline

        switch (choice) {
            case 1:
                playGame(player);
                updateHighScores(highScores, highCount, player);
                break;
            case 2:
                displayHighScores(highScores, highCount);
                break;
            case 3:
                displayStats(player);
                break;
            case 4:
                cout << GREEN
                     << "\n  Goodbye, " << player.name
                     << "!  Final Score: " << player.score
                     << RESET << "\n";
                break;
            default:
                cout << RED << "  Invalid choice! Enter 1-4.\n" << RESET;
        }

        if (choice != 4) {
            cout << "\n  Press any key to continue...";
#ifdef _WIN32
            _getch();
#else
            cin.get();
#endif
        }

    } while (choice != 4);

    return 0;
}

// ================= WELCOME SCREEN =================
void displayWelcome() {
    clearScreen();
    printLine('=', 54, MAGENTA);
    cout << MAGENTA
         << "        NUMBER GUESSING ARENA\n"
         << RESET;
    printLine('=', 54, MAGENTA);
    cout << CYAN
         << "\n  HOW TO PLAY:\n"
         << "  - Guess the hidden number\n"
         << "  - You have a LIMITED number of guesses\n"
         << "  - Run out of guesses  =  GAME OVER!\n"
         << "  - BOILING HOT = Extremely close\n"
         << "  - HOT         = Very close\n"
         << "  - WARM        = Getting there\n"
         << "  - COLD        = Far away\n"
         << "  - Fewer guesses used  =  Higher score!\n\n"
         << RESET;
    printLine('-', 54, BLUE);
    cout << "\n";
}

// ================= MENU =================
void showMenu(Player& p) {
    clearScreen();
    printLine('=', 54, YELLOW);
    cout << YELLOW
         << "  PLAYER  : " << p.name << "\n"
         << RESET;
    cout << CYAN
         << "  Score   : " << p.score
         << "  |  Best: " << p.bestScore
         << "  |  Streak: " << p.streak
         << "\n" << RESET;
    printLine('=', 54, YELLOW);
    cout << WHITE
         << "\n  [1]  Play Game\n"
         << "  [2]  High Scores\n"
         << "  [3]  My Stats\n"
         << "  [4]  Exit\n\n"
         << RESET;
    printLine('-', 54, BLUE);
    cout << "\n";
}

// ================= GET MAX ATTEMPTS =================
// Returns the guess limit for each difficulty.
// This is what FIXES the infinite loop from the old code.
int getMaxAttempts(int level) {
    switch (level) {
        case 1:  return 8;    // Easy
        case 2:  return 7;    // Medium
        case 3:  return 6;    // Hard
        default: return 7;
    }
}

// ================= CLOSENESS HINT =================
// Shows HOT / WARM / COLD based on how near the guess is.
// Uses percentage of total range so all difficulties are fair.
void showCloseness(int guess, int secret, int range) {
    int   diff    = abs(guess - secret);
    float percent = (float)diff / range * 100.0f;

    if (percent <= 5.0f) {
        cout << RED    << "  >>> BOILING HOT!! Extremely close!\n" << RESET;
    }
    else if (percent <= 15.0f) {
        cout << YELLOW << "  >>  HOT! Very close.\n"               << RESET;
    }
    else if (percent <= 30.0f) {
        cout << CYAN   << "  >   WARM. Getting closer.\n"          << RESET;
    }
    else {
        cout << BLUE   << "  .   COLD. Quite far away.\n"          << RESET;
    }
}

// ================= PLAY GAME =================
void playGame(Player& player) {

    int  level;
    int  minRange = 1;
    int  maxRange = 100;
    char playAgain;

    // -- Select difficulty ------------------------------------
    clearScreen();
    printLine('=', 54, CYAN);
    cout << CYAN << "  SELECT DIFFICULTY\n" << RESET;
    printLine('=', 54, CYAN);
    cout << WHITE
         << "\n  [1]  Easy   (1 to  50)  --  8 guesses\n"
         << "  [2]  Medium (1 to 100)  --  7 guesses\n"
         << "  [3]  Hard   (1 to 200)  --  6 guesses\n\n"
         << RESET;
    cout << YELLOW << "  Choose difficulty (1/2/3): " << RESET;

    // FIX [4]: guard bad difficulty input
    if (!(cin >> level) || level < 1 || level > 3) {
        cin.clear();
        cin.ignore(1000, '\n');
        cout << YELLOW << "  Defaulting to Medium.\n" << RESET;
        level = 2;
    }
    cin.ignore(); // FIX [3]

    switch (level) {
        case 1: maxRange = 50;  break;
        case 3: maxRange = 200; break;
        default: maxRange = 100;
    }

    int maxAttempts = getMaxAttempts(level);
    int range       = maxRange - minRange; // used in closeness %

    // -- Game round loop --------------------------------------
    do {
        clearScreen();

        int  secret   = minRange + rand() % (maxRange - minRange + 1);
        int  attempts = 0;
        bool won      = false;

        printLine('=', 54, BLUE);
        cout << BLUE << "  === ROUND START ===\n" << RESET;
        printLine('=', 54, BLUE);
        cout << WHITE
             << "\n  Guess the number between "
             << minRange << " and " << maxRange << "\n"
             << RESET;
        cout << YELLOW
             << "  You have " << maxAttempts
             << " guesses. Use them wisely!\n\n"
             << RESET;

        // -- Main guess loop (HAS LIMIT — old bug fixed here) --
        while (attempts < maxAttempts) {

            int remaining = maxAttempts - attempts;

            // Show urgency when 3 or fewer guesses left
            if (remaining <= 3) {
                cout << RED
                     << "  !! ONLY " << remaining
                     << " guess" << (remaining == 1 ? "" : "es")
                     << " left !!\n"
                     << RESET;
            }
            else {
                cout << CYAN
                     << "  Guesses remaining: " << remaining << "\n"
                     << RESET;
            }

            int guess;
            cout << YELLOW << "  Enter your guess: " << RESET;

            // FIX [4]: handle letters/symbols entered by mistake
            if (!(cin >> guess)) {
                cin.clear();
                cin.ignore(1000, '\n');
                cout << RED << "  Please enter a valid number!\n\n" << RESET;
                continue; // don't count as an attempt
            }

            // FIX [5]: out-of-range input doesn't waste a guess
            if (guess < minRange || guess > maxRange) {
                cout << RED
                     << "  Enter a number between "
                     << minRange << " and " << maxRange << "!\n\n"
                     << RESET;
                continue;
            }

            attempts++;

            if (guess == secret) {
                // ---- WIN ----------------------------------------
                won = true;
                playSound(800, 100);
#ifdef _WIN32
                Sleep(50);
#endif
                playSound(1000, 100);
#ifdef _WIN32
                Sleep(50);
#endif
                playSound(1200, 300);
                break;

            }
            else {
                // ---- WRONG: direction + closeness hint ----------
                // FIX [2]: ASCII [^^^] and [vvv] instead of Unicode arrows
                if (guess < secret) {
                    cout << RED << "  Too LOW!  Go HIGHER! [^^^]\n" << RESET;
                }
                else {
                    cout << RED << "  Too HIGH! Go LOWER!  [vvv]\n" << RESET;
                }

                showCloseness(guess, secret, range);
                playSound(400, 150);
                cout << "\n";
            }
        }
        // -- End guess loop ------------------------------------

        // ---- LOSE CONDITION ----------------------------------
        if (!won) {
            playSound(200, 600);
            printLine('!', 54, RED);
            cout << RED   << "\n  GAME OVER! You ran out of guesses.\n" << RESET;
            cout << WHITE << "  The secret number was: "
                 << YELLOW << secret << RESET << "\n";
            printLine('!', 54, RED);
            player.streak = 0;
        }

        // ---- WIN + SCORE CALCULATION -------------------------
        if (won) {
            int baseScore  = (maxRange - minRange + 1) * 8;
            int roundScore = baseScore - (attempts * 6);
            if (roundScore < 10) roundScore = 10;

            player.streak++;

            if (player.streak >= 3) {
                cout << MAGENTA
                     << "\n  STREAK BONUS! +25 pts ("
                     << player.streak << " wins in a row!)\n"
                     << RESET;
                roundScore += 25;
            }

            player.score += roundScore;
            if (roundScore > player.bestScore)
                player.bestScore = roundScore;

            printLine('*', 54, GREEN);
            cout << GREEN   << "\n  CORRECT! The number was " << secret << "\n" << RESET;
            cout << WHITE   << "  Attempts used : " << attempts << " / " << maxAttempts << "\n" << RESET;
            cout << GREEN   << "  Round score   : " << roundScore << " pts\n"             << RESET;
            cout << CYAN    << "  Total score   : " << player.score << " pts\n"           << RESET;
            cout << MAGENTA << "  Win streak    : " << player.streak << "\n"              << RESET;
            printLine('*', 54, GREEN);
        }

        player.gamesPlayed++;
        printLine('-', 54, BLUE);

        cout << YELLOW << "\n  Play again? (y/n): " << RESET;
        cin >> playAgain;
        cin.ignore();

    } while (playAgain == 'y' || playAgain == 'Y');
}

// ================= UPDATE HIGH SCORES =================
// Inserts player best score and keeps array sorted descending.
// Uses bubble sort — same as taught in PF course.
void updateHighScores(HighScore scores[], int& count, Player& player) {

    if (count < 5 || player.bestScore > scores[count - 1].score) {

        if (count < 5) count++;

        scores[count - 1].name  = player.name;
        scores[count - 1].score = player.bestScore;

        // Bubble sort — descending order
        for (int i = 0; i < count - 1; i++) {
            for (int j = 0; j < count - i - 1; j++) {
                if (scores[j].score < scores[j + 1].score) {
                    HighScore temp = scores[j];
                    scores[j]      = scores[j + 1];
                    scores[j + 1]  = temp;
                }
            }
        }
    }
}

// ================= DISPLAY HIGH SCORES =================
void displayHighScores(HighScore scores[], int count) {

    clearScreen();
    printLine('=', 54, MAGENTA);
    cout << MAGENTA << "  HIGH SCORES\n" << RESET;
    printLine('=', 54, MAGENTA);

    if (count == 0) {
        cout << YELLOW << "\n  No scores yet! Play a game first.\n" << RESET;
        return;
    }

    string medals[5] = { "[1st]", "[2nd]", "[3rd]", "[4th]", "[5th]" };
    cout << "\n";

    for (int i = 0; i < count; i++) {
        cout << CYAN   << "  " << medals[i]
             << "  "   << scores[i].name
             << "  -  "
             << YELLOW << scores[i].score << " pts"
             << RESET  << "\n";
    }
    cout << "\n";
}

// ================= PLAYER STATS =================
void displayStats(Player& p) {

    clearScreen();
    printLine('=', 54, CYAN);
    cout << CYAN << "  PLAYER STATS\n" << RESET;
    printLine('=', 54, CYAN);

    cout << WHITE   << "\n  Name         : " << GREEN   << p.name        << RESET << "\n";
    cout << WHITE   << "  Games Played : " << YELLOW  << p.gamesPlayed  << RESET << "\n";
    cout << WHITE   << "  Total Score  : " << YELLOW  << p.score        << RESET << "\n";
    cout << WHITE   << "  Best Score   : " << YELLOW  << p.bestScore    << RESET << "\n";
    cout << WHITE   << "  Win Streak   : " << MAGENTA << p.streak       << RESET << "\n\n";
}

// ================= UTILITY FUNCTIONS =================
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void playSound(int freq, int duration) {
#ifdef _WIN32
    Beep(freq, duration);
#else
    cout << "\a";
    cout.flush();
    (void)freq;
    (void)duration;
#endif
}

void printLine(char ch, int len, const string& color) {
    cout << color;
    for (int i = 0; i < len; i++) cout << ch;
    cout << RESET << "\n";
}

// ============================================================
//  HOW TO COMPILE AND RUN
//
//  Visual Studio:
//    1. Create new Empty C++ Project
//    2. Right-click Source Files > Add > New Item > main.cpp
//    3. Paste this code
//    4. Press Ctrl+F5 (Run without debugging)
//
//  Code::Blocks:
//    1. File > New > Empty File > Save as main.cpp
//    2. Press F9 (Build and Run)
//
//  MinGW / Terminal:
//    g++ main.cpp -o game.exe
//    game.exe
// ============================================================
