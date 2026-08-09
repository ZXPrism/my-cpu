import argparse
import subprocess
import tempfile

from dataclasses import dataclass
from pathlib import Path
from typing import Callable

from asm.codegen import Codegen
from asm.parser import Parser
from asm.tokenizer import Tokenizer


@dataclass(frozen=True)
class TestContext:
    source_path: Path
    codegen: Codegen
    memory: bytes


TestJudge = Callable[[TestContext], None]


@dataclass(frozen=True)
class TestCase:
    asm_file: str
    judge: TestJudge


_tests: list[TestCase] = []


def register_test(asm_file: str):
    def decorator(judge: TestJudge) -> TestJudge:
        if any(test.asm_file == asm_file for test in _tests):
            raise RuntimeError(f"A test is already registered for {asm_file}")

        _tests.append(TestCase(asm_file, judge))
        return judge

    return decorator


@register_test("fib.s")
def test_fib(context: TestContext):
    result_addr = context.codegen.get_symbol_addr("result")
    expected = bytes([1, 2, 3, 5, 8, 13, 21, 34, 55, 89])
    actual = context.memory[result_addr : result_addr + len(expected)]

    assert actual == expected, f"Expected {list(expected)}, found {list(actual)}"


def _assemble(source_path: Path, bytecode_path: Path) -> Codegen:
    source = source_path.read_text(encoding="utf-8")
    tokenizer_errors: list[str] = []

    tokenizer = Tokenizer(
        source,
        lambda line_no, message: tokenizer_errors.append(f"line {line_no}: {message}"),
    )
    tokens = tokenizer.scan_tokens()

    if tokenizer_errors:
        raise RuntimeError("\n".join(tokenizer_errors))

    program = Parser(tokens).parse()
    codegen = Codegen(program)
    codegen.emit(str(bytecode_path))
    return codegen


def _run_test(test: TestCase, vm_path: Path, test_dir: Path) -> tuple[bool, str]:
    source_path = test_dir / test.asm_file

    try:
        with tempfile.TemporaryDirectory() as temp_dir:
            temp_path = Path(temp_dir)
            bytecode_path = temp_path / "program.bin"
            memory_path = temp_path / "memory.bin"

            codegen = _assemble(source_path, bytecode_path)
            vm_result = subprocess.run(
                [
                    str(vm_path),
                    "--input",
                    str(bytecode_path),
                    "--dump-memory",
                ],
                cwd=temp_path,
                capture_output=True,
                text=True,
            )

            if vm_result.returncode != 0:
                output = vm_result.stderr.strip() or vm_result.stdout.strip()
                raise RuntimeError(
                    f"VM exited with code {vm_result.returncode}: {output}"
                )

            if not memory_path.is_file():
                raise RuntimeError("VM did not produce memory.bin")

            context = TestContext(
                source_path=source_path,
                codegen=codegen,
                memory=memory_path.read_bytes(),
            )
            test.judge(context)
    except Exception as error:
        return False, str(error)

    return True, ""


def main() -> int:
    argument_parser = argparse.ArgumentParser()
    argument_parser.add_argument(
        "--vm",
        required=True,
        type=Path,
        help="Path to the VM executable",
    )
    args = argument_parser.parse_args()

    vm_path = args.vm.resolve()
    if not vm_path.is_file():
        argument_parser.error(f"VM executable does not exist: {vm_path}")

    test_dir = Path(__file__).parent / "test"
    passed = 0

    for test in _tests:
        succeeded, message = _run_test(test, vm_path, test_dir)
        if succeeded:
            passed += 1
            print(f"PASS {test.asm_file}")
        else:
            print(f"FAIL {test.asm_file}: {message}")

    print(f"{passed}/{len(_tests)} tests passed")
    return 0 if passed == len(_tests) else 1


if __name__ == "__main__":
    raise SystemExit(main())
