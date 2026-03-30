#include <complex.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <raylib.h>

// structs
struct Tile{
    int posx;
    int posy;
    char colorid;
};
struct Tetromino{
    struct Tile tiles[4];
};

// definitions
int WIDTH = 800;
int HEIGHT = 600;
int CENTERX = 400; // WIDTH/2
int CENTERY = 300; // HEIGHT/2
int CELL_SIZE = 30; // HEIGHT/20

int SPEED = 40;

Color a = {94, 216, 125, 255}; // color 1
Color b = {72, 211, 176, 0xFF}; // color 2
Color c = {91, 184, 215, 0xFF}; // color 3
Color d = {49, 87, 202, 0xFF}; // color 4
Color e = {147, 121, 223, 0xFF}; // color 5
Color f = {168, 54, 206, 0xFF}; // color 6
Color g = {215, 90, 184, 0xFF}; // color 7

Color x = {0xD0, 0xD0, 0xD0, 0xFF}; // light gray
Color y = {0x80, 0x80, 0x80, 0xFF}; // gray
Color z = {0x10, 0x10, 0x10, 0xFF}; // dark gray

int frame_counter = 0;
int ret; // logic value
int removed = 0;


int rows_cleared = 0;
int level = 0;
int score = 0;

void draw_board(char (*board)[30][10]);

void draw_tile(struct Tile tile);
void move_tile(struct Tile* tile, char dir);
void write_tile(struct Tile tile, char (*board)[30][10]);

void draw_tetromino(struct Tetromino tetromino);
int move_tetromino(struct Tetromino* tetromino, char dir, char (*board)[30][10]);
void write_tetromino(struct Tetromino tetromino, char (*board)[30][10]);
struct Tetromino generate_tetromino();
void rotate_tetromino(struct Tetromino* tetromino, char dir, char (*board)[30][10]);
bool full_row(char (*board)[30][10], int row);
int clear_full_rows(char (*board)[30][10]);


int main() {

    InitWindow(WIDTH, HEIGHT, "tetris pico");
    SetTargetFPS(60);

    char (*board)[30][10] = malloc(sizeof *board);
    memset(board, ' ', sizeof *board);

    srand(time(NULL));

    struct Tetromino active_tetromino = generate_tetromino();

    while(!WindowShouldClose()) {
        // app logic
        float dt = GetFrameTime();
        frame_counter++;
        // here update all the tetris logic
        // INPUT
        char dir = ' ';
        char keycode = GetCharPressed();
        switch (keycode) {
            case 's': dir = 'd'; break;
            case 'a': dir = 'l'; break;
            case 'd': dir = 'r'; break;
            case 'q': dir = 'a'; break;
            case 'e': dir = 'c'; break;
            default: break;
        }
        //printf("keycode: %c\n", keycode);
        
        // FALLING LOGIC
        if (frame_counter % SPEED == 0)
            ret = move_tetromino(&active_tetromino, 'd', board);
        if (ret == 2) {
            write_tetromino(active_tetromino, board);
            active_tetromino = generate_tetromino();
            ret = 0;
        }
        // PLAYER INPUT LOGIC
        if (dir == 'd' || dir == 'r' || dir == 'l')
            ret = move_tetromino(&active_tetromino, dir, board);
        else if (dir == 'c' || dir == 'a') // rotate
            rotate_tetromino(&active_tetromino, dir, board);
        //ret?printf("ret: %d\n", ret):0;
        if (ret == 2) {
            write_tetromino(active_tetromino, board);
            active_tetromino = generate_tetromino();
            ret = 0;
        }

        // remove full rows
        removed = clear_full_rows(board);
        rows_cleared += removed;
        // update level
        if (rows_cleared >= 10) {
            level++;
            SPEED--;
            rows_cleared -= 10;
        }

        // update score
        if (removed == 1)
            score += 40 * (level + 1);
        else if (removed == 2)
            score += 100 * (level + 1);
        else if (removed == 3)
            score += 300 * (level + 1);
        else if (removed == 4)
            score += 1200 * (level + 1);
        removed = 0;
    
        
        // draw logic
        BeginDrawing();
        ClearBackground(BLACK);

        draw_board(board);

        DrawText(TextFormat("Score: %d", score), WIDTH * 0.75, HEIGHT *0.40, 30, WHITE);
        DrawText(TextFormat("Level: %d", level), WIDTH * 0.75, HEIGHT *0.50, 30, WHITE);
        
        draw_tetromino(active_tetromino);

        EndDrawing();
    }


    free(board);
    return 0;
}

void draw_board(char (*board)[30][10]){
    DrawRectangleLines(CENTERX - 5*CELL_SIZE -1, -1, 10*CELL_SIZE + 3, HEIGHT + 1, x);
    for (size_t y = 0; y < 20; y++) // draw just 20 rows
    {
        for (size_t x = 0; x < 10; x++) // ten cells in each row
        {
            draw_tile((struct Tile){x, y, (*board)[y][x]});
        }
        
    }
    
    return;
}

float DARKFACTOR = 0.60;
float MEDIUMFACTOR = 0.775;
int DIAMONDFACTOR = 7;
void draw_tile(struct Tile tile){
    int x = CENTERX - 5*CELL_SIZE + tile.posx*CELL_SIZE ;
    int y = HEIGHT - CELL_SIZE - tile.posy*CELL_SIZE;

    Color color;
    switch (tile.colorid)
            {
                case ' ': color = z; break;
                case 'a': color = a; break;
                case 'b': color = b; break;
                case 'c': color = c; break;
                case 'd': color = d; break;
                case 'e': color = e; break;
                case 'f': color = f; break;
                case 'g': color = g; break;
                default:  color = RED; // something went very wrong
            }

    if (color.r == z.r && color.g == z.g && color.b == z.b) {
        DrawRectangle(x, y, CELL_SIZE, CELL_SIZE, color);
        DrawRectangleLines(x, y, CELL_SIZE, CELL_SIZE, BLACK);
    } else {    
        Color colordark = {color.r * DARKFACTOR, color.g * DARKFACTOR, color.b * DARKFACTOR, 0xFF};
        Color colormedium = {color.r * MEDIUMFACTOR, color.g * MEDIUMFACTOR, color.b * MEDIUMFACTOR, 0xFF};
        DrawTriangle((Vector2){x, y}, (Vector2){x + CELL_SIZE, y+CELL_SIZE}, (Vector2){x + CELL_SIZE, y}, color); 
        DrawTriangle((Vector2){x, y}, (Vector2){x, y+CELL_SIZE}, (Vector2){x + CELL_SIZE, y + CELL_SIZE}, colordark); 
        DrawRectangle(x+DIAMONDFACTOR, y+DIAMONDFACTOR, CELL_SIZE-2*DIAMONDFACTOR, CELL_SIZE-2*DIAMONDFACTOR, colormedium);
        DrawRectangleLines(x, y, CELL_SIZE, CELL_SIZE, z);
    }
    
    
}

void move_tile(struct Tile *tile, char dir) {
    switch (dir) {
    case 'u':
        tile->posy = tile->posy +1; break;
    case 'd':
        tile->posy = tile->posy -1; break;   
    case 'r':
        tile->posx = tile->posx +1; break;
    case 'l':
        tile->posx = tile->posx -1; break;
    default: break;
    }
    return;
}

void write_tile(struct Tile tile, char (*board)[30][10]){
    (*board)[tile.posy][tile.posx] = tile.colorid;
}

void draw_tetromino(struct Tetromino tetromino){
    for (int i = 0; i < 4; i++)
        draw_tile(tetromino.tiles[i]);
}

int move_tetromino(struct Tetromino *tetromino, char dir, char (*board)[30][10]){
    // return if dir is invalid
    if (dir != 'd' && dir != 'r' && dir != 'l')
        return 1;

    // check if the tetromino is out of bounds
    for (int i = 0; i < 4; i++) {
        if (dir == 'd' && tetromino->tiles[i].posy == 0) // bottom of screen -> write to board
            return 2;
        else if ((dir == 'r' && tetromino->tiles[i].posx == 9) || (dir == 'l' && tetromino->tiles[i].posx == 0)) // right or left of screen -> out of bounds
            return 1;
    }

    // check if the tetromino is colliding with another tile
    for (int i = 0; i < 4; i++) {
        int newx = tetromino->tiles[i].posx;
        int newy = tetromino->tiles[i].posy - 1;

        // check if another tile of same piece is there
        int self = 0;
        for (int j = 0; j < 4; j++) {
            if (tetromino->tiles[j].posx == newx &&
                tetromino->tiles[j].posy == newy) {
                self = 1;
                break;
            }
        }

        if (!self && (*board)[newy][newx] != ' ')
            return 2;
    }
    // nothing failed, move the tetromino
    for (int i = 0; i < 4; i++)            
        move_tile(&tetromino->tiles[i], dir);

    return 0;
}

void write_tetromino(struct Tetromino tetromino, char (*board)[30][10]){
    for (int i = 0; i < 4; i++)
        write_tile(tetromino.tiles[i], board);
}

struct Tetromino generate_tetromino(){
    struct Tetromino tetromino;
    int r = rand() % 7;
    struct Tile start_tile = {5, 20, ' '}; // This is the initial position of the tetromino
    switch (r) {
        case 0: // T
            start_tile.colorid = 'a';
            tetromino.tiles[0] = start_tile;
            tetromino.tiles[1] = (struct Tile){start_tile.posx-1, start_tile.posy, 'a'};
            tetromino.tiles[2] = (struct Tile){start_tile.posx+1, start_tile.posy, 'a'};
            tetromino.tiles[3] = (struct Tile){start_tile.posx, start_tile.posy+1, 'a'};
            break;
        case 1: // J
            start_tile.colorid = 'b';
            tetromino.tiles[0] = start_tile;
            tetromino.tiles[1] = (struct Tile){start_tile.posx, start_tile.posy-1, 'b'};
            tetromino.tiles[2] = (struct Tile){start_tile.posx-1, start_tile.posy-1, 'b'};
            tetromino.tiles[3] = (struct Tile){start_tile.posx, start_tile.posy+1, 'b'};
            break;
        case 2: // L
            start_tile.colorid = 'c';
            tetromino.tiles[0] = start_tile;
            tetromino.tiles[1] = (struct Tile){start_tile.posx, start_tile.posy-1, 'c'};
            tetromino.tiles[2] = (struct Tile){start_tile.posx+1, start_tile.posy-1, 'c'};
            tetromino.tiles[3] = (struct Tile){start_tile.posx, start_tile.posy+1, 'c'};
            break;
        case 3: // I
            start_tile.colorid = 'd';
            tetromino.tiles[0] = start_tile;
            tetromino.tiles[1] = (struct Tile){start_tile.posx-1, start_tile.posy, 'd'};
            tetromino.tiles[2] = (struct Tile){start_tile.posx+1, start_tile.posy, 'd'};
            tetromino.tiles[3] = (struct Tile){start_tile.posx+2, start_tile.posy, 'd'};
            break;
        case 4: // O
            start_tile.colorid = 'e';
            tetromino.tiles[0] = start_tile;
            tetromino.tiles[1] = (struct Tile){start_tile.posx+1, start_tile.posy, 'e'};
            tetromino.tiles[2] = (struct Tile){start_tile.posx, start_tile.posy+1, 'e'};
            tetromino.tiles[3] = (struct Tile){start_tile.posx+1, start_tile.posy+1, 'e'};
            break;
        case 5: // Z
            start_tile.colorid = 'f';
            tetromino.tiles[0] = start_tile;
            tetromino.tiles[1] = (struct Tile){start_tile.posx+1, start_tile.posy, 'f'};
            tetromino.tiles[2] = (struct Tile){start_tile.posx, start_tile.posy+1, 'f'};
            tetromino.tiles[3] = (struct Tile){start_tile.posx-1, start_tile.posy+1, 'f'};
            break;
        case 6: // S
            start_tile.colorid = 'g';
            tetromino.tiles[0] = start_tile;
            tetromino.tiles[1] = (struct Tile){start_tile.posx+1, start_tile.posy, 'g'};
            tetromino.tiles[2] = (struct Tile){start_tile.posx, start_tile.posy-1, 'g'};
            tetromino.tiles[3] = (struct Tile){start_tile.posx-1, start_tile.posy-1, 'g'};
            break;
    }
    return tetromino;
}

void rotate_tetromino(struct Tetromino *tetromino, char dir, char (*board)[30][10]) {
    if (dir != 'c' && dir != 'a')
        return;

    int newx[4];
    int newy[4];

    // pivot = tiles[0]
    int px = tetromino->tiles[0].posx;
    int py = tetromino->tiles[0].posy;

    for (int i = 0; i < 4; i++) {
        int relx = tetromino->tiles[i].posx - px;
        int rely = tetromino->tiles[i].posy - py;

        int rx, ry;

        if (dir == 'c') {
            // clockwise: (x, y) -> (y, -x)
            rx = rely;
            ry = -relx;
        } else {
            // anticlockwise: (x, y) -> (-y, x)
            rx = -rely;
            ry = relx;
        }

        newx[i] = px + rx;
        newy[i] = py + ry;
    }

    // check bounds + collisions BEFORE applying
    for (int i = 0; i < 4; i++) {
        // out of bounds
        if (newx[i] < 0 || newx[i] > 9 || newy[i] < 0 || newy[i] > 29)
            return;

        // check collision with board
        if ((*board)[newy[i]][newx[i]] != ' ') {
            // allow overlap with itself
            int overlap = 0;
            for (int j = 0; j < 4; j++) {
                if (tetromino->tiles[j].posx == newx[i] &&
                    tetromino->tiles[j].posy == newy[i]) {
                    overlap = 1;
                    break;
                }
            }
            if (!overlap)
                return;
        }
    }

    // apply rotation
    for (int i = 0; i < 4; i++) {
        tetromino->tiles[i].posx = newx[i];
        tetromino->tiles[i].posy = newy[i];
    }

    return;
}

bool full_row(char (*board)[30][10], int row) {
    // returns an array of either 0 or 1 for each row whether it's full or not
    bool full = true;
    for (int i = 0; i < 10; i++) {
        if ((*board)[row][i] == ' ') {
            full = false;
            break;
        }
    }
    return full;
}

int clear_full_rows(char (*board)[30][10]) {
    // returns number of rows cleared
    int removed = 0;
    for (int l = 0; l < 4; l++) {
        for (int i = 0; i < 20; i++) {
            bool full = full_row(board, i);
            if (full) {
                removed++;
                for (int j = i; j < 21; j++) {
                    for (int k = 0; k < 10; k++) {
                        (*board)[j][k] = (*board)[j+1][k];
                    }
                }
            }
        }
    }
    

    return removed;
}