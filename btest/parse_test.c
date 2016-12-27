/*
 * Copyright (c) 2015, Henry Corrigan-Gibbs
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */


#include <stdlib.h>
#include <string.h>
#include "mutest.h"

#include "libballoon/constants.h"
#include "libballoon/errors.h"
#include "libballoon/parse.h"


void 
mu_test_parse__n_tokens (void) 
{
  mu_check (n_tokens("", 0, 'a') == 1);
  mu_check (n_tokens("", 0, '\0') == 1);
  mu_check (n_tokens("a", 0, 'a') == 1);
  mu_check (n_tokens("a", 1, 'a') == 2);
  mu_check (n_tokens("a$b", 3, '$') == 2);
  mu_check (n_tokens("a$b$", 4, '$') == 3);
  mu_check (n_tokens("$a$b$", 5, '$') == 4);
  mu_check (n_tokens("$a$b$", 4, '$') == 3);
  mu_check (n_tokens("$$$", 3, '$') == 4);
  mu_check (n_tokens("  $  ", 5, '$') == 2);
  mu_check (n_tokens("  $  ", 5, 'a') == 1);
}


void 
mu_test_parse__tokenize (void) 
{
  char str[1024];
  char *tokens[10];
  strcpy (str, "a$b$c");
  tokenize (tokens, str, '$');

  mu_check (!strcmp(tokens[0], "a"));
  mu_check (!strcmp(tokens[1], "b")); 
  mu_check (!strcmp(tokens[2], "c"));

  strcpy (str, "abc");
  tokenize (tokens, str, '$');

  mu_check (!strcmp(tokens[0], "abc"));

  strcpy (str, "");
  tokenize (tokens, str, '$');
  mu_check (!strcmp(tokens[0], ""));

  strcpy (str, "a-b--c");
  tokenize (tokens, str, '-');
  mu_check (!strcmp(tokens[0], "a"));
  mu_check (!strcmp(tokens[1], "b"));
  mu_check (!strcmp(tokens[2], ""));
  mu_check (!strcmp(tokens[3], "c"));

  strcpy (str, "a-b--c--");
  tokenize (tokens, str, '-');
  mu_check (!strcmp(tokens[0], "a"));
  mu_check (!strcmp(tokens[1], "b"));
  mu_check (!strcmp(tokens[2], ""));
  mu_check (!strcmp(tokens[3], "c"));
  mu_check (!strcmp(tokens[4], ""));
  mu_check (!strcmp(tokens[5], ""));

  strcpy (str, "-a-b--c");
  tokenize (tokens, str, '-');
  mu_check (!strcmp(tokens[0], ""));
  mu_check (!strcmp(tokens[1], "a"));
  mu_check (!strcmp(tokens[2], "b"));
  mu_check (!strcmp(tokens[3], ""));
  mu_check (!strcmp(tokens[4], "c"));
}

void 
mu_test_parse__int_parse (void) 
{
  uint32_t x;
  mu_check (int_parse ("3", &x) == ERROR_NONE);
  mu_check (x == 3);
  mu_check (int_parse ("1", &x) == ERROR_NONE);
  mu_check (x == 1);

  mu_check (int_parse ("0423", &x) == ERROR_NONE);
  mu_check (x == 423);
  mu_check (int_parse ("00000423", &x) == ERROR_NONE);
  mu_check (x == 423);
  mu_check (int_parse ("999", &x) == ERROR_NONE);
  mu_check (x == 999);

  mu_check (int_parse ("0", &x) == ERROR_NONE);
  mu_check (x == 0);

  mu_check (int_parse ("-1", &x) != ERROR_NONE);
  mu_check (int_parse ("cow", &x) != ERROR_NONE);
  mu_check (int_parse ("05cow", &x) != ERROR_NONE);
  mu_check (int_parse ("05cow05", &x) != ERROR_NONE);
  mu_check (int_parse ("400000000000000000000000000000000000000000000000000000000000000000023", &x) != ERROR_NONE);
}


void 
mu_test_parse__options0 (void) 
{
  uint32_t s_cost, t_cost, p_cost;
  const char *str = "t=15,s=014,p=1";
  mu_check (parse_options (str, strlen (str) + 1, &s_cost, &t_cost, &p_cost) == ERROR_NONE);

  mu_check (s_cost == 14);
  mu_check (t_cost == 15);
  mu_check (p_cost == 1);
}

void 
mu_test_parse__options1 (void) 
{
  uint32_t s_cost, t_cost, p_cost;
  const char *str = "t=12,t=15,s=014,p=1";
  mu_check (parse_options (str, strlen (str) + 1, &s_cost, &t_cost, &p_cost) == ERROR_NONE);

  mu_check (s_cost == 14);
  mu_check (t_cost == 15);
  mu_check (p_cost == 1);
}


void 
mu_test_parse__options2 (void) 
{
  uint32_t s_cost, t_cost, p_cost;
  const char *str = "t=12,  t=15,  s=014,p=1";
  mu_check (parse_options (str, strlen (str) + 1, &s_cost, &t_cost, &p_cost) != ERROR_NONE);

  mu_check (s_cost == 14);
  mu_check (t_cost == 15);
  mu_check (p_cost == 1);
}

void 
mu_test_parse__options3 (void) 
{
  uint32_t s_cost, t_cost, p_cost;
  const char *str = "t=-12,s=0140,p=1";
  mu_check (parse_options (str, strlen (str) + 1, &s_cost, &t_cost, &p_cost) != ERROR_NONE);

  mu_check (s_cost == 14);
  mu_check (t_cost == 15);
  mu_check (p_cost == 1);
}

void 
mu_test_parse__verify (void) 
{
  const char *str = "$balloon$v=1$t=12,s=0140,p=1$YmxhaAo=$YmxhaAo=";
  int len = strlen (str);

  uint32_t s_cost, t_cost, p_cost;
  uint8_t out[1024];
  uint8_t salt[SALT_LEN];
  mu_check (read_blob (str, len+1, salt, out, 1024, &s_cost, &t_cost, &p_cost) == ERROR_NONE);

  mu_check (s_cost == 140);
  mu_check (t_cost == 12);
  mu_check (p_cost == 1);

  mu_check (!strncmp((char *)salt, "blah", 4));
  mu_check (!strncmp((char *)out, "blah", 4));
}

void 
mu_test_parse__fail (void) 
{
  const char *str = "$balloon$v=1$t=12,s=0140,p=1$YmxhaAo=$Ymxha?Ao=";
  int len = strlen (str);

  uint32_t s_cost, t_cost, p_cost;
  uint8_t out[1024];
  uint8_t salt[SALT_LEN];
  mu_check (read_blob (str, len+1, salt, out, 1024, &s_cost, &t_cost, &p_cost) != ERROR_NONE);
}

void 
mu_test_parse__fail2 (void) 
{
  const char *str = "$balloon$v=1$t=12,s=0140,p=1$$YmxhaAo=";
  int len = strlen (str);

  uint32_t s_cost, t_cost, p_cost;
  uint8_t out[1024];
  uint8_t salt[SALT_LEN];
  mu_check (read_blob (str, len+1, salt, out, 1024, &s_cost, &t_cost, &p_cost) != ERROR_NONE);
}

void 
mu_test_parse__fail3 (void) 
{
  const char *str = "$bxlwalloon$v=1$t=12,s=0140,p=1$YmxhaAo=$YmxhaAo=";
  int len = strlen (str);

  uint32_t s_cost, t_cost, p_cost;
  uint8_t out[1024];
  uint8_t salt[SALT_LEN];
  mu_check (read_blob (str, len+1, salt, out, 1024, &s_cost, &t_cost, &p_cost) != ERROR_NONE);
}

void 
mu_test_parse__fail4 (void) 
{
  const char *str = "$balloon$v=2$t=12,s=0140,p=1$YmxhaAo=$YmxhaAo=";
  int len = strlen (str);

  uint32_t s_cost, t_cost, p_cost;
  uint8_t out[1024];
  uint8_t salt[SALT_LEN];
  mu_check (read_blob (str, len+1, salt, out, 1024, &s_cost, &t_cost, &p_cost) != ERROR_NONE);
}

void 
mu_test_parse__fail5 (void) 
{
  const char *str = "$balloon$v=1$t=12,s=0140,p=1$YmxhaAo=";
  int len = strlen (str);

  uint32_t s_cost, t_cost, p_cost;
  uint8_t out[1024];
  uint8_t salt[SALT_LEN];
  mu_check (read_blob (str, len+1, salt, out, 1024, &s_cost, &t_cost, &p_cost) != ERROR_NONE);
}
