/*
 * ============================================================
 *  MAZE GAME  -  Terminal puzzle game inspired by Pac-Man
 *  Cross-platform: Linux & Windows
 *  Compile : g++ -std=c++17 -Wall -O2 -o mazegame MazeAmaze.cpp
 *  Controls: W A S D to move  |  / for commands
 * ============================================================
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cctype>

#ifdef _WIN32
    #include <conio.h>
    #include <windows.h>
#else
    #include <termios.h>
    #include <unistd.h>
#endif

using namespace std;

// ============================================================
//  SECTION 1 : PLATFORM LAYER
//  getch()      – read one keystroke without echo
//  clearScreen()– erase the terminal
//  init()       – one-time platform setup (enables ANSI on Win)
// ============================================================

#ifdef _WIN32

static void enableWindowsANSI() {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD  mode = 0;
    GetConsoleMode(h, &mode);
    SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}

namespace platform {
    int  getch()       { return ::_getch(); }
    void clearScreen() { system("cls"); }
    void init()        { enableWindowsANSI(); }
}

#else   // Linux / macOS

// Custom getch under its own namespace, as requested.
namespace linux_term {
    int getch() {
        struct termios oldt, newt;
        int ch;
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);   // raw + silent
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        ch = getchar();
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // restore
        return ch;
    }
}

namespace platform {
    int  getch() { return linux_term::getch(); }
    void clearScreen() {
        // POSIX: write ANSI clear-screen + cursor-home instead of system()
        // Avoids system() return-value warning and is faster.
        const char* cls = "\033[2J\033[H";
        if (write(STDOUT_FILENO, cls, 7)) {}
    }
    void init() {}
}

#endif  // _WIN32

// ============================================================
//  SECTION 2 : ANSI COLOR HELPERS
// ============================================================

#define ANSI_RESET      "\033[0m"
#define ANSI_BOLD       "\033[1m"
#define ANSI_RED        "\033[91m"   // wall
#define ANSI_GREEN      "\033[92m"   // start
#define ANSI_YELLOW     "\033[93m"   // finish
#define ANSI_CYAN       "\033[96m"   // player
#define ANSI_GRAY       "\033[90m"   // open path
#define ANSI_WHITE      "\033[97m"   // special
#define ANSI_TITLE      "\033[1;95m" // bold magenta – title bar
#define ANSI_WARN       "\033[1;93m" // bold yellow  – messages
#define ANSI_DIM        "\033[2m"    // dimmed       – hints

// ============================================================
//  SECTION 3 : MAP CONSTANTS  (symbols used in .txt files)
// ============================================================

const char CELL_WALL    = 'X';   // impassable
const char CELL_PATH    = '*';   // open path
const char CELL_START   = '.';   // player spawn
const char CELL_FINISH  = '!';   // goal / exit
const char CELL_SPECIAL = '?';   // special area (reserved)
const char CELL_PLAYER  = '@';   // rendered player icon

// ============================================================
//  SECTION 4 : DATA STRUCTURES
// ============================================================

struct Stage {
    vector<vector<char>> grid;
    int rows     = 0;
    int cols     = 0;
    int startRow = -1, startCol = -1;
    int finishRow= -1, finishCol= -1;
    bool valid   = false;
};

struct Level {
    string        name;
    vector<Stage> stages;
};

// ============================================================
//  SECTION 5 : MAP FILE PARSER
//
//  File format  (maps.txt):
//
//    > LEVEL 1
//    X . X X
//    X * * X
//    X X * X
//    X X ! X
//    ,
//    X . X X
//    ...more stages...
//    EndMap
//    > LEVEL 2
//    ...
//    EndMap
//
//  Rules:
//    - '>' starts a new level, text after '>' is the level name.
//    - ',' separates stages inside a level.
//    - 'EndMap' closes a level block.
//    - Spacing between symbols is ignored; only X * . ! ? matter.
//    - Lines starting with '#' are treated as comments.
// ============================================================

static string trim(const string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

static bool isValidCell(char c) {
    return c == CELL_WALL || c == CELL_PATH ||
           c == CELL_START || c == CELL_FINISH || c == CELL_SPECIAL;
}

static Stage parseStage(const vector<string>& rawLines) {
    Stage s;
    int maxCols = 0;

    for (const auto& line : rawLines) {
        if (trim(line).empty()) continue;

        vector<char> row;
        for (char c : line) {
            if (isValidCell(c)) row.push_back(c);
        }
        if (!row.empty()) {
            s.grid.push_back(row);
            maxCols = max(maxCols, (int)row.size());
        }
    }

    s.rows = (int)s.grid.size();
    s.cols = maxCols;

    // Locate start and finish
    for (int r = 0; r < s.rows; r++) {
        for (int c = 0; c < (int)s.grid[r].size(); c++) {
            if (s.grid[r][c] == CELL_START)  { s.startRow  = r; s.startCol  = c; }
            if (s.grid[r][c] == CELL_FINISH) { s.finishRow = r; s.finishCol = c; }
        }
    }

    s.valid = (s.rows > 0 && s.startRow >= 0 && s.finishRow >= 0);
    return s;
}

vector<Level> loadMapFile(const string& filename) {
    vector<Level> levels;
    ifstream file(filename);
    if (!file.is_open()) return levels;

    Level          currentLevel;
    vector<string> stageLines;
    bool           inLevel = false;
    string         line;

    auto commitStage = [&]() {
        if (!stageLines.empty()) {
            Stage s = parseStage(stageLines);
            if (s.valid) currentLevel.stages.push_back(s);
            stageLines.clear();
        }
    };

    auto commitLevel = [&]() {
        commitStage();
        if (!currentLevel.stages.empty()) levels.push_back(currentLevel);
        currentLevel = Level();
        inLevel = false;
    };

    while (getline(file, line)) {
        // Strip Windows \r
        if (!line.empty() && line.back() == '\r') line.pop_back();

        string t = trim(line);

        if (t.empty() || t[0] == '#') continue;   // blank / comment

        if (t[0] == '>') {
            if (inLevel) commitLevel();
            currentLevel.name = trim(t.substr(1));
            inLevel = true;
        } else if (t == "EndMap") {
            if (inLevel) commitLevel();
        } else if (t == ",") {
            if (inLevel) commitStage();
        } else if (inLevel) {
            stageLines.push_back(line);
        }
    }

    // Handle file that has no trailing EndMap
    if (inLevel) commitLevel();

    return levels;
}

// ============================================================
//  SECTION 6 : RENDERER
// ============================================================

static string coloredCell(char cell, bool isPlayer) {
    if (isPlayer)
        return string(ANSI_CYAN) + ANSI_BOLD + CELL_PLAYER + ANSI_RESET;

    switch (cell) {
        case CELL_WALL:    return string(ANSI_RED)    + CELL_WALL    + ANSI_RESET;
        case CELL_START:   return string(ANSI_GREEN)  + CELL_START   + ANSI_RESET;
        case CELL_FINISH:  return string(ANSI_YELLOW) + ANSI_BOLD + CELL_FINISH + ANSI_RESET;
        case CELL_PATH:    return string(ANSI_GRAY)   + CELL_PATH    + ANSI_RESET;
        case CELL_SPECIAL: return string(ANSI_WHITE)  + ANSI_BOLD + CELL_SPECIAL + ANSI_RESET;
        default:           return string(1, cell);
    }
}

static string divider(int len, char ch = '=') { return string(len, ch); }

void renderGame(const Stage& stage,
                int playerRow, int playerCol,
                const string& levelName, int stageIdx, int totalStages,
                const string& message) {
    platform::clearScreen();

    // ---- header ----
    int barWidth = max(40, stage.cols * 2 + 6);
    cout << ANSI_TITLE
         << " MAZE GAME  |  " << levelName
         << "  |  Stage " << (stageIdx + 1) << " / " << totalStages
         << ANSI_RESET << "\n"
         << divider(barWidth) << "\n\n";

    // ---- map grid ----
    for (int r = 0; r < stage.rows; r++) {
        cout << "  ";
        for (int c = 0; c < (int)stage.grid[r].size(); c++) {
            bool isPlayer = (r == playerRow && c == playerCol);
            cout << coloredCell(stage.grid[r][c], isPlayer) << " ";
        }
        cout << "\n";
    }

    // ---- status bar ----
    cout << "\n" << divider(barWidth, '-') << "\n";
    cout << "  " << ANSI_BOLD << "Pos : " << ANSI_RESET
         << "[" << playerRow << "][" << playerCol << "]"
         << "    "
         << ANSI_BOLD << "Goal: " << ANSI_RESET
         << "[" << stage.finishRow << "][" << stage.finishCol << "]\n";

    // ---- message ----
    if (!message.empty()) {
        cout << "  " << ANSI_WARN << ">> " << message << ANSI_RESET << "\n";
    }

    // ---- controls hint ----
    cout << "\n"
         << ANSI_DIM
         << "  [W] Up  [A] Left  [S] Down  [D] Right  "
         << "[/] Command"
         << ANSI_RESET << "\n\n";
}

// ============================================================
//  SECTION 7 : GAME STATE
// ============================================================

struct GameState {
    vector<Level> levels;
    int  levelIdx  = 0;
    int  stageIdx  = 0;
    int  playerRow = 0;
    int  playerCol = 0;
    bool running   = true;
    bool won       = false;
    string message;

    // ---- accessors ----
    const Stage& currentStage() const {
        return levels[levelIdx].stages[stageIdx];
    }
    const string& currentLevelName() const {
        return levels[levelIdx].name;
    }
    int totalStages() const {
        return (int)levels[levelIdx].stages.size();
    }

    // ---- place player at start point ----
    void spawnPlayer() {
        const Stage& s = currentStage();
        playerRow = s.startRow;
        playerCol = s.startCol;
    }

    // ---- attempt movement by direction vector (dr, dc) ----
    //  returns  true  if the move succeeded
    //  returns  false if blocked by a wall or boundary
    bool tryMove(int dr, int dc) {
        int newR = playerRow + dr;
        int newC = playerCol + dc;
        const Stage& s = currentStage();

        // Boundary check
        if (newR < 0 || newR >= s.rows) return false;
        if (newC < 0 || newC >= (int)s.grid[newR].size()) return false;

        char target = s.grid[newR][newC];

        // Wall check → return false (cannot enter)
        if (target == CELL_WALL) return false;

        // Move accepted → update currentPoint
        playerRow = newR;
        playerCol = newC;

        // Reached the finish?
        if (target == CELL_FINISH) advanceStage();

        return true;
    }

    // ---- advance to next stage or level ----
    void advanceStage() {
        int nextStage = stageIdx + 1;
        int nextLevel = levelIdx + 1;

        if (nextStage < (int)levels[levelIdx].stages.size()) {
            stageIdx = nextStage;
            spawnPlayer();
            message = "Stage cleared!  -->  Moving to stage "
                      + to_string(stageIdx + 1) + "...";
        } else if (nextLevel < (int)levels.size()) {
            levelIdx = nextLevel;
            stageIdx = 0;
            spawnPlayer();
            message = "Level complete!  -->  Entering "
                      + levels[levelIdx].name + "...";
        } else {
            won     = true;
            running = false;
        }
    }
};

// ============================================================
//  SECTION 8 : COMMAND SYSTEM  (/ prefix)
// ============================================================

static void showHelp() {
    platform::clearScreen();
    cout << ANSI_TITLE << "=== MAZE GAME  -  HELP ===" << ANSI_RESET << "\n\n";

    cout << ANSI_BOLD << "  Movement\n" << ANSI_RESET;
    cout << "    W  -  Move up\n"
         << "    A  -  Move left\n"
         << "    S  -  Move down\n"
         << "    D  -  Move right\n\n";

    cout << ANSI_BOLD << "  Map Symbols\n" << ANSI_RESET;
    cout << "    " << ANSI_RED    << "X" << ANSI_RESET << "  -  Wall      (impassable)\n";
    cout << "    " << ANSI_GRAY   << "*" << ANSI_RESET << "  -  Open path (walkable)\n";
    cout << "    " << ANSI_GREEN  << "." << ANSI_RESET << "  -  Start     (spawn point)\n";
    cout << "    " << ANSI_YELLOW << "!" << ANSI_RESET << "  -  Finish    (reach to win)\n";
    cout << "    " << ANSI_CYAN   << "@" << ANSI_RESET << "  -  Player    (you)\n\n";

    cout << ANSI_BOLD << "  Commands  (press '/' then type + Enter)\n" << ANSI_RESET;
    cout << "    /help  -  Show this screen\n"
         << "    /q     -  Quit the game\n\n";

    cout << ANSI_BOLD << "  Goal\n" << ANSI_RESET;
    cout << "    Navigate from " << ANSI_GREEN << "." << ANSI_RESET
         << " to " << ANSI_YELLOW << "!" << ANSI_RESET
         << " in every stage.\n"
         << "    Complete all stages to finish a level.\n\n";

    cout << divider(40, '-') << "\n";
    cout << "  Press any key to return to game...\n";
    platform::getch();
}

// Read a full line from stdin, restoring line-mode on Linux first.
static string readLine() {
    string cmd;
#ifndef _WIN32
    // Restore canonical + echo so the user can type normally
    struct termios oldt, cooked;
    tcgetattr(STDIN_FILENO, &oldt);
    cooked = oldt;
    cooked.c_lflag |= (ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &cooked);
    getline(cin, cmd);
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#else
    getline(cin, cmd);
#endif
    return cmd;
}

static void handleCommand(const string& rawCmd, GameState& state) {
    string cmd = trim(rawCmd);
    string low  = cmd;
    transform(low.begin(), low.end(), low.begin(), ::tolower);

    if (low == "q" || low == "quit" || low == "exit") {
        state.running = false;
    } else if (low == "help") {
        showHelp();
    } else if (cmd.empty()) {
        /* nothing */
    } else {
        state.message = "Unknown command '/" + cmd + "'  |  Try /help";
    }
}

// ============================================================
//  SECTION 9 : MAIN GAME LOOP
// ============================================================

static void gameLoop(GameState& state) {
    while (state.running) {
        const Stage& stage = state.currentStage();

        renderGame(stage,
                   state.playerRow, state.playerCol,
                   state.currentLevelName(),
                   state.stageIdx, state.totalStages(),
                   state.message);
        state.message = "";

        int  key = platform::getch();
        char ch  = (char)tolower((unsigned char)key);

        // ---- movement (direction vectors) ----
        if      (ch == 'w') { if (!state.tryMove(-1,  0)) state.message = "Blocked by wall!"; }
        else if (ch == 's') { if (!state.tryMove( 1,  0)) state.message = "Blocked by wall!"; }
        else if (ch == 'a') { if (!state.tryMove( 0, -1)) state.message = "Blocked by wall!"; }
        else if (ch == 'd') { if (!state.tryMove( 0,  1)) state.message = "Blocked by wall!"; }

        // ---- command input session ----
        else if (ch == '/') {
            // Re-draw the current game screen cleanly, then show input prompt
            renderGame(stage,
                       state.playerRow, state.playerCol,
                       state.currentLevelName(),
                       state.stageIdx, state.totalStages(), "");

            cout << "  " << ANSI_BOLD << "Command" << ANSI_RESET
                 << "  /";
            cout.flush();

            string cmd = readLine();
            handleCommand(cmd, state);
        }
    }
}

// ============================================================
//  SECTION 10 : ENTRY POINT
// ============================================================

int main(int argc, char* argv[]) {
    platform::init();

    string mapFile = "maps.txt";
    if (argc > 1) mapFile = argv[1];

    // Load levels from file
    vector<Level> levels = loadMapFile(mapFile);

    if (levels.empty()) {
        cout << ANSI_RED << "Error:" << ANSI_RESET
             << " Cannot load map file '"<< mapFile << "'.\n"
             << "Usage: " << argv[0] << " [map_file.txt]\n";
        return 1;
    }

    // ---- start screen ----
    platform::clearScreen();
    cout << ANSI_TITLE << R"(
  +================================+
  |         M A Z E  G A M E      |
  |   Find your way to the exit   |
  +================================+
)" << ANSI_RESET;
    cout << "\n  Map file : " << mapFile
         << "\n  Levels   : " << levels.size() << "\n\n";
    cout << "  " << ANSI_GREEN << "W / A / S / D" << ANSI_RESET << "  to move\n";
    cout << "  " << ANSI_GREEN << "/            " << ANSI_RESET << "  to enter a command\n";
    cout << "  " << ANSI_GREEN << "/help        " << ANSI_RESET << "  for full instructions\n\n";
    cout << "  Press any key to start...\n";
    platform::getch();

    // ---- run game ----
    GameState state;
    state.levels = levels;
    state.spawnPlayer();
    gameLoop(state);

    // ---- end screen ----
    platform::clearScreen();
    if (state.won) {
        cout << ANSI_TITLE << R"(
  +================================+
  |  YOU WIN!  CONGRATULATIONS!   |
  |  All levels cleared.  Bravo!  |
  +================================+
)" << ANSI_RESET << "\n";
    } else {
        cout << "\n  Thanks for playing Maze Game!\n\n";
    }

    return 0;
}
