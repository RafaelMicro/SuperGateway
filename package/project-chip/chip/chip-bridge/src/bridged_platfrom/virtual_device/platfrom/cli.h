
#ifndef __CLI_H__
#define __CLI_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#ifndef CLI_MAX_BUFFER_SIZE
#define CLI_MAX_BUFFER_SIZE 256
#endif

#ifndef CLI_MAX_TOKEN_COUNT
#define CLI_MAX_TOKEN_COUNT 20
#endif

typedef enum {
  PARSED = 0,
  NOTFOUND = 1,
  BAD_REQUEST = 2,
  NULL_DATA = 3,
} CLI_Return_t;

typedef struct {
  char *ptr;
  uint16_t len;
} str_space_t;

typedef struct {
  str_space_t token_list[CLI_MAX_TOKEN_COUNT];
  uint16_t count;
} CLI_Token_t;

typedef void (*cli_func_ptr)(CLI_Token_t *TK);

typedef struct {
  const char *name;
  uint16_t len;
  cli_func_ptr func;
  const char *help_msg;
} CLI_Func_t;

uint8_t cli_interpreter(str_space_t *input, CLI_Token_t *TK);

#ifdef __cplusplus
}
#endif

#endif // __CLI_H__
