#include "state.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "snake_utils.h"

/* Helper function definitions */
static void set_board_at(game_state_t* state, unsigned int row, unsigned int col, char ch);
static bool is_tail(char c);
static bool is_head(char c);
static bool is_snake(char c);
static char body_to_tail(char c);
static char head_to_body(char c);
static unsigned int get_next_row(unsigned int cur_row, char c);
static unsigned int get_next_col(unsigned int cur_col, char c);
static void find_head(game_state_t* state, unsigned int snum);
static char next_square(game_state_t* state, unsigned int snum);
static void update_tail(game_state_t* state, unsigned int snum);
static void update_head(game_state_t* state, unsigned int snum);

/* Task 1 */
game_state_t* create_default_state() {
  // TODO: Implement this function.
  game_state_t *default_state = (game_state_t*)malloc(sizeof(game_state_t));
  default_state->num_snakes = 1;
  default_state->snakes = (snake_t *)malloc(sizeof(snake_t));
  default_state->snakes->tail_row = 2;
  default_state->snakes->tail_col = 2;
  default_state->snakes->head_row = 2;
  default_state->snakes->head_col = 4;
  default_state->snakes->live = true;

  default_state->num_rows = 18;
  default_state->board = (char**)malloc(sizeof(char*) * default_state->num_rows);
  for (unsigned int i = 0; i < default_state->num_rows; i++) {
    default_state->board[i] = (char*)malloc(sizeof(char) * (20));
    if (i == 17 || i == 0) {
        strcpy(default_state->board[i], "####################");
        continue;
    }
    if (i == 1) {
        strcpy(default_state->board[i], "#                  #");
        continue;
    }
    if (i == 2) {
        strcpy(default_state->board[i], "# d>D    *         #");
        continue;
    }
    strcpy(default_state->board[i], "#                  #");
  }
  return default_state;
}

/* Task 2 */
void free_state(game_state_t* state) {
  // TODO: Implement this function.
  for (unsigned int i = 0; i < state->num_rows; i++) {
    free(state->board[i]);
  }
  free(state->board);
  free(state->snakes);
  free(state);
  return;
}


/* Task 3 */
void print_board(game_state_t* state, FILE* fp) {
  // TODO: Implement this function.
  for (unsigned int i = 0; i < state->num_rows; i++) {
    fprintf(fp, "%s", state->board[i]);
  }
  return;
}


/*
  Saves the current state into filename. Does not modify the state object.
  (already implemented for you).
*/
void save_board(game_state_t* state, char* filename) {
  FILE* f = fopen(filename, "w");
  print_board(state, f);
  fclose(f);
}

/* Task 4.1 */

/*
  Helper function to get a character from the board
  (already implemented for you).
*/
char get_board_at(game_state_t* state, unsigned int row, unsigned int col) {
  return state->board[row][col];
}

/*
  Helper function to set a character on the board
  (already implemented for you).
*/
static void set_board_at(game_state_t* state, unsigned int row, unsigned int col, char ch) {
  state->board[row][col] = ch;
}

/*
  Returns true if c is part of the snake's tail.
  The snake consists of these characters: "wasd"
  Returns false otherwise.
*/
static bool is_tail(char c) {
  // TODO: Implement this function.
  if (c == 'w' || c == 'a' || c == 's' || c == 'd') {
      return true;
  } else {
      return false;
  }
}

/*
  Returns true if c is part of the snake's head.
  The snake consists of these characters: "WASDx"
  Returns false otherwise.
*/
static bool is_head(char c) {
  // TODO: Implement this function.
  if (c == 'W' || c == 'A' || c == 'S' || c == 'D' || c == 'x') {
      return true;
  } else {
      return false;
  }
}

/*
  Returns true if c is part of the snake.
  The snake consists of these characters: "wasd^<v>WASDx"
*/
static bool is_snake(char c) {
  // TODO: Implement this function.
  if (is_tail(c) || is_head(c) || c == '^' || c == '<' || c== '>' || c == 'v') {
      return true;
  } else {
      return false;
  }
}

/*
  Converts a character in the snake's body ("^<v>")
  to the matching character representing the snake's
  tail ("wasd").
*/
static char body_to_tail(char c) {
  // TODO: Implement this function.
  switch(c){
    case '^':
        return 'w';
    case '<':
        return 'a';
    case 'v':
        return 's';
    case '>':
        return 'd';
    default:
        return '?';
  }
}

/*
  Converts a character in the snake's head ("WASD")
  to the matching character representing the snake's
  body ("^<v>").
*/
static char head_to_body(char c) {
  // TODO: Implement this function.
  switch(c){
    case 'W':
        return '^';
    case 'A':
        return '<';
    case 'S':
        return 'v';
    case 'D':
        return '>';
    default:
        return '?';
  }
}

/*
  Returns cur_row + 1 if c is 'v' or 's' or 'S'.
  Returns cur_row - 1 if c is '^' or 'w' or 'W'.
  Returns cur_row otherwise.
*/
static unsigned int get_next_row(unsigned int cur_row, char c) {
  // TODO: Implement this function.
  if (c == 'v' || c == 's' || c == 'S') {
    return cur_row + 1;
  } else if (c == '^' || c == 'w' || c == 'W') {
    return cur_row - 1;
  } else {
    return cur_row;
  }
}

/*
  Returns cur_col + 1 if c is '>' or 'd' or 'D'.
  Returns cur_col - 1 if c is '<' or 'a' or 'A'.
  Returns cur_col otherwise.
*/
static unsigned int get_next_col(unsigned int cur_col, char c) {
  // TODO: Implement this function.
  if (c == '>' || c == 'd' || c == 'D') {
    return cur_col + 1;
  } else if (c == '<' || c == 'a' || c == 'A') {
    return cur_col - 1;
  } else {
    return cur_col;
  }
}

/*
  Task 4.2

  Helper function for update_state. Return the character in the cell the snake is moving into.

  This function should not modify anything.
*/
static char next_square(game_state_t* state, unsigned int snum) {
  // TODO: Implement this function.
  char curr = get_board_at(state, state->snakes[snum].head_row, state->snakes[snum].head_col);
  unsigned int nRow = get_next_row(state->snakes[snum].head_row, curr);
  unsigned int nCol = get_next_col(state->snakes[snum].head_col, curr);
  return get_board_at(state, nRow, nCol);
}

/*
  Task 4.3

  Helper function for update_state. Update the head...

  ...on the board: add a character where the snake is moving

  ...in the snake struct: update the row and col of the head

  Note that this function ignores food, walls, and snake bodies when moving the head.
*/
static void update_head(game_state_t* state, unsigned int snum) {
  // TODO: Implement this function.
  unsigned int cRow = state->snakes[snum].head_row;
  unsigned int cCol = state->snakes[snum].head_col;
  char curr = get_board_at(state, cRow, cCol);

  unsigned int nRow = get_next_row(state->snakes[snum].head_row, curr);
  unsigned int nCol = get_next_col(state->snakes[snum].head_col, curr);
  set_board_at(state, nRow, nCol, curr);
  set_board_at(state, cRow, cCol, head_to_body(curr));

  state->snakes[snum].head_row = nRow;
  state->snakes[snum].head_col = nCol;
  return;
}

/*
  Task 4.4

  Helper function for update_state. Update the tail...

  ...on the board: blank out the current tail, and change the new
  tail from a body character (^<v>) into a tail character (wasd)

  ...in the snake struct: update the row and col of the tail
*/
static void update_tail(game_state_t* state, unsigned int snum) {
  // TODO: Implement this function.
  unsigned int cRow = state->snakes[snum].tail_row;
  unsigned int cCol = state->snakes[snum].tail_col;
  char curr = get_board_at(state, cRow, cCol);

  unsigned int nRow = get_next_row(state->snakes[snum].tail_row, curr);
  unsigned int nCol = get_next_col(state->snakes[snum].tail_col, curr);

  set_board_at(state, nRow, nCol, body_to_tail(get_board_at(state, nRow, nCol)));
  set_board_at(state, cRow, cCol, ' ');

  state->snakes[snum].tail_row = nRow;
  state->snakes[snum].tail_col = nCol;
  return;
}

/* Task 4.5 */
void update_state(game_state_t* state, int (*add_food)(game_state_t* state)) {
  // TODO: Implement this function.
  for (unsigned int i = 0; i < state->num_snakes; i++) {
      char next = next_square(state, i);
      if (next == '#' || is_snake(next)) {
          state->snakes[i].live = false;
          set_board_at(state, state->snakes[i].head_row, state->snakes[i].head_col, 'x');
      } else if (next == '*') {
          update_head(state, i);
          add_food(state);
      } else {
          update_head(state, i);
          update_tail(state, i);
      }
  }
  return;
}

/* Task 5 */
game_state_t* load_board(FILE* fp) {
  // TODO: Implement this function.
  game_state_t* nstate = malloc(sizeof(game_state_t));
  unsigned int rows = 0, cols = 0;
  char c;
  while ((c = fgetc(fp)) != EOF) {
      if (rows == 0) {
          nstate->board = malloc(sizeof(char*));
          nstate->num_rows = 1;
          rows++;
      }

      if (c == '\n') {
        cols += 2;
        nstate->board[rows - 1] = realloc(nstate->board[rows - 1], cols * sizeof(char));
        nstate->board[rows - 1][cols - 2] = '\n';
        nstate->board[rows - 1][cols - 1] = '\0';
        rows++;
        nstate->num_rows++;
        nstate->board = realloc(nstate->board, sizeof(char*) * rows);
        cols = 0;
    } else {
      cols++;
      if (cols == 1) {
          nstate->board[rows - 1] = malloc(sizeof(char));
      } else {
          nstate->board[rows - 1] = realloc(nstate->board[rows - 1], sizeof(char) * cols);
      }
      nstate->board[rows - 1][cols - 1] = c;
    }
  }

  nstate->num_rows--;
  nstate->board = realloc(nstate->board, sizeof(char*) * nstate->num_rows);
  nstate->num_snakes = 0;
  nstate->snakes = NULL;
  return nstate;
}

/*
  Task 6.1

  Helper function for initialize_snakes.
  Given a snake struct with the tail row and col filled in,
  trace through the board to find the head row and col, and
  fill in the head row and col in the struct.
*/
static void find_head(game_state_t* state, unsigned int snum) {
  // TODO: Implement this function.
  unsigned int row = state->snakes[snum].tail_row;
  unsigned int col = state->snakes[snum].tail_col;
  char curr = get_board_at(state, row, col);

  while (!is_head(curr)) {
      row = get_next_row(row, curr);
      col = get_next_col(col, curr);
      curr = get_board_at(state, row, col);
  }

  state->snakes[snum].head_row = row;
  state->snakes[snum].head_col = col;
  return;
}

/* Task 6.2 */
game_state_t* initialize_snakes(game_state_t* state) {
  // TODO: Implement this function.
  unsigned int num = 0;

  for (unsigned int r = 0; r < state->num_rows; r++) {
      for (unsigned int c = 0; state->board[r][c]; c++) {
          if (is_tail(state->board[r][c])) {
              num++;
          }
      }
  }
  state->num_snakes = num;
  state->snakes = malloc(sizeof(snake_t) * num);
  unsigned int i = 0;
  for (unsigned int r = 0; r < state->num_rows; r++) {
      for (unsigned int c = 0; state->board[r][c]; c++) {
          if (is_tail(state->board[r][c])) {
              state->snakes[i].tail_col = c;
              state->snakes[i].tail_row = r;
              state->snakes[i].live = true;
              find_head(state, i);
              i++;
          }
      }
  }
  return state;
}

