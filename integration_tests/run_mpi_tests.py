#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Globalizer — интеграционные тесты параллельной MPI-версии.
==========================================================
Запускает консольные программы Globalizer (GlobalizerMain и др.) через
mpiexec в двух схемах распараллеливания по точкам:

  * синхронной  (-TypeCalculation MPI_calc): процесс 0 ведёт метод, на каждой
    итерации рассылает по MpiBlockSize точек каждому из ProcNum - 1 рабочих
    процессов и ждёт все результаты (NumPoints = (ProcNum - 1) * MpiBlockSize);
  * асинхронной (-TypeCalculation AsyncMPI): первая итерация - по точке на
    каждый рабочий процесс, затем освободившийся процесс сразу получает новую
    точку (NumPoints = 1), не дожидаясь остальных.

Набор отдельный от остальных интеграционных тестов, так как MPI есть не везде:
он регистрируется в CTest только в сборке с GLOBALIZER_USE_MPI=ON (метка mpi).
Без mpiexec все тесты получают SKIP, а код выхода - 77 (в CTest - пропуск).

Описание тестов - references/mpi_tests.json. Проверки (поле "expect"):
  optimum, tolerance  |f - optimum| <= tolerance;
  trials              точное число испытаний (вычислений первой функции);
                      синхронная схема детерминирована, асинхронная - нет;
  max_trials          испытаний меньше (остановка по точности, а не по лимиту);
  same_as_serial      число испытаний и значение совпадают с последовательным
                      запуском той же программы с теми же параметрами (без
                      mpiexec и без -TypeCalculation);
  points_per_iteration  в среднем столько испытаний на итерацию (проверка,
                      что работали все рабочие процессы синхронной схемы).
Поле теста "repeat" - число повторов (асинхронная схема недетерминирована,
сбои в ней проявлялись не в каждом запуске).
Всегда проверяется, что все процессы завершились с кодом 0, без
"BAD TERMINATION" и без зависания (timeout - завершается всё дерево процессов).

Коды выхода: 0 - нет FAIL; 1 - есть FAIL; 2 - ошибка конфигурации; 77 - нет mpiexec.

Примеры:
  python run_mpi_tests.py --bin-dir ../_bin_mpi
  python run_mpi_tests.py --bin-dir ../_bin_mpi --group mpi_async --verbose
  python run_mpi_tests.py --bin-dir ../_bin_mpi --mpiexec "C:/Program Files (x86)/Intel/oneAPI/mpi/latest/bin/mpiexec.exe"
Переменные окружения: GLOBALIZER_MPIEXEC (путь к mpiexec),
GLOBALIZER_MPIEXEC_ARGS (дополнительные ключи mpiexec, например "-localonly").
"""

import argparse
import json
import os
import re
import shlex
import shutil
import subprocess
import sys
import tempfile
import time
from pathlib import Path

for _stream in (sys.stdout, sys.stderr):
    try:
        _stream.reconfigure(encoding="utf-8", errors="replace")
    except Exception:
        pass

SCRIPT_DIR = Path(__file__).resolve().parent
REFERENCES = SCRIPT_DIR / "references" / "mpi_tests.json"
IS_WINDOWS = os.name == "nt"
EXE_SUFFIX = ".exe" if IS_WINDOWS else ""
SKIP_RC = 77

_FLOAT = r"[-+]?(?:\d+\.?\d*|\.\d+)(?:[eE][-+]?\d+)?"


# --------------------------------------------------------------------------- #
# Поиск mpiexec
# --------------------------------------------------------------------------- #
def find_mpiexec(explicit):
    """Путь к mpiexec: ключ (CMake передаёт MPIEXEC_EXECUTABLE той реализации MPI, с которой
    собраны программы), GLOBALIZER_MPIEXEC, I_MPI_ROOT (Intel MPI), MSMPI_BIN, стандартные каталоги
    установки и лишь затем PATH: в PATH может оказаться чужой mpiexec (например, из anaconda)."""
    candidates = [explicit, os.environ.get("GLOBALIZER_MPIEXEC")]
    exe = "mpiexec" + EXE_SUFFIX
    if os.environ.get("I_MPI_ROOT"):
        candidates.append(os.path.join(os.environ["I_MPI_ROOT"], "bin", exe))
    if os.environ.get("MSMPI_BIN"):
        candidates.append(os.path.join(os.environ["MSMPI_BIN"], exe))
    if IS_WINDOWS:
        for root in (r"C:\Program Files (x86)\Intel\oneAPI\mpi\latest", r"C:\Program Files\Microsoft MPI"):
            candidates.append(os.path.join(root, "bin", exe))
    candidates.append(shutil.which("mpiexec"))
    for c in candidates:
        if c and os.path.isfile(c):
            return os.path.abspath(c)
    return None


def make_env(mpiexec):
    """Окружение запуска: каталог mpiexec в PATH (там же impi.dll / msmpi.dll), I_MPI_ROOT для Intel MPI."""
    env = dict(os.environ)
    bindir = os.path.dirname(mpiexec)
    env["PATH"] = bindir + os.pathsep + env.get("PATH", "")
    if IS_WINDOWS and "oneapi" in mpiexec.lower() and not env.get("I_MPI_ROOT"):
        env["I_MPI_ROOT"] = os.path.dirname(bindir)
    env.setdefault("MPLBACKEND", "Agg")
    return env


# --------------------------------------------------------------------------- #
# Запуск с таймаутом. При зависании нужно завершить ВСЕ процессы задания:
# Intel MPI запускает процессы через hydra_pmi_proxy, и taskkill /T от mpiexec
# до них не доходит (после гибели mpiexec они остаются висеть). Поэтому на
# Windows mpiexec помещается в Job Object (KILL_ON_JOB_CLOSE) - потомки
# наследуют задание и завершаются вместе с ним; в Unix - своя группа процессов.
# Вывод пишется во временный файл, а не в канал: уцелевший процесс не
# заблокирует раннер.
# --------------------------------------------------------------------------- #
class _Job:
    """Job Object Windows: TerminateJobObject завершает процесс и всех его потомков."""

    def __init__(self):
        self.handle = None
        if not IS_WINDOWS:
            return
        try:
            import ctypes
            from ctypes import wintypes

            class _Basic(ctypes.Structure):
                _fields_ = [("PerProcessUserTimeLimit", ctypes.c_int64), ("PerJobUserTimeLimit", ctypes.c_int64),
                            ("LimitFlags", wintypes.DWORD), ("MinimumWorkingSetSize", ctypes.c_size_t),
                            ("MaximumWorkingSetSize", ctypes.c_size_t), ("ActiveProcessLimit", wintypes.DWORD),
                            ("Affinity", ctypes.c_size_t), ("PriorityClass", wintypes.DWORD),
                            ("SchedulingClass", wintypes.DWORD)]

            class _Io(ctypes.Structure):
                _fields_ = [(n, ctypes.c_uint64) for n in ("ReadOperationCount", "WriteOperationCount",
                            "OtherOperationCount", "ReadTransferCount", "WriteTransferCount", "OtherTransferCount")]

            class _Ext(ctypes.Structure):
                _fields_ = [("BasicLimitInformation", _Basic), ("IoInfo", _Io),
                            ("ProcessMemoryLimit", ctypes.c_size_t), ("JobMemoryLimit", ctypes.c_size_t),
                            ("PeakProcessMemoryUsed", ctypes.c_size_t), ("PeakJobMemoryUsed", ctypes.c_size_t)]

            k32 = ctypes.WinDLL("kernel32", use_last_error=True)
            k32.CreateJobObjectW.restype = wintypes.HANDLE
            k32.OpenProcess.restype = wintypes.HANDLE
            self._k32 = k32
            h = k32.CreateJobObjectW(None, None)
            if not h:
                return
            info = _Ext()
            info.BasicLimitInformation.LimitFlags = 0x2000          # JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE
            k32.SetInformationJobObject(wintypes.HANDLE(h), 9, ctypes.byref(info), ctypes.sizeof(info))
            self.handle = h
        except Exception:                                    # noqa: BLE001
            self.handle = None

    def assign(self, pid):
        if not self.handle:
            return
        from ctypes import wintypes
        hp = self._k32.OpenProcess(0x0101, False, pid)          # PROCESS_TERMINATE | PROCESS_SET_QUOTA
        if hp:
            self._k32.AssignProcessToJobObject(wintypes.HANDLE(self.handle), wintypes.HANDLE(hp))
            self._k32.CloseHandle(wintypes.HANDLE(hp))

    def terminate(self):
        if self.handle:
            from ctypes import wintypes
            self._k32.TerminateJobObject(wintypes.HANDLE(self.handle), 1)

    def close(self):
        if self.handle:
            from ctypes import wintypes
            self._k32.CloseHandle(wintypes.HANDLE(self.handle))
            self.handle = None


def kill_tree(proc, job):
    if IS_WINDOWS:
        job.terminate()
        subprocess.run(["taskkill", "/T", "/F", "/PID", str(proc.pid)],
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    else:
        try:
            os.killpg(proc.pid, 9)
        except Exception:                                    # noqa: BLE001
            proc.kill()


def run_cmd(cmd, cwd, env, timeout):
    t0 = time.time()
    kwargs = {}
    if not IS_WINDOWS:
        kwargs["start_new_session"] = True
    job = _Job()
    with tempfile.TemporaryFile() as out:
        proc = subprocess.Popen(cmd, cwd=cwd, env=env, stdout=out, stderr=subprocess.STDOUT,
                                stdin=subprocess.DEVNULL, **kwargs)
        # mpiexec ещё не успел запустить процессы MPI: они унаследуют задание
        job.assign(proc.pid)
        try:
            proc.wait(timeout=timeout)
            timed_out = False
        except subprocess.TimeoutExpired:
            kill_tree(proc, job)
            proc.wait()
            timed_out = True
        job.close()
        out.seek(0)
        text = out.read().decode("utf-8", errors="replace")
    return proc.returncode, text, timed_out, time.time() - t0


# --------------------------------------------------------------------------- #
# Разбор вывода PrintResultToConsole
# --------------------------------------------------------------------------- #
def parse_result(text):
    res = {}
    m = re.findall(r"Minimum value\s*=\s*(" + _FLOAT + ")", text)
    if m:
        res["f"] = float(m[-1])
    m = re.findall(r"^Iterations\s*=\s*(\d+)", text, flags=re.MULTILINE)
    if m:
        res["iterations"] = int(m[-1])
    # "\tObjective function\t0\tcalculations =\t226" / "Constraint 0 ... calculations ="
    calcs = [int(v) for v in re.findall(r"calculations\s*=\s*(\d+)", text)]
    if calcs:
        res["calculations"] = calcs
        res["trials"] = calcs[0]          # первая функция вычисляется в каждом испытании
    return res


def bad_termination(text):
    return "BAD TERMINATION" in text or "job aborted" in text.lower()


# --------------------------------------------------------------------------- #
# Один тест
# --------------------------------------------------------------------------- #
def build_args(case, defaults):
    args = list(defaults.get("args", [])) + list(case.get("args", []))
    mode = case.get("mode")
    if mode:
        args += ["-TypeCalculation", mode]
    return [str(a) for a in args]


def run_case(case, defaults, bin_dir, mpiexec, env, mpiexec_extra, serial_cache, verbose):
    exe_name = case.get("exe", defaults.get("exe", "GlobalizerMain"))
    exe = bin_dir / (exe_name + EXE_SUFFIX)
    if not exe.is_file():
        return "SKIP", f"нет {exe.name} в {bin_dir}", {}
    for lib in case.get("requires_libs", []):
        if not any((bin_dir / f"{p}{lib}{e}").is_file() for p in ("", "lib") for e in (".dll", ".so")):
            return "SKIP", f"нет библиотеки задачи {lib}", {}

    timeout = case.get("timeout", defaults.get("timeout", 300))
    args = build_args(case, defaults)
    procs = int(case.get("procs", 1))
    if procs > 0:
        cmd = [mpiexec, "-n", str(procs)] + mpiexec_extra + [str(exe)] + args
    else:                                  # procs = 0: без mpiexec
        cmd = [str(exe)] + args
    rc, text, timed_out, dt = run_cmd(cmd, str(bin_dir), env, timeout)
    res = parse_result(text)
    res["time"] = dt
    if verbose:
        print("    $ " + " ".join(shlex.quote(c) for c in cmd))
        for line in text.splitlines()[-25:]:
            print("    | " + line)

    if timed_out:
        return "FAIL", f"зависание: не завершился за {timeout} с", res
    if rc != 0 or bad_termination(text):
        tail = " / ".join(l.strip() for l in text.splitlines()[-4:] if l.strip())
        return "FAIL", f"код выхода {rc}: {tail[:300]}", res
    if "f" not in res or "trials" not in res:
        return "FAIL", "в выводе нет результата (Minimum value / calculations)", res

    exp = case.get("expect", {})
    problems = []
    if "optimum" in exp:
        tol = exp.get("tolerance", 0.01)
        if abs(res["f"] - exp["optimum"]) > tol:
            problems.append(f"f={res['f']:g} вне допуска |f-{exp['optimum']:g}|<={tol:g}")
    if "trials" in exp and res["trials"] != exp["trials"]:
        problems.append(f"trials={res['trials']} (эталон {exp['trials']})")
    if "max_trials" in exp and res["trials"] >= exp["max_trials"]:
        problems.append(f"trials={res['trials']} >= {exp['max_trials']} (остановка по лимиту, а не по точности)")
    if "points_per_iteration" in exp and res.get("iterations"):
        ppi = exp["points_per_iteration"]
        # первая итерация и итерация остановки могут отличаться - допуск в одну итерацию с каждой стороны
        lo, hi = ppi * (res["iterations"] - 2), ppi * (res["iterations"] + 1)
        if not (lo <= res["trials"] <= hi):
            problems.append(f"trials={res['trials']} при {res['iterations']} итерациях: не {ppi} точки на итерацию")
    if exp.get("same_as_serial"):
        # Обычный последовательный запуск: те же параметры, но без mpiexec и без -TypeCalculation
        serial_args = build_args(dict(case, mode=None), defaults)
        key = (exe_name, tuple(serial_args))
        if key not in serial_cache:
            src, stext, sto, _ = run_cmd([str(exe)] + serial_args, str(bin_dir), env, timeout)
            serial_cache[key] = (src, sto, parse_result(stext))
        src, sto, sres = serial_cache[key]
        if src != 0 or sto or "trials" not in sres:
            problems.append(f"последовательный запуск без mpiexec не удался (код {src})")
        else:
            res["serial_trials"] = sres["trials"]
            if sres["trials"] != res["trials"] or abs(sres["f"] - res["f"]) > 1e-12:
                problems.append(f"trials={res['trials']}, f={res['f']:g}; без MPI: trials={sres['trials']}, f={sres['f']:g}")

    msg = f"f={res['f']:g} trials={res['trials']}"
    if res.get("iterations"):
        msg += f" it={res['iterations']}"
    if "serial_trials" in res:
        msg += f" (без MPI: {res['serial_trials']})"
    msg += f"  {dt:.1f}s"
    if problems:
        return "FAIL", msg + "  " + "; ".join(problems), res
    return "PASS", msg, res


# --------------------------------------------------------------------------- #
def main():
    ap = argparse.ArgumentParser(description="Интеграционные тесты MPI-версии Globalizer")
    ap.add_argument("--bin-dir", default=str(SCRIPT_DIR.parent / "_bin"), help="каталог программ и библиотек задач")
    ap.add_argument("--references", default=str(REFERENCES))
    ap.add_argument("--group", action="append", help="только эти группы (можно несколько раз)")
    ap.add_argument("--filter", help="только тесты, в имени которых есть эта подстрока")
    ap.add_argument("--mpiexec", help="путь к mpiexec")
    ap.add_argument("--max-procs", type=int, default=int(os.environ.get("GLOBALIZER_MPI_MAX_PROCS", "0")),
                    help="пропускать тесты, которым нужно больше процессов (0 - без ограничения)")
    ap.add_argument("--repeat", type=int, default=1, help="повторить каждый тест N раз (поиск редких сбоев)")
    ap.add_argument("--verbose", action="store_true")
    a = ap.parse_args()

    try:
        refs = json.loads(Path(a.references).read_text(encoding="utf-8"))
    except Exception as e:                                   # noqa: BLE001
        print(f"Не удалось прочитать {a.references}: {e}")
        return 2
    defaults = refs.get("defaults", {})
    cases = refs.get("tests", [])
    if a.group:
        cases = [c for c in cases if c.get("group") in a.group]
    if a.filter:
        cases = [c for c in cases if a.filter in c["name"]]
    if not cases:
        print("Нет тестов для запуска")
        return 2

    bin_dir = Path(a.bin_dir).resolve()
    print("=" * 78)
    print(f"  GLOBALIZER MPI INTEGRATION TESTS  ({len(cases)} тестов, {bin_dir})")
    print("=" * 78)

    mpiexec = find_mpiexec(a.mpiexec)
    if not mpiexec:
        for c in cases:
            print(f"  [SKIP] {c['name']:<44} нет mpiexec (I_MPI_ROOT / MSMPI_BIN / PATH / --mpiexec)")
        print("Total: 0 PASS, 0 FAIL, %d SKIP" % len(cases))
        return SKIP_RC
    print(f"  mpiexec: {mpiexec}")
    env = make_env(mpiexec)
    extra = shlex.split(os.environ.get("GLOBALIZER_MPIEXEC_ARGS", ""), posix=not IS_WINDOWS)

    counts = {"PASS": 0, "FAIL": 0, "SKIP": 0}
    serial_cache = {}
    for c in cases:
        if a.max_procs and int(c.get("procs", 1)) > a.max_procs:
            counts["SKIP"] += 1
            print(f"  [SKIP] {c['name']:<44} нужно {c['procs']} процессов (--max-procs {a.max_procs})")
            continue
        repeat = max(a.repeat, int(c.get("repeat", 1)))     # асинхронная схема недетерминирована
        issue_printed = False
        for k in range(repeat):
            status, msg, _ = run_case(c, defaults, bin_dir, mpiexec, env, extra, serial_cache, a.verbose)
            counts[status] += 1
            rep = f" #{k + 1}" if repeat > 1 else ""
            print(f"  [{status}] {c['name'] + rep:<44} {msg}", flush=True)
            if status == "FAIL" and c.get("issue") and not issue_printed:
                print(f"         ^ проверяется: {c['issue']}", flush=True)
                issue_printed = True
            if status == "SKIP":
                break

    print("=" * 78)
    print(f"Total: {counts['PASS']} PASS, {counts['FAIL']} FAIL, {counts['SKIP']} SKIP")
    print("=" * 78)
    if counts["FAIL"]:
        return 1
    if counts["PASS"] == 0:
        return SKIP_RC
    return 0


if __name__ == "__main__":
    sys.exit(main())
