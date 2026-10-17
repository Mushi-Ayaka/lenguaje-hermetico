# Librería Estándar — Lenguaje Hermético

**Migradas:** 2026-10-16 (Fase 0, plan de reforma).
**Origen:** `proyecto de IA artistica/Lenguaje Hermetico/.gitignored/libreria/` (copia local sin git, creada 2026-07-13).
**Spec:** `docs/11-protocolo-librerias.md` (árbol ontológico de 8 capas).

## Estado del test (2026-10-16, herm.exe con DL-14)

Test: cada librería importada en una escena mínima (`import + sphere`), compilada con `herm.exe`.

**Resultado: 35/55 compilan. 20/55 fallan por sintaxis. 1 test combinado explota en RAM.**

### Compilan (35)

| Capa | Librerías |
|------|-----------|
| 1 Matemáticas (6/6) | `std_math_core`, `algebra`, `calculus`, `noise`, `dynamics`, `probability` |
| 2 Física (10/10) | `std_phys_sdf`, `kinematics`, `quantum`, `qed`, `qcd`, `qft`, `relativity`, `statmech`, `thermo`, `condensed` |
| 3-4 Química (6/11) | `std_chem_atoms`, `bonds`, `reactions`, `organic`, `polymer`, `crystal` |
| Puente→Bio (0/13) | — ninguna compila |
| 6 Biología (6/8) | `std_bio_cell`, `ecology`, `evolution`, `neuro`, `physiology`, `systems` |
| 7 Geociencias (4/4) | `std_geo_atmosphere`, `landscape`, `ocean`, `terrain` |
| 8 Astrofísica (3/3) | `std_astro_mechanics`, `stellar`, `cosmology` |

### Fallan (20) — se reescriben a mano (pendiente, decisión del usuario)

Tras DL-14 (parser acepta `let` internos en `fn`), los 20 errores restantes son de sintaxis que el lenguaje **rechaza por diseño** (DL-13/DR-03) y que se arreglan reescribiendo las librerías:

- **`for` dentro de `fn`** (2): `std_bridge_quantchem`, `std_chem_theoretical` (anidados) → convertir a `def/instance` repetidos.
- **`if`/`else`** (3): `std_bridge_biophysics`, `std_bridge_enzymology`, `std_chem_coordination` → `mix`/`step`/`smoothstep`.
- **Keyword como identificador** (8): `alpha`, `position`, `volume`, `union`, `roughness`, `sum`, `spot_amplitude` → renombrar params/vars (`std_bridge_biochem`, `electrochem`, `genomics`, `statmech_mol`, `chembio`, `physchem`, `chem_optics`, `bio_development`).
- **`let` dentro de cuerpo de `node`** (3): `std_bridge_atomic`, `molbio`, `spectral` → mover lets a `fn` o top-level.
- **Otros** (4): `std_bio_genetics` (coma rara L173), `std_bridge_kinetics` (`q_ratio`), `std_chem_inorganic`/`std_chem_supramolecular` (llaves desbalanceadas → EOF).

### RAM bomb

`v_lets.herm` (6 libs importadas + 31 `let` que invocan sus funciones): **peak 1568 MB, 12,973 ms** (medido 2026-10-16 con poll de 5 ms por pid). Causa: el resolver expande `fn`/`let` por sustitución textual sin memoización ni CSE (DL-09 A+B+C, pendiente, objetivo < 100 MB).

## Contenido

- `std_*.hm` — 55 librerías estándar (8 capas del árbol ontológico).
- `materiales.hm` — catálogo de 55 materiales PBR (pre-existente, no forma parte del protocolo).
- `tests/` — 44 `.herm` de prueba migrados del origen (más `verify_optics.hm`).
  - Los tests que importan `std_*` requieren que el compilador los encuentre: ejecutar `herm.exe` con cwd = raíz de la repo (usa `cwd/libreria/`) o tener la carpeta en el path de imports.

## Cómo testear una librería

```bat
:: desde la raíz de la repo:
build\herm.exe libreria\tests\test_calculus_simple.herm -o salida
```
