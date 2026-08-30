#include <unity.h>
#include <string.h>
#include <stdio.h>
#include "../../cyan/cyan_shell.h"

static char buffer[CYAN_SHELL_LINE_MAX];
static ShellParse parse;

void setUp(void){
    memset(buffer, 0, sizeof(buffer));
    memset(&parse, 0, sizeof(parse));
}

void tearDown(void){}

static void decompose(const char* line){
    snprintf(buffer, sizeof(buffer), "%s", line);
    cyan_shell_line_decompose(&parse, buffer);
}

/* --- basic splitting --- */

static void test_empty_line_is_empty(void){
    decompose("");
    TEST_ASSERT_EQUAL_INT(SHELL_PARSE_EMPTY, parse.status);
    TEST_ASSERT_EQUAL_INT(0, parse.argc);
}

static void test_whitespace_only_is_empty(void){
    decompose("   ");
    TEST_ASSERT_EQUAL_INT(SHELL_PARSE_EMPTY, parse.status);
}

static void test_single_word(void){
    decompose("ls");
    TEST_ASSERT_EQUAL_INT(SHELL_PARSE_OK, parse.status);
    TEST_ASSERT_EQUAL_INT(1, parse.argc);
    TEST_ASSERT_EQUAL_STRING("ls", parse.argv[0]);
}

static void test_padding_and_repeated_spaces(void){
    decompose("  ls  /apps  ");
    TEST_ASSERT_EQUAL_INT(2, parse.argc);
    TEST_ASSERT_EQUAL_STRING("ls", parse.argv[0]);
    TEST_ASSERT_EQUAL_STRING("/apps", parse.argv[1]);
}

static void test_newline_terminates(void){
    decompose("ls\n");
    TEST_ASSERT_EQUAL_INT(1, parse.argc);
    TEST_ASSERT_EQUAL_STRING("ls", parse.argv[0]);
}

static void test_nested_command_path(void){
    decompose("app permission add storage");
    TEST_ASSERT_EQUAL_INT(4, parse.argc);
    TEST_ASSERT_EQUAL_STRING("permission", parse.argv[1]);
    TEST_ASSERT_EQUAL_STRING("storage", parse.argv[3]);
}

/* --- quoting --- */

static void test_quoted_argument_keeps_spaces(void){
    decompose("say \"hello world\"");
    TEST_ASSERT_EQUAL_INT(2, parse.argc);
    TEST_ASSERT_EQUAL_STRING("hello world", parse.argv[1]);
}

static void test_quotes_join_adjacent_text(void){
    decompose("a\"b\"c");
    TEST_ASSERT_EQUAL_INT(1, parse.argc);
    TEST_ASSERT_EQUAL_STRING("abc", parse.argv[0]);
}

static void test_empty_quotes_produce_empty_argument(void){
    decompose("say \"\"");
    TEST_ASSERT_EQUAL_INT(2, parse.argc);
    TEST_ASSERT_EQUAL_STRING("", parse.argv[1]);
}

/* --- escapes --- */

static void test_escaped_quote_is_literal(void){
    decompose("say \\\"hi\\\"");        /* shell input: say \"hi\" */
    TEST_ASSERT_EQUAL_INT(2, parse.argc);
    TEST_ASSERT_EQUAL_STRING("\"hi\"", parse.argv[1]);
}

static void test_escaped_space_does_not_split(void){
    decompose("a\\ b");                 /* shell input: a\ b */
    TEST_ASSERT_EQUAL_INT(1, parse.argc);
    TEST_ASSERT_EQUAL_STRING("a b", parse.argv[0]);
}

/* --- error cases --- */

static void test_unterminated_quote_is_error(void){
    decompose("say \"unterminated");
    TEST_ASSERT_EQUAL_INT(SHELL_PARSE_UNTERMINATED_QUOTE, parse.status);
}

static void test_trailing_escape_is_error(void){
    decompose("trailing\\");
    TEST_ASSERT_EQUAL_INT(SHELL_PARSE_TRAILING_ESCAPE, parse.status);
}

static void test_too_many_arguments_is_error(void){
    char line[CYAN_SHELL_LINE_MAX] = {0};
    for (int i = 0; i < CYAN_SHELL_MAX_ARGS + 4; i++) strcat(line, "x ");
    decompose(line);
    TEST_ASSERT_EQUAL_INT(SHELL_PARSE_TOO_MANY_ARGS, parse.status);
}

int main(void){
    UNITY_BEGIN();

    RUN_TEST(test_empty_line_is_empty);
    RUN_TEST(test_whitespace_only_is_empty);
    RUN_TEST(test_single_word);
    RUN_TEST(test_padding_and_repeated_spaces);
    RUN_TEST(test_newline_terminates);
    RUN_TEST(test_nested_command_path);

    RUN_TEST(test_quoted_argument_keeps_spaces);
    RUN_TEST(test_quotes_join_adjacent_text);
    RUN_TEST(test_empty_quotes_produce_empty_argument);

    RUN_TEST(test_escaped_quote_is_literal);
    RUN_TEST(test_escaped_space_does_not_split);

    RUN_TEST(test_unterminated_quote_is_error);
    RUN_TEST(test_trailing_escape_is_error);
    RUN_TEST(test_too_many_arguments_is_error);

    return UNITY_END();
}