# std_chem_organic.hm

**Capa:** 3.4 — Química Orgánica y Biomoléculas
**Versión:** 0.2
**Fuente:** Geometría sp3/sp2/sp, cadenas carbonadas, SMILES-SDF
**Dependencias:** std_math_core.hm, std_math_algebra.hm, std_chem_bonds.hm
**Fecha:** 6 de junio de 2026

## Cómo
Define constantes de radios de Van der Waals y longitudes de enlace para C, H, O, N, S. Proporciona SDFs de grupos funcionales orgánicos canónicos (metilo, etileno, benceno), cadenas lineales, hélices alfa, láminas beta, y funciones de propiedades fisicoquímicas (LogP, peso molecular, regla de Lipinski).

## Por qué
Habilita el modelado de moléculas orgánicas y biomoléculas (proteínas, carbohidratos). La química orgánica es central para fármacos, materiales y sistemas biológicos.

## Para qué
- Construir visualmente moléculas orgánicas (metano, etileno, benceno)
- Modelar estructuras secundarias de proteínas (hélice alfa, lámina beta)
- Calcular propiedades drug-like (LogP, peso molecular, Lipinski)
- Servir como base para química supramolecular y polímeros

## Funciones principales
| Función | Descripción |
|---|---|
| `logP_approx(nC, nO, nN, nS)` | LogP por modelo Crippen simplificado |
| `mol_weight(nC, nH, nO, nN, nS)` | Peso molecular aproximado |
| `lipinski_score(MW, LogP, HBD, HBA)` | Regla de Lipinski (druglikeness) |

## SDFs definidas
| SDF | Descripción |
|---|---|
| `methyl_group(sz)` | Grupo metilo CH₄ tetraédrico |
| `ethylene(sz)` | Grupo etileno C=C planar sp2 |
| `benzene(r_ring, tube_r)` | Anillo aromático de 6 carbonos |
| `sixmember_ring(r_ring, tube_r)` | Anillo de 6 miembros (piranosa) |
| `linear_chain(n, bond_len, tube_r)` | Cadena de carbonos lineal (n-alcano) |
| `alpha_helix(r, pitch, tube_r, t_start, t_end)` | Hélice alfa de proteína |
| `beta_sheet(width, wave_amp, wave_freq, thickness)` | Lámina beta con pliegues |

## Notas
Radios y longitudes de enlace en nanómetros. La hélice alfa usa la variable w para animación. La regla de Lipinski retorna 1 si los 4 criterios se cumplen (MW ≤ 500, LogP ≤ 5, HBD ≤ 5, HBA ≤ 10).
