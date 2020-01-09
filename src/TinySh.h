/*
* TinySh.h
*
*/

#ifndef TINYSH_H_
#define TINYSH_H_

#include "ByteStream.h"

#ifndef TINYSH_BUFFER_SIZE
#define TINYSH_BUFFER_SIZE 80
#endif

#ifndef TINYSH_HISTORY_DEPTH
#define TINYSH_HISTORY_DEPTH 8
#endif

#ifndef TINYSH_MAX_ARGS
#define TINYSH_MAX_ARGS 16
#endif

#ifndef TINYSH_TOPCHAR
#define TINYSH_TOPCHAR '/'
#endif

namespace Shell
{
  class TinySh;

  typedef void (*CommandFunction_t)(TinySh& shell, int argc, const char **argv);

  struct CommandDescription
  {
    const char *name; /* command input name, not 0 */
    const char *help; /* help string, can be 0 */
    const char *usage; /* usage string, can be 0 */
    CommandFunction_t function; /* function to launch on cmd, can be 0 */
    void *arg; /* current argument when function called */
    const CommandDescription mutable *nextPtr; /* must be set to 0 at init, must be ~0 on array members, last array member must be 0 on init */
    const CommandDescription mutable *child; /* must be set to 0 at init */

    const CommandDescription* next() const;
  };

  class TinySh
  {
  public:
    TinySh(void * container = 0);
//    virtual
//    ~TinySh();

    /***
     * add a new command
     *
     * parent == 0, if toplevel command
     **/
    TinySh& add_command(CommandDescription *cmd, CommandDescription* parent = 0);
    TinySh& add_command(const CommandDescription *cmd, CommandDescription* parent = 0);

    /* connect the IO channel */
    TinySh& setIo(ByteStream& io);

    /* set the input echo */
    void setEcho(bool echo);

    /* change tinysh prompt */
    TinySh& set_prompt(const char *str);

    /* get command argument back */
    void* get_arg();

    /* get the instance container ptr back */
    void* get_container();

    /* provide conversion string to scalar (decimal or hexadecimal) */
    static unsigned long atoxi(const char *s, bool isHex = false);

    /* process new character input */
    void checkInput();

    /* feed literal input characters as command script without echo and prompt*/
    void feed(const char* script);

    /* get the IO object for character input and output */
    ByteStream& io();

    void triggerPrompt(); // das ist dann z.B. gut für eine neue Netzwerkverbindung

  private:
    int parse_command(const CommandDescription **_cmd, char **_str);
    void do_context(const CommandDescription *cmd, const char *str);
    void exec_command(const CommandDescription *cmd, char *str);
    int exec_command_line(const CommandDescription *cmd, char *_str);
    void display_child_help(const CommandDescription *cmd);
    int help_command_line(const CommandDescription *cmd, char *_str);
    int complete_command_line(const CommandDescription *cmd, char *_str);
    void start_of_line();

    static const int BUFFER_SIZE = TINYSH_BUFFER_SIZE;
    static const unsigned HISTORY_DEPTH = TINYSH_HISTORY_DEPTH;
    static const unsigned MAX_ARGS = TINYSH_MAX_ARGS;
    static const char TOPCHAR = TINYSH_TOPCHAR;

    static void cmd_help(TinySh& shell, int argc, const char **argv);

    static const CommandDescription help_cmd_template;
    CommandDescription help_cmd;

    char input_buffers[HISTORY_DEPTH][BUFFER_SIZE + 1];
    char trash_buffer[BUFFER_SIZE + 1];
    int cur_buf_index;
    char context_buffer[BUFFER_SIZE + 1];
    int cur_context;
    int cursorPos;
    int echo;
    const char *prompt;
    CommandDescription *root_cmd;
    const CommandDescription *cur_cmd_ctx;
    void *plusArg;
    void * const containerPtr;

    bool cmdListIsWritable;

    ByteStream* ioStream;

    void char_in(char c);

    template<typename argT>
    argT CTRL(argT c)
    {
      return (c - '@');
    }

  };

  inline
  ByteStream& TinySh::io()
  {
    return *ioStream;
  }

  inline
  TinySh& TinySh::setIo(ByteStream& io_)
  {
    ioStream = &io_;
    /* force prompt display by generating empty command */
    char_in('\r');

    return *this;
  }

  inline
  void TinySh::setEcho(bool e)
  {
    echo = e;
  }

  inline
  TinySh& TinySh::set_prompt(const char *str)
  {
    prompt = str;
    /* force prompt display by generating empty command */
    char_in('\r');

    return *this;
  }

  inline
  void* TinySh::get_arg()
  {
    return plusArg;
  }

  inline
  void* TinySh::get_container()
  {
    return containerPtr;
  }

  inline
  void TinySh::triggerPrompt()
  {
    char_in('\n');
  }

}

#endif /* TINYSH_H_ */
