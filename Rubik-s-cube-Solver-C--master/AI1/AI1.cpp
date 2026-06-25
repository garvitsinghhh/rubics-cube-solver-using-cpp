// AI1.cpp : Rubik's Cube Solver in C++23
//

#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <stack>
#include <queue>
#include <unordered_set>
#include <memory>
#include <algorithm>

// Representation: 6 faces, each is a 3x3 grid of integers
using CubeState = std::array<std::array<std::array<int, 3>, 3>, 6>;

// Opposite moves mapping to prevent immediate back-and-forth undoing in IDS
// 1 (L)  <-> 2 (L')
// 3 (R)  <-> 4 (R')
// 5 (U)  <-> 6 (U')
// 7 (D)  <-> 8 (D')
// 9 (F)  <-> 10 (F')
// 11 (B) <-> 12 (B')
inline bool isOppositeMove(int m1, int m2) {
    if (m1 == 0 || m2 == 0) return false;
    return ((m1 - 1) ^ 1) == (m2 - 1);
}

// Check if two states are identical
bool compareState(const CubeState& a, const CubeState& b) {
    return a == b;
}

// Print a face layout for debugging
void print(const CubeState& arr) {
    std::cout << "Printing " << std::endl;
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++) {
                std::cout << arr[i][j][k];
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

// Rotation functions mapped exactly from original C-style logic
CubeState leftClock(const CubeState& cube) {
    CubeState temp = cube;
    temp[2][0][1] = cube[2][1][0];
    temp[2][1][0] = cube[2][2][1];
    temp[2][2][1] = cube[2][1][2];
    temp[2][1][2] = cube[2][0][1];

    temp[2][0][0] = cube[2][2][0];
    temp[2][2][0] = cube[2][2][2];
    temp[2][2][2] = cube[2][0][2];
    temp[2][0][2] = cube[2][0][0];

    temp[0][0][0] = cube[3][0][0];
    temp[0][1][0] = cube[3][1][0];
    temp[0][2][0] = cube[3][2][0];

    temp[3][0][0] = cube[5][0][0];
    temp[3][1][0] = cube[5][1][0];
    temp[3][2][0] = cube[5][2][0];

    temp[5][0][0] = cube[1][0][0];
    temp[5][1][0] = cube[1][1][0];
    temp[5][2][0] = cube[1][2][0];

    temp[1][0][0] = cube[0][0][0];
    temp[1][1][0] = cube[0][1][0];
    temp[1][2][0] = cube[0][2][0];
    return temp;
}

CubeState leftAntiClock(const CubeState& cube) {
    CubeState temp = cube;
    temp[2][0][1] = cube[2][1][2];
    temp[2][1][0] = cube[2][0][1];
    temp[2][2][1] = cube[2][1][0];
    temp[2][1][2] = cube[2][2][1];

    temp[2][0][0] = cube[2][0][2];
    temp[2][2][0] = cube[2][0][0];
    temp[2][2][2] = cube[2][2][0];
    temp[2][0][2] = cube[2][2][2];

    temp[0][0][0] = cube[1][0][0];
    temp[0][1][0] = cube[1][1][0];
    temp[0][2][0] = cube[1][2][0];

    temp[3][0][0] = cube[0][0][0];
    temp[3][1][0] = cube[0][1][0];
    temp[3][2][0] = cube[0][2][0];

    temp[5][0][0] = cube[3][0][0];
    temp[5][1][0] = cube[3][1][0];
    temp[5][2][0] = cube[3][2][0];

    temp[1][0][0] = cube[5][0][0];
    temp[1][1][0] = cube[5][1][0];
    temp[1][2][0] = cube[5][2][0];
    return temp;
}

CubeState rightClock(const CubeState& cube) {
    CubeState temp = cube;
    temp[4][0][1] = cube[4][1][2];
    temp[4][1][0] = cube[4][0][1];
    temp[4][2][1] = cube[4][1][0];
    temp[4][1][2] = cube[4][2][1];

    temp[4][0][0] = cube[4][0][2];
    temp[4][2][0] = cube[4][0][0];
    temp[4][2][2] = cube[4][2][0];
    temp[4][0][2] = cube[4][2][2];

    temp[0][0][2] = cube[1][0][2];
    temp[0][1][2] = cube[1][1][2];
    temp[0][2][2] = cube[1][2][2];

    temp[3][0][2] = cube[0][0][2];
    temp[3][1][2] = cube[0][1][2];
    temp[3][2][2] = cube[0][2][2];

    temp[5][0][2] = cube[3][0][2];
    temp[5][1][2] = cube[3][1][2];
    temp[5][2][2] = cube[3][2][2];

    temp[1][0][2] = cube[5][0][2];
    temp[1][1][2] = cube[5][1][2];
    temp[1][2][2] = cube[5][2][2];
    return temp;
}

CubeState rightAntiClock(const CubeState& cube) {
    CubeState temp = cube;
    temp[4][0][1] = cube[4][1][0];
    temp[4][1][0] = cube[4][2][1];
    temp[4][2][1] = cube[4][1][2];
    temp[4][1][2] = cube[4][0][1];

    temp[4][0][0] = cube[4][2][0];
    temp[4][2][0] = cube[4][2][2];
    temp[4][2][2] = cube[4][0][2];
    temp[4][0][2] = cube[4][0][0];

    temp[0][0][2] = cube[3][0][2];
    temp[0][1][2] = cube[3][1][2];
    temp[0][2][2] = cube[3][2][2];

    temp[3][0][2] = cube[5][0][2];
    temp[3][1][2] = cube[5][1][2];
    temp[3][2][2] = cube[5][2][2];

    temp[5][0][2] = cube[1][0][2];
    temp[5][1][2] = cube[1][1][2];
    temp[5][2][2] = cube[1][2][2];

    temp[1][0][2] = cube[0][0][2];
    temp[1][1][2] = cube[0][1][2];
    temp[1][2][2] = cube[0][2][2];
    return temp;
}

CubeState upClock(const CubeState& cube) {
    CubeState temp = cube;
    temp[0][0][1] = cube[0][1][2];
    temp[0][1][0] = cube[0][0][1];
    temp[0][2][1] = cube[0][1][0];
    temp[0][1][2] = cube[0][2][1];

    temp[0][0][0] = cube[0][0][2];
    temp[0][2][0] = cube[0][0][0];
    temp[0][2][2] = cube[0][2][0];
    temp[0][0][2] = cube[0][2][2];

    temp[2][0][0] = cube[1][0][0];
    temp[2][1][0] = cube[1][1][0];
    temp[2][2][0] = cube[1][2][0];

    temp[3][0][0] = cube[2][0][0];
    temp[3][1][0] = cube[2][1][0];
    temp[3][2][0] = cube[2][2][0];

    temp[4][0][0] = cube[3][0][0];
    temp[4][1][0] = cube[3][1][0];
    temp[4][2][0] = cube[3][2][0];

    temp[1][0][0] = cube[4][0][0];
    temp[1][1][0] = cube[4][1][0];
    temp[1][2][0] = cube[4][2][0];
    return temp;
}

CubeState upAntiClock(const CubeState& cube) {
    CubeState temp = cube;
    temp[0][0][1] = cube[0][1][0];
    temp[0][1][0] = cube[0][2][1];
    temp[0][2][1] = cube[0][1][2];
    temp[0][1][2] = cube[0][0][1];

    temp[0][0][0] = cube[0][2][0];
    temp[0][2][0] = cube[0][2][2];
    temp[0][2][2] = cube[0][0][2];
    temp[0][0][2] = cube[0][0][0];

    temp[2][0][0] = cube[3][0][0];
    temp[2][1][0] = cube[3][1][0];
    temp[2][2][0] = cube[3][2][0];

    temp[3][0][0] = cube[4][0][0];
    temp[3][1][0] = cube[4][1][0];
    temp[3][2][0] = cube[4][2][0];

    temp[4][0][0] = cube[1][0][0];
    temp[4][1][0] = cube[1][1][0];
    temp[4][2][0] = cube[1][2][0];

    temp[1][0][0] = cube[2][0][0];
    temp[1][1][0] = cube[2][1][0];
    temp[1][2][0] = cube[2][2][0];
    return temp;
}

CubeState downClock(const CubeState& cube) {
    CubeState temp = cube;
    temp[5][0][1] = cube[5][1][0];
    temp[5][1][0] = cube[5][2][1];
    temp[5][2][1] = cube[5][1][2];
    temp[5][1][2] = cube[5][0][1];

    temp[5][0][0] = cube[5][2][0];
    temp[5][2][0] = cube[5][2][2];
    temp[5][2][2] = cube[5][0][2];
    temp[5][0][2] = cube[5][0][0];

    temp[2][0][2] = cube[3][0][2];
    temp[2][1][2] = cube[3][1][2];
    temp[2][2][2] = cube[3][2][2];

    temp[3][0][2] = cube[4][0][2];
    temp[3][1][2] = cube[4][1][2];
    temp[3][2][2] = cube[4][2][2];

    temp[4][0][2] = cube[1][0][2];
    temp[4][1][2] = cube[1][1][2];
    temp[4][2][2] = cube[1][2][2];

    temp[1][0][2] = cube[2][0][2];
    temp[1][1][2] = cube[2][1][2];
    temp[1][2][2] = cube[2][2][2];
    return temp;
}

CubeState downAntiClock(const CubeState& cube) {
    CubeState temp = cube;
    temp[5][0][1] = cube[5][1][2];
    temp[5][1][0] = cube[5][0][1];
    temp[5][2][1] = cube[5][1][0];
    temp[5][1][2] = cube[5][2][1];

    temp[5][0][0] = cube[5][0][2];
    temp[5][2][0] = cube[5][0][0];
    temp[5][2][2] = cube[5][2][0];
    temp[5][0][2] = cube[5][2][2];

    temp[2][0][2] = cube[1][0][2];
    temp[2][1][2] = cube[1][1][2];
    temp[2][2][2] = cube[1][2][2];

    temp[3][0][2] = cube[2][0][2];
    temp[3][1][2] = cube[2][1][2];
    temp[3][2][2] = cube[2][2][2];

    temp[4][0][2] = cube[3][0][2];
    temp[4][1][2] = cube[3][1][2];
    temp[4][2][2] = cube[3][2][2];

    temp[1][0][2] = cube[4][0][2];
    temp[1][1][2] = cube[4][1][2];
    temp[1][2][2] = cube[4][2][2];
    return temp;
}

CubeState frontClock(const CubeState& cube) {
    CubeState temp = cube;
    temp[1][0][1] = cube[1][1][2];
    temp[1][1][0] = cube[1][0][1];
    temp[1][2][1] = cube[1][1][0];
    temp[1][1][2] = cube[1][2][1];

    temp[1][0][0] = cube[1][0][2];
    temp[1][2][0] = cube[1][0][0];
    temp[1][2][2] = cube[1][2][0];
    temp[1][0][2] = cube[1][2][2];

    temp[0][0][2] = cube[2][0][2];
    temp[0][1][2] = cube[2][1][2];
    temp[0][2][2] = cube[2][2][2];

    temp[4][0][2] = cube[0][0][2];
    temp[4][1][2] = cube[0][1][2];
    temp[4][2][2] = cube[0][2][2];

    temp[5][0][2] = cube[4][0][2];
    temp[5][1][2] = cube[4][1][2];
    temp[5][2][2] = cube[4][2][2];

    temp[2][0][2] = cube[5][0][2];
    temp[2][1][2] = cube[5][1][2];
    temp[2][2][2] = cube[5][2][2];
    return temp;
}

CubeState frontAntiClock(const CubeState& cube) {
    CubeState temp = cube;
    temp[1][0][1] = cube[1][1][0];
    temp[1][1][0] = cube[1][2][1];
    temp[1][2][1] = cube[1][1][2];
    temp[1][1][2] = cube[1][0][1];

    temp[1][0][0] = cube[1][2][0];
    temp[1][2][0] = cube[1][2][2];
    temp[1][2][2] = cube[1][0][2];
    temp[1][0][2] = cube[1][0][0];

    temp[5][0][2] = cube[2][0][2];
    temp[5][1][2] = cube[2][1][2];
    temp[5][2][2] = cube[2][2][2];

    temp[2][0][2] = cube[0][0][2];
    temp[2][1][2] = cube[0][1][2];
    temp[2][2][2] = cube[0][2][2];

    temp[0][0][2] = cube[4][0][2];
    temp[0][1][2] = cube[4][1][2];
    temp[0][2][2] = cube[4][2][2];

    temp[4][0][2] = cube[5][0][2];
    temp[4][1][2] = cube[5][1][2];
    temp[4][2][2] = cube[5][2][2];
    return temp;
}

CubeState backClock(const CubeState& cube) {
    CubeState temp = cube;
    temp[3][0][1] = cube[3][1][0];
    temp[3][1][0] = cube[3][2][1];
    temp[3][2][1] = cube[3][1][2];
    temp[3][1][2] = cube[3][0][1];

    temp[3][0][0] = cube[3][2][0];
    temp[3][2][0] = cube[3][2][2];
    temp[3][2][2] = cube[3][0][2];
    temp[3][0][2] = cube[3][0][0];

    temp[5][0][0] = cube[2][0][0];
    temp[5][1][0] = cube[2][1][0];
    temp[5][2][0] = cube[2][2][0];

    temp[2][0][0] = cube[0][0][0];
    temp[2][1][0] = cube[0][1][0];
    temp[2][2][0] = cube[0][2][0];

    temp[0][0][0] = cube[4][0][0];
    temp[0][1][0] = cube[4][1][0];
    temp[0][2][0] = cube[4][2][0];

    temp[4][0][0] = cube[5][0][0];
    temp[4][1][0] = cube[5][1][0];
    temp[4][2][0] = cube[5][2][0];
    return temp;
}

CubeState backAntiClock(const CubeState& cube) {
    CubeState temp = cube;
    temp[3][0][1] = cube[3][1][2];
    temp[3][1][0] = cube[3][0][1];
    temp[3][2][1] = cube[3][1][0];
    temp[3][1][2] = cube[3][2][1];

    temp[3][0][0] = cube[3][0][2];
    temp[3][2][0] = cube[3][0][0];
    temp[3][2][2] = cube[3][2][0];
    temp[3][0][2] = cube[3][2][2];

    temp[0][0][0] = cube[2][0][0];
    temp[0][1][0] = cube[2][1][0];
    temp[0][2][0] = cube[2][2][0];

    temp[4][0][0] = cube[0][0][0];
    temp[4][1][0] = cube[0][1][0];
    temp[4][2][0] = cube[0][2][0];

    temp[5][0][0] = cube[4][0][0];
    temp[5][1][0] = cube[4][1][0];
    temp[5][2][0] = cube[4][2][0];

    temp[2][0][0] = cube[5][0][0];
    temp[2][1][0] = cube[5][1][0];
    temp[2][2][0] = cube[5][2][0];
    return temp;
}

// Dispatch function to apply a move by index
CubeState applyMove(const CubeState& state, int move) {
    switch (move) {
        case 1:  return leftClock(state);
        case 2:  return leftAntiClock(state);
        case 3:  return rightClock(state);
        case 4:  return rightAntiClock(state);
        case 5:  return upClock(state);
        case 6:  return upAntiClock(state);
        case 7:  return downClock(state);
        case 8:  return downAntiClock(state);
        case 9:  return frontClock(state);
        case 10: return frontAntiClock(state);
        case 11: return backClock(state);
        case 12: return backAntiClock(state);
        default: return state;
    }
}

// Print move descriptions
void printMove(int p) {
    switch (p) {
        case 1:  std::cout << "move Left ClockWise" << std::endl; break;
        case 2:  std::cout << "move Left AntiClockWise" << std::endl; break;
        case 3:  std::cout << "move Right ClockWise" << std::endl; break;
        case 4:  std::cout << "move Right AntiClockWise" << std::endl; break;
        case 5:  std::cout << "move Up ClockWise" << std::endl; break;
        case 6:  std::cout << "move Up AntiClockWise" << std::endl; break;
        case 7:  std::cout << "move Down ClockWise" << std::endl; break;
        case 8:  std::cout << "move Down AntiClockWise" << std::endl; break;
        case 9:  std::cout << "move Front ClockWise" << std::endl; break;
        case 10: std::cout << "move Front AntiClockWise" << std::endl; break;
        case 11: std::cout << "move Back ClockWise" << std::endl; break;
        case 12: std::cout << "move Back AntiClockWise" << std::endl; break;
    }
}

// Parse state from file
bool takeInput(CubeState& startState, CubeState& finalState) {
    std::ifstream file("input.txt");
    if (!file.is_open()) {
        std::cerr << "Cannot open file input.txt" << std::endl;
        return false;
    }

    int value = 0;
    // read start state
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++) {
                if (!(file >> value)) return false;
                startState[i][j][k] = value;
            }
        }
    }

    // read final state
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++) {
                if (!(file >> value)) return false;
                finalState[i][j][k] = value;
            }
        }
    }
    return true;
}

// Heuristic function for A* (number of mismatches)
int getMismatches(const CubeState& current, const CubeState& finalState) {
    int count = 0;
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++) {
                if (current[i][j][k] != finalState[i][j][k]) {
                    count++;
                }
            }
        }
    }
    return count;
}

// ----------------------------------------------------
// Iterative Deepening Search (IDS) Implementation
// ----------------------------------------------------
bool dls(const CubeState& current, const CubeState& finalState, int depthLimit, std::vector<int>& path, int prevMove) {
    if (current == finalState) {
        return true;
    }
    if (depthLimit <= 0) {
        return false;
    }

    for (int move = 1; move <= 12; ++move) {
        // Pruning: skip immediate opposite moves
        if (isOppositeMove(prevMove, move)) {
            continue;
        }

        CubeState nextState = applyMove(current, move);
        path.push_back(move);
        if (dls(nextState, finalState, depthLimit - 1, path, move)) {
            return true;
        }
        path.pop_back();
    }
    return false;
}

std::vector<int> solveIDS(const CubeState& start, const CubeState& finalState, int maxDepth = 30) {
    std::vector<int> path;
    for (int depth = 0; depth <= maxDepth; ++depth) {
        path.clear();
        if (dls(start, finalState, depth, path, 0)) {
            return path;
        }
    }
    return {};
}

// Hash function for CubeState to use with std::unordered_set
struct CubeStateHash {
    std::size_t operator()(const CubeState& state) const {
        std::size_t h = 0;
        for (int i = 0; i < 6; ++i) {
            for (int j = 0; j < 3; ++j) {
                for (int k = 0; k < 3; ++k) {
                    h ^= std::hash<int>{}(state[i][j][k]) + 0x9e3779b9 + (h << 6) + (h >> 2);
                }
            }
        }
        return h;
    }
};

// ----------------------------------------------------
// A* Search Implementation
// ----------------------------------------------------
struct AStarNode {
    CubeState state;
    int g = 0;
    int h = 0;
    int prevMove = 0;
    std::shared_ptr<AStarNode> parent = nullptr;

    int f() const { return g + h; }
};

struct AStarNodeCompare {
    bool operator()(const std::shared_ptr<AStarNode>& a, const std::shared_ptr<AStarNode>& b) const {
        return a->f() > b->f(); // Min-priority queue
    }
};

std::vector<int> solveAStar(const CubeState& start, const CubeState& finalState, int maxNodes = 50000) {
    std::priority_queue<std::shared_ptr<AStarNode>, std::vector<std::shared_ptr<AStarNode>>, AStarNodeCompare> openList;
    std::unordered_set<CubeState, CubeStateHash> closedList;

    auto root = std::make_shared<AStarNode>();
    root->state = start;
    root->g = 0;
    root->h = getMismatches(start, finalState);
    root->prevMove = 0;
    root->parent = nullptr;

    openList.push(root);

    int nodesExplored = 0;

    while (!openList.empty()) {
        auto curr = openList.top();
        openList.pop();

        if (curr->state == finalState) {
            std::vector<int> path;
            auto node = curr;
            while (node && node->parent) {
                path.push_back(node->prevMove);
                node = node->parent;
            }
            std::reverse(path.begin(), path.end());
            return path;
        }

        if (closedList.contains(curr->state)) {
            continue;
        }
        closedList.insert(curr->state);
        nodesExplored++;

        if (nodesExplored >= maxNodes) {
            // Stop early to prevent hanging on unsolvable/too complex configurations
            break;
        }

        for (int move = 1; move <= 12; ++move) {
            if (isOppositeMove(curr->prevMove, move)) {
                continue;
            }

            CubeState nextState = applyMove(curr->state, move);
            if (closedList.contains(nextState)) {
                continue;
            }

            auto child = std::make_shared<AStarNode>();
            child->state = nextState;
            child->g = curr->g + 1;
            child->h = getMismatches(nextState, finalState);
            child->prevMove = move;
            child->parent = curr;

            openList.push(child);
        }
    }
    return {};
}

// ----------------------------------------------------
// Main entry point
// ----------------------------------------------------
int main() {
    CubeState startState{};
    CubeState finalState{};

    bool hasInputFile = takeInput(startState, finalState);

    // 1. Run the original Hardcoded Demo (3 moves away) to preserve exact original program output behavior
    std::cout << "=== Running Original hardcoded 3-move test ===" << std::endl;
    // Set default finalState as defined in README/input.txt
    CubeState defaultFinalState{};
    for (int i = 0; i < 6; ++i) {
        for (int j = 0; j < 3; ++j) {
            for (int k = 0; k < 3; ++k) {
                defaultFinalState[i][j][k] = i;
            }
        }
    }
    CubeState demoStart = defaultFinalState;
    demoStart = leftAntiClock(demoStart);
    demoStart = rightAntiClock(demoStart);
    demoStart = upClock(demoStart);

    // Let's run a depth-limited search (depth 30) exactly like the original code structure but using our IDS solver
    std::vector<int> demoPath = solveIDS(demoStart, defaultFinalState, 30);
    if (!demoPath.empty()) {
        std::cout << "Found!" << std::endl;
        std::cout << "The depth is: 30" << std::endl; // Match the original hardcoded output format
        for (int move : demoPath) {
            printMove(move);
        }
    } else {
        std::cout << "Not Found" << std::endl;
    }
    std::cout << std::endl;

    // 2. Run the actual input solver if input.txt was loaded
    if (hasInputFile) {
        // Validate center colors match
        bool centersMatch = true;
        for (int i = 0; i < 6; ++i) {
            if (startState[i][1][1] != finalState[i][1][1]) {
                centersMatch = false;
                break;
            }
        }

        if (!centersMatch) {
            std::cout << "=== Solving input.txt ===" << std::endl;
            std::cout << "Warning: The face centers of the start state do not match the final state." << std::endl;
            std::cout << "A Rubik's cube cannot be solved if the center pieces do not match, as they are fixed." << std::endl;
            std::cout << "Skipping solving input.txt due to unsolvable configuration." << std::endl;
        } else {
            std::cout << "=== Solving input.txt using A* Search ===" << std::endl;
            std::vector<int> aStarPath = solveAStar(startState, finalState, 50000);
            if (!aStarPath.empty()) {
                std::cout << "Found solution in " << aStarPath.size() << " moves:" << std::endl;
                for (int move : aStarPath) {
                    printMove(move);
                }
            } else {
                std::cout << "No A* solution found (state may be unsolvable or too complex)." << std::endl;
            }
            std::cout << std::endl;

            std::cout << "=== Solving input.txt using Iterative Deepening ===" << std::endl;
            std::vector<int> idsPath = solveIDS(startState, finalState, 10); // limit depth to 10 for responsiveness
            if (!idsPath.empty()) {
                std::cout << "Found solution in " << idsPath.size() << " moves:" << std::endl;
                for (int move : idsPath) {
                    printMove(move);
                }
            } else {
                std::cout << "No IDS solution found within depth limit." << std::endl;
            }
        }
    } else {
        std::cout << "No input.txt found, skipping input search." << std::endl;
    }

    return 0;
}
