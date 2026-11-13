# Librería Estándar — Lenguaje Hermético

**Migradas:** 2026-10-16 (Fase 0, plan de reforma).
**Origen:** `proyecto de IA artistica/Lenguaje Hermetico/.gitignored/libreria/` (copia local sin git, creada 2026-07-13).
**Spec:** `docs/11-protocolo-librerias.md` (árbol ontológico de 8 capas).

## Estado del test (2026-11-04, `build/herm.exe` con DL-14 + DL-26)

Test: cada librería compilada directamente con `herm.exe` (`build\herm.exe libreria\<lib>.hm`), exit code 0.

**Resultado: 55/55 compilan. 0 fallan.** Barrido completo en ~1 s.

### Historial

| Fecha | Resultado | Nota |
|-------|-----------|------|
| 2026-10-16 | 35/55 | Tras DL-14; los 20 fallos eran sintaxis rechazada por diseño |
| 2026-11-04 | **55/55** | Reescritura manual de las 20 restantes, sin tocar el lenguaje |

### Compilan (55)

| Capa | Librerías |
|------|-----------|
| 1 Matemáticas (6/6) | `std_math_core`, `algebra`, `calculus`, `noise`, `dynamics`, `probability` |
| 2 Física (10/10) | `std_phys_sdf`, `kinematics`, `quantum`, `qed`, `qcd`, `qft`, `relativity`, `statmech`, `thermo`, `condensed` |
| 3-4 Química (6/11) | `std_chem_atoms`, `bonds`, `reactions`, `organic`, `polymer`, `crystal` |
| Puente→Bio (0/13) | — ninguna compila |
| 6 Biología (6/8) | `std_bio_cell`, `ecology`, `evolution`, `neuro`, `physiology`, `systems` |
| 7 Geociencias (4/4) | `std_geo_atmosphere`, `landscape`, `ocean`, `terrain` |
| 8 Astrofísica (3/3) | `std_astro_mechanics`, `stellar`, `cosmology` |

### Reescritas (20) — cerrado 2026-11-04

Los 20 fallos eran sintaxis que el lenguaje **rechaza por diseño** (DL-13/DR-03). Se reescribieron a mano sin añadir tokens ni construcciones:

- **`for` dentro de `fn`** (2): `std_bridge_quantchem`, `std_chem_theoretical` → `def`/`instance` repetidos y sumas explícitas.
- **`if`/`else`** (3): `std_bridge_biophysics`, `std_bridge_enzymology`, `std_chem_coordination` → `mix`/`step`/`smoothstep`.
- **Keyword como identificador** (8): `alpha`, `position`, `volume`, `union`, `roughness`, `sum`, `spot_amplitude` → renombrados (params/vars) en `std_bridge_biochem`, `electrochem`, `genomics`, `statmech_mol`, `chembio`, `physchem`, `chem_optics`, `bio_development`.
- **`let` dentro de cuerpo de `node`** (3): `std_bridge_atomic`, `molbio`, `spectral` → movidos a `fn` o a nivel superior.
- **Otros** (4): `std_bio_genetics` (coma espuria), `std_bridge_kinetics` (`q_ratio`), `std_chem_inorganic`/`std_chem_supramolecular` (llaves desbalanceadas).

Detalle por librería en `docs/librerias/*.md`.

### RAM bomb

`v_lets.herm` (6 libs importadas + 31 `let` que invocan sus funciones): **peak 2,5–5,3 MB, 47–204 ms** (medido 2026-11-12 con DL-09 A+B; el poll de 5 ms no captura el pico exacto). Histórico: **peak 1568 MB, 12.973 ms** (2026-10-16). Causa: el resolver expandía `fn`/`let` por sustitución textual sin memoria. **DL-09 A+B implementado** (plegado constante y memoización de `fn`, con límite ruidoso de 16 MB por expansión); C (CSE) queda diferido porque el RIH no tiene tabla de referencias. El objetivo < 100 MB está cumplido; `domain_warp2d` supera el límite y falla con error explícito.

## Contenido

- `std_*.hm` — 55 librerías estándar (8 capas del árbol ontológico).
- `tests/` — 44 `.herm` de prueba migrados del origen (más `verify_optics.hm`).
  - Los tests que importan `std_*` requieren que el compilador los encuentre: ejecutar `herm.exe` con cwd = raíz de la repo (usa `cwd/libreria/`) o tener la carpeta en el path de imports.

## Fuera de `libreria/`

- `materiales.hm` — **archivado** en `archived/libreria/materiales.hm` (DL-27, 2026-11-04). Catálogo legado pre-tensor: usa `ior`/`transmission`, campos que `parse_material` no acepta (el diseño tensorial 1x8 los sustituyó, v0.14). No compila y no forma parte del protocolo; queda excluido del barrido 55/55.

## Cómo testear una librería

```bat
:: desde la raíz de la repo:
build\herm.exe libreria\tests\test_calculus_simple.herm -o salida
```
