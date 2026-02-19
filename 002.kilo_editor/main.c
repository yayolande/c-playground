#include "strings.h"
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#define CTRL_KEY(k) ((k) & 0x1F)

typedef struct {
  bool quit;
  struct {
    int rows;
    int cols;
  } term_size;
  struct termios original_state;
} EditorState;

EditorState newEditorState() {
  EditorState state = {.quit = false};
  return state;
}

void die(const char *);
EditorState editorConfig;

void disableRawMode() {
  int err = tcsetattr(STDIN_FILENO, TCSAFLUSH, &editorConfig.original_state);
  if (err == -1)
    die("unable to disable Raw Mode");
}

void enableRawMode(EditorState *editor) {
  struct termios raw;

  int err_get = tcgetattr(STDIN_FILENO, &raw);
  if (err_get == -1)
    die("unable to obtain Terminal Attribute 'termios' struct");

  editor->original_state = raw;

  raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  int err_set = tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
  if (err_set == -1)
    die("unable to enable Raw Mode");

  atexit(disableRawMode);
}

void printToScreen(char character) {
  if (iscntrl(character)) {
    printf("control char = {%d}\r\n", character);
  } else {
    printf("%d (%c)\r\n", character, character);
  }
}

void printToScreenBuffer(const char *buffer, int size) {
  for (int i = 0; i < size; i++) {
    write(STDOUT_FILENO, buffer + i, 1);
  }
}

void editorDrawRows(int rows) {
  for (int y = 0; y < rows; y++) {
    write(STDOUT_FILENO, "~", 1);

    if (y < rows - 1)
      write(STDOUT_FILENO, "\r\n", 2);
  }
}

void editorDrawRowsToBuffer(int rows, String *buffer) {
  for (int y = 0; y < rows; y++) {
    strings.append(buffer, "~");
    strings.append(buffer, "\x1b[K"); // clear till the end of current line

    if (y < rows - 1)
      strings.append(buffer, "\r\n");
  }
}

void editorRefreshScreen(EditorState *editor) {
  String buffer;
  strings.init(&buffer);

  strings.append(&buffer, "\x1b[?25l");
  // strings.append(&buffer, "\x1b[2J"); // clear terminal
  // const char *term_refresh_command = "\x1b[2J";
  // write(STDOUT_FILENO, term_refresh_command, 4);

  editorDrawRowsToBuffer(editor->term_size.rows, &buffer);
  // editorDrawRows(editor->term_size.rows);

  strings.append(&buffer, "\x1b[H"); // place cursor at the top right
  strings.append(&buffer, "\x1b[?25h");
  // write(STDOUT_FILENO, "\x1b[H", 3);

  write(STDOUT_FILENO, buffer.data, buffer.len);

  strings.free(&buffer);
}

char editorReadKeyPress() {
  char character = '\0';

  do {
    ssize_t nread = read(STDIN_FILENO, &character, 1);

    if (nread == 1)
      break;

    if (nread == -1 && errno != EAGAIN) {
      die("unable to read user input");
    }
  } while (1);

  return character;
}

void editorProcessKeyPressed(char character, EditorState *editor) {
  switch (character) {
  case CTRL_KEY('q'):
  case 'q':
    editor->quit = true;
    break;
  case 'k':
    write(STDOUT_FILENO, "\x1b[1A", 4);
    break;

  case 'j':
    write(STDOUT_FILENO, "\x1b[1B", 4);
    break;

  case 'h':
    write(STDOUT_FILENO, "\x1b[1D", 4);
    break;

  case 'l':
    write(STDOUT_FILENO, "\x1b[1C", 4);
    break;

  default:
    break;
  }
}

int getCursorPosition(int *row, int *col) {
  int counter = 0;
  char buffer[20];
  char character;

  if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4)
    return -1;

  while (read(STDIN_FILENO, &character, 1) == 1) {
    if (counter + 1 >= (int)sizeof(buffer))
      break;
    else if (character == 'R')
      break;

    buffer[counter] = character;
    counter++;
  }

  buffer[counter] = '\0';
  if (buffer[0] != '\x1b' || buffer[1] != '[')
    return -1;
  if (sscanf(&buffer[2], "%d;%d", row, col) != 2)
    return -1;

  return 0;
}

int getTerminalSize(int *row, int *col) {
  struct winsize ws;

  int err = ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
  if (err == -1 || ws.ws_col == 0) {
    int nwritten = write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12);
    if (nwritten != 12)
      return -1;

    return getCursorPosition(row, col);
  }

  *row = ws.ws_row;
  *col = ws.ws_col;

  return 0;
}

int main() {

  editorConfig = newEditorState();
  EditorState *editor = &editorConfig;

  enableRawMode(editor);

  if (getTerminalSize(&editor->term_size.rows, &editor->term_size.cols) == -1)
    die("unable to get the terminal size");

  editorRefreshScreen(editor);

  char string[100];
  int counter = 0;
  char character = '\0';

  while (true) {
    character = editorReadKeyPress();

    editorRefreshScreen(editor);

    printToScreen(character);
    printToScreenBuffer(string, counter);

    editorProcessKeyPressed(character, editor);

    if (counter < 100)
      string[counter] = character;

    counter++;

    if (editor->quit)
      break;
  }

  const char *term_refresh_command = "\x1b[2J";
  write(STDOUT_FILENO, term_refresh_command, 4);
  write(STDOUT_FILENO, "\x1b[H", 3);

  printf("\r\n=================\r\n Final string = %s\r\n=================\r\n",
         string);

  return 0;
}

int oldMain() {
  char string[100];
  char character;
  // ssize_t bytes_read;

  // enableRawMode();

  int counter = 0;
  do {
    character = '\0';
    // bytes_read = read(STDIN_FILENO, &character, 1);

    int err = read(STDIN_FILENO, &character, 1);
    if (err == -1 && errno != EAGAIN)
      die("unable to read user input");

    printToScreen(character);

    if ((character >= 'a' && character <= 'z') ||
        (character >= 'A' && character <= 'Z') || (character == ' ')) {
      // write(STDOUT_FILENO, &character, 1);
      // write(STDIN_FILENO, &character, 1);

      if (counter >= 100)
        break;

      string[counter] = character;
      counter++;
    }

    if (character == 'p')
      break;

    char quit_sequence = CTRL_KEY('q');
    if (character == quit_sequence)
      break;

  } while (1);

  printf("final string = %s\n", string);

  return 0;
}

void die(const char *character) {
  const char *term_refresh_command = "\x1b[2J";
  write(STDOUT_FILENO, term_refresh_command, 4);
  write(STDOUT_FILENO, "\x1b[H", 3);

  perror(character);
  exit(1);
}
