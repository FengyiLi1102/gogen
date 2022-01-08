#include <iostream>
#include <fstream>
#include <cassert>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <cmath>

#include "common.h"
#include "mask.h"
#include "gogen.h"

using namespace std;

/* You are pre-supplied with the functions below. Add your own 
   function definitions to the end of this file. */

/* internal helper function which allocates a dynamic 2D array */
char **allocate_2D_array(int rows, int columns) {
  char **m = new char *[rows];
  assert(m);
  for (int r=0; r<rows; r++) {
    m[r] = new char[columns];
    assert(m[r]);
  }
  return m;
}

/* internal helper function which deallocates a dynamic 2D array */
void deallocate_2D_array(char **m, int rows) {
  for (int r=0; r<rows; r++)
    delete [] m[r];
  delete [] m;
}

/* internal helper function which removes unprintable characters like carriage returns and newlines from strings */
void filter (char *line) {
  while (*line) {
    if (!isprint(*line))
     *line = '\0';
    line++;
  }
}

/* loads a Gogen board from a file */
char **load_board(const char *filename) {
  char **board = allocate_2D_array(5, 6);
  ifstream input(filename);
  assert(input);
  char buffer[512];
  int lines = 0;
  input.getline(buffer, 512);
  while (input && lines < HEIGHT) {
    filter(buffer);
    if (strlen(buffer) != WIDTH)
      cout << "WARNING bad input = [" << buffer << "]" << endl;
    assert(strlen(buffer) == WIDTH);
    strcpy(board[lines], buffer);
    input.getline(buffer, 512);
    lines++;
  }
  input.close();
  return board;
}

/* saves a Gogen board to a file */
bool save_board(char **board, const char *filename) {
  ofstream out(filename); 
  if (!out)
    return false;
  for (int r=0; r<HEIGHT; r++) {
    for (int c=0; c<WIDTH; c++) {
      out << board[r][c];
    }
    out << endl;
  }
  bool result = out.good();
  out.close();
  return result;
}

/* internal helper function for counting number of words in a file */
int count_words(const char *filename) {
  char word[512];
  int count = 0;
  ifstream in(filename);
  while (in >> word)
    count++;
  in.close();
  return count;
}

/* loads a word list from a file into a NULL-terminated array of char *'s */
char **load_words(const char *filename) {
  int count = count_words(filename);
  ifstream in(filename);
  assert(in);
  int n=0;
  char **buffer = new char *[count+1]; // +1 because we NULL terminate 
  char word[512];
  for (; (in >> word) && n<count; n++) {
    buffer[n] = new char[strlen(word) + 1];
    strcpy(buffer[n], word);
  }
  buffer[n] = NULL;
  in.close();
  return buffer;
}

/* prints a Gogen board in appropriate format */
void print_board(char **board) {
  for (int r=0; r<HEIGHT; r++) {
    for (int c=0; c<WIDTH; c++) {
      cout << "[" << board[r][c] << "]";
      if (c < WIDTH-1)
	cout << "--";
    }
    cout <<endl;
    if (r < HEIGHT-1) {
      cout << " | \\/ | \\/ | \\/ | \\/ |" << endl;
      cout << " | /\\ | /\\ | /\\ | /\\ |" << endl;
    }
  }
}

/* prints a NULL-terminated list of words */
void print_words(char **words) {
  for (int n=0; words[n]; n++) 
    cout << words[n] << endl;
}

/* frees up the memory allocated in load_board(...) */
void delete_board(char **board) {
  deallocate_2D_array(board, HEIGHT);
}

/* frees up the memory allocated in load_words(...) */
void delete_words(char **words) {
  int count = 0;
  for (; words[count]; count++);
  deallocate_2D_array(words, count);
}

/* add your own function definitions here */
bool get_position(char **board, char ch, int &row, int &column) {
    for (auto thisRow = 0; thisRow < HEIGHT; thisRow++) {
        for (auto thisCol = 0; thisCol < WIDTH; thisCol++) {
            if (board[thisRow][thisCol] == ch) {
                row = thisRow;
                column = thisCol;
                return true;
            }
        }
    }

    row = -1;
    column = -1;
    return false;
}

bool valid_solution(char **board, char **words) {
    for (auto row = 0; words[row] != NULL; row++) {
        int thisRow = -1, thisCol = -1;
        bool success = get_position(board, words[row][0], thisRow, thisCol);

        if (!success) {
            cerr << "The first character cannot find in the board.";
            return false;
        }

        for (auto col = 0;; col++) {
            if (!isalpha(words[row][col+1])) break;

            int nextRow = -1, nextCol = -1;
            success = get_position(board, words[row][col+1], nextRow, nextCol);

            if (success) {
                if (abs(thisRow - nextRow) <= 1 && abs(thisCol - nextCol) <= 1) {
                    thisRow = nextRow;
                    thisCol = nextCol;
                } else {
                    cerr << "Cannot reach the next character.";
                    return false;
                }
            } else {
                cerr << "Cannot find the next character in the board.";
                return false;
            }
        }
    }

    return true;
}


void update(char **board, char ch, Mask &mask) {
    int thisRow = -1, thisCol = -1;

    if (get_position(board, ch, thisRow, thisCol)) {
        mask.set_all(false);
        mask[thisRow][thisCol] = true;
        return;
    } else {
        for (auto row = 0; row < HEIGHT; ++row) {
            for (auto col = 0; col < WIDTH; ++col) {
                if (isalpha(board[row][col])) mask[row][col] = false;
            }
        }
    }

    if (mask.count() == 1) {
        bool success = mask.get_position(true, thisRow, thisCol);

        if (success) {
            board[thisRow][thisCol] = ch;
        }
    }
}


void neighbourhood_intersect(Mask &one, Mask &two) {
    Mask nbrOne = one.neighbourhood();
    Mask nbrTwo = two.neighbourhood();
    one.intersect_with(nbrTwo);
    two.intersect_with(nbrOne);
}


bool solve_board(char **board, char **words) {
    Mask myArray[NUM_MASK];
    return process(board, words, 0, 1, myArray);
}


bool process(char **board, char **words, int row, int col, Mask myArray[]) {
    if (words[row] == NULL) return process(board, words, 0, 1, myArray);

    int nextRow = row, nextCol = col + 1, lastRow = row, lastCol = col - 1;

    if (isalpha(words[nextRow][nextCol]) && isalpha(words[lastRow][lastCol])) {
        Mask lastMask = &myArray[static_cast<int>(words[lastRow][lastCol] - 'A')];
        update(board, words[lastRow][lastCol], lastMask);

        Mask midMask = &myArray[static_cast<int>(words[row][col] - 'A')];
        update(board, words[row][col], midMask);

        Mask nextMask = &myArray[static_cast<int>(words[nextRow][nextCol] - 'A')];
        update(board, words[nextRow][nextCol], nextMask);

        neighbourhood_intersect(lastMask, midMask);
        neighbourhood_intersect(midMask, nextMask);

        update(board, words[row][col], midMask);
        printf("%c\n", words[row][col]);
        midMask.print();
        cout << "------------------" << endl;
        print_board(board);
    }

    int numTotalTrue = 0;

    for (auto maskIndex = 0; maskIndex < NUM_MASK; ++maskIndex) {
        numTotalTrue += myArray[maskIndex].count();
    }

    if (numTotalTrue == NUM_MASK) return true;

    col = ((isalpha(words[row][col+1])) ? col+1 : 1);
    row = ((col == 1) ? row+1 : row);

    return process(board, words, row, col ,myArray);
}