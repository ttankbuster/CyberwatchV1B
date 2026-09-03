# Custom test runner for the shell command REPL harness.
#
# shell_repl.c supplies its own main() and is an interactive harness rather than
# a Unity assertion suite, so this env uses `test_framework = custom`. The bare
# TestRunnerBase keeps PlatformIO's normal build + run flow without injecting a
# Unity runner: the program is built and executed with stdin inherited from the
# terminal, and a clean (exit code 0) run counts as a pass. No TESTCASE_PARSE_RE
# is set, so no assertions are collected - this suite just proves the command
# harness builds and runs.
#
# For an interactive session, run the built binary directly:
#   build/shell_repl/program        (Linux/macOS)
#   build\shell_repl\program.exe    (Windows)
from platformio.test.runners.base import TestRunnerBase


class CustomTestRunner(TestRunnerBase):
    pass
