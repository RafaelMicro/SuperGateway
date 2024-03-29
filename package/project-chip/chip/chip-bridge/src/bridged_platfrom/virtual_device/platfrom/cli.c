#include "cli.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
extern CLI_Func_t cli_func[];

static int command_parser(str_space_t* input, CLI_Token_t *TK) {
  uint16_t pos = 0;
  bool first = true;
  TK->count = 0;
  while (pos < input->len){
    if(!isspace(input->ptr[pos])) break;
    pos++;
  }
  while (pos != input->len && input->ptr[pos] != '\n' && input->ptr[pos] != '\0') {
    if (first) {
      TK->token_list[TK->count].ptr = (char *)&input->ptr[pos];
      TK->token_list[TK->count].len = 1;
      first = false;
      continue;
    }
    if(!isspace(input->ptr[pos+1])) { TK->token_list[TK->count].len++; }
    else {
      printf("Token: %d, len: %d\n", TK->count, TK->token_list[TK->count].len);
      TK->count++;
      first = true;
    }
    pos++;
  }
  return TK->count;
}

uint8_t is_str_eql(str_space_t* A, str_space_t* B)
{
  uint16_t len = A->len < B->len ? A->len : B->len;
  for(int i = 0; i < len; i++) if(A->ptr[i] != B->ptr[i]) return 0;
  return 1;
}

uint8_t cli_interpreter(str_space_t* input, CLI_Token_t *TK){
  if (input->ptr == NULL || input->len == 0 ) return BAD_REQUEST;
  if (command_parser(input, TK) == 0) return NULL_DATA;
  str_space_t* test_str = (str_space_t*)malloc(sizeof(str_space_t));
  
  for(CLI_Func_t *ptr = cli_func; ptr->func != NULL; ptr++) {
    test_str->ptr = (char*)ptr->name;
    test_str->len = ptr->len;
    if(is_str_eql(&TK->token_list[0], test_str)){
      ptr->func(TK);
      free(test_str);
      return PARSED;
    }
  }
  printf("Command not Found, cmd: %s> ", input->ptr);
  free(test_str);
  return NOTFOUND;
}
