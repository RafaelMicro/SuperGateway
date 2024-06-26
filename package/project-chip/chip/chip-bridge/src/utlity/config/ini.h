/**
 * Yet another .ini parser for modern c++ (made for cpp17), inspired and extend
 * from @benhoyt's inih. See project page: https://github.com/SSARCandy/ini-cpp
 */

#ifndef __INI_H__
#define __INI_H__

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <sys/stat.h>

namespace inih {

/* Typedef for prototype of handler function. */
typedef int (*ini_handler)(void *user, const char *section, const char *name,
                           const char *value);

/* Typedef for prototype of fgets-style reader function. */
typedef char *(*ini_reader)(char *str, int num, void *stream);

#define INI_STOP_ON_FIRST_ERROR 1
#define INI_MAX_LINE 2000
#define INI_INITIAL_ALLOC 200
#define MAX_SECTION 50
#define MAX_NAME 50
#define INI_START_COMMENT_PREFIXES ";#"
#define INI_INLINE_COMMENT_PREFIXES ";"
#define INI_HANDLER_LINENO 0

/* Strip whitespace chars off end of given string, in place. Return s. */
inline static char *rstrip(char *s) {
  char *p = s + strlen(s);
  while (p > s && isspace((unsigned char)(*--p)))
    *p = '\0';
  return s;
}

/* Return pointer to first non-whitespace char in given string. */
inline static char *lskip(const char *s) {
  while (*s && isspace((unsigned char)(*s)))
    s++;
  return (char *)s;
}

/* Return pointer to first char (of chars) or inline comment in given string,
   or pointer to null at end of string if neither found. Inline comment must
   be prefixed by a whitespace character to register as a comment. */
inline static char *find_chars_or_comment(const char *s, const char *chars) {
  int was_space = 0;
  while (*s && (!chars || !strchr(chars, *s)) &&
         !(was_space && strchr(INI_INLINE_COMMENT_PREFIXES, *s))) {
    was_space = isspace((unsigned char)(*s));
    s++;
  }
  return (char *)s;
}

/* Version of strncpy that ensures dest (size bytes) is null-terminated. */
inline static char *strncpy0(char *dest, const char *src, size_t size) {
  strncpy(dest, src, size - 1);
  dest[size - 1] = '\0';
  return dest;
}

/* See documentation in header file. */
inline int ini_parse_stream(ini_reader reader, void *stream,
                            ini_handler handler, void *user) {
  /* Uses a fair bit of stack (use heap instead if you need to) */
  char *line, *new_line;
  size_t max_line = INI_INITIAL_ALLOC;
  size_t offset;
  char section[MAX_SECTION] = "";
  char prev_name[MAX_NAME] = "";

  char *start, *end, *name, *value;
  int lineno = 0, error = 0;

  line = (char *)malloc(INI_INITIAL_ALLOC);
  if (!line)
    return -2;

#if INI_HANDLER_LINENO
#define HANDLER(u, s, n, v) handler(u, s, n, v, lineno)
#else
#define HANDLER(u, s, n, v) handler(u, s, n, v)
#endif

  /* Scan through stream line by line */
  while (reader(line, (int)max_line, stream) != NULL) {
    offset = strlen(line);
    while (offset == max_line - 1 && line[offset - 1] != '\n') {
      max_line *= 2;
      if (max_line > INI_MAX_LINE)
        max_line = INI_MAX_LINE;
      new_line = (char *)realloc(line, max_line);
      if (!new_line) {
        free(line);
        return -2;
      }
      line = new_line;
      if (reader(line + offset, (int)(max_line - offset), stream) == NULL)
        break;
      if (max_line >= INI_MAX_LINE)
        break;
      offset += strlen(line + offset);
    }

    lineno++;

    start = line;
    if (lineno == 1 && (unsigned char)start[0] == 0xEF &&
        (unsigned char)start[1] == 0xBB && (unsigned char)start[2] == 0xBF) {
      start += 3;
    }
    start = lskip(rstrip(start));

    if (strchr(INI_START_COMMENT_PREFIXES, *start)) {
      /* Start-of-line comment */
    } else if (*start == '[') {
      /* A "[section]" line */
      end = find_chars_or_comment(start + 1, "]");
      if (*end == ']') {
        *end = '\0';
        strncpy0(section, start + 1, sizeof(section));
        *prev_name = '\0';
      } else if (!error) {
        /* No ']' found on section line */
        error = lineno;
      }
    } else if (*start) {
      /* Not a comment, must be a name[=:]value pair */
      end = find_chars_or_comment(start, "=:");
      if (*end == '=' || *end == ':') {
        *end = '\0';
        name = rstrip(start);
        value = end + 1;
        end = find_chars_or_comment(value, NULL);
        if (*end)
          *end = '\0';
        value = lskip(value);
        rstrip(value);

        /* Valid name[=:]value pair found, call handler */
        strncpy0(prev_name, name, sizeof(prev_name));
        if (!HANDLER(user, section, name, value) && !error)
          error = lineno;
      } else if (!error) {
        /* No '=' or ':' found on name[=:]value line */
        error = lineno;
      }
    }
    if (error)
      break;
  }
  free(line);
  return error;
}

inline int ini_parse_file(FILE *file, ini_handler handler, void *user) {
  return ini_parse_stream((ini_reader)fgets, file, handler, user);
}

inline int ini_parse(const char *filename, ini_handler handler, void *user) {
  FILE *file;
  int error;
  file = fopen(filename, "r");
  if (!file) file = fopen(filename, "w");
  error = ini_parse_file(file, handler, user);
  fclose(file);
  return error;
}

} // namespace inih
#endif /* __INI_H__ */
