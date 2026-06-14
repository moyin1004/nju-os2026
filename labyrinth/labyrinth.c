#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <testkit.h>
#include "labyrinth.h"

int main(int argc, char *argv[]) {
    Labyrinth labyrinth;
    memset(&labyrinth, 0, sizeof(Labyrinth));
    char *mapFilename = NULL;
    char player_id = -1;
    char *move_direction = NULL;
    bool is_print_version = false;
    bool has_other_argument = false;
    for (int i = 1; i < argc; i++) {
        // printf("%s \n", argv[i]);
        if (strcmp(argv[i], "--map") == 0 || strcmp(argv[i], "-m") == 0) {
            mapFilename = argv[i + 1];
            has_other_argument = true;
        }
        else if (strcmp(argv[i], "--player") == 0 || strcmp(argv[i], "-p") == 0) {
            if (strlen(argv[i + 1]) != 1) {
                return INVALID_ARGUMENT;
            }
            player_id = *argv[i + 1];
            has_other_argument = true;
        }
        else if (strcmp(argv[i], "--move") == 0) {
            move_direction = argv[i + 1];
            has_other_argument = true;
        } else if (strcmp(argv[i], "--version") == 0) {
            is_print_version = true;
        } else if (is_print_version) {
            has_other_argument = true;
        }
        // printf("%d\n", strcmp(argv[i], "--version"));
    }
    // printf("is_print_version %d\n", is_print_version);
    if (is_print_version) {
        if (has_other_argument) {
            return INVALID_ARGUMENT;
        } else {
            printf("Labyrinth Game 1.0\n");
            return 0;
        }
    }
    if (!loadMap(&labyrinth, mapFilename)) {
        return INVALID_MAP_FILE;
    }
    if (!isValidPlayer(player_id)) {
        return INVALID_PLAYER_ID;
    }
    if (move_direction == NULL) {
        for (int i = 0; i < labyrinth.rows; i++) {
            for (int j = 0; j < labyrinth.cols; j++) {
                printf("%c", labyrinth.map[i][j]);
            }
            printf("\n");
        }
    } else {
        if (!movePlayer(&labyrinth, player_id, move_direction)) {
            return INVALID_MOVE_DIRECTION;
        }
        if (!saveMap(&labyrinth, mapFilename)) {
            return INVALID_MAP_FILE;
        }
    }
    return 0;
}

void printUsage() {
    printf("Usage:\n");
    printf("  labyrinth --map map.txt --player id\n");
    printf("  labyrinth -m map.txt -p id\n");
    printf("  labyrinth --map map.txt --player id --move direction\n");
    printf("  labyrinth --version\n");
}

bool isValidPlayer(char playerId) {
    return playerId >= '0' && playerId <= '9';
}

bool loadMap(Labyrinth *labyrinth, const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        return false;
    }

    char line[MAX_COLS + 2];
    int row = 0;
    int cols = -1;

    while (fgets(line, sizeof(line), fp) != NULL && row < MAX_ROWS) {
        int len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[--len] = '\0';
        }
        if (len == 0) {
            continue;
        }

        if (cols == -1) {
            cols = len;
        } else if (len != cols) {
            fclose(fp);
            return false;
        }

        if (len > MAX_COLS) {
            fclose(fp);
            return false;
        }

        for (int col = 0; col < len; col++) {
            char c = line[col];
            if (c != '#' && c != '.' && !isValidPlayer(c)) {
                fclose(fp);
                return false;
            }
            labyrinth->map[row][col] = c;
        }
        row++;
    }

    fclose(fp);

    if (row == 0 || cols == -1) {
        return false;
    }

    labyrinth->rows = row;
    labyrinth->cols = cols;

    if (!isConnected(labyrinth)) {
        printf("Disconnected maze\n");
        return false;
    }

    return true;
}

Position findPlayer(Labyrinth *labyrinth, char playerId) {
    Position pos = {-1, -1};
    for (int i = 0; i < labyrinth->rows; i++) {
        for (int j = 0; j < labyrinth->cols; j++) {
            if (labyrinth->map[i][j] == playerId) {
                pos.row = i;
                pos.col = j;
                return pos;
            }
        }
    }
    return pos;
}

Position findFirstEmptySpace(Labyrinth *labyrinth) {
    Position pos = {-1, -1};
    for (int i = 0; i < labyrinth->rows; i++) {
        for (int j = 0; j < labyrinth->cols; j++) {
            if (isEmptySpace(labyrinth, i, j)) {
                pos.row = i;
                pos.col = j;
                return pos;
            }
        }
    }
    return pos;
}

bool isEmptySpace(Labyrinth *labyrinth, int row, int col) {
    if (row < 0 || row >= labyrinth->rows || col < 0 || col >= labyrinth->cols) {
        return false;
    }
    return labyrinth->map[row][col] == '.';
}

bool movePlayer(Labyrinth *labyrinth, char playerId, const char *direction) {
    printf("movePlayer %c %s\n", playerId, direction);
    Position pos = findPlayer(labyrinth, playerId);
    if (pos.row == -1 || pos.col == -1) {
        pos = findFirstEmptySpace(labyrinth);
        labyrinth->map[pos.row][pos.col] = playerId;
        if (pos.row == -1 || pos.col == -1) {
            return false;
        }
    }
    if (strcmp(direction, "up") == 0) {
        if (pos.row > 0 && isEmptySpace(labyrinth, pos.row - 1, pos.col)) {
            labyrinth->map[pos.row][pos.col] = '.';
            labyrinth->map[pos.row - 1][pos.col] = playerId;
            return true;
        }
    }
    if (strcmp(direction, "down") == 0) {
        if (pos.row < labyrinth->rows - 1 && isEmptySpace(labyrinth, pos.row + 1, pos.col)) {
            labyrinth->map[pos.row][pos.col] = '.';
            labyrinth->map[pos.row + 1][pos.col] = playerId;
            return true;
        }
    }
    if (strcmp(direction, "left") == 0) {
        if (pos.col > 0 && isEmptySpace(labyrinth, pos.row, pos.col - 1)) {
            labyrinth->map[pos.row][pos.col] = '.';
            labyrinth->map[pos.row][pos.col - 1] = playerId;
            return true;
        }
    }
    if (strcmp(direction, "right") == 0) {
        if (pos.col < labyrinth->cols - 1 && isEmptySpace(labyrinth, pos.row, pos.col + 1)) {
            labyrinth->map[pos.row][pos.col] = '.';
            labyrinth->map[pos.row][pos.col + 1] = playerId;
            return true;
        }
    }
    return false;
}

bool saveMap(Labyrinth *labyrinth, const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        return false;
    }
    for (int i = 0; i < labyrinth->rows; i++) {
        for (int j = 0; j < labyrinth->cols; j++) {
            fprintf(fp, "%c", labyrinth->map[i][j]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
    return true;
}

// Check if all empty spaces are connected using DFS
void dfs(Labyrinth *labyrinth, int row, int col, bool visited[MAX_ROWS][MAX_COLS]) {
    if (row < 0 || row >= labyrinth->rows || col < 0 || col >= labyrinth->cols) {
        return ;
    }
    // 这里玩家也算联通
    if (!isEmptySpace(labyrinth, row, col) && !isValidPlayer(labyrinth->map[row][col])) {
        return ;
    }
    if (visited[row][col]) {
        return ;
    }
    visited[row][col] = true;
    dfs(labyrinth, row - 1, col, visited);
    dfs(labyrinth, row + 1, col, visited);
    dfs(labyrinth, row, col - 1, visited);
    dfs(labyrinth, row, col + 1, visited);
}

bool isConnected(Labyrinth *labyrinth) {
    bool visited[MAX_ROWS][MAX_COLS] = {false};
    Position start = findFirstEmptySpace(labyrinth);
    if (start.row != -1) {
        dfs(labyrinth, start.row, start.col, visited);
    }
    for (int i = 0; i < labyrinth->rows; i++) {
        for (int j = 0; j < labyrinth->cols; j++) {
            if (isEmptySpace(labyrinth, i, j) && !visited[i][j]) {
                return false;
            }
        }
    }
    return true;
}
