#!/usr/bin/env python3
"""Genera el corpus de paridad de 2.0c.3d.

Para cada libreria .hm:
  - extrae las firmas `fn "nombre"(params)`;
  - genera una escena con un nodo por funcion: sphere(abs(fn(args)) + 0.5);
  - compila cada funcion por separado con build/herm.exe --ont;
  - conserva las que compilan y genera un .ont por libreria + un .map
    (indice de nodo | nombre de funcion).

Los argumentos dependen del punto (x/y/z/w) para que glslc no pliegue la
funcion entera a una constante: el test debe medir los opcodes en ejecucion,
no el plegador del compilador GLSL. En la CPU el mismo bytecode calcula los
mismos argumentos.

Uso (desde la raiz de lenguaje-hermetico):
    python libreria/gen_parity_corpus.py [--out DIR]
"""
import argparse
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
LIBDIR = ROOT / "libreria"
HERM = ROOT / "build" / "herm.exe"

ARGS = [
    "0.7 + x",
    "1.3 + y",
    "0.4 + z",
    "2.1 + w",
    "0.9 + x * 0.5",
    "1.7 + y * 0.5",
    "0.2 + z * 0.5",
    "2.5 + w * 0.5",
]

SCENE_HEAD = """import "{lib}.hm";

scene "parity_{lib}" {{
    axes {{ X: 8, Y: 8, W: 1 }}
    camera {{ position: [0, 0, 3]; target: [0, 0, 0]; fov: 50; }}
    background: [0.05, 0.05, 0.1];
}}

material "m" {{
    base_color: [0.8, 0.8, 0.8];
    roughness: 0.5;
    metallic: 0.0;
}}
"""


def fn_signatures(text):
    out = []
    for m in re.finditer(r'fn\s+"([^"]+)"\s*\(([^)]*)\)', text):
        name = m.group(1)
        params = [p.strip() for p in m.group(2).split(",") if p.strip()]
        out.append((name, params))
    return out


def scene_text(lib, funcs):
    parts = [SCENE_HEAD.format(lib=lib)]
    for i, (name, params) in enumerate(funcs):
        args = ", ".join(ARGS[:len(params)])
        parts.append(
            f'\nnode "f{i}" {{\n'
            f'    sdf: sphere(abs({name}({args})) + 0.5);\n'
            f'    material: "m";\n'
            f'}}\n'
        )
    return "".join(parts)


def compile_scene(lib, funcs, workdir, outdir):
    src = workdir / f"{lib}.herm"
    src.write_text(scene_text(lib, funcs), encoding="utf-8")
    out = outdir / lib
    proc = subprocess.run(
        [str(HERM), str(src), "--ont", "-o", str(out)],
        cwd=str(ROOT), capture_output=True, text=True, errors="replace",
    )
    return proc.returncode == 0, (proc.stdout + proc.stderr).strip()


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", default=str(ROOT / "build" / "parity_corpus"))
    args = ap.parse_args()

    outdir = Path(args.out)
    workdir = outdir / "scenes"
    outdir.mkdir(parents=True, exist_ok=True)
    workdir.mkdir(parents=True, exist_ok=True)

    if not HERM.exists():
        sys.exit(f"no existe {HERM}; compila herm/build.bat primero")

    summary = []
    maps = {}

    for hm in sorted(LIBDIR.glob("*.hm")):
        lib = hm.stem
        funcs = fn_signatures(hm.read_text(encoding="utf-8", errors="replace"))
        if not funcs:
            summary.append((lib, 0, 0, "sin fn"))
            continue

        good = []
        bad = []
        for name, params in funcs:
            ok, log = compile_scene(lib, [(name, params)], workdir, outdir)
            if ok:
                good.append((name, params))
            else:
                reason = " ".join(log.split())[:220]
                bad.append((name, len(params), reason))

        if good:
            ok, log = compile_scene(lib, good, workdir, outdir)
            if not ok:
                # Interaccion entre funciones: conservar solo las que compilan
                # juntas mediante biseccion.
                print(f"  {lib}: fallo el grupo, bisecando...")
                good = bisect_subset(lib, good, workdir, outdir)
                ok, log = compile_scene(lib, good, workdir, outdir)
            if ok:
                maps[lib] = [g[0] for g in good]
                summary.append((lib, len(good), len(bad), ""))
            else:
                summary.append((lib, 0, len(funcs), "grupo no compila"))
        else:
            summary.append((lib, 0, len(bad), "ninguna funcion compila"))

        for name, nparams, reason in bad:
            print(f"  EXCLUIDA {lib}:{name}/{nparams} -- {reason}")

    # .map por libreria
    for lib, names in maps.items():
        (outdir / f"{lib}.map").write_text(
            "\n".join(f"{i}|{n}" for i, n in enumerate(names)) + "\n",
            encoding="utf-8",
        )
        # Renombrar el .ont final (compile_scene ya escribio lib.ont)
        ont = outdir / f"{lib}.ont"
        if not ont.exists():
            print(f"  AVISO: no se genero {ont}")

    # Resumen
    lines = []
    lines.append(f"{'libreria':32} {'ok':>4} {'excl':>5}  nota")
    for lib, ok_n, bad_n, note in summary:
        lines.append(f"{lib:32} {ok_n:4d} {bad_n:5d}  {note}")
    total_ok = sum(s[1] for s in summary)
    total_bad = sum(s[2] for s in summary)
    lines.append("")
    lines.append(f"TOTAL: {total_ok} funciones compiladas, {total_bad} excluidas, "
                 f"{sum(1 for s in summary if s[1] > 0)}/{len(summary)} librerias con corpus")
    text = "\n".join(lines)
    (outdir / "summary.txt").write_text(text + "\n", encoding="utf-8")
    print("\n" + text)


def bisect_subset(lib, funcs, workdir, outdir):
    """Devuelve el mayor subconjunto compilable por division recursiva."""
    ok, _ = compile_scene(lib, funcs, workdir, outdir)
    if ok:
        return funcs
    if len(funcs) == 1:
        return []
    mid = len(funcs) // 2
    return (bisect_subset(lib, funcs[:mid], workdir, outdir) +
            bisect_subset(lib, funcs[mid:], workdir, outdir))


if __name__ == "__main__":
    main()
