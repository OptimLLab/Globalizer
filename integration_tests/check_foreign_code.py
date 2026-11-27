"""Проверка Python-пакета globalizer на дословные заимствования из scipy и Optuna.

Интерфейсы globalizer.optimize и globalizer.tuning повторяют API scipy.optimize
и Optuna, но код и тексты в них должны быть свои (иначе в пакет пришлось бы
включать лицензии scipy и Optuna). Скрипт ищет в пакете:
  * строковые литералы (сообщения, docstring) длиной от 30 символов,
    встречающиеся в исходниках scipy / Optuna;
  * блоки из 3 и более подряд одинаковых строк кода.

Запуск (нужны установленные scipy и optuna):
    python integration_tests/check_foreign_code.py

Код возврата 0 - совпадений нет; 1 - есть (печатаются). Присваивания вида
``self.lb = lb`` (одноимённые публичные атрибуты, нужные для совместимости)
заимствованием не считаются.
"""

import ast
import importlib.util
import re
import sys
from pathlib import Path

PACKAGE = Path(__file__).resolve().parent.parent / "PYGlobalizer" / "globalizer"
MIN_STRING = 30
MIN_BLOCK = 3
# self.name = name - одноимённые атрибуты API
_SAME_ATTR = re.compile(r"^self\._?(\w+) = (?:\w+\()?\1\)?$")


def _strings(path):
    try:
        tree = ast.parse(path.read_text(encoding="utf-8"))
    except (SyntaxError, UnicodeDecodeError):
        return
    for node in ast.walk(tree):
        if isinstance(node, ast.Constant) and isinstance(node.value, str):
            yield node.lineno, node.value
        elif isinstance(node, ast.JoinedStr):
            for part in node.values:
                if isinstance(part, ast.Constant):
                    yield node.lineno, part.value


def _code_lines(text):
    """Строки кода без комментариев; короткие и служебные - None."""
    lines = []
    for raw in text.splitlines():
        s = re.sub(r"\s+", " ", raw.split("#", 1)[0].strip())
        trivial = (len(s) < 12 or _SAME_ATTR.match(s)
                   or s.startswith(("import ", "from ", "return ", "raise ", "pass")))
        lines.append(None if trivial else s)
    return lines


def _reference_sources(names):
    sources = []
    for name in names:
        spec = importlib.util.find_spec(name)
        if spec is None or not spec.submodule_search_locations:
            sys.exit(f"{name} is not installed: pip install -r requirements_compare.txt")
        for root in spec.submodule_search_locations:
            for f in Path(root).rglob("*.py"):
                sources.append(f.read_text(encoding="utf-8", errors="replace"))
    return sources


def main():
    sources = _reference_sources(["scipy", "optuna"])
    flat = re.sub(r"\s+", " ", "\n".join(sources))
    known = set()
    for text in sources:
        known.update(l for l in _code_lines(text) if l)

    found = 0
    for f in sorted(PACKAGE.rglob("*.py")):
        rel = f.relative_to(PACKAGE.parent)
        for line, value in _strings(f):
            for chunk in re.split(r"\n\s*\n", value):
                c = re.sub(r"\s+", " ", chunk).strip()
                if len(c) >= MIN_STRING and c in flat:
                    print(f"string  {rel}:{line}: {c[:100]!r}")
                    found += 1
        block = []
        for i, l in enumerate(_code_lines(f.read_text(encoding="utf-8")) + [None], 1):
            if l and l in known:
                block.append(l)
                continue
            if len(block) >= MIN_BLOCK:
                print(f"code    {rel}:{i - len(block)}-{i - 1}:")
                for b in block:
                    print("          " + b[:100])
                found += 1
            block = []
    print("no borrowed fragments found" if not found else f"{found} borrowed fragment(s)")
    return 1 if found else 0


if __name__ == "__main__":
    sys.exit(main())
