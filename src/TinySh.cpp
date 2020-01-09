/*
 * TinySh.cpp
 *
 */

#include "TinySh.h"

#include <assert.h>

namespace Shell
{

const CommandDescription TinySh::help_cmd_template = { "help", "display help", "<cr>", cmd_help, 0, 0, 0 };

TinySh::TinySh(void * container) :
      help_cmd(help_cmd_template), cur_buf_index(0), cur_context(0), cursorPos(0), echo(1), prompt("$ "), root_cmd(&help_cmd),
      cur_cmd_ctx(0), plusArg(0), containerPtr(container),
      cmdListIsWritable(true), ioStream(0)
{
  unsigned i;

  trash_buffer[0] = 0;
  context_buffer[0] = 0;
  for (i = 0; i < HISTORY_DEPTH; ++i)
  {
    input_buffers[i][0] = 0;
  }
}

//  TinySh::~TinySh()
//  {
//    // Auto-generated destructor stub
//  }

/* redefine some useful and maybe missing utilities to avoid conflicts */

/* few useful utilities that may be missing */

static int tinysh_strlen(const char *s)
{
  int i;
  for (i = 0; *s; s++, i++)
    ;
  return i;
}

/* callback for help function
 */
void TinySh::cmd_help(TinySh& shell, int argc, const char **argv)
{
  shell.io().writeBlock("?            display help on given or available commands\n");
  shell.io().writeBlock("<TAB>        auto-completion\n");
  shell.io().write(TOPCHAR);
  shell.io().writeBlock( "            return to command root (must be very first char on input line)\n");
  shell.io().writeBlock("<cr>         execute command line\n");
  shell.io().writeBlock("CTRL-P       recall previous input line\n");
  shell.io().writeBlock("CTRL-N       recall next input line\n");
  shell.io().writeBlock("<any>        treat as input character\n");
}

/*
 */

enum
{
  NULLMATCH, FULLMATCH, PARTMATCH, UNMATCH, MATCH, AMBIG
};

/* verify if the non-spaced part of s2 is included at the begining
 * of s1.
 * return FULLMATCH if s2 equal to s1, PARTMATCH if s1 starts with s2
 * but there are remaining chars in s1, UNMATCH if s1 does not start with
 * s2
 */
int strstart(const char *s1, const char *s2)
{
  while (*s1 && *s1 == *s2)
  {
    s1++;
    s2++;
  }

  if (*s2 == ' ' || *s2 == 0)
  {
    if (*s1 == 0)
      return FULLMATCH; /* full match */
    else
      return PARTMATCH; /* partial match */
  }
  else
    return UNMATCH; /* no match */
}

const CommandDescription* CommandDescription::next() const
{
  if ((const CommandDescription*)~0 == nextPtr)
    return this + 1;
  else
    return nextPtr;
}

/*
 * check commands at given level with input string.
 * _cmd: point to first command at this level, return matched cmd
 * _str: point to current unprocessed input, return next unprocessed
 */
int TinySh::parse_command(const CommandDescription **_cmd, char **_str)
{
  char *str = *_str;
  const CommandDescription *cmd;
  const CommandDescription *matched_cmd = 0;

  /* first eliminate first blanks */
  while (*str == ' ')
    str++;
  if (!*str)
  {
    *_str = str;
    return NULLMATCH; /* end of input */
  }

  /* first pass: count matches */
  for (cmd = *_cmd; cmd; cmd = cmd->next())
  {
    int ret = strstart(cmd->name, str);

    if (ret == FULLMATCH)
    {
      /* found full match */
      while (*str && *str != ' ')
        str++;
      while (*str == ' ')
        str++;
      *_str = str;
      *_cmd = cmd;
      return MATCH;
    }
    else if (ret == PARTMATCH)
    {
      if (matched_cmd)
      {
        *_cmd = matched_cmd;
        return AMBIG;
      }
      else
      {
        matched_cmd = cmd;
      }
    }
    else /* UNMATCH */
    {
    }
  }
  if (matched_cmd)
  {
    while (*str && *str != ' ')
      str++;
    while (*str == ' ')
      str++;
    *_cmd = matched_cmd;
    *_str = str;
    return MATCH;
  }
  else
    return UNMATCH;
}

/* create a context from current input line
 */
void TinySh::do_context(const CommandDescription *cmd, const char *str)
{
  while (*str)
    context_buffer[cur_context++] = *str++;

  while (' ' == context_buffer[cur_context - 1])
    --cur_context;

  context_buffer[cur_context] = 0;
  cur_cmd_ctx = cmd;
}

/* execute the given command by calling callback with appropriate
 * arguments
 */
void TinySh::exec_command(const CommandDescription *cmd, char *str)
{
  const char *argv[MAX_ARGS];
  unsigned argc = 0;
  int i;

  /* copy command line to preserve it for history */
  for (i = 0; i < BUFFER_SIZE; i++)
    trash_buffer[i] = str[i];
  str = trash_buffer;

  /* cut into arguments */
  argv[argc++] = cmd->name;
  while (*str && argc < MAX_ARGS)
  {
    // skip over leading spaces
    while (*str == ' ')
      str++;
    // break, if end of string
    if (*str == 0)
      break;
    // have an other argument
    argv[argc++] = str;
    // skip to the end of argument
    while (*str != ' ' && *str)
      str++;
    // break, if end of string
    if (!*str)
      break;
    // cut arguments on first trailing space
    *str++ = 0;
  }
  /* call command function if present */
  if (cmd->function)
  {
    plusArg = cmd->arg;
    cmd->function(*this, argc, &argv[0]);
  }
}

/* try to execute the current command line
 */
int TinySh::exec_command_line(const CommandDescription *cmd, char *_str)
{
  char *str = _str;

  while (1)
  {
    int ret;
    ret = parse_command(&cmd, &str);
    if (ret == MATCH) /* found unique match */
    {
      if (cmd)
      {
        if (!cmd->child) /* no sub-command, execute */
        {
          exec_command(cmd, str);
          return 0;
        }
        else
        {
          if (*str == 0) /* no more input, this is a context */
          {
            do_context(cmd, _str);
            return 0;
          }
          else /* process next command word */
          {
            cmd = cmd->child;
          }
        }
      }
      else /* cmd == 0 */
      {
        return 0;
      }
    }
    else if (ret == AMBIG)
    {
      ioStream->writeBlock("ambiguity: ");
      ioStream->writeBlock(str);
      ioStream->write('\n');
      return 0;
    }
    else if (ret == UNMATCH) /* UNMATCH */
    {
      ioStream->writeBlock("no match: ");
      ioStream->writeBlock(str);
      ioStream->write('\n');
      return 0;
    }
    else
      /* NULLMATCH */
      return 0;
  }

  return 0;
}

/* display help for list of commands
 */
void TinySh::display_child_help(const CommandDescription *cmd)
{
  const CommandDescription *cm;
  int len = 0;

  ioStream->write('\n');
  for (cm = cmd; cm; cm = cm->next())
    if (len < tinysh_strlen(cm->name))
      len = tinysh_strlen(cm->name);
  for (cm = cmd; cm; cm = cm->next())
    if (cm->help)
    {
      int i;
      ioStream->writeBlock(cm->name);
      for (i = tinysh_strlen(cm->name); i < len + 2; i++)
        ioStream->write(' ');
      ioStream->writeBlock(cm->help);
      ioStream->write('\n');
    }
}

/* try to display help for current comand line
 */
int TinySh::help_command_line(const CommandDescription *cmd, char *_str)
{
  char *str = _str;

  while (1)
  {
    int ret;
    ret = parse_command(&cmd, &str);
    if (ret == MATCH && *str == 0) /* found unique match or empty line */
    {
      if (cmd->child) /* display sub-commands help */
      {
        display_child_help(cmd->child);
        return 0;
      }
      else /* no sub-command, show single help */
      {
        if (*(str - 1) != ' ')
          ioStream->write(' ');
        if (cmd->usage)
          ioStream->writeBlock(cmd->usage);
        ioStream->writeBlock(": ");
        if (cmd->help)
          ioStream->writeBlock(cmd->help);
        else
          ioStream->writeBlock("no help available");
        ioStream->write('\n');
      }
      return 0;
    }
    else if (ret == MATCH && *str)
    { /* continue processing the line */
      cmd = cmd->child;
    }
    else if (ret == AMBIG)
    {
      ioStream->writeBlock("\nambiguity: ");
      ioStream->writeBlock(str);
      ioStream->write('\n');
      return 0;
    }
    else if (ret == UNMATCH)
    {
      ioStream->writeBlock("\nno match: ");
      ioStream->writeBlock(str);
      ioStream->write('\n');
      return 0;
    }
    else /* NULLMATCH */
    {
      if (cur_cmd_ctx)
        display_child_help(cur_cmd_ctx->child);
      else
        display_child_help(root_cmd);
      return 0;
    }
  }

  return 0;
}

/* try to complete current command line
 */
int TinySh::complete_command_line(const CommandDescription *cmd, char *_str)
{
  char *str = _str;

  while (1)
  {
    int ret;
    int common_len = BUFFER_SIZE;
    int _str_len;
    int i;
    char *__str = str;

    ret = parse_command(&cmd, &str);
    for (_str_len = 0; __str[_str_len] && __str[_str_len] != ' '; _str_len++)
      ;
    if (ret == MATCH && *str)
    {
      cmd = cmd->child;
    }
    else if (ret == AMBIG || ret == MATCH || ret == NULLMATCH)
    {
      const CommandDescription *cm;
      const CommandDescription *matched_cmd = 0;
      int nb_match = 0;

      for (cm = cmd; cm; cm = cm->next())
      {
        int r = strstart(cm->name, __str);
        if (r == FULLMATCH)
        {
          for (i = _str_len; cmd->name[i]; i++)
            char_in(cmd->name[i]);
          if (*(str - 1) != ' ')
            char_in(' ');
          if (!cmd->child)
          {
            if (cmd->usage)
            {
              ioStream->writeBlock(cmd->usage);
              ioStream->write('\n');
              return 1;
            }
            else
              return 0;
          }
          else
          {
            cmd = cmd->child;
            break;
          }
        }
        else if (r == PARTMATCH)
        {
          nb_match++;
          if (!matched_cmd)
          {
            matched_cmd = cm;
            common_len = tinysh_strlen(cm->name);
          }
          else
          {
            for (i = _str_len; cm->name[i] && i < common_len && cm->name[i] == matched_cmd->name[i]; i++)
              ;
            if (i < common_len)
              common_len = i;
          }
        }
      }
      if (cm)
        continue;
      if (matched_cmd)
      {
        if (_str_len == common_len)
        {
          ioStream->write('\n');
          for (cm = cmd; cm; cm = cm->next())
          {
            int r = strstart(cm->name, __str);
            if (r == FULLMATCH || r == PARTMATCH)
            {
              ioStream->writeBlock(cm->name);
              ioStream->write('\n');
            }
          }
          return 1;
        }
        else
        {
          for (i = _str_len; i < common_len; i++)
            char_in(matched_cmd->name[i]);
          if (nb_match == 1)
            char_in(' ');
        }
      }
      return 0;
    }
    else /* UNMATCH */
    {
      return 0;
    }
  }

  return 0;
}

/* start a new line
 */
void TinySh::start_of_line()
{
  /* display start of new line */
  ioStream->writeBlock(prompt);
  if (cur_context)
  {
    ioStream->writeBlock(context_buffer);
    ioStream->writeBlock(" > ");
  }
  cursorPos = 0;
}

/* new character input */
void TinySh::char_in(char c)
{
  char *line = input_buffers[cur_buf_index];

  if (c == '\n' || c == '\r') /* validate command */
  {
    const CommandDescription *cmd;

    /* first, echo the newline */
    if (echo)
      ioStream->write(c);

    if (echo && ('\r' == c))
      ioStream->write('\n');

    while (*line && *line == ' ')
      line++;
    if (*line) /* not empty line */
    {
      cmd = cur_cmd_ctx ? cur_cmd_ctx->child : root_cmd;
      exec_command_line(cmd, line);
      cur_buf_index = (cur_buf_index + 1) % HISTORY_DEPTH;
      cursorPos = 0;
      input_buffers[cur_buf_index][0] = 0;
    }
    start_of_line();
  }
  else if ((c == TOPCHAR) && (cursorPos == 0)) /* return to top level */
  {
    if (echo)
      ioStream->write(c);

    cur_context = 0;
    cur_cmd_ctx = 0;
  }
  else if (c == CTRL('H') || c == 127) /* backspace */
  {
    if (cursorPos > 0)
    {
      if (echo)
        ioStream->writeBlock("\b \b");
      cursorPos--;
      line[cursorPos] = 0;
    }
  }
  else if (c == CTRL('P')) /* CTRL-P: back in history */
  {
    int prevline = (cur_buf_index + HISTORY_DEPTH - 1) % HISTORY_DEPTH;

    if (input_buffers[prevline][0])
    {
      line = input_buffers[prevline];
      /* fill the rest of the line with spaces */
      while (cursorPos-- > tinysh_strlen(line))
        ioStream->writeBlock("\b \b");
      ioStream->write('\r');
      start_of_line();
      ioStream->writeBlock(line);
      cursorPos = tinysh_strlen(line);
      cur_buf_index = prevline;
    }
  }
  else if (c == CTRL('N')) /* CTRL-N: next in history */
  {
    int nextline = (cur_buf_index + 1) % HISTORY_DEPTH;

    if (input_buffers[nextline][0])
    {
      line = input_buffers[nextline];
      /* fill the rest of the line with spaces */
      while (cursorPos-- > tinysh_strlen(line))
        ioStream->writeBlock("\b \b");
      ioStream->write('\r');
      start_of_line();
      ioStream->writeBlock(line);
      cursorPos = tinysh_strlen(line);
      cur_buf_index = nextline;
    }
  }
  else if (c == '?') /* display help */
  {
    const CommandDescription *cmd;
    cmd = cur_cmd_ctx ? cur_cmd_ctx->child : root_cmd;
    help_command_line(cmd, line);
    start_of_line();
    ioStream->writeBlock(line);
    cursorPos = tinysh_strlen(line);
  }
  else if (c == CTRL('I') || c == '!') /* TAB: autocompletion */
  {
    const CommandDescription *cmd;
    cmd = cur_cmd_ctx ? cur_cmd_ctx->child : root_cmd;
    if (complete_command_line(cmd, line))
    {
      start_of_line();
      ioStream->writeBlock(line);
    }
    cursorPos = tinysh_strlen(line);
  }
  else if ((' ' <= c) && (127 >= c))/* any input character */
  {
    if (cursorPos < BUFFER_SIZE) // accept warning for comparison signed vs unsigned
    {
      if (echo)
        ioStream->write(c);
      line[cursorPos++] = c;
      line[cursorPos] = 0;
    }
  }
}

/* add a new command */
Shell::TinySh& TinySh::add_command(CommandDescription *cmd, CommandDescription *parent)
{
  const CommandDescription *cm;

  assert(cmdListIsWritable); // oder es wurde ein Kommando (eine Liste) const hinzugefügt, danach ist die gesamte Liste readonly

  if (parent)
  {
    cm = parent->child;
    if (!cm)
    {
      parent->child = cmd;
    }
    else
    {
      while (cm->nextPtr)
        cm = cm->next();
      cm->nextPtr = cmd;
    }
  }
  else if (!root_cmd)
  {
    root_cmd = cmd;
  }
  else
  {
    cm = root_cmd;
    while (cm->nextPtr)
      cm = cm->next();
    cm->nextPtr = cmd;
  }

  return *this;
}

TinySh& TinySh::add_command(const CommandDescription* cmd, CommandDescription* parent)
{
  add_command((CommandDescription*) cmd, parent);

  cmdListIsWritable = false;

  return *this;
}

/* string to decimal/hexadecimal conversion
 */
unsigned long TinySh::atoxi(const char *s, bool isHex)
{
  int ishex = int(isHex);
  unsigned long res = 0;

  if (*s == 0)
    return 0;

  if (*s == '0' && *(s + 1) == 'x')
  {
    ishex = 1;
    s += 2;
  }

  while (*s)
  {
    if (ishex)
      res *= 16;
    else
      res *= 10;

    if (*s >= '0' && *s <= '9')
      res += *s - '0';
    else if (ishex && *s >= 'a' && *s <= 'f')
      res += *s + 10 - 'a';
    else if (ishex && *s >= 'A' && *s <= 'F')
      res += *s + 10 - 'A';
    else
      break;

    s++;
  }

  return res;
}

void TinySh::checkInput()
{
  assert(0 != ioStream); // erst setIo() ausführen, bevor die ersten Ausgaben gemacht werden

  char c;

  if (ioStream->read(c))
  {
    char_in(c);
  }
}

void TinySh::feed(const char* script)
{
  if (script)
  {
    bool oldEcho = echo;
    const char* oldPrompt = prompt;
    echo = false;
    prompt = "";
    while (*script)
      char_in(*script++);
    prompt = oldPrompt;
    echo = oldEcho;
    triggerPrompt();
  }
}

}
