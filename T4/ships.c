typedef struct ship{
    char* name;
    int length;
    int main_pos_x;
    int main_pos_y;
    int is_vertical;
    char board_id;
    int external_id;
    int list_index;
} Ship;

Ship* carrier_init(int is_vertical, int list_index){
    Ship* al = malloc(sizeof(Ship));
    al -> name = malloc(strlen("Carrier")+1);
    strcpy(al -> name, "Carrier");
    al -> length = 5;
    al -> is_vertical = is_vertical;
    al -> board_id = 'C';
    al -> list_index = list_index;
    al -> external_id = 1;

    return al;
}

Ship* battleship_init(int is_vertical, int list_index){
    Ship* al = malloc(sizeof(Ship));
    al -> name = malloc(strlen("Battleship")+1);
    strcpy(al -> name, "Battleship");
    al -> length = 4;
    al -> is_vertical = is_vertical;
    al -> board_id = 'B';
    al -> list_index = list_index;
    al -> external_id = 2;

    return al;
}

Ship* cruiser_init(int is_vertical, int list_index){
    Ship* al = malloc(sizeof(Ship));
    al -> name = malloc(strlen("Cruiser")+1);
    strcpy(al -> name, "Cruiser");
    al -> length = 3;
    al -> is_vertical = is_vertical;
    al -> board_id = 'c';
    al -> list_index = list_index;
    al -> external_id = 3;

    return al;
}

Ship* submarine_init(int is_vertical, int list_index){
    Ship* al = malloc(sizeof(Ship));
    al -> name = malloc(strlen("Submarine")+1);
    strcpy(al -> name, "Submarine");
    al -> length = 3;
    al -> is_vertical = is_vertical;
    al -> board_id = 'S';
    al -> list_index = list_index;
    al -> external_id = 4;

    return al;
}

Ship* destroyer_init(int is_vertical, int list_index){
    Ship* al = malloc(sizeof(Ship));
    al -> name = malloc(strlen("Destroyer")+1);
    strcpy(al -> name, "Destroyer");
    al -> length = 2;
    al -> is_vertical = is_vertical;
    al -> board_id = 'D';
    al -> list_index = list_index;
    al -> external_id = 5;

    return al;
}

void destroy_ship(Ship* ship){
    free(ship -> name);
    free(ship);
}

typedef struct cell{
    char board_id;
    int list_index;
    int hurt;
} Cell;

Cell* cell_init(){
    Cell* al = malloc(sizeof(Cell));
    al -> board_id = ' ';
    al -> list_index = -1;
    al -> hurt = -1;
    return al;
}

void place_ship(Cell*** grid, Ship* ship, int x, int y){
    // Returns 0 if placed correctly, and -1 if there was another ship
    if (ship->is_vertical)
    {
        for (int i = 0; i < ship->length; ++i)
        {
            grid[y+i][x] -> board_id = ship->board_id;
            grid[y+i][x] -> list_index = ship->list_index;
            grid[y+i][x] -> hurt = 0;
        }
    }
    else
    {
        for (int i = 0; i < ship->length; ++i)
        {
            grid[y][x+i] -> board_id = ship->board_id;
            grid[y][x+i] -> list_index = ship->list_index;
            grid[y][x+i] -> hurt = 0;
        }
    }
    ship -> main_pos_x = x;
    ship -> main_pos_y = y;
}

int is_alive(Ship* ship, Cell*** grid)
{
    int alive = 0;
    if (ship->is_vertical)
    {
        for (int i = 0; i < ship->length; ++i)
        {
            if (grid[ship->main_pos_y+i][ship->main_pos_x] -> hurt == 0)
            {
                alive = 1;
            }
        }
    }
    else
    {
        for (int i = 0; i < ship->length; ++i)
        {
            if (grid[ship->main_pos_y][ship->main_pos_x+i] -> hurt == 0)
            {
                alive = 1;
            }
        }
    }
    return alive;
}

int is_occupied(Ship* ship, Cell*** grid, int x, int y)
{
    int occupied = 0;
    if (ship->is_vertical)
    {
        for (int i = 0; i < ship->length; ++i)
        {
            if (grid[y+i][x] -> list_index != -1)
            {
                occupied = 1;
            }
        }
    }
    else
    {
        for (int i = 0; i < ship->length; ++i)
        {
            if (grid[y][x+i] -> list_index != -1)
            {
                occupied = 1;
            }
        }
    }
    if (occupied)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int receive_attack(Cell*** grid,Ship** my_ships, int x, int y)
{
    // Hurt corresponding ship tile if there is a ship there
    // if not, ignore it. Returns 1 if destroyed a ship,
    // else returns 0
    if (grid[y][x] -> list_index != -1)
    {
        grid[y][x] -> hurt = 1;
        grid[y][x] -> board_id = 'x';
        if (is_alive(my_ships[grid[y][x]->list_index], grid))
        {
            return 0;
        }
        else
        {
            return 1;
        }
    }
    else
    {
        grid[y][x] -> board_id = 'x';
        return 0;
    }
}

Ship* choose_ship(int list_index){
    printf("Select ship %d of 3:\n", list_index);
    printf("[1] Carrier -> C C C C C\n");
    printf("[2] Battleship -> B B B B\n");
    printf("[3] Cruiser -> c c c\n");
    printf("[4] Submarine -> S S S\n");
    printf("[5] Destroyer -> D D\n");

    char tmp[30];
    int selected;
    fgets(tmp, 30, stdin);
    sscanf(tmp, "%d",&selected);

    int incorrect = 0;
    if (selected < 1 || selected > 5)
    {
        incorrect = 1;
        printf("Invalid selection. Try again.\n");
    }
    while (incorrect)
    {
        incorrect = 0;
        fgets(tmp, 30, stdin);
        sscanf(tmp, "%d\n",&selected);
        if (selected < 1 || selected > 5)
        {
            incorrect = 1;
            printf("Invalid selection. Try again.\n");
        }
    }

    printf("Do you want to place it vertically or horizontally? Vertically: 1, Horizontally: 0\n");

    int orientation_selected;
    fgets(tmp, 30, stdin);
    sscanf(tmp, "%d",&orientation_selected);

    incorrect = 0;
    if (orientation_selected != 1 && orientation_selected != 0)
    {
        incorrect = 1;
        printf("Invalid selection. Try again.\n");
    }
    while (incorrect)
    {
        incorrect = 0;
        fgets(tmp, 30, stdin);
        sscanf(tmp, "%d",&orientation_selected);
        if (orientation_selected != 1 && orientation_selected != 0)
        {
            incorrect = 1;
            printf("Invalid selection. Try again.\n");
        }
    }

    if (selected == 1){
        return carrier_init(orientation_selected, list_index);
    }
    else if (selected == 2)
    {
        return battleship_init(orientation_selected, list_index);
    }
    else if (selected == 3)
    {
        return cruiser_init(orientation_selected, list_index);
    }
    else if (selected == 4)
    {
        return submarine_init(orientation_selected, list_index);
    }
    else
    {
        return destroyer_init(orientation_selected, list_index);
    }
}

void choose_position(Ship* ship, Cell*** grid, int* out_x, int* out_y)
{
    printf("Choose %s position (x, y) in format: 'x y'.\n", ship->name);
    printf("NOTE: If ship is horizontally oriented, (x, y) will be position of leftmost tile.");
    printf("If it is vertically oriented, (x, y) will be position of upmost tile\n");
    char tmp[30];
    int x,y;
    fgets(tmp, 30, stdin);
    sscanf(tmp, "%d %d",&x, &y);

    int x_axis_bound = 9;
    int y_axis_bound = 9;

    if (ship->is_vertical)
    {
        y_axis_bound = 10 - ship->length;
    }
    else
    {
        x_axis_bound = 10 - ship->length;
    }


    int incorrect = 0;
    if (x < 0 || y < 0 || x > x_axis_bound || y > y_axis_bound)
    {
        incorrect = 1;
        printf("Position out of bounds.(0 <= x <= %d) and (0 <= y <= %d).Try again.\n", x_axis_bound, y_axis_bound);
    }
    else if (is_occupied(ship, grid, x, y))
    {
        incorrect = 1;
        printf("This position overlaps with another ship. Try again.");
    }
    while (incorrect)
    {
        incorrect = 0;
        fgets(tmp, 30, stdin);
        sscanf(tmp, "%d %d",&x, &y);

        if (x < 0 || y < 0 || x > x_axis_bound || y > y_axis_bound)
        {
            incorrect = 1;
            printf("Position out of bounds.(0 <= x <= %d) and (0 <= y <= %d).Try again.\n", x_axis_bound, y_axis_bound);
        }
        else if (is_occupied(ship, grid, x, y))
        {
            incorrect = 1;
            printf("This position overlaps with another ship. Try again.\n");
        }
    }

    place_ship(grid, ship, x, y);
    *out_x = x;
    *out_y = y;
}

void print_grid(Cell*** grid)
{
    printf("|   | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |\n");
    for (int row = 0; row < 10; ++row)
    {
        printf("| %d | %c | %c | %c | %c | %c | %c | %c | %c | %c | %c |\n",
                row, grid[row][0]->board_id, grid[row][1]->board_id, grid[row][2]->board_id,
                grid[row][3]->board_id, grid[row][4]->board_id, grid[row][5]->board_id,
                grid[row][6]->board_id, grid[row][7]->board_id, grid[row][8]->board_id,
                grid[row][9]->board_id);
    }
}

void print_both_grids(Cell*** grid, char** enemy_grid)
{
    for (int i = 0; i < 2; ++i)
    {
        printf("\n");
    }
    printf("Current State:\n");
    printf("|                Your Board                 | \t\t |                   Enemy Board             |\n");
    printf("|   | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | \t\t |   | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |\n");
    for (int row = 0; row < 10; ++row)
    {
        printf("| %d | %c | %c | %c | %c | %c | %c | %c | %c | %c | %c | \t\t | %d | %c | %c | %c | %c | %c | %c | %c | %c | %c | %c |\n",
                row, grid[row][0]->board_id, grid[row][1]->board_id, grid[row][2]->board_id,
                grid[row][3]->board_id, grid[row][4]->board_id, grid[row][5]->board_id,
                grid[row][6]->board_id, grid[row][7]->board_id, grid[row][8]->board_id,
                grid[row][9]->board_id, row, enemy_grid[row][0],enemy_grid[row][1],enemy_grid[row][2],
                enemy_grid[row][3],enemy_grid[row][4],enemy_grid[row][5],enemy_grid[row][6],
                enemy_grid[row][7],enemy_grid[row][8],enemy_grid[row][9]);
    }
}

void mark_sent_attack(char** enemy_grid, int x, int y)
{
    enemy_grid[y][x] = 'x';
}