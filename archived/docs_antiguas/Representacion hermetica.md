# 1. El Espacio de Estado: Definición del Cosmos ($\mathcal{C}$)

El universo no es una cuadrícula finita, sino un campo vectorial continuo en un espacio de cuatro dimensiones.

Definimos el dominio espacio**temporal (la geometría) como $\mathbb{P}$ y el codominio de manifestación (la luz y la masa) como $\mathbb{S}$.

* **Vector** de Posición (Dominio):** $\mathbf{p} = [X, Y, Z, W] \in \mathbb{R}^4$
* **Vector** de Estado (Codominio):** $\mathbf{s}$ puede ser un vector RGBA simple o una estructura rica que combina geometría y materia.

El Cosmos $\mathcal{C}$ es la función fundamental que mapea cada punto del espacio-tiempo a su estado interno:

$$\mathcal{C}: \mathbb{R}^4 \rightarrow \mathcal{S}$$

$$\mathcal{C}(\mathbf{p}) = \mathbf{s}$$

Donde $\mathcal{S}$ puede ser:

* un vector simple $[R, G, B, A]$ (salida colapsada)
* una estructura interna $[d, m, \mathbf{c}, A, P]$ con distancia y materiales
* un registro completo con propiedades físicas y químicas

En una arquitectura limpia, el campo interno es una capa rica y consistente, y la representación $1 \times 8$ es solo una interfaz de salida hacia GTE.

La matriz de $1 \times 8$ que diseñaste debe entenderse como:

* una forma de empaquetar el estado visible para el motor de render,
* no como la estructura de datos interna que define el universo.

$$\mathbf{M}_{obs} = \begin{bmatrix} X & Y & Z & W & R & G & B & A \end{bmatrix}$$

La relación entre representación interna y salida es:

$$\mathbf{s}_{internal} = \mathcal{C}(\mathbf{p})$$
$$\mathbf{s}_{output} = \text{collapse}(\mathbf{s}_{internal}) = [R, G, B, A]$$

En términos claros:

* `internal` es el estado semántico del universo:
  $$\mathbf{s}_{internal} = (d, m, \mathbf{c}, A, P)$$
* `output` es la interfaz de renderizado:
  $$\mathbf{s}_{output} = \text{collapse}(\mathbf{s}_{internal})$$

Mantener esta distinción evita confundir la representación visible con la “verdad” del campo.

---

## 2. Los Tres Operadores Fundamentales (Las Transformaciones)

Dado que el universo está pre-poblado por el Vacío ($\mathbf{s} = \mathbf{0}$ para todo $\mathbf{p}$), la "creación" se define mediante operadores matemáticos que alteran el campo $\mathcal{C}$.

### A. Operador de Perturbación ($\mathcal{P}$)

Inyecta un estado no nulo en una región espacial. Define una relación donde el estado interno $\mathbf{s}$ pasa a depender de una función generatriz $f(\mathbf{p})$:

$$\mathcal{P}(\mathcal{C}): \mathbf{s} = f(X, Y, Z, W) \in \mathcal{S}$$

En un campo estructurado, $f$ puede devolver distancia, material, color, opacidad y propiedades físicas al mismo tiempo.

### B. Operador de Influencia ($\mathcal{I}$)

Modifica una perturbación existente aplicando una transformación sobre el estado completo. En un modelo estructurado, $\mathbf{T}$ puede afectar no solo el color, sino también el material, la densidad y otras propiedades:

$$\mathcal{I}(\mathcal{C}): \mathbf{s}_{nueva} = \operatorname{transform}(\mathbf{T}, \mathbf{s}_{previa})$$

Para un estado simple $[R,G,B,A]$, esto puede reducirse a una multiplicación de canales. Para un estado rico, un ejemplo concreto es:

$$\mathbf{s}_{previa} = (d, m, \mathbf{c}, A, P)$$
$$\mathbf{T} = (\Delta\mathbf{c}, \Delta A, m_{new}, P_{update})$$

$$\operatorname{transform}(\mathbf{T}, \mathbf{s}_{previa}) = (d, m', \mathbf{c} + \Delta\mathbf{c}, A + \Delta A, P')$$

con

$$m' = \begin{cases} m_{new} & \text{si la transformación cambia el material} \\ m & \text{en otro caso} \end{cases}$$

$$P' = P \oplus P_{update}$$

Esto muestra que la transformación puede ser un módulo de cálculo de variables dentro de la pirámide, pero aplicado siempre sobre el estado interno estructurado.

### C. Operador de Aniquilación ($\mathcal{K}$)

Suprime la manifestación física sin alterar la coordenada. En un estado simple, equivale a anular el vector RGBA. En un estado estructurado, equivale a anular la ocupación/densidad y resetear la materia relevante:

$$\mathcal{K}(\mathcal{C}): \mathbf{s}_{nueva} = (d = +\infty, m = \emptyset, \mathbf{c} = [0,0,0], A = 0, P = \emptyset)$$

En el caso simplificado:

$$\mathcal{K}(\mathcal{C}): \mathbf{s}_{nueva} = \mathbf{s} \circ [0,0,0,0]^T$$

---

## 3. Leyes Tensoriales: Geometría Implícita y Densidad

Las entidades no tienen polígonos. Se definen formalmente mediante **Campos Escalares de Distancia ($\Phi$)**.

Para modelar un objeto (como una esfera o un cuerpo humano), definimos una función $\Phi: \mathbb{R}^4 \rightarrow \mathbb{R}$ que calcula la distancia de cualquier punto $\mathbf{p}$ a la frontera de la entidad.

La densidad (el canal $A$) del sistema se acopla a esta geometría mediante una función escalón o gradiente:

Para un sólido duro:

$$A(\mathbf{p}) = H(-\Phi(\mathbf{p})) = \begin{cases} 1 & \text{si } \Phi(X, Y, Z, W) \le 0 \\ 0 & \text{si } \Phi(X, Y, Z, W) > 0 \end{cases}$$

Para un volumen continuo:

$$A(\mathbf{p}) = \sigma(\Phi(\mathbf{p})) \in [0,1]$$

con $\sigma$ una función de densidad suave, por ejemplo una versión atenuada de la distancia.

En un modelo de superficie dura, $A$ actúa como máscara de ocupación; en un modelo volumétrico, $A$ es una densidad integrable que participa en la atenuación y la emisión del haz.

**Interacciones y Colisiones:**
Si dos campos $\mathcal{C}_1$ y $\mathcal{C}_2$ existen, su unión no es solo una `min` escalar, sino una operación estructurada sobre estados:

$$\mathcal{C}_{union}(\mathbf{p}) = \operatorname{select\_closer}(\mathcal{C}_1(\mathbf{p}), \mathcal{C}_2(\mathbf{p}))$$

donde para dos estados $s_i = (d_i, m_i, \mathbf{c}_i, A_i, P_i)$:

$$d_r = \min(d_1, d_2)$$

$$m_r = \begin{cases} m_1 & \text{si } d_1 < d_2 \\ m_2 & \text{si } d_2 < d_1 \end{cases}$$

$$\mathbf{c}_r = \begin{cases} \mathbf{c}_1 & \text{si } d_1 < d_2 \\ \mathbf{c}_2 & \text{si } d_2 < d_1 \end{cases}$$

$$A_r = \begin{cases} A_1 & \text{si } d_1 < d_2 \\ A_2 & \text{si } d_2 < d_1 \end{cases}$$

---

## 4. Leyes Temporales: La Cinemática como Geometría (Eje $W$)

En este modelo matemático no existe la derivada temporal clásica respecto a un reloj externo $\frac{d\mathbf{p}}{dt}$. El tiempo es una dimensión estática. El movimiento de un cuerpo se modela como una deformación del campo de distancia a lo largo del eje $W$.

Si una entidad está en la posición $\mathbf{x}_0$ y se desplaza a una velocidad constante $\mathbf{v}$, su centro $\mathbf{c}$ a lo largo del hipervolumen se define como:

$$\mathbf{c}(W) = \mathbf{x}_0 + \mathbf{v} \cdot W$$

La función de distancia para esa entidad móvil (por ejemplo, una esfera de radio $r$) se expresa entonces como una ecuación que involucra a las 4 dimensiones simultáneamente:

$$\Phi(X, Y, Z, W) = \left\| \begin{bmatrix} X \\ Y \\ Z \end{bmatrix} - \mathbf{c}(W) \right\| - r$$

---

## 5. El Teorema de Proyección

Aquí está el puente matemático definitivo que valida tu arquitectura. El universo se modela como un campo continuo en $\mathbb{R}^4$, y GTE consume una rebanada de ese campo en cada pixel y tiempo. El renderizador no necesita un producto cartesiano $\mathbb{R}^4 \times \mathbb{R}^4$, sino un mapeo desde posición 4D a un estado que luego se colapsa a RGBA.

Definimos el **Operador de Proyección Observacional $\mathcal{O}$**, que reduce nuestro campo abstracto al subdominio exigido por GTE.

Requiere dos parámetros:

1. **Matriz de Cámara ($\mathbf{C}$):** Mapea $(X, Y, Z) \rightarrow (x_{gte}, y_{gte})$.
2. **Función Cronológica ($g$):** Mapea $t_{gte} \rightarrow W$.

La ecuación de colapso final que el *Resolvedor* debe calcular y entregar a GTE es la composición de estas funciones y el colapso de estado a color:

$$F_{gte}(x, y, t) = \text{collapse}\left(\mathcal{C}\left( \mathbf{C}^{-1}(x, y), \, Z_{depth}, \, g(t) \right)\right)$$

### El Truco Analítico (Evaluación de Rayos / Depth)

Como $(x, y)$ en la pantalla representan infinitos puntos en el eje $Z$ del universo, la función matemática que resuelve el píxel debe evaluar una trayectoria en el campo 4D. Para un punto $(x, y)$ fijo y un tiempo $t$ fijo, el Resolvedor puede buscar el primer valor de $Z$ donde el campo de ocupación se hace relevante, o integrar la densidad si el medio es semi-transparente.

En la versión simplificada de superficie dura:

$$Z_{depth} = \min \{ Z \in \mathbb{R} \mid \mathcal{C}(x, y, Z, g(t)).A > 0 \}$$

En una versión continua y volumétrica:

$$T(Z) = \exp\left(-\int_{Z_0}^{Z} \mathcal{C}(x,y,z',g(t)).A\,dz'\right)$$

$$C_{final} = \int_{Z_0}^{\infty} T(z)\,\mathcal{C}(x,y,z,g(t)).\mathbf{c}\,\mathcal{C}(x,y,z,g(t)).A\,dz$$

Una vez definido el punto o la integración, los canales finales se extraen a partir del estado estructurado seleccionado:

$$(R, G, B, A)_{final} = \text{resolve}(\mathcal{C}(x, y, Z_{depth}, g(t)))$$

---

## 6. El Papel de RGBA y la Importancia del Canal A

En este modelo, `RGBA` no es simplemente un color fijo colocado en el universo. Es el valor final de manifestación que obtiene el renderizador después de decidir qué punto del campo 4D es visible.

* `R`, `G`, `B` representan la emisión o el material visible de un punto.
* `A` representa la densidad/ocupación del espacio en ese punto, es decir, si ese punto pertenece a una superficie/volumen real o si es vacío.

En un modelo de superficie dura, `A` se usa como máscara de ocupación. En un modelo volumétrico, `A` puede interpretarse como una densidad continua y la visibilidad se integra en el haz.

El canal `A` es crítico por varias razones:

1. `A` decide qué puntos son opacos y cuáles son transparentes o inexistentes desde el punto de vista óptico.
2. Sin `A`, el renderizador no puede distinguir entre un punto que forma parte de un objeto y un punto de vacío que simplemente tiene un color accidental.
3. `A` permite que el límite de la geometría se convierta en visibilidad. Es el canal que convierte la ambigüedad del campo implícito en una frontera concreta para el raycast.

En este sentido, el `RGBA` del tensor es una observación, no una entidad absoluta. La entidad real puede ser:

* un campo de distancia $\boldsymbol{\Phi}$
* un identificador de material o de región
* un valor de profundidad
* un canal `A` que marca la presencia física

Y `RGBA` es el colapso visible de esa entidad cuando se mira desde una cámara.

### ¿Por qué no basta con sólo `RGB`?

Porque `RGB` puede existir matemáticamente en puntos donde no hay objeto. El canal `A` es lo que transforma ese valor en un punto que “existe” ópticamente.

Un punto con `A = 0` puede tener `R,G,B` definidos, pero en el renderizado ese punto se comporta como vacío. Un punto con `A > 0` es un punto que puede bloquear rayos y alterar la visibilidad de lo que hay detrás.

### Ejemplo conceptual

* Un punto dentro de la silla: `A = 1`, `R,G,B` según el material.
* Un punto fuera de la silla: `A = 0`, `R,G,B` irrelevantes para visibilidad.

Esto explica por qué el `A` no es un “efecto óptico” opcional: es la variable de existencia visual del campo.

---

### Resumen del Modelo

Con esta formalización, tu propuesta deja de ser "una idea sobre variables" y se convierte en una **Topología Tensorial**. Has demostrado que mediante la manipulación de una sola matriz de 8 variables ($\mathbf{M}$) y utilizando funciones matemáticas cerradas, se puede calcular el estado lumínico y físico de cualquier punto de un universo 4D determinista, para luego ser colapsado en un formato consumible por un renderizador sin estado.

Tocar la tierra con un objeto compuesto como una silla es el ejercicio perfecto. Es aquí donde las teorías herméticas se rompen y muestran sus costuras.

Vamos a construir una silla utilizando estrictamente las herramientas matemáticas que hemos definido hasta ahora en nuestro modelo de $1 \times 8$ ($\mathbf{M} = [X, Y, Z, W, R, G, B, A]$), y veamos exactamente en qué punto el sistema se "queda corto" o se vuelve impracticable.

---

## 6. La Resolución con la Pirámide del Universo: Física, Química y Biología

Este modelo se fortalece si lo vemos como una pirámide de capas:

1. Física: espacio-temporal y visibilidad
2. Química: materia y propiedades de los tensores
3. Biología: sistemas emergentes y estructuras complejas

### Física: el dominio del movimiento y la óptica

La base física es el espacio-temporal continuo $\mathbb{P} = \mathbb{R}^4$ con la función central:

$$\mathcal{C}_f: \mathbb{R}^4 \rightarrow \{\Phi, A\}$$

Donde:

* $\Phi(X,Y,Z,W)$ describe la distancia implícita a una frontera.
* $A(X,Y,Z,W)$ describe la presencia/densidad del espacio.

La visibilidad se resuelve con el operador óptico que busca el primer $Z$ con $A > 0$ y proyecta al plano de imagen. La física responde a preguntas como:

* ¿Dónde hay objeto?
* ¿Qué puntos son visibles?
* ¿Cómo se transforman las coordenadas con matrices locales $T_{local} \in \mathbb{R}^{4 \times 4}$?

Formalmente, una transformación local para una entidad es:

$$\mathbf{p}_{mundo} = T_{local} \cdot \mathbf{p}_{local}$$

### Química: materia, interacción y propiedades tensoriales

La química extiende el codominio del sistema con metadata de materia y parámetros físicos:

$$\mathcal{C}_q: \mathbb{R}^4 \rightarrow [R, G, B, A, m, \rho, \kappa, \ldots]$$

Aquí se definen:

* $m$ = identificador de material o región
* $\rho$ = densidad interna
* $\kappa$ = reflectancia / rugosidad / absorción
* $R,G,B$ = color base dependiendo de la materia

La separación semántica entre geometría y materia se expresa mejor si el campo devuelve una estructura:

$$\mathcal{C}(\mathbf{p}) = (d, m, \mathbf{c}, A, \dots)$$

Donde

* $d$ = distancia al límite más cercano ($\Phi$)
* $m$ = material_id
* $\mathbf{c} = [R,G,B]$ = color base
* $A$ = densidad u opacidad local

Entonces, la unión de dos campos no se reduce a un solo `min`, sino a una operación estructurada:

$$d_{r} = \min(d_1, d_2)$$

$$m_{r} = \begin{cases} m_1 & \text{si } d_1 < d_2 \\ m_2 & \text{si } d_2 < d_1 \end{cases}$$

$$\mathbf{c}_{r} = \begin{cases} \mathbf{c}_1 & \text{si } d_1 < d_2 \\ \mathbf{c}_2 & \text{si } d_2 < d_1 \end{cases}$$

$$A_{r} = \begin{cases} A_1 & \text{si } d_1 < d_2 \\ A_2 & \text{si } d_2 < d_1 \end{cases}$$

Esto hace explícita la separación entre forma y material, y elimina la necesidad de un canal adicional ad hoc en el mismo tensor 1×8.

### Biología: sistemas y ensamblaje de entidades

La biología representa todo lo que emerge sobre la física y la química: estructuras compuestas, relaciones jerárquicas y comportamiento.

Una entidad biológica puede definirse como un conjunto:

$$E = \{\Phi_i, T_i, m_i, R_i\}$$

Donde cada $i$ es una subparte del sistema y $R_i$ son reglas de ensamblaje o dependencia.

En este nivel, se imponen relaciones padre-hijo y transformaciones conjuntas:

$$T_{hijo} = T_{padre} \cdot T_{local\_hijo}$$

Esto es esencial para que una silla funcione como un sistema:

* el respaldo se mueve con el asiento
* las patas siguen la misma transformación global
* la estructura compuesta mantiene coherencia temporal en $W$

### Subcampos de la física y campos entrelazados

La física no es un único bloque; tiene subcampos bien definidos que deben ser explícitos en la pirámide:

* Geometría implícita: cálculo de $\Phi$ y distancias en $\mathbb{R}^4$.
* Visibilidad óptica: selección de $\mathbf{p}_{visible}$ mediante operadores de corte y rayos.
* Transformaciones: matrices locales $T_{local}$, jerarquías padre-hijo y coordenadas globales.
* Dinámica estática: campos de velocidad y movimiento en la dimensión $W$.
* Contención: bounding boxes y radios de influencia para optimización.

Los campos entrelazados de las tres capas aparecen cuando las disciplinas se cruzan:

* Física + Química: materia geométrica. Aquí $m$ y $\mathbf{c}$ se asocian con la frontera implícita $\Phi$ para crear un campo materializado.
* Física + Biología: movimiento articulado. Las transformaciones jerárquicas $T_{padre} \cdot T_{local}$ hacen que la forma geométrica evolucione como un cuerpo coherente.
* Química + Biología: materialidad estructural. El `material_id`, la densidad $A$ y las propiedades $P$ se propagan a lo largo de la jerarquía de subpartes para mantener coherencia de apariencia y comportamiento.
* Física + Química + Biología: el campo compuesto completo, donde una estructura articulada con transformaciones locales y materiales definidos se colapsa a `RGBA` solo en la etapa final.

Estos campos entrelazados deben mapearse en la implementación como subespacios del mismo campo interno, no como tres dominios separados sin comunicación.

### Relación con el modelo tensorial temporal

En la base del universo, $[X,Y,Z,W]$ sigue siendo el dominio único. La pirámide añade capas de significado sobre ese dominio:

* Física: define la forma y la visibilidad del campo.
* Química: define la materia que ocupa el campo y cómo interactúa.
* Biología: define la organización y la coherencia de conjuntos de entidades.

Esto hace que el tensor temporal no sea sólo un espacio de posiciones, sino el soporte para un sistema completo donde:

* $A$ decide presencia visual
* $m$ decide materialidad
* $T$ decide cómo se ensamblan los componentes

---

## 7. Modelo de Datos Estructurado: Geometría vs Materia

La solución más consistente es mantener la geometría y el material como capas separadas dentro del mismo campo.

Definimos un resultado estructurado para cada punto $\mathbf{p}$:

$$\mathcal{R}(\mathbf{p}) = (d, m, \mathbf{c}, A, P)$$

Donde:

* $d = \Phi(\mathbf{p})$ es la distancia al límite más cercano.
* $m$ es el identificador de material o región.
* $\mathbf{c} = [R, G, B]$ es el color base asociado al material.
* $A$ es la densidad u opacidad local.
* $P$ son propiedades químicas adicionales ($\rho$, $\kappa$, etc.).

Bajo esta estructura, la unión de dos campos no es sólo un `min` escalar, sino una selección de la superficie más cercana con su material correspondiente:

$$d_r = \min(d_1, d_2)$$

$$m_r = \begin{cases} m_1 & \text{si } d_1 < d_2 \\ m_2 & \text{si } d_2 < d_1 \end{cases}$$

$$\mathbf{c}_r = \begin{cases} \mathbf{c}_1 & \text{si } d_1 < d_2 \\ \mathbf{c}_2 & \text{si } d_2 < d_1 \end{cases}$$

$$A_r = \begin{cases} A_1 & \text{si } d_1 < d_2 \\ A_2 & \text{si } d_2 < d_1 \end{cases}$$

Esto permite:

* separar claramente la forma de la materia
* conservar identidad de material en operaciones booleanas
* alimentar al resolvedor de GTE con una estructura que puede colapsarse a `RGBA`

### 7.1 Formalización de la Pirámide

Para que la pirámide sea ejecutable, cada capa debe tener un contrato de entrada y salida claro.

* Física produce un candidato de visibilidad y la geometría del campo:

  $$\mathcal{F}(\mathbf{p}) = (d, A, T_{local}, \text{bbox}, \text{motion})$$

  donde:
  - $d = \Phi(\mathbf{p})$ es la distancia implícita,
  - $A$ es la ocupación o densidad local,
  - $T_{local}$ son las transformaciones de coordenadas,
  - $\text{bbox}$ es la caja de contención de la entidad,
  - $\text{motion}$ son los parámetros de desplazamiento en $W$.

* Química consume la geometría y devuelve materia y parámetros de shading:

  $$\mathcal{Q}(\mathbf{p}) = (m, \mathbf{c}, P, \text{mode})$$

  donde:
  - $m$ = `material_id`,
  - $\mathbf{c} = [R,G,B]$ es el color base asociado,
  - $P$ son propiedades físicas adicionales ($\rho, \kappa, \ldots$),
  - `mode` indica sólido o volumen.

* Biología organiza y controla la coherencia estructural:

  $$\mathcal{B}(\mathbf{p}) = (parent, children, T_{hierarchy}, rules)$$

  donde:
  - $parent/children$ describe la relación jerárquica,
  - $T_{hierarchy}$ define cómo las transformaciones se heredan,
  - $rules$ especifica cuándo un hijo sobrescribe o combina datos con el padre.

La composición de las tres capas produce el estado interno completo:

$$\mathcal{R}(\mathbf{p}) = \operatorname{compose}(\mathcal{F}(\mathbf{p}), \mathcal{Q}(\mathbf{p}), \mathcal{B}(\mathbf{p}))$$

La interpretación práctica de esta composición es que:

* la física define qué puntos son candidatos visibles,
* la química define qué material y color corresponde a cada candidato,
* la biología define cómo se agrupan y transforman los candidatos en estructuras coherentes.

La pirámide deja de ser una metáfora cuando cada capa entrega una interfaz formalizada en el flujo de datos.

La etapa final de renderizado es entonces:

$$ (R,G,B,A)_{final} = \text{resolve}(\mathcal{R}(\mathbf{p}_{visible})) $$

donde $\mathbf{p}_{visible}$ es el punto seleccionado por la visibilidad óptica.

---

## Experimentos

### Experimento I: Construyendo la Silla en $\mathbb{R}^4$ (motivación pre-pirámide)

Para que renderice una silla de madera con patas de metal, nuestro *Resolvedor* necesita generar una función $\mathcal{C}(\mathbf{p}) = [R, G, B, A]$. Una silla no es una figura geométrica simple, sino un **sistema compuesto** por:

1. Un asiento (un cubo aplanado).
2. Un respaldo (un cubo vertical).
3. Cuatro patas (cilindros o prismas alargados).

#### 1. Definición de las Geometrías Implícitas (Campos de Distancia)

Utilizando geometría analítica pura, definimos las funciones de distancia de cada componente respecto a las coordenadas absolutas del universo ($X, Y, Z$):

* **Asiento (Cubo en el plano $Y=0$):**

$$\Phi_{asiento}(X, Y, Z) = \max(|X| - 4, |Y| - 0.5, |Z| - 4)$$

* **Respaldo (Cubo desplazado en $Z=-4$ e $Y=4$):**

$$\Phi_{respaldo}(X, Y, Z) = \max(|X| - 4, |Y - 4| - 4, |Z + 4| - 0.5)$$

* **Las 4 Patas (Desplazadas en las esquinas, extendiéndose hacia $Y = -6$):**

$$\Phi_{pata1}(X, Y, Z) = \max(|X - 3.5| - 0.5, |Y + 3| - 3, |Z - 3.5| - 0.5)$$

*(Y lo mismo para $\Phi_{pata2}, \Phi_{pata3}, \Phi_{pata4}$ cambiando los signos de $X$ y $Z$)*

#### 2. La Unión de las Partes (Operador de Perturbación $\mathcal{P}$)

Para unificar estos componentes en una sola entidad "Silla", recurrimos al operador booleano de mínimo analítico, el cual consolida el volumen final del objeto:

$$\Phi_{silla}(X, Y, Z) = \min(\Phi_{asiento}, \Phi_{respaldo}, \Phi_{pata1}, \Phi_{pata2}, \Phi_{pata3}, \Phi_{pata4})$$

Finalmente, el canal de densidad $A$ se define de forma binaria:

$$A(X, Y, Z) = \begin{cases} 1 & \text{si } \Phi_{silla}(X, Y, Z) \le 0 \\
0 & \text{si } \Phi_{silla}(X, Y, Z) > 0 \end{cases}$$

---

### Experimento II: La Ecuación de Vida de la Silla en el Espacio-Tiempo

En esta versión, la silla se describe como un sistema compuesto que respeta la pirámide:

* Física: coordenadas y transformaciones en $\mathbb{R}^4$
* Química: material, color y densidad de cada subparte
* Biología: jerarquía de objetos y dependencias padre-hijo

### Definición de la silla como conjunto de subpartes

Sea $\mathcal{E}_{silla} = \{\text{asiento}, \text{respaldo}, \text{pata}_1, \dots, \text{pata}_4\}$.

Cada subparte $i$ tiene:

* una transformación local $T_i(W) \in \mathbb{R}^{4 \times 4}$
* una función de distancia local $\Phi_i(\mathbf{p}_{local})$
* un material $m_i$
* un color base $\mathbf{c}_i$
* un modo de densidad $A_i(\mathbf{p}_{local})$

### Física: coordenadas y movimiento

La posición global se obtiene con la transformación local:

$$\mathbf{p}_{local} = T_i(W)^{-1} \cdot \mathbf{p}_{mundo}$$

y cada subparte define su distancia local:

$$d_i(\mathbf{p}_{mundo}, W) = \Phi_i(\mathbf{p}_{local})$$

Para una silla en movimiento uniforme a lo largo de $W$:

$$T_{silla}(W) = \begin{bmatrix} I_{3} & \mathbf{x}_0 + \mathbf{v} W \\ 0 & 1 \end{bmatrix}$$

y las subpartes se anclan jerárquicamente:

$$T_i(W) = T_{silla}(W) \cdot T_{local,i}$$

### Química: estado interno de cada subparte

Cada subparte genera un estado interno estructurado:

$$\mathcal{R}_i(\mathbf{p}_{mundo}, W) = \left(d_i, m_i, \mathbf{c}_i, A_i, P_i\right)$$

con:

* $A_i = H(-d_i)$ para un sólido duro, o
* $A_i = \sigma(d_i)$ para un volumen continuo.

### Biología: unión estructurada de la silla

La silla completa se define por la selección de la subparte más cercana en cada punto global:

$$\mathcal{R}_{silla}(\mathbf{p}_{mundo}, W) = \operatorname{fold}\left(\operatorname{select\_closer}, \{\mathcal{R}_i(\mathbf{p}_{mundo}, W)\}_{i \in \mathcal{E}_{silla}}\right)$$

donde para dos estados $s_1$ y $s_2$:

$$\operatorname{select\_closer}(s_1, s_2) = \begin{cases}
 s_1 & \text{si } d_1 < d_2 \\
 s_2 & \text{si } d_2 < d_1
\end{cases}$$

y la comparación se aplica a todos los campos relevantes: $d$, $m$, $\mathbf{c}$ y $A$.

### Ecuación de vida de la silla

La fórmula que resume la existencia de la silla en el espacio-tiempo es:

$$\mathcal{S}_{silla}(\mathbf{p}_{mundo}, W) = \text{collapse}\left(\mathcal{R}_{silla}(\mathbf{p}_{mundo}, W)\right)$$

con

$$\mathcal{R}_{silla}(\mathbf{p}_{mundo}, W) = \operatorname{fold}(\operatorname{select\_closer}, \{\mathcal{R}_{asiento}, \mathcal{R}_{respaldo}, \mathcal{R}_{pata1}, \dots, \mathcal{R}_{pata4}\})$$

y

$$\mathcal{S}_{silla}(\mathbf{p}_{mundo}, W) = [R,G,B,A]$$

finalmente proyectado a la imagen mediante:

$$F_{gte}(x,y,t) = \mathcal{S}_{silla}\left( \mathbf{C}^{-1}(x,y,Z_{depth}), g(t) \right)$$

### Comentario sobre capas

* En la capa física, $T_i(W)$ y $\Phi_i$ definen la geometría y el movimiento.
* En la capa química, $m_i$, $\mathbf{c}_i$ y $A_i$ definen la materia y la densidad.
* En la capa biológica, el conjunto $\mathcal{E}_{silla}$ y las transformaciones jerárquicas $T_{hijo} = T_{padre} \cdot T_{local}$ definen la coherencia estructural.

Con esta ecuación de vida, la silla deja de ser un solo tensor RGBA roto en pedazos y se convierte en un sistema matemático completo: una estructura interna rica que luego se colapsa a `RGBA` solo en la etapa final.

### Experimento III: Pirámide aplicada a estructura, materiales y optimización

En este experimento, integramos de manera explícita las tres capas de la pirámide en un solo flujo de datos:

* Física define el candidato visible $\mathbf{p}_{visible}$ con $d$, $A$, $T_{local}$ y bounding boxes.
* Química resuelve el material con $m$, $\mathbf{c}$ y propiedades $P$.
* Biología controla la coherencia de la jerarquía y la herencia de transformaciones.

Formalizamos la composición como:

$$\mathcal{R}(\mathbf{p}) = \operatorname{compose}(\mathcal{F}(\mathbf{p}), \mathcal{Q}(\mathbf{p}), \mathcal{B}(\mathbf{p}))$$

donde cada capa contribuye subcampos claramente delimitados y enlazados.

Para cada subparte $i$ de la silla:

$$\mathcal{F}_i = (d_i, A_i, T_{local,i}, \text{bbox}_i, \text{motion}_i)$$
$$\mathcal{Q}_i = (m_i, \mathbf{c}_i, P_i, \text{mode}_i)$$
$$\mathcal{B}_i = (parent_i, children_i, T_{hierarchy,i}, rules_i)$$

La selección global usa `select_closer` con una política de empates que puede ser:

* prioridad determinista por `material_id`, o
* blending de volúmenes si ambas partes son semitransparentes.

La optimización se concreta con:

* `bbox_i` o `influence_radius_i` para descartar subpartes fuera del rayo,
* BVH/rejillas espaciales para limitar la evaluación al conjunto relevante,
* ray marching con `eps`, `max_steps` y early-out cuando la transmitancia cae a cero.

Así, el experimento III demuestra que la pirámide no es solo un esquema conceptual, sino una arquitectura ejecutable para campos tensoriales 4D complejos.

### Capa Biológica — Implementación práctica

Data model (Node)
- `id`: string
- `parent`: int | null
- `children`: int[]
- `local_transform`: mat4
- `world_transform`: mat4 (computed lazily)
- `geometry`: reference to SDF/primitive or a function `Phi_local(p_local)`
- `material_id`: id (nullable)
- `local_params`: { tint?, density_factor?, roughness_offset? }
- `mode`: `solid` | `volume`
- `bbox_local`: AABB in local space
- `bbox_world`: AABB in world space (transformed by `world_transform`)

Reglas de herencia y override
- Material: `effective_material(node) = node.material_id ?? effective_material(parent)`
- Local params: merge functions per-parameter (e.g. tint multiply, roughness add).
- Transform: `world_transform = parent.world_transform * local_transform`; recompute lazily and mark dirty on changes.

Política de empates y resolución (jerarquía)
- Evaluación de un punto global `p`: filtrar nodos por `bbox_world` (BVH/spatial index), transformar `p` a espacio local y evaluar SDF/volumetría.
- `select_closer` aplica:
  - si `|d1-d2| > eps`: elegir menor distancia
  - si modos conflictivos y uno es `solid`: preferir `solid`
  - si ambos `volume`: componer por volumetric compositing (Beer-Lambert u otra)
  - si empate: resolver por prioridad determinista usando `material_id` (hash estable)

Pseudocódigo: evaluación jerárquica de punto
```
function eval_point(p_world):
  candidates = query_spatial_index(p_world)
  state = empty_state() // d = +inf
  for node in candidates:
    if not point_in_aabb(p_world, node.bbox_world): continue
    p_local = inv(node.world_transform) * p_world
    s = node.geometry(p_local) // returns (d, A, mode)
    s.material = effective_material(node)
    s.params = merged_local_params(node)
    state = select_closer(state, s)
  return state
```

Pseudocódigo: `select_closer`
```
function select_closer(s1, s2, eps=1e-5):
  if s1.d + eps < s2.d: return s1
  if s2.d + eps < s1.d: return s2
  if s1.mode == 'solid' and s2.mode != 'solid': return s1
  if s2.mode == 'solid' and s1.mode != 'solid': return s2
  if s1.mode == 'volume' and s2.mode == 'volume':
    return blend_volumes(s1, s2)
  return material_priority(s1.material, s2.material)
```

Bounding boxes, BVH & LOD
- `bbox_local` para primitivos; expandir por `influence_radius` para volúmenes.
- Propagar y unir bboxes ascendiendo en el grafo para obtener `bbox_world`.
- Construir BVH top-level; opcionalmente sub-BVH por grupo. LOD definido por `lod_thresholds` en nodos.

Update / animación
- Al cambiar `local_transform` o `geometry`: marcar `dirty`; en el siguiente frame recomputar `world_transform` y `bbox_world` para el nodo y sus descendientes.
- Reconstruir BVH incrementalmente o usar un spatial grid dinámico para muchos objetos en movimiento.

Estrategia GPU-friendly
- Aplanar el grafo a arrays: `nodes[]`, `parent_idx[]`, `local_transform[]`, `bbox_world[]`, `material_idx[]`, `geometry_idx[]`.
- Precalcular `world_transforms` en CPU (o con compute shader); subir buffers a GPU.
- Kernel GPU: realizar culling con BVH, evaluar SDF/trace por instancia; usar instancing para una geometría compartida con muchas transformaciones.

API / modelo de autor
- Autor define `Node` con referencia a `geometry`, `material_id`, `local_params`, `local_transform` y opcional `bbox` override.
- Materiales centralizados en `MaterialTable`; nodos referencian `material_id` y solo almacenan overrides locales.

Esta sección formaliza cómo "las patas pertenecen a la silla": se crea un `Node` padre `silla` con transform y `children` que son las patas; al cambiar la transform de `silla`, las patas heredan la composición de `world_transform` y se mueven coherentemente.

### Especificación de herencia y reglas de desempate

1) Materiales y parámetros efectivos
- `effective_material(node)`: si `node.material_id != null` usarlo; si no, preguntar al ancestro más cercano. Si ninguno existe, `null`.
- `effective_params(node)`: combinar `local_params` del nodo con los del ancestro aplicando funciones de merge por clave:
  - `tint`: multiplicar colores (vec3) -> `t = parent.tint * node.tint` (con default `[1,1,1]`)
  - `roughness`: sumar offsets -> `r = clamp(parent.roughness + node.roughness_offset, 0, 1)`
  - `density_factor`: multiplicar factores
  - reglas específicas pueden definirse en `material_table` para parámetros compuestos.

2) Resolución determinista de empates (`resolve_tie`)
- Si `|d1-d2| > eps`: elegir menor `d`.
- Si un elemento tiene `mode = solid` y el otro `volume`: elegir `solid` (desplaza aunque volumen tenga mayor A).
- Si ambos `volume`: aplicar compositing (Beer-Lambert) con prioridad por orden de evaluación si es necesario.
- Si empate fino (`|d1-d2| <= eps`) y ambos `mode` iguales: usar `material_priority(material_a, material_b)` donde `material_priority` = compare(hash(material_id)) para garantizar estabilidad.

3) Políticas adicionales
- `child_overrides`: por defecto, un `child` puede sobrescribir `material_id` y `local_params`. Para evitar sobrescrituras, el `parent` puede marcar `lock_material=true`.
- `merge_strategy` puede ser `override`, `blend`, o `accumulate` y se especifica por parámetro en el `Node` o en `MaterialTable`.

Pseudocódigo: resolver material y parámetros efectivos
```
function resolve_inheritance(node):
  if node.effective_cached and not node.dirty: return node.effective
  mat = node.material_id
  params = default_params()
  if node.parent != null:
    parent_eff = resolve_inheritance(node.parent)
    if mat == null: mat = parent_eff.material
    params = merge_params(parent_eff.params, node.local_params)
  else:
    params = merge_params(params, node.local_params)
  node.effective = { material: mat, params: params }
  node.effective_cached = true
  return node.effective
```

### API: aplanado del grafo y layout de buffers GPU

Objetivo: transformar el grafo jerárquico a buffers planos que la GPU puede consumir eficientemente.

1) Estructuras en CPU
- `NodeArray[]` con campos compactos: `parent_idx:int32, first_child_idx:int32, child_count:int16, local_transform:mat4, bbox_min:vec3, bbox_max:vec3, geometry_idx:int32, material_idx:int32, param_offset:int32`.
- `GeometryTable[]`: descriptores de SDF/primitive (tipo, params index, lod level).
- `MaterialTable[]`: ya existente, indexable por `material_idx`.
- `ParamPool[]`: empaqueta overrides `local_params` en un buffer contiguo y `param_offset` apunta al inicio.

2) Funciones
```
function flatten_scenegraph(root_nodes):
  nodes_out = []
  stack = root_nodes
  while stack not empty:
    node = stack.pop()
    eff = resolve_inheritance(node)
    nodes_out.append(pack_node(node, eff))
    push children of node onto stack (pre-order) 
  return nodes_out

function prepare_instance_buffer(nodes_out):
  node_buffer = serialize_nodes(nodes_out)
  transforms_buffer = serialize_transforms(nodes_out)
  bbox_buffer = serialize_bboxes(nodes_out)
  params_buffer = serialize_parampool(nodes_out)
  build_top_level_bvh(node_buffer)
  upload_buffers_to_gpu(node_buffer, transforms_buffer, bbox_buffer, params_buffer)
```

3) Layout recomendado (GL/DX friendly)
- `SSBO nodes`: struct NodeGPU { int parent; int geometry_idx; int material_idx; int param_offset; float bbox_min[3]; float bbox_max[3]; } (aligned to 16 bytes)
- `SSBO transforms`: array of mat4 (column-major)
- `SSBO params`: packed floats/vec3 per-instance
- `BVH buffers`: index/offsets para culling en shader

4) Evaluación híbrida CPU/GPU
- Hacer culling top-level en CPU (frustum + BVH) y enviar solo nodos visibles a GPU cuando la escena es grande.
- Para objetos animados con muchos instanced duplicates, enviar una única `GeometryTable` y un `instance_transform[]` corto por draw call.

Notas finales
- Mantener `resolve_inheritance` con caché y `dirty` propagation es crítico para evitar recomputaciones.
- El merge determinista de parámetros garantiza reproducibilidad entre CPU y GPU.

---

## 9. Leyes Formales del Campo Tensorial Hermético

Para mitigar las ausencias prácticas del modelo, es útil añadir una capa de leyes que gobiernen:

1. intervención y perturbación,
2. cohesión y discontinuidad de cuerpos,
3. temporalidad y proyección al renderizado.

### 9.1 Leyes de intervención

Definimos la intervención como una perturbación del campo interno que se traduce en presencia visual.

* Perturbación = presencia: $\delta \mathcal{R} \Rightarrow A > 0$.
* Si $v > 0$ entonces el campo está activo y puede influir en la selección visual.
* Influencia = operación: puede modelarse como interpolación o acoplamiento suave.

Formalmente:

$$
\mathcal{R}' = \operatorname{intervene}(\mathcal{R}, n) = \operatorname{LERP}(\mathcal{R}, \mathcal{R}_{obj}, n)
$$

donde $n \in [0,1]$ controla la intensidad de la perturbación y puede generalizarse a:

$$
\operatorname{influence}(\mathcal{R}, n) = \mathcal{R} + n^2 \cdot (\mathcal{R}_{obj} - \mathcal{R})
$$

Esto permite modelar transiciones suaves entre campos y representaciones parciales.

* Aniquilación = máscara: cuando una entidad elimina presencia, la operación es

$$
\mathcal{R}_{out} = n \cdot 0 = 0, \quad n = 0.
$$

Aquí $0$ representa un estado sin presencia visual ($A=0$) y sin material definido.

### 9.2 Leyes de geometría y cohesión de campos tensoriales

El campo debe diferenciar tipos de cuerpos según su acoplamiento y comportamiento físico:

* cuerpos rígidos: sólidos con geometría estable y transformaciones afines,
* cuerpos propagativos: volúmenes que pueden viajar, disiparse o rebotar,
* cuerpos deformables: regiones con dinámica interna y morfología variable.

Definimos la cohesión como una función de acoplamiento entre campos:

$$
\operatorname{cohesion}(\mathcal{R}_a, \mathcal{R}_b) = \begin{cases}
\text{rigido} & \text{si } A_a, A_b \text{ son sólidos y } d_a \neq d_b \\
\text{propagativo} & \text{si } A_a, A_b \text{ son volúmenes con gradientes continuos} \\
\text{deformable} & \text{si } \nabla d \text{ cambia con } W
\end{cases}
$$

Esta ley requiere también definir la interfaz y la discontinuidad:

* interfaz: la superficie donde dos campos se encuentran,
* discontinuidad: salto en $m$, $\mathbf{c}$ o $A$.

Para un punto $\mathbf{p}$ de frontera:

$$
\operatorname{interface}(\mathcal{R}_1, \mathcal{R}_2) = \{\mathbf{p} \mid d_1(\mathbf{p}) = d_2(\mathbf{p})\}
$$

y la discontinuidad se observa cuando

$$
(m_1, \mathbf{c}_1, A_1) \neq (m_2, \mathbf{c}_2, A_2).
$$

### 9.3 Leyes temporales

La dimensión temporal se trata como una extensión volumétrica del mundo:

* la línea del mundo es una prolongación en volumen 4D,
* la causalidad atemporal describe leyes de cambio que no dependen de un “ahora” local,
* el renderizado se obtiene por corte transversal del volumen 4D.

Definimos la línea de mundo de un objeto $o$ como:

$$
\mathcal{W}_o = \{(\mathbf{p}, W) \mid \mathbf{p} \in \text{objeto } o \text{ en } W\}.
$$

La causalidad atemporal se formula como leyes de mecánica de cambio:

$$
\frac{d}{dW}\mathcal{R}(\mathbf{p}, W) = \mathcal{F}(\mathcal{R}, W)
$$

donde $\mathcal{F}$ puede incluir fuerzas internas, acoplamientos y perturbaciones sin necesidad de un orden explícito.

El renderizado final se realiza por un corte transversal en $W$:

$$
F_{gte}(x,y,t) = \text{slice}_{W=g(t)}\left( \mathcal{R}(\mathbf{p}, W) \right).
$$

Esta visión deja claro que la imagen es solo una sección del campo 4D, y que la estructura interna del modelo debe manejar presencia, coherencia y cambio antes de ser colapsada a RGBA.

---

## 10. Notas de implementación

Para que el modelo se pueda aplicar y no quede solo como una abstracción, conviene añadir un contrato mínimo para los datos internos y los operadores.

### 10.1 Distinción interna vs salida

El campo interno debe definirse como:

$$internal = (d, m, \mathbf{c}, A, P)$$

Y la salida como:

$$output = collapse(internal) = [R, G, B, A]$$

Así:

```text
state_internal = C(p)
state_output = collapse(state_internal)
```

De esta forma queda claro que `M = [X,Y,Z,W,R,G,B,A]` es una interfaz de observación, no la estructura semántica del universo.

### 10.2 Pseudocódigo de selección y empates

```text
function select_closer(s1, s2, eps = 1e-5):
    if s1.d + eps < s2.d: return s1
    if s2.d + eps < s1.d: return s2
    return resolve_tie(s1, s2)

function resolve_tie(s1, s2):
    if s1.material_id == s2.material_id:
        return blend_volume(s1, s2)
    return priority_by_material(s1, s2)
```

`resolve_tie` puede elegir una regla determinista basada en prioridad de `material_id`, o mezclar cuando ambos son volúmenes semitransparentes.

### 10.3 Modo de `A`: sólido vs volumen

Cada entidad debe declarar un modo:

* `mode = solid`
* `mode = volume`

Reglas de colapso:

* `solid`: $A = H(-d)$, donde $H$ es la función de Heaviside. Si cualquier sólido tiene $d \le 0$, se trata como superficie opaca.
* `volume`: $A = \sigma(d)$, donde $\sigma$ es una función suave de densidad.

Para compositing de volúmenes se usa transmitancia:

$$
T(z) = \exp\left(-\int_0^z A(z') \, dz'\right)
$$

y el color acumulado puede aproximarse como:

$$
C_{final} = \int T(z) \, c(z) \, A(z) \, dz.
$$

### 10.4 Contrato de `material_id`

`material_id` debe indexar una tabla de materiales con campos obligatorios mínimos para que `collapse()` pueda resolver color y shading.

Formato mínimo:

```json
{
  "materials": {
    "wood01": {
      "id": "wood01",
      "base_color": [0.545, 0.27, 0.075],
      "roughness": 0.6,
      "metallic": 0.0,
      "shader_params": {
        "specular": 0.2,
        "opacity": 1.0
      }
    }
  }
}
```

Campos obligatorios:

* `id`: identificador único del material.
* `base_color`: color base RGB en [0,1].
* `roughness`: factor de rugosidad para shading.
* `metallic`: factor metálico para shading.
* `shader_params`: parámetros adicionales como `specular`, `emission`, `opacity`, etc.

Un esquema JSON mínimo sería:

```json
{
  "type": "object",
  "properties": {
    "materials": {
      "type": "object",
      "patternProperties": {
        "^.+$": {
          "type": "object",
          "properties": {
            "id": {"type": "string"},
            "base_color": {
              "type": "array",
              "items": {"type": "number"},
              "minItems": 3,
              "maxItems": 3
            },
            "roughness": {"type": "number"},
            "metallic": {"type": "number"},
            "shader_params": {"type": "object"}
          },
          "required": ["id", "base_color", "roughness", "metallic", "shader_params"]
        }
      }
    }
  },
  "required": ["materials"]
}
```

### 10.4.1 Ejemplo de `collapse()` con lookup

```text
function collapse(internal, material_table):
    if internal.m == null or internal.A <= 0:
        return [0,0,0,0]

    material = material_table[internal.m]
    if material == null:
        return [0,0,0,0]  // material desconocido -> invisible

    base_color = material.base_color
    shade = apply_shader(material.shader_params, internal)
    alpha = internal.A

    return [base_color.r * shade.r,
            base_color.g * shade.g,
            base_color.b * shade.b,
            alpha]
```

En este ejemplo, `collapse()` depende de `material_id` para resolver la apariencia y utiliza `shader_params` para ajustar el resultado final.

### 10.5 Optimización y contención

El campo interno es infinito en principio, por lo que conviene no evaluar todo el dominio.

* Cada entidad debe declarar un `bbox` (AABB) o un `influence_radius`.
* Usar estructuras espaciales (BVH, rejilla, hash espacial) para evitar evaluar entidades enteras fuera de un ray or tile.
* Aplicar LOD: evaluaciones SDF aproximadas cuando la entidad está lejos, evaluaciones completas cuando está cerca.
* Cachear muestras de `\mathcal{R}(\mathbf{p})` por tile/página cuando sea posible.
* Early-out:
  * si un ray no cruza el bbox de la entidad, saltarla;
  * si la transmitancia acumulada `T(z)` cae por debajo de un umbral, detener la integración;
  * si `A` es cero a lo largo de un segmento, no sumar contribución.

### 10.6 Bounding boxes, influencia y LOD (estrategia detallada)

Para que la evaluación del campo sea práctica y eficiente, es necesario formalizar cómo se calculan cajas de contención conservadoras (AABB), radios de influencia para volúmenes y reglas de LOD. Esta sección detalla algoritmos y decisiones de ingeniería.

1) Cálculo de `bbox_local` por primitiva
- Caja analítica: si la primitiva es una caja o cubo, `bbox_local` es directamente los límites del parámetro.
- Esfera: `bbox_local = [cx-r, cy-r, cz-r, cx+r, cy+r, cz+r]`.
- Cilindro (eje Y): `bbox_local = [cx-r, cy-h/2, cz-r, cx+r, cy+h/2, cz+r]`.
- Cápsula: similar a cilindro más extremos esféricos.
- SDF compuestas (union/intersection/difference): `bbox = union(bbox_i)` para `union`, `bbox = intersection(bbox_i)` para `intersection` (si intersection vacía, usar fallback conservador: union).

2) Transformar `bbox_local` a `bbox_world`
- Transformar AABB por una matriz `M` requiere convertir a OBB y volver a AABB conservador:
  - calcular las 8 esquinas de `bbox_local`, transformarlas por `M`, y tomar min/max de resultados.
  - Esta conversión es conservadora y estable numéricamente.

3) Radios de influencia para volúmenes
- Definir un umbral práctico `eps_density` (p.ej. 1e-3). Para una función de densidad $\sigma(d)$, el radio de influencia $r_{inf}$ satisface `sigma(r_inf) <= eps_density`.
- Ejemplos:
  - Gaussiana: $\sigma(d)=
ho\,e^{-k d^2}$ ⇒ $r_{inf} = \sqrt{-\ln(\frac{eps}{\rho})/k}$.
  - Exponencial lineal: $
ho e^{-k d}$ ⇒ $r_{inf} = -\ln(eps/\rho)/k$.
- Para volúmenes definidos por SDF con suavizado, usar `r_inf = max_support + margin` donde `max_support` es la distancia límite donde la densidad cae por debajo del umbral.

4) BBox conservador para SDF paramétricos
- Si SDF es función `Phi(p; params)` y existe un parámetro `r_max` (p.ej. radio, half-extent), construir `bbox_local` en torno al centro extendido por `r_max + r_inf`.
- Si no hay parámetro directo, estimar por muestras: muestrear la SDF en una malla coarsa local y expandir por la máxima distancia negativa encontrada más `r_inf`.

5) Estrategia LOD
- Métrica: `screen_error = (object_world_radius * projection_scale) / viewport_px` o usar `projected_extent_px`.
- Niveles LOD disponibles: `full_SDF`, `simplified_SDF`, `primitive_proxy` (bounding box / sphere), `impostor_2D`.
- Regla práctica:
  - si `projected_extent_px > thresh_high`: `full_SDF`.
  - si `thresh_low < projected_extent_px <= thresh_high`: `simplified_SDF`.
  - si `projected_extent_px <= thresh_low`: `primitive_proxy` o `impostor_2D`.

6) BVH dinámico y actualización incremental
- Refit vs Rebuild:
  - `refit`: recalcular min/max de nodos del BVH utilizando `bbox_world` actual de las hojas (O(n) pero barato), útil si las estructuras se mueven pero no cambian topología.
  - `rebuild`: reconstruir BVH desde cero (O(n log n)), necesario cuando muchas `bbox` cambian de forma drástica o cuando se alteran jerarquías.
- Estrategia híbrida:
  - Refit cada frame para objetos animados (si cambios pequeños).
  - Rebuild cada N frames o cuando el porcentaje de nodos con movimiento > threshold.
  - Para escenas con muchos objetos dinámicos usar spatial hash o grillas temporales y limitar BVH a objetos estáticos.

7) Pseudocódigo: refit BVH (top-level)
```
function refit_bvh(bvh_root, leaves):
    for leaf in leaves:
        leaf.bbox = leaf.node.bbox_world
    postorder_traverse(bvh_root, node => node.bbox = union(node.left.bbox, node.right.bbox))
```

8) Pseudocódigo: seleccionar LOD por nodo
```
function select_lod(node, camera):
    projected = project_radius(node.bbox_world_center, node.bbox_world_radius, camera)
    if projected > HIGH_PX: return FULL_SDF
    if projected > LOW_PX: return SIMPLIFIED_SDF
    return PRIMITIVE_PROXY
```

9) Culling y trabajo híbrido CPU/GPU
- CPU: frustum cull + top-level BVH cull (rápido), preparar compact list de nodos visibles.
- GPU: recibir `nodes_visible[]` y ejecutar culling fino por shader si es necesario (compute), seguido de sphere-tracing/ SDF evaluation por instancia.

10) Notas sobre tolerancias y estabilidad
- Elegir `eps_density` coherente con `collapse()` para evitar fugas visibles.
- Mantener margen de seguridad `margin = max(eps_z, machine_epsilon * scale)` al expandir bboxes para evitar rechazos debido a errores numéricos.

11) Referencias a prototipo
- El prototipo `prototipo/flatten_scene.py` incluye un flujo mínimo para serializar nodos y material table. Se recomienda ampliar `bbox_local` y `bbox_world` en ese prototipo y añadir `r_inf` calculado por primitiva para que el pipeline use BVH y LOD en el ejemplo de la silla.

### 10.6 Resolución numérica y tolerancias

No se debe asumir que `Z_{depth}` se obtiene analíticamente. En la práctica, para superficies implicitas y SDF se recomienda sphere-tracing con un máximo de pasos y una tolerancia de distancia.

```text
function sphere_trace(ray, max_steps = 128, eps = 1e-4, z_max = 100.0):
    z = ray.origin_z
    for i in range(max_steps):
        p = ray.at(z)
        s = C(p)
        if is_nan(s.d): return no_hit
        if abs(s.d) < eps: return z
        z += clamp(s.d, eps, z_max - z)
        if z >= z_max: return no_hit
    return no_hit
```

Notas de tolerancia:

* `eps` define la precisión de intersección. Valores más pequeños dan mejor exactitud pero aumentan el número de pasos.
* `max_steps` protege contra iteraciones infinitas en campos complejos.
* `z_max` limita la búsqueda a una franja razonable del universo visible.
* `min_step` o `clamp` evitan avances nulos cuando `d` es muy pequeño.

### 10.7 Manejo de casos degenerados

El modelo debe definir políticas de fallback explícitas.

* Empates exactos (`d1 == d2` dentro de `eps`): usar `resolve_tie` con prioridad de `material_id` o blending de volúmenes.
* NaN en `d`: tratar como estado sin presencia (`A=0`, `d=+\infty`) y continuar sin interrumpir el render.
* `T_i(W)` no invertible: fallback a la transformación identidad o devolver el objeto como no presente, en lugar de fallar.
* Transformaciones singulares: registrar el error y usar un estado seguro de no-presencia o una transformación alterna.
* Si `find_depth` o `sphere_trace` agotan `max_steps`, devolver `no_hit` y considerar ese píxel vacío o aplicar un fallback de baja resolución.

Estas reglas prácticas cierran las brechas más importantes entre la especificación teórica y una implementación robusta.
 
### 10.8 API mínima v1 (contratos JSON y ejemplos)

Esta sección documenta un contrato mínimo para interoperar entre herramientas (CPU <-> GPU, prototipos y pruebas). Los esquemas aquí son intencionalmente compactos: sirven para la primera implementación (`v1`) y deben extenderse con campos de perfil y rendimiento en `v2`.

1) `internal` — estado devuelto por `C(p)`

JSON Schema (resumen):

{
  "type": "object",
  "properties": {
    "d": {"type":"number"},
    "material_id": {"type":["string","null"]},
    "c": {"type":"array","items":{"type":"number"},"minItems":3,"maxItems":3},
    "A": {"type":"number"},
    "P": {"type":"object"}
  },
  "required": ["d","A"]
}

Ejemplo práctico:

{
  "d": -0.12,
  "material_id": "wood01",
  "c": [0.545,0.27,0.075],
  "A": 1.0,
  "P": {"rho": 700}
}

2) `material_table` — tabla global de materiales (JSON)

Schema mínimo (resumen): cada entrada indexada por `id` debe incluir `base_color` y parámetros de shading. Se admiten hooks a texturas y BRDF.

{
  "type":"object",
  "patternProperties":{
    "^.+$": {
      "type":"object",
      "properties":{
        "id":{"type":"string"},
        "base_color":{"type":"array","minItems":3,"maxItems":3},
        "roughness":{"type":"number"},
        "metallic":{"type":"number"},
        "base_color_map":{"type":"string"},
        "normal_map":{"type":"string"},
        "brdf":{"type":"object"}
      },
      "required":["id","base_color"]
    }
  }
}

Ejemplo:

{
  "wood01": {
    "id":"wood01",
    "base_color":[0.545,0.27,0.075],
    "roughness":0.6,
    "metallic":0.0,
    "base_color_map":"textures/wood01_albedo.png",
    "brdf": { "type": "pbr" }
  }
}

3) `Node` — contrato de la Capa Biológica (ejemplo JSON Schema resumido)

{
  "type":"object",
  "properties":{
    "id":{"type":"string"},
    "parent":{"type":["string","null"]},
    "local_transform":{"type":"array","minItems":16,"maxItems":16},
    "geometry":{"type":"object"},
    "material_id":{"type":["string","null"]},
    "local_params":{"type":"object"},
    "bbox_local":{"type":"array","minItems":6,"maxItems":6},
    "mode":{"type":"string","enum":["solid","volume"]}
  },
  "required":["id","local_transform"]
}

Ejemplo de `Node` (silla - asiento):

{
  "id": "seat",
  "parent": null,
  "local_transform": [1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1],
  "geometry": {"type":"box","extents":[4,0.5,4]},
  "material_id": "wood01",
  "local_params": {},
  "bbox_local": [-4,-0.5,-4,4,0.5,4],
  "mode": "solid"
}

4) `collapse(internal, material_table, shading_context)` — pseudocódigo v1 con hooks para texturas/BRDF

```
function collapse(internal, material_table, shading_ctx):
    if internal.m == null or internal.A <= 0: return [0,0,0,0]
    mat = material_table[internal.m]
    if mat == null: return [0,0,0,0]
    // base color: either sampled texture or base_color
    if mat.base_color_map:
        tex_color = sample_texture(mat.base_color_map, shading_ctx.uv)
        base = tex_color * mat.base_color
    else:
        base = mat.base_color
    // normal mapping hook (if present)
    N = shading_ctx.N
    if mat.normal_map:
        N = apply_normal_map(mat.normal_map, shading_ctx)
    // simple PBR evaluation placeholder
    color = evaluate_brdf(mat.brdf or {type:'lambert'}, base, N, shading_ctx.V, shading_ctx.L)
    alpha = internal.A * (mat.shader_params.opacity if present else 1.0)
    return [color.r, color.g, color.b, alpha]
```

Notas:
- `shading_ctx` es un pequeño struct con `uv`, `N` (normal), `V` (view dir), `L` (light dir) y otros datos necesarios. En la pipeline de ray-marching este contexto se construye a partir del punto y del material (puntos de referencia para shading).
- Los hooks de textura devuelven valores de color/normal que integran fácilmente en el `evaluate_brdf`.

5) Recomendaciones prácticas para `v1`
- Unificar unidades (metros) y documentarlas claramente en la cabecera del documento.
- Valores iniciales sugeridos (pueden ajustarse): `eps=1e-4`, `max_steps=128`, `r_inf` según función de densidad (usar `eps_density=1e-3`), `z_max=100.0`.
- Serialización: `Node`/`material_table` en JSON para herramientas; buffers binarios para runtime GPU.

Esta especificación `v1` permite que `flatten_scene.py` y motores de prueba intercambien datos de forma inmediata. En `v2` propondremos una versión tipada (OpenAPI/JSON-Schema completos) y ejemplos de test vectors para validación automática.

---
## 11. Limitaciones y caminos para estabilizar el modelo

Este documento ya tiene una base conceptual sólida, pero aún requiere formalizar varias interfaces para que el modelo sea científicamente estable.

### Lo que aún falta formalizar

* Contrato físico-químico-biológico:
  - Física produce $(d, A, T_{local}, \text{bbox}, \text{motion})$.
  - Química produce $(m, \mathbf{c}, P, \text{mode})$.
  - Biología produce jerarquía y reglas de herencia.

* Normalización entre interno y externo:
  - `internal = \mathcal{C}(\mathbf{p})` debe ser la única verdad.
  - `output = collapse(internal)` debe ser el único punto en que se convierte a `RGBA`.

* Reglas operativas de `A`:
  - `solid`: $A = H(-d)$.
  - `volume`: $A = \sigma(d)$.
  - mezcla de sólidos y volúmenes debe estar definida con una política clara.

* Política de resolución de empates en `select_closer`:
  - `d_1 == d_2` dentro de `\eps`.
  - prioridades deterministas por `material_id` o blending de volúmenes.

* Contrato de `material_id`:
  - `id`, `base_color`, `roughness`, `metallic`, `shader_params`.
  - lookup obligatorio en `collapse()`.

* Estrategia de render práctico:
  - `Z_{depth}` por ray marching / sphere tracing.
  - AABB/BVH/early-out.
  - tolerancias numéricas y límites de pasos.

### Qué hace falta documentar explícitamente

1. Cómo la capa física produce primero un candidato visible `\mathbf{p}_{visible}`.
2. Cómo la capa química traduce `material_id` a parámetros reales de shading.
3. Cómo la capa biológica decide herencia y sobrescritura entre padre e hijo.
4. Cómo la optimización limita la evaluación del campo infinito.

### Camino para estabilizar el modelo

* Añadir una sección de API formal al comienzo del documento.
* Incluir pseudocódigo de `select_closer`, `collapse`, `sphere_trace` y `compose`.
* Definir un esquema json mínimo para `material_id`.
* Especificar la política de mezcla de sólidos y volúmenes.
* Señalar claramente las limitaciones prácticas actuales.

Con esta sección, el documento pasa de ser un diseño conceptual a un marco claramente delimitado para implementar un campo tensorial 4D con visibilidad, materialidad y estructura.

### 11.1 Política sólido–volumen (matemática y pseudocódigo)

Problema: en puntos o trayectorias donde interactúan sólidos y volúmenes, hay varias operaciones posibles (selección por distancia, compositing volumétrico, mezcla de iluminación). La política que proponemos distingue dos contextos:

- Evaluación puntual (`select_closer` / `C(p)`): devuelve el estado dominante en esa posición. Regla: si algún sólido tiene `d <= 0` (punto dentro de sólido), el sólido domina la identidad del punto (`material_id`, `c`, `A=1`). Volúmenes no desplazan la identidad local del sólido.
- Integración de rayo (ray-marching / render): los volúmenes contribuyen por transmisión/emisión hasta el punto de intersección con un sólido. Es decir, la visibilidad del sólido se atenúa por la transmitancia acumulada de volúmenes en frente.

Matemáticamente, para un rayo que atraviesa un intervalo [z0, z_hit] con volúmenes definidos por densidad `alpha(z)` y color `c_v(z)`, la transmitancia es:

$$T(z) = \exp\left(-\int_{z_0}^{z} \alpha(z')\,dz'\right)$$

La contribución volumétrica acumulada hasta el impacto es:

$$C_{vol} = \int_{z_0}^{z_{hit}} T(z)\, c_v(z)\, \alpha(z)\, dz$$

El color final observado cuando el rayo golpea un sólido de color `c_s` en `z_hit` es:

$$C_{final} = C_{vol} + T(z_{hit})\, c_s$$

Si el rayo no encuentra ningún sólido, el color es la integral volumétrica hasta `z = \infty`.

Pseudocódigo práctico (discretizado):

```
function integrate_ray(ray, z0, z_max, step):
  T = 1.0
  C = [0,0,0]
  z = z0
  while z < z_max:
    p = ray.at(z)
    s = C(p) // internal state (may be volume or solid)
    if s.mode == 'volume' and s.A > 0:
      delta = step
      alpha = s.A / step_scale   // convert density to per-distance unit
      C += T * s.c * (1 - exp(-alpha*delta))
      T *= exp(-alpha*delta)
    if s.mode == 'solid' and s.d <= eps:
      // hit solid
      C += T * s.c
      return clamp(C)
    z += step
  return clamp(C) // no solid hit, return accumulated volume
```

Notas de implementación:

- `alpha` debe estar en unidades por distancia; el `s.A` del estado interno puede ser una densidad adimensional y requiere un factor `density_scale` para convertirlo a unidades físicas.
- La fórmula usa aproximación por pasos; para precisión usar integración adaptativa o step size basado en `r_inf`.
- Esta política satisface la intuición física: los volúmenes pueden teñir/atenúar lo que hay detrás, pero no sustituyen la identidad de un sólido local.

### 11.2 Valores numéricos guía y calibración

Proponemos valores iniciales y una breve guía para su ajuste. Estos valores son conservadores y adecuados para prototipos en unidades métricas (metros):

- `eps` (tolerancia de intersección SDF): 1e-4 — reduce Z-fighting y mantiene pasos razonables.
- `max_steps` (sphere-tracing / ray-march): 128 — suficiente para escenas medianas; aumentar para SDF complejas.
- `z_max` (máxima profundidad de búsqueda): 100.0 — ajustable según rango de cámara; en metros por defecto.
- `eps_density` (umbral para radio de influencia `r_inf`): 1e-3 — densidad por debajo de esto se considera insignificante para visibilidad.
- `step_size` (integración volumétrica inicial): usar `min(eps, r_inf/8)` o `0.01` m por defecto.
- `density_scale`: factor para convertir `A` a `alpha` por unidad de distancia. Recomendación: documentar `A` como densidad adimensional en [0,1] y usar `density_scale = 5.0` (significa `A=1` → alpha≈5 m^-1, fuerte atenuación en ~0.6 m).

Cómo calibrar:

1. Establecer unidades del mundo (recomendado: metros). Documentarlo.
2. Renderizar una escena de calibración: una esfera volumétrica con `rho=1` y un sólido detrás a 1 m de distancia.
3. Ajustar `density_scale` hasta que la transmisión `T(1m)` tenga el rango deseado (p.ej. T≈0.6 para una niebla ligera).
4. Aumentar `max_steps` o reducir `eps` si los contornos finos muestran aliasing.

Ejemplo práctico: con `density_scale=5.0` y `step_size=0.01`, una columna uniforme de `A=0.2` sobre 1 m produce `alpha=1.0 m^-1`, `T = exp(-1*1)=0.3679`.

Medición de rendimiento (micro-benchmarks):

- Medir `samples_per_second` para la evaluación SDF básica en la máquina objetivo.
- Medir `trace_throughput = pixels/sec` con `max_steps` y `eps` configurados; usar como referencia para decidir LOD thresholds.


---
## Conclusión Técnica

El modelo de $1 \times 8$ funciona bien como una observación de salida, pero **el universo interno debe modelarse como un campo estructurado**.

La solución no es añadir un noveno canal al tensor. La solución es aceptar que:

* el campo interno es un estado estructurado $\mathcal{R}(\mathbf{p}) = (d, m, \mathbf{c}, A, P)$,
* las operaciones de unión seleccionan el estado más cercano con su material y color,
* y solo al final se colapsa ese estado a `RGBA` para el renderizador.

En otras palabras: no estamos buscando una dimensión extra en el mismo tensor, sino una semántica diferente para el campo interno y un colapso controlado hacia `RGBA` en la etapa final.

Esto deja claro que el cambio es semántico, no simplemente dimensional.

---

## 12. Protocolo temporal para `W`: actualización, coherencia y sincronización

Esta sección define un contrato operativo para manejar la dimensión temporal (`W`) en tiempo de ejecución. Describe cómo propagar cambios, cuándo recomputar `world_transform` y `bbox_world`, cómo sincronizar BVH/SSBOs y cómo mantener determinismo y rendimiento en escenas dinámicas.

Objetivos:
- Garantizar coherencia entre transformaciones jerárquicas y estado interno.
- Minimizar trabajo recomputado mediante `dirty` propagation y cachés.
- Ofrecer reglas claras para `refit`/`rebuild` del BVH y para sincronización GPU-friendly (doble buffer).
- Suportar muestreo temporal para motion blur y sampling en W.

12.1 Conceptos claves
- `frame_time` / `w_time`: valor escalar de tiempo presente en la simulación (mapeado a la dimensión `W`).
- `dirty` flag: marca en cada `Node` indicando que `local_transform` o `geometry` o `local_params` cambiaron desde la última actualización.
- `effective` / `cached_effective`: valores derivados (materiales, params) cacheados y validados por `dirty`.
- `world_transform` y `bbox_world`: computados de forma perezosa pero consolidados en la etapa de actualización por frame.
- `BVH` lifecycle: `refit` (barato) vs `rebuild` (costoso). Reglas heurísticas controlan cuándo cada uno.
- `GPU buffers` (SSBOs): deben ser actualizados con un esquema de doble-buffering o versionado para evitar tearing y garantizar consistencia en shaders en ejecución.

12.2 Orden de operaciones por frame (síncrono)
1. Aplicar entradas externas (animaciones, keyframes, scripts) que modifican `local_transform`, `local_params`, `geometry`, o `material_id`. Estas operaciones deben marcar `node.dirty = true`.
2. Llamar `propagate_dirty()` para marcar descendientes si la transformación afecta jerárquicamente.
3. Ejecutar `resolve_inheritance()` solo en nodos `dirty` para actualizar `effective_material` y `effective_params`.
4. Recomponer `world_transform` y `bbox_world` para nodos `dirty` en preorder (padre primero).
5. Preparar lista de `leaves_changed` y decidir `refit` vs `rebuild` del BVH según heurística (p.ej. % hojas movidas).
6. Serializar buffers modificados en CPU y subirlos a GPU con doble buffer: escribir en `SSBO_back` y luego `swap()`.
7. Ejecutar paso de render/trace usando `SSBO_front` estable durante el frame.
8. Post-update: limpiar `dirty` y confirmar caches `effective_cached = true`.

Pseudocódigo: ciclo por frame
```
function frame_step(frame_time, inputs):
  apply_inputs(inputs)
  propagate_dirty(roots)
  for node in nodes_in_preorder(roots):
    if node.dirty:
      resolve_inheritance(node)
      node.world_transform = node.parent ? node.parent.world_transform * node.local_transform : node.local_transform
      node.bbox_world = transform_aabb(node.bbox_local, node.world_transform)
  leaves = collect_leaf_nodes_changed()
  if should_rebuild_bvh(leaves):
    bvh = rebuild_bvh(all_leaf_bboxes)
  else:
    refit_bvh(bvh, leaves)
  upload_changed_buffers_double_buffer(nodes_changed)
  render_pass(SSBO_front)
  clear_dirty_flags(nodes_changed)
```

12.3 Heurística `refit` vs `rebuild`
- `refit`: preferible si <= 15% de las hojas cambiaron o si los cambios son pequeñas traslaciones/rotaciones.
- `rebuild`: preferible si > 15% de hojas cambiaron, si muchas `bbox` cambiaron de forma drásticamente, o cuando la topología/jerarquía cambió.
- Alternativa híbrida: `rebuild` en background thread y swap al terminar (mantener el BVH antiguo hasta que el nuevo esté listo).

12.4 Propagación perezosa y `dirty` flags
- Cambios en `local_transform` marcan `node.transform_dirty = true`.
- `propagate_dirty(node)` marca `node` y todos sus hijos con `world_dirty = true`.
- `resolve_inheritance(node)` usa `effective_cached` para evitar recomputaciones si no hay `dirty` ascendentes.

Pseudocódigo: dirty propagation
```
function mark_dirty(node, reason):
  node.dirty = true
  for child in node.children: mark_dirty(child, reason)

function propagate_dirty(roots):
  for r in roots: if r.dirty: mark_dirty(r, 'external')
```

12.5 Sincronización GPU-friendly (doble buffer y versión)
- Mantener dos conjuntos de SSBOs: `SSBO_front` (usado por render) y `SSBO_back` (escritura).
- CPU escribe cambios en `SSBO_back`; cuando la escritura termina se incrementa `buffer_version` y se hace `swap()` atómico con `SSBO_front` cuando la GPU no está leyendo la versión anterior (o usar fence/semáforo).
- Para cambios parciales, usar `range_update` si la API lo soporta para minimizar tráfico PCIe.

Consideraciones de implementación:
- Evitar sincronización bloqueante en el hilo de render; preferir encolamiento de uploads y fences.
- Si la renderización es asíncrona, mantener historiales de transformaciones por versión para reconstruir trazos de motion vectors o reproyección.

12.6 Muestreo temporal y motion blur (W sampling)
- Para motion blur y efectos temporales, no basta un único `world_transform` por nodo. Soportar `time_samples[]` por nodo (p.ej. 2–8 muestras) o almacenar `prev_world_transform` además de `world_transform`.
- Durante la etapa de render, el shader o el integrador puede pedir `C(p, W_sample)` con `W_sample = g(t_sample)` para cada muestra de tiempo. Integrar resultados por pantalla por Monte Carlo o reconstrucción temporal.

API de sampling (ejemplo):
```
function sample_world(node, w):
  // si node tiene baked_samples: return interpolate_samples(node.samples, w)
  // else return node.world_transform (single sample)
```

Motion vectors: almacenar `prev_world_transform` para cada instancia y calcular `motion_vector = project(T_prev * p) - project(T_curr * p)` durante shading.

12.7 Determinismo y orden de ejecución
- Para reproducibilidad se debe garantizar un orden de actualización determinista: recorrer nodos en pre-order con un orden estable de `children`.
- Las operaciones que dependan de números aleatorios deben usar un RNG determinista por `node.id` y `frame_number` (p.ej. hash64(node.id, frame)).

12.8 Concurrentización y jobs
- División de trabajo típica:
  - tareas I/O: cargar texturas/materiales
  - resolve_inheritance: recorrido por subárboles independientes (paralelizables)
  - recompute_transforms: por subárbol o por paquetes de nodos
  - BVH refit/rebuild: background job con prioridad baja
  - buffer upload: encolado al hilo de transferencia

Diseño de job-graph: crear dependencias explícitas (p.ej. `resolve_inheritance(node)` debe terminar antes que `compute_world_transform(node)`). Las bibliotecas de ejecución (task scheduler) facilitan este grafo.

12.9 Integración con el resolvedor de rayos (sphere_trace)
- `sphere_trace` debe invocar `C(p)` usando `world_state` correspondiente al `W` mapeado por `g(t)`.
- Si el render usa múltiples muestras temporales, `sphere_trace` se ejecuta por muestra con el `w_sample` adecuado.
- Para performance, evaluar la viabilidad de cachear `C(p)` en regiones coherentes por frame/sample.

12.10 Eventos, hooks y control de autor
- Hooks recomendados en la API:
  - `on_pre_frame(frame_time)` — preparar animaciones y scripts
  - `on_update_node(node, frame_time)` — hook por nodo para actualizaciones específicas
  - `on_post_frame(frame_time)` — limpieza, estadísticas

12.11 Pruebas y validación
- Tests unitarios recomendados:
  - verificación de `world_transform` con jerarquía simple (parent->child) tras varios updates.
  - verificación de `bbox_world` tras rotaciones no ortogonales (comprobando AABB conservador).
  - test de `refit` vs `rebuild`: medir que BVH refit mantiene consenso topológico y que rebuild produce estructura más eficiente tras cambios masivos.
  - test de doble-buffer: asegurar que `SSBO_front` no cambia mientras render usa la versión actual.

12.12 Ejemplo mínimo de API (Python-esque)
```
class Scene:
  def frame_step(self, frame_time, inputs):
    self.apply_inputs(inputs)
    self.propagate_dirty()
    self.update_world_transforms()
    self.update_bvh()
    self.upload_changed_buffers()
    self.render(frame_time)

  def apply_inputs(self, inputs):
    # mutaciones: set_transform(node_id, mat), set_param(node_id, k, v), etc.

  def propagate_dirty(self): ...
  def update_world_transforms(self): ...
  def update_bvh(self): ...
  def upload_changed_buffers(self): ...
  def render(self, t): ...
```

12.13 Notas finales y recomendaciones
- Documentar explícitamente la unidad de `W` (recomendado: segundos en floating point) y la función `g(t)` que mapea tiempo real a `W`.
- Preferir refit frecuente y rebuild ocasional con background swap para mantener latencia baja.
- Implementar doble-buffer y fences para evitar stalls en CPU-GPU.
- Ofrecer herramientas de profiling para medir `nodes_updated_per_frame`, `ssbo_upload_bytes`, `bvh_rebuild_time`.

Con esto queda formalizado un protocolo práctico y ejecutable para que la dimensión `W` sea una primera clase en el motor: actualizable por frame, determinista, escalable y compatible con pipelines GPU.

---

## 13. Formalización matemática, teoremas y análisis numérico

Objetivo: cubrir las lagunas señaladas por la crítica añadiendo definiciones formales, enunciados de propiedades, esbozos de demostración, análisis numérico y bibliografía. Esta sección no es una monografía completa, sino un conjunto de condiciones suficientes y pruebas heurísticas que garantizan comportamiento correcto y reproducible del resolvedor.

### 13.1 Espacios funcionales y notación
- Dominio espacio-temporal: \(\mathbb{P}=\mathbb{R}^4\), puntos \(p=(X,Y,Z,W)\).
- Estado interno: \(\mathcal{C}:\mathbb{P}\to\mathcal{S}\), con \(\mathcal{S}\) el espacio de estados \((d,m,c,A,P)\).
- Clase de SDFs: definimos \(\mathcal{SDF}(L)\) como las \(\Phi:\mathbb{R}^4\to\mathbb{R}\) localmente Lipschitz con constante \(L<\infty\). Para SDF exactas habituales se suele tener \(L=1\).
- Densidades: \(A:\mathbb{R}^4\to[0,\infty)\) y para cada rayo fijo la restricción \(A(z)\in L^1_{loc}(\mathbb{R})\). Para aplicar Beer–Lambert se requiere \(A\in L^1([z_0,\infty))\).

### 13.2 Definición formal de operadores y propiedades
- `select_closer:\mathcal{S}\times\mathcal{S}\to\mathcal{S}`. Versión pura: selecciona el estado con menor distancia; heredando propiedades de `min` sobre distancias la reducción es asociativa, conmutativa e idempotente. Si se introducen desempates no simétricos o mezclas, esas propiedades pueden fallar: documentar explícitamente la semántica usada en la implementación.
- `compose(\mathcal{F},\mathcal{Q},\mathcal{B})`: especificar firma y invariantes (p. ej. `compose` preserva `d = min(...)` si se define así).
- `intervene(\mathcal{R},\alpha)`: interpolador continuo en \(\alpha\in[0,1]\). Exigir continuidad en \(\alpha\) para estabilidad numérica.

### 13.3 Convergencia de sphere-tracing (teorema esbozo)
Teorema (convergencia, esbozo). Si \(\Phi\in\mathcal{SDF}(L)\) y existe una raíz aislada a lo largo de la trayectoria del rayo dentro de \([z_0,z_{max}]\), entonces el algoritmo de sphere-tracing con pasos
\(s_k=\max(\epsilon,|\Phi(p_k)|/L)\) alcanza un punto \(p_N\) con \(|\Phi(p_N)|\le\epsilon\) en un número finito de pasos. Además,
\[ N \le \left\lceil \frac{|\Phi(p_0)|}{\epsilon} \right\rceil. \]
Esbozo: uso de Lipschitz y elección segura del paso que garantiza reducción del residuo en cada iteración.

Observación: si \(L\) se subestima, pueden ocurrir overshoots; por ello usar \(L_{est}\ge L\) conservador.

### 13.4 Error numérico y bounds por paso
- Si cada evaluación de \(\Phi\) tiene error acotado \(\delta\), la acumulación tras \(N\) pasos es $O(N\delta)$. Por tanto elegir \(\epsilon\) tal que \(\epsilon \gg N\delta\) evita que el ruido domine la tolerancia.
- Para integración volumétrica por pasos de tamaño \(\Delta z\) el error de la suma Riemann es $O(\Delta z)$ bajo regularidad básica; con trapecio mejora a $O(\Delta z^2)$.

### 13.5 Condiciones para Beer–Lambert e integrabilidad
- Requisitos: \(A(z)\ge0\) y \(A\in L^1([z_0,z_1])\). Con estas hipótesis la transmitancia
\(T(z)=\exp(-\int_{z_0}^z A(s)ds)\) está bien definida y la integral de emisión convergente si \(c(z)A(z)\in L^1\).

### 13.6 Estabilidad temporal (W) y well-posedness
Planteamos la ecuación en W:
\[ \partial_W \mathcal{R}(\cdot,W) = \mathcal{F}(\mathcal{R}(\cdot,W),W). \]
Si \(\mathcal{F}\) es Lipschitz en la norma escogida (p. ej. \(L^\infty\)) y continua en W, por Picard–Lindelöf existe y es única solución local; condiciones de crecimiento lineal garantizan extensión global.

Implicación práctica: diseñar actualizadores (animación, físicas) con restricciones Lipschitz controladas o usar regularización/limitadores.

### 13.7 Complejidad y análisis de coste
- Coste esperado por píxel con BVH: $O(\log n + k)$ donde $n$=nodos, $k$=coste local. Sphere-trace coste proporcional a $N_{steps}$. BVH refit: $O(n)$, rebuild: $O(n\log n)$.
- Trade-off: precisión vs coste (\(\epsilon\) pequeño más pasos). Proveer benchmarks de referencia para seleccionar parámetros.

### 13.8 Lemas formales y demostraciones (selección)

Lema 1 (Idempotencia de `select_closer`). Para todo estado \(s\in\mathcal{S}\) vale:
\[\operatorname{select\_closer}(s,s)=s.\]
Prueba. Si ambas entradas son idénticas, la distancia mínima es la misma y la copia del estado mínimo devuelve el mismo registro; por construcción `select_closer` copia campos del argumento mínimo, por tanto el resultado coincide con la entrada.

Lema 2 (Asociatividad bajo `min` puro). Sea una versión de \(\operatorname{select\_closer}\) que únicamente compare el campo distancia \(d\) y copie el resto de campos del argmínimo. Entonces la operación de plegado sobre un conjunto finito usando esa versión es asociativa.
Prueba (esbozo). La operación subyacente sobre distancias es la función \(\min:\mathbb{R}^n\to\mathbb{R}\) que es asociativa. La copia de los campos asociados al mínimo preserva el mínimo obtenido; por tanto la reducción en cualquier orden produce el mismo resultado.

Teorema 1 (Convergencia de sphere-tracing, versión adaptada). Sea \(\Phi\in\mathcal{SDF}(L)\) localmente Lipschitz con constante \(L\) sobre la trayectoria del rayo. Sea \(p_0\) el punto inicial y supóngase que existe una raíz aislada en \([z_0,z_{max}]\). Si se emplea el paso
\[ s_k=\max\left(\epsilon,\frac{|\Phi(p_k)|}{L_{est}}\right)\]
con \(L_{est}\ge L\), entonces el algoritmo alcanza un punto \(p_N\) con \(|\Phi(p_N)|\le\epsilon\) en número finito de pasos. Además,
\[ N \le \left\lceil \frac{|\Phi(p_0)|}{\epsilon} \right\rceil.\]
Prueba (esbozo). Por Lipschitz, \(|\Phi(p_{k+1})-\Phi(p_k)|\le L\, s_k\). Con la elección de \(s_k\) se garantiza que el residuo disminuye al menos en \(\epsilon\) cada paso salvo cuando ya se alcanza la tolerancia, de donde la cota para \(N\).

Nota sobre hipótesis: si \(L_{est}<L\) pueden darse sobrepasos que invaliden la cota; por eso se recomienda tomar \(L_{est}\) conservador.

Lema 3 (Condiciones de integrabilidad para Beer–Lambert). Si \(A(z)\ge0\) y \(A\in L^1([z_0,\infty))\) entonces la transmitancia
\(T(z)=\exp\left(-\int_{z_0}^z A(s)ds\right)\) está bien definida y acotada en \((0,1]\). Si además \(c(z)A(z)\in L^1([z_0,\infty))\) la integral de emisión es convergente.
Prueba. Por las propiedades de las funciones integrables, la integral del exponente es finita y la exponencial de un número real finito está en \((0,\infty)\); con \(A\ge0\) se obtiene \(T\in(0,1]\). La integrabilidad de \(cA\) garantiza la convergencia de la integral de emisión por criterio de integrabilidad absoluta.

Teorema 2 (Bien planteamiento temporal - Picard–Lindelöf). Considere la ecuación en \(W\)
\[ \partial_W \mathcal{R}(\cdot,W) = \mathcal{F}(\mathcal{R}(\cdot,W),W) \]
sobre un espacio de estados con norma adecuada (por ejemplo \(L^\infty\)). Si \(\mathcal{F}\) es Lipschitz en \(\mathcal{R}\) y continua en \(W\), entonces existe una única solución local en \(W\). Si además se satisfacen condiciones de crecimiento lineal, la solución se extiende globalmente.
Prueba (esbozo). Aplicar Picard–Lindelöf en el espacio funcional escogido; la hipótesis Lipschitz en \(\mathcal{R}\) y continuidad en \(W\) cumplen los requisitos para la existencia y unicidad local. La extensión global sigue de acotaciones a priori y crecimiento controlado.

Estas lemas y teoremas proporcionan las garantías matemáticas básicas necesarias para justificar las prácticas numéricas recomendadas en este documento (elección de \(\epsilon\), estimación conservadora de \(L\), uso de step adaptativo y criterios de integrabilidad para volúmenes).

### 13.9 Apéndice: Benchmarks reproducibles (resumen)

Se incluyen scripts de referencia en `prototipo/benchmarks/` que permiten reproducir tablas básicas de sensibilidad y rendimiento:

- `bench_sphere_trace.py` — mide número de pasos y error frente a \(\epsilon\) y \(L_{est}\) para SDFs sencillas (esfera).
- `bench_volume_transmission.py` — compara integración numérica con la solución analítica para una losa uniforme, variando `density_scale` y `step_size`.
- `bench_bvh_refit_rebuild.py` — script microbenchmark que simula cargas de trabajo y mide tiempo relativo de `refit` vs `rebuild` con arrays de cajas.

Instrucciones rápidas para ejecutar los benchmarks (desde la raíz del repositorio):

```
python Lienzo\ y\ renderizado\prototipo\benchmarks\bench_sphere_trace.py
python Lienzo\ y\ renderizado\prototipo\benchmarks\bench_volume_transmission.py
python Lienzo\ y\ renderizado\prototipo\benchmarks\bench_bvh_refit_rebuild.py
```

Los scripts generan CSV en `prototipo/benchmarks/out/` y muestran tablas con resultados esperados como referencia.

#### Resumen de resultados (v1.0)

1. **Sphere Tracing (Sensibilidad a \(L_{est}\))**:
   - Con \(L_{est} = 1.0\) (exacto): Convergencia en 1 paso para \(\epsilon \in [10^{-2}, 10^{-5}]\).
   - Con \(L_{est} = 2.0\) (conservador): Convergencia en 9 a 19 pasos.
   - Con \(L_{est} = 0.5\) (sobreestimación): Fallo de convergencia (overshoot masivo).
   - *Conclusión*: Usar siempre \(L_{est} \ge L\).

2. **Transmisión Volumétrica (Precisión)**:
   - Error \(L_1\) frente a solución analítica: \(< 10^{-14}\) para todos los `step_size` probados.
   - La aproximación discretizada es extremadamente estable para medios uniformes.

3. **BVH (Refit vs Rebuild)**:
   - N=1000: Speedup de ~1.5x usando `refit`.
   - N=5000: Speedup de ~2.8x usando `refit`.
   - *Conclusión*: `refit` es significativamente más rápido para actualizaciones de movimiento en escenas densas.

---

Fin de la sección formal.

### 13.8 Política sólido–volumen: formulación matemática
Para un rayo $p(z)$, \(\alpha(z)=density\_scale\cdot A(z)\). Si el primer sólido aparece en $z_{hit}$,
\[ C_{final} = \int_{z_0}^{z_{hit}} T(z)\,c_v(z)\,\alpha(z)\,dz + T(z_{hit})\,c_s. \]
Esta expresión es monótona en coeficientes positivos y compatible con interpretaciones físicas de atenuación y emisión; para coherencia energética exigir BRDF/materials energy-conserving.

### 13.9 Tolerancias y sensibilidad
- Reglas prácticas: elegir \(\epsilon\) relativo a la escala de escena $S$ (ej. $\epsilon\in[10^{-6}S,10^{-4}S]$). Elegir \(\Delta z\le r_{inf}/8\) para volúmenes.
- Estudiar sensibilidad numérica mediante vectores de prueba (ver sección de benchmarks).

### 13.10 Contrato formal de `material_id` y BRDF
- `material_id \mapsto M \in \mathcal{P}` donde \(M\) contiene parámetros y un BRDF que satisfaga la cota de energía
\[ \int_{\Omega} f_r(\omega_i,\omega_o) (\omega_i\cdot n) d\omega_i \le 1, \]\n+para todo \(\omega_o\).

### 13.11 Casos degenerados y medidas correctivas
- NaN en \(\Phi\) o \(A\): tratar como vacío ($A=0$, $d=+\infty$).
- Transformaciones singulares: detectar \(|\det T|<\tau\) y usar fallback (identidad o marcar nodo no renderizable).
- Empates dentro de \(\epsilon\): aplicar regla determinista (priority hash) o blending simétrico si ambos volumes.

### 13.12 Benchmarks y vectores de referencia (sugeridos)
- Caso 1: esfera SDF — medir pasos vs \(\epsilon\) y error en distancia.
- Caso 2: slab volumétrico + sólido — comparar transmisión analítica vs numérica para \(\Delta z\) variables.
- Caso 3: escena con N instancias — medir `refit` vs `rebuild` tiempo y throughput.

### 13.13 Bibliografía mínima
- Hart, R., "Sphere tracing: a geometric method for the antialiased ray tracing of implicit surfaces", 1996.
- Pharr, Jakob, Humphreys, "Physically Based Rendering" (BRDF, energía).
- Kajiya, "The Rendering Equation", SIGGRAPH 1986.
- Osher & Sethian, "Level set methods" (JCP, 1988).
- K. Atkinson, "An Introduction to Numerical Analysis" (cuadratura y errores).

### 13.14 Checklist de reproducibilidad
- Registrar \(L\) estimado para primitivas; declarar `eps`, `max_steps`, `density_scale`, `r_inf`, `z_max` en fichero de escena.
- Proveer test vectors (JSON + resultado) como referencia para verificaciones inter-implementación.

Con esto cerramos las principales faltas formales: añadimos hipótesis, propiedades de operadores, resultados de convergencia, criterios de estabilidad temporal, análisis de complejidad y bibliografía para fundamentar la especificación.

---

## 14. Protocolo de Escalabilidad: Contratos para un Cosmos Determinista Ejecutable

Esta sección formaliza las soluciones a las vulnerabilidades críticas identificadas mediante análisis adversarial del modelo. El objetivo es que el universo 4D descrito sea no solo matemáticamente coherente, sino **computacionalmente ejecutable a escala**, sin comprometer la solidez del modelo formal.

Las cuatro vulnerabilidades primarias y sus contratos de solución son:

1. **Lipschitz roto en SDFs compuestas** → Jacobian Step Scaling
2. **Tiempo $W$ estático vs. física dinámica reactiva** → Modelo híbrido cinemática/dinámica bakeada
3. **Cuello de botella de BVH y PCIe** → Arquitectura TLAS/BLAS con dirty flags a tres niveles
4. **Aliasing de fronteras de material** → SDF-Aware Material Blending

---

### 14.1 Jacobian Step Scaling: Garantía de Convergencia con SDFs No Lipschitz-1

#### Problema formal

El `sphere_trace` estándar avanza con paso $s_k = |\Phi(p_k)| / L_{est}$. Si el campo es resultado de uniones CSG de múltiples SDFs, transformaciones no rígidas, o deformaciones espaciales, el gradiente local puede exceder $L_{est}$, causando overshoot y agujeros geométricos.

#### Contrato de Solución

El paso del sphere tracer se escala localmente usando la norma del gradiente evaluada numéricamente:

$$s_k = \frac{|\Phi(p_k)|}{\max(1, \|\nabla\Phi(p_k)\|_2)}$$

donde $\nabla\Phi(p_k)$ se aproxima por diferencias finitas centradas de primer orden:

$$\nabla\Phi \approx \frac{1}{2h}\begin{bmatrix}
\Phi(p + h\hat{x}) - \Phi(p - h\hat{x}) \\
\Phi(p + h\hat{y}) - \Phi(p - h\hat{y}) \\
\Phi(p + h\hat{z}) - \Phi(p - h\hat{z})
\end{bmatrix}$$

con $h = \max(\epsilon, 10^{-3} \cdot |\Phi(p_k)|)$ para estabilidad numérica.

**Coste adicional:** 6 evaluaciones extra de SDF por paso. Aceptable en GPU donde los accesos de memoria son la operación dominante.

**Invariante garantizado:** Si $\|\nabla\Phi\| \le L$ casi en todas partes, entonces $s_k \ge |\Phi(p_k)| / L$, preservando la cota de convergencia del Teorema 1 (Sec. 13.3).

**Pseudocódigo actualizado:**

```
function sphere_trace_safe(ray, max_steps=128, eps=1e-4, z_max=100.0, h=1e-3):
    z = ray.origin_z
    for i in range(max_steps):
        p = ray.at(z)
        s = C(p)
        if is_nan(s.d): return no_hit
        if abs(s.d) < eps: return z
        grad = central_diff_gradient(p, h)
        grad_norm = max(1.0, norm(grad))
        step = clamp(s.d / grad_norm, eps, z_max - z)
        z += step
        if z >= z_max: return no_hit
    return no_hit
```

**Nota de implementación:** En shaders GLSL/HLSL, `central_diff_gradient` se implementa como 6 muestras de textura o 6 llamadas al evaluador SDF. Para SDFs analíticas de primitivas estándar (esferas, cajas, cilindros), el gradiente puede calcularse analíticamente como $\hat{n}$ sin coste adicional.

---

### 14.2 Modelo Híbrido de Temporalidad: Cinemática Analítica y Dinámica Bakeada

#### Problema formal

El modelo trata $W$ como una dimensión espacial estática: toda la geometría en $\mathbb{R}^4$ debe estar definida de antemano. Esto es válido para cinemática prescrita (velocidades constantes, keyframes interpolados), pero es computacionalmente inviable para física dinámica reactiva (colisiones, deformaciones plásticas, fracturas).

#### Contrato de Solución: Dos Capas Temporales

El sistema distingue formalmente dos tipos de nodos por su comportamiento en $W$:

| Tipo | Descripción | Representación de $T(W)$ | Evaluación en runtime |
|------|-------------|--------------------------|----------------------|
| **Cinemático** | Trayectoria analítica prescrita | $T(W) = T_0 + vW$ o polinomio en $W$ | Evaluación directa |
| **Dinámico-Bakeado** | Física simulada offline | Tabla de muestras $\{(W_j, T_j)\}$ | Interpolación en GPU |

**Protocolo de bakeado:**

Un simulador físico externo al motor corre offline para el intervalo $[W_{min}, W_{max}]$ y genera una tabla de keyframes $\{(W_j, T_j, \text{topology}_j)\}$ para cada nodo dinámico:

$$\text{node.time\_samples} = \{(W_0, T_0), (W_1, T_1), \ldots, (W_k, T_k)\}$$

La interpolación de transformaciones en GPU usa B-Spline cúbica para preservar continuidad $C^2$ del movimiento:

$$T(W) = \text{BSpline}(\{(W_j, T_j)\}, W) \quad \text{si nodo es dinámico-bakeado}$$

**Eventos discretos (colisiones, fracturas):**

Un evento discreto en $W = W_e$ se representa como un **cambio de topología del grafo** a ese instante exacto. El contrato formal es:

$$\forall W < W_e : \text{grafo}(W) = \mathcal{G}_{antes}$$
$$\forall W \ge W_e : \text{grafo}(W) = \mathcal{G}_{despues}$$

El campo biológico gestiona esto como una colección de grafos etiquetados por intervalo temporal, no como un único grafo mutable.

**Definición formal del grafo etiquetado por intervalo:**

$$\mathcal{B}(W) = \mathcal{G}_k \quad \text{si } W \in [W_k, W_{k+1})$$

donde $\{W_k\}$ son los instantes de cambio topológico. En runtime, el motor busca el intervalo correcto y evalúa $\mathcal{G}_k$.

**Implicación para el modelo:** El universo 4D sigue siendo determinista y estático. La física fue horneada antes de la ejecución. El motor solo interpola y selecciona; nunca simula en tiempo real.

---

### 14.3 Arquitectura TLAS/BLAS: Dirty Flags a Tres Niveles

#### Problema formal

Actualizar el BVH completo y subir todos los SSBOs cada frame genera cuellos de botella en CPU y PCIe cuando hay muchos nodos dinámicos. La causa raíz es no distinguir el tipo de cambio: no es igual mover un objeto (transformación) que deformarlo (geometría) o añadir un hijo (topología).

#### Contrato de Solución: Dos Niveles de Aceleración

El grafo de escena se organiza en dos niveles de aceleración inspirados en la arquitectura de ray tracing de hardware (DXR / Vulkan RT):

**BLAS (Bottom-Level Acceleration Structure):**
- Representa la geometría local de cada primitiva (SDF, caja, esfera, etc.)
- Se reconstruye **solo cuando `geometry_dirty = true`** en un nodo hoja
- Coste: $O(m)$ donde $m$ son los parámetros de la primitiva
- En la terminología del documento: corresponde a `GeometryTable[]`

**TLAS (Top-Level Acceleration Structure):**
- Representa las instancias del mundo: referencias a BLAS + transformaciones world
- Se **refita o actualiza parcialmente cada frame** cuando cambian transformaciones
- Coste: $O(n_{changed})$ con range updates
- En la terminología del documento: corresponde a `NodeArray[] + transforms_buffer[]`

#### Contrato de Dirty Flags (tres niveles)

Cada nodo debe declarar qué tipo de cambio ha ocurrido mediante flags independientes:

```
struct DirtyFlags:
    transform_dirty: bool   // solo cambió mat4 → update parcial de transforms_buffer
    geometry_dirty:  bool   // cambió la SDF / primitiva → rebuild BLAS del nodo
    hierarchy_dirty: bool   // cambió parent/children → rebuild parcial TLAS
    material_dirty:  bool   // cambió material_id o local_params → update parcial de params_buffer
```

**Reglas de propagación y coste:**

| Flag activo | Acción requerida | Coste |
|-------------|-----------------|-------|
| `transform_dirty` | `range_update(transforms_buffer, node_idx)` + `refit_tlas()` | $O(1)$ upload + $O(\log n)$ refit |
| `geometry_dirty` | `rebuild_blas(node)` + `update_blas_reference_in_tlas()` | $O(m)$ rebuild |
| `hierarchy_dirty` | `rebuild_tlas_partial()` o `rebuild_tlas_full()` | $O(n \log n)$ rebuild |
| `material_dirty` | `range_update(params_buffer, node_idx)` | $O(1)$ upload |

**Pseudocódigo del ciclo de frame actualizado:**

```
function frame_step(frame_time, inputs):
    apply_inputs(inputs)
    propagate_dirty(roots)
    for node in nodes_in_preorder(roots):
        if node.dirty.transform:
            node.world_transform = compose_transforms(node)
            node.bbox_world = transform_aabb(node.bbox_local, node.world_transform)
            upload_range(transforms_buffer, node.idx)
        if node.dirty.geometry:
            rebuild_blas(node)
            upload_blas_descriptor(node)
        if node.dirty.material:
            upload_range(params_buffer, node.idx)
    if any(node.dirty.hierarchy for node in changed_nodes):
        rebuild_tlas_partial(changed_nodes)
    elif any(node.dirty.transform for node in changed_nodes):
        refit_tlas(changed_nodes)
    swap_ssbo_buffers()
    render_pass(SSBO_front)
    clear_all_dirty_flags()
```

**Invariante de doble buffer:** `SSBO_front` nunca se modifica durante `render_pass`. Todas las escrituras ocurren en `SSBO_back`. El swap es atómico y ocurre solo tras completar todos los uploads del frame.

---

### 14.4 SDF-Aware Material Blending: Eliminación del Aliasing de Fronteras

#### Problema formal

El operador `select_closer` actúa como una función escalón en la frontera entre dos materiales. Cuando dos objetos son coplanares o casi coplanares a escala de subpíxel, el cambio discontinuo de material genera flickering visual y bordes dentados.

#### Contrato de Solución

En la zona de transición subpíxel, se reemplaza la selección rígida por una interpolación suave ponderada por la diferencia de distancias:

**Zona de transición:** El blending aplica cuando $|d_1 - d_2| < \delta_{px}$, donde $\delta_{px}$ es el ancho de un píxel proyectado en unidades del mundo:

$$\delta_{px} = \frac{\partial_{xy} \text{proj}}{res_{px}} \cdot k_{blend}$$

con $k_{blend} \in [1, 3]$ como factor de suavizado configurable por escena.

**Factor de interpolación:**

$$\alpha_{blend} = \text{clamp}\left(\frac{d_2 - d_1}{2\delta_{px}} + 0.5,\ 0,\ 1\right)$$

**Resultado blended:**

$$\mathbf{c}_{out} = \text{LERP}(\mathbf{c}_1, \mathbf{c}_2,\ \alpha_{blend})$$
$$A_{out} = \text{LERP}(A_1, A_2,\ \alpha_{blend})$$

**Materiales:**
- Si $\alpha_{blend} < 0.5$: $m_{out} = m_1$ (material dominante para lookup de propiedades como rugosidad)
- Si $\alpha_{blend} \ge 0.5$: $m_{out} = m_2$

**Nota importante:** El blending solo aplica a $\mathbf{c}$ y $A$ (canales visuales). Las propiedades físicas $P$ (densidad, conductividad) no se mezclan; pertenecen al material dominante. Esto garantiza que el blending sea puramente óptico y no altere la semántica física del campo.

**Pseudocódigo del `select_closer` actualizado:**

```
function select_closer_blended(s1, s2, delta_px=1e-3, k_blend=2.0):
    threshold = delta_px * k_blend
    if abs(s1.d - s2.d) >= threshold:
        // selección rígida estándar
        if s1.d + eps < s2.d: return s1
        if s2.d + eps < s1.d: return s2
        return resolve_tie(s1, s2)
    // zona de transición subpíxel: blending óptico
    alpha = clamp((s2.d - s1.d) / (2 * threshold) + 0.5, 0.0, 1.0)
    s_out.c = lerp(s1.c, s2.c, alpha)
    s_out.A = lerp(s1.A, s2.A, alpha)
    s_out.d = min(s1.d, s2.d)
    s_out.m = s1.m if alpha < 0.5 else s2.m
    s_out.P = s1.P if alpha < 0.5 else s2.P
    return s_out
```

---

### 14.5 Tabla de Contratos Completa: Invariantes Garantizados por el Sistema

Con esta sección, el modelo garantiza los siguientes invariantes de forma explícita:

| Invariante | Garantía Formal | Mecanismo |
|------------|----------------|-----------|
| **Convergencia de sphere_trace** | $N_{pasos} \le \lceil |\Phi(p_0)| / \epsilon \rceil$ | Jacobian Step Scaling (14.1) |
| **Determinismo temporal** | $\mathcal{C}(p, W)$ es estático para todo $W$ | Bakeado offline de física (14.2) |
| **Consistencia de topología** | $\mathcal{G}(W)$ es constante en cada intervalo $[W_k, W_{k+1})$ | Grafos etiquetados por intervalo (14.2) |
| **Ausencia de stalls en GPU** | `SSBO_front` no se modifica durante render | Doble buffer con swap atómico (14.3) |
| **Costo proporcional al cambio** | Solo se suben a GPU los buffers marcados `dirty` | Dirty flags a tres niveles (14.3) |
| **Continuidad visual en fronteras** | Sin flickering de material en $|d_1 - d_2| < \delta_{px}$ | SDF-Aware Blending (14.4) |
| **Integrabilidad volumétrica** | $T(z) \in (0,1]$ y $C_{final}$ converge | $A \ge 0$, $A \in L^1$ (Sec. 13.5) |
| **Unicidad de solución en $W$** | $\partial_W \mathcal{R}$ bien planteado | $\mathcal{F}$ Lipschitz en $\mathcal{R}$ (Sec. 13.6) |

---

### 14.6 Checklist de Solidez del Modelo v2.0

Antes de considerar el modelo implementable, verificar:

- [ ] Todo nodo declara `node_type: kinematic | dynamic_baked` y `dirty_flags` a cuatro campos
- [ ] `sphere_trace` usa Jacobian Step Scaling o gradient analítico para primitivas estándar
- [ ] Las transformaciones dinámicas tienen `time_samples[]` generados por simulador offline
- [ ] Los eventos discretos (colisiones, fracturas) están representados como intervalos en $\mathcal{G}(W)$
- [ ] `select_closer` tiene blending óptico activado en zonas de transición subpíxel
- [ ] El ciclo de frame distingue entre transform/geometry/hierarchy/material dirty flags
- [ ] Los SSBOs usan doble buffer con swap atómico tras fence de GPU
- [ ] Los benchmarks de `bench_sphere_trace.py`, `bench_volume_transmission.py` y `bench_bvh_refit_rebuild.py` pasan con los parámetros guía declarados en Sec. 11.2

---

## 15. Contratos de Frontera: Cierre de Vulnerabilidades de Segunda Generación

Esta sección formaliza los contratos que cierran las seis vulnerabilidades identificadas en el análisis adversarial de segunda ronda sobre el modelo v2.0. Cada subsección define el contrato, la ecuación que lo fija, y el pseudocódigo o esquema de datos correspondiente.

---

### 15.1 Contrato de Integración con el Simulador Externo

#### Problema

La Sec. 14.2 delega la física dinámica a un simulador externo pero no especifica el protocolo de entrega: frecuencia mínima de muestreo, validación de energía, ni compatibilidad con la jerarquía del grafo biológico.

#### Contrato Formal

**Formato de entrega de keyframes (JSON Schema v1):**

```json
{
  "type": "object",
  "properties": {
    "node_id":   { "type": "string" },
    "node_type": { "enum": ["kinematic", "dynamic_baked"] },
    "samples": {
      "type": "array",
      "items": {
        "type": "object",
        "properties": {
          "W":         { "type": "number" },
          "transform": { "type": "array", "minItems": 16, "maxItems": 16 },
          "energy":    { "type": "number", "minimum": 0 }
        },
        "required": ["W", "transform"]
      }
    },
    "topology_events": {
      "type": "array",
      "items": {
        "type": "object",
        "properties": {
          "W_event":   { "type": "number" },
          "add_nodes": { "type": "array", "items": { "type": "string" } },
          "del_nodes": { "type": "array", "items": { "type": "string" } }
        },
        "required": ["W_event"]
      }
    }
  },
  "required": ["node_id", "node_type", "samples"]
}
```

**Frecuencia mínima de muestreo:**

Sea $f_{max}$ la frecuencia máxima de movimiento del nodo (estimada como $v_{max} / r_{min}$ donde $v_{max}$ es la velocidad lineal máxima y $r_{min}$ el radio mínimo del AABB del nodo). Entonces la frecuencia de muestreo debe satisfacer el criterio de Nyquist:

$$f_{samples} \ge 2 \cdot f_{max}$$

En la práctica se recomienda $f_{samples} \ge 4 \cdot f_{max}$ para interpolación cúbica estable. Si el simulador no puede garantizar esto, el importador rechaza el archivo con error `E_UNDERSAMPLE`.

**Validación de energía (opcional pero recomendada):**

Para cada par de keyframes consecutivos $(W_j, E_j)$ y $(W_{j+1}, E_{j+1})$, se valida:

$$E_{j+1} \le E_j + W_{ext,j}$$

donde $W_{ext,j}$ es el trabajo externo aplicado en ese intervalo (provisto por el simulador). Si la condición falla, el motor emite advertencia `W_ENERGY_VIOLATION` y permite importar con flag `--allow-energy-violation` para uso artístico consciente.

**Validación de jerarquía:**

Antes de integrar la tabla de keyframes, el importador verifica que todo `node_id` referenciado en `topology_events.add_nodes` y `topology_events.del_nodes` existe o puede crearse en el grafo biológico sin violar la política de herencia definida en Sec. 7.1. Si se detecta un `node_id` huérfano, el motor emite error `E_ORPHAN_NODE`.

**Pseudocódigo del importador:**

```
function import_baked_simulation(sim_data, scene_graph):
    for node_data in sim_data:
        validate_sampling_rate(node_data.samples)   // falla con E_UNDERSAMPLE
        validate_hierarchy(node_data, scene_graph)   // falla con E_ORPHAN_NODE
        validate_energy(node_data.samples)           // advierte W_ENERGY_VIOLATION
        node = scene_graph.find(node_data.node_id)
        node.node_type = node_data.node_type
        node.time_samples = node_data.samples
        for event in node_data.topology_events:
            scene_graph.register_topology_event(event.W_event, event.add_nodes, event.del_nodes)
```

---

### 15.2 `h_min` Estable para el Gradiente Numérico en Frontera

#### Problema

El paso de diferencias finitas $h = \max(\epsilon, 10^{-3} \cdot |\Phi(p)|)$ colapsa a $\epsilon$ cuando el rayo está en la frontera exacta ($|\Phi| \to 0$), produciendo un gradiente numéricamente inestable o nulo.

#### Contrato Formal

Se introduce un parámetro de escena $h_{min}$ **independiente del valor de $\Phi$**, fijado en función de la escala geométrica de la escena:

$$h_{min} = s_{scene} \times 10^{-3}$$

donde $s_{scene}$ es la longitud característica de la escena (recomendado: radio del AABB raíz del grafo biológico o, en su defecto, `z_max * 1e-3`).

El paso de diferencias finitas pasa a ser:

$$h = \max(h_{min}, \; 10^{-3} \cdot |\Phi(p_k)|)$$

**Invariante garantizado:** $h \ge h_{min} > 0$ en todo punto del dominio, incluyendo la frontera exacta. El gradiente estimado permanece finito y no degenerado.

**Pseudocódigo actualizado del gradiente:**

```
function central_diff_gradient(p, Phi_at_p, h_min, scene_scale):
    h = max(h_min, 1e-3 * abs(Phi_at_p))
    gx = (C(p + h*[1,0,0]).d - C(p - h*[1,0,0]).d) / (2*h)
    gy = (C(p + h*[0,1,0]).d - C(p - h*[0,1,0]).d) / (2*h)
    gz = (C(p + h*[0,0,1]).d - C(p - h*[0,0,1]).d) / (2*h)
    return [gx, gy, gz]
```

**Declaración obligatoria en fichero de escena:**

```json
{ "h_min": 0.001, "scene_scale": 10.0 }
```

---

### 15.3 Blending en Espacio de Parámetros: Conservación de Energía del BRDF

#### Problema

La Sec. 14.4 mezcla linealmente los colores de salida $\mathbf{c}_1$ y $\mathbf{c}_2$. Si los materiales tienen BRDFs especulares distintas, el color de salida ya ha integrado la distribución de normales del material, y mezclarlo puede producir una radianza que viola la cota de conservación de energía del BRDF: $\int_\Omega f_r(\omega_i, \omega_o)(\omega_i \cdot n)d\omega_i \le 1$.

#### Contrato Formal

El blending de frontera **no opera sobre colores de salida**. Opera sobre los **parámetros del material** antes de la evaluación del BRDF. La mezcla produce un material compuesto temporal:

$$\text{roughness}_{out} = \text{LERP}(\text{roughness}_1, \text{roughness}_2, \; \alpha_{blend})$$
$$\text{metallic}_{out} = \text{LERP}(\text{metallic}_1, \text{metallic}_2, \; \alpha_{blend})$$
$$\text{base\_color}_{out} = \text{LERP}(\text{base\_color}_1, \text{base\_color}_2, \; \alpha_{blend})$$

El BRDF se evalúa **una sola vez** sobre este material compuesto. La conservación de energía queda garantizada porque el BRDF de cada componente satisface la cota individualmente, y la interpolación convexa de parámetros produce un BRDF interpolado que también la satisface (por convexidad del espacio de BRDFs físicamente plausibles).

**Demostración de conservación (esbozo):** Si $f_{r,1}$ y $f_{r,2}$ son BRDFs energéticamente correctas con $\int f_{r,i} \le 1$, entonces para $\alpha \in [0,1]$, el BRDF interpolado $f_{r,\alpha} = \text{BRDF}(\alpha \cdot \text{params}_1 + (1-\alpha) \cdot \text{params}_2)$ hereda la cota si el espacio de parámetros está parametrizado de forma que la interpolación preserve la monotonía de la integral de hemiesfera. Esto es cierto para BRDFs de Cook-Torrance y GGX con los parámetros estándar.

**Pseudocódigo del `select_closer_blended` corregido:**

```
function select_closer_blended(s1, s2, shading_ctx, delta_px, k_blend=2.0):
    threshold = delta_px * k_blend
    diff = abs(s1.d - s2.d)

    if diff >= threshold:
        // selección rígida: retornar el estado más cercano directamente
        closer = s1 if s1.d < s2.d else s2
        color = evaluate_brdf(material_table[closer.m], closer.c, shading_ctx)
        return make_output(color, closer.A, closer.d)

    // zona de transición: blending en espacio de parámetros
    alpha = clamp((s2.d - s1.d) / (2*threshold) + 0.5, 0.0, 1.0)
    mat1 = material_table[s1.m]
    mat2 = material_table[s2.m]

    blended_mat = {
        base_color: lerp(mat1.base_color, mat2.base_color, alpha),
        roughness:  lerp(mat1.roughness,  mat2.roughness,  alpha),
        metallic:   lerp(mat1.metallic,   mat2.metallic,   alpha)
    }
    blended_A = lerp(s1.A, s2.A, alpha)
    blended_d = min(s1.d, s2.d)

    color = evaluate_brdf(blended_mat, blended_mat.base_color, shading_ctx)
    return make_output(color, blended_A, blended_d)
```

**Nota:** Las propiedades físicas $P$ (densidad, conductividad) no se mezclan y pertenecen al material dominante (el de menor $d$), para no alterar la semántica de la capa química.

---

### 15.4 Delta-Encoding de Grafos por Intervalo: Control de Explosión de Memoria

#### Problema

El modelo de grafos etiquetados por intervalo $\mathcal{B}(W) = \mathcal{G}_k$ para $W \in [W_k, W_{k+1})$ almacena cada $\mathcal{G}_k$ completo. Con muchos eventos discretos (fracturas, colisiones masivas), la memoria crece como $O(n_{nodos} \times n_{intervalos})$.

#### Contrato Formal: Delta-Encoding

En lugar de grafos completos, el sistema almacena:
- Un **grafo base** $\mathcal{G}_0$ completo en $W = W_0$.
- Una **secuencia de deltas** $\{\Delta_k\}$ donde cada delta describe solo los cambios ocurridos en $W_k$:

$$\Delta_k = (\text{nodes\_added}_k, \; \text{nodes\_removed}_k, \; \text{transforms\_changed}_k, \; \text{materials\_changed}_k)$$

La reconstrucción del grafo en cualquier $W$ se obtiene aplicando los deltas acumulados:

$$\mathcal{G}(W) = \mathcal{G}_0 \oplus \Delta_1 \oplus \Delta_2 \oplus \cdots \oplus \Delta_{k(W)}$$

donde $k(W) = \max\{k : W_k \le W\}$.

**Complejidad de memoria:** $O(n_0 + \sum_k |\Delta_k|)$ donde $n_0$ es el tamaño del grafo base y $|\Delta_k|$ el número de cambios por evento. Para escenas donde la mayoría de nodos son estáticos, $|\Delta_k| \ll n_0$.

**Snapshots periódicos para acceso aleatorio eficiente:**

Para evitar que la reconstrucción de $\mathcal{G}(W)$ requiera aplicar todos los deltas desde el inicio, el sistema genera snapshots completos cada $N_{snap}$ eventos:

$$\mathcal{G}_{snap,j} = \mathcal{G}_0 \oplus \Delta_1 \oplus \cdots \oplus \Delta_{j \cdot N_{snap}}$$

La reconstrucción de cualquier $W$ requiere como máximo $N_{snap}$ aplicaciones de delta desde el snapshot anterior. Se recomienda $N_{snap} = 64$ como valor por defecto.

**Esquema de datos:**

```json
{
  "base_graph": { /* grafo completo G_0 */ },
  "snapshot_interval": 64,
  "snapshots": [
    { "event_index": 64, "graph": { /* snapshot completo */ } }
  ],
  "deltas": [
    {
      "W_event": 1.23,
      "nodes_added":   [{ "id": "frag_001", "parent": "object_A", "geometry": { ... } }],
      "nodes_removed": ["object_A_leg_3"],
      "transforms_changed": [{ "id": "frag_002", "transform": [ ... ] }],
      "materials_changed": []
    }
  ]
}
```

**Pseudocódigo de reconstrucción:**

```
function reconstruct_graph(W, base_graph, deltas, snapshots, N_snap):
    k = find_interval_index(W, deltas)
    snap_idx = floor(k / N_snap)
    if snap_idx > 0 and snapshots[snap_idx] exists:
        graph = clone(snapshots[snap_idx].graph)
        start = snap_idx * N_snap
    else:
        graph = clone(base_graph)
        start = 0
    for i in range(start, k+1):
        apply_delta(graph, deltas[i])
    return graph
```

---

### 15.5 `delta_px` por Cone Footprint: Definición GPU-Eficiente

#### Problema

El parámetro $\delta_{px}$ del SDF-Aware Blending (Sec. 14.4) depende de la proyección de la cámara, pero no estaba definido de forma evaluable dentro del shader de ray marching, donde varía por rayo y por profundidad.

#### Contrato Formal

$\delta_{px}$ se define como el **footprint del cono del rayo** en el punto de evaluación a profundidad $z$:

$$\delta_{px}(z) = z \cdot \tan\!\left(\frac{\theta_{px}}{2}\right)$$

donde $\theta_{px}$ es el ángulo subtendido por un píxel en la dirección horizontal:

$$\theta_{px} = \frac{\text{fov}_y}{res_y}$$

con $\text{fov}_y$ el campo visual vertical (en radianes) y $res_y$ la resolución vertical en píxeles. Esta fórmula es evaluable en $O(1)$ por rayo usando solo la profundidad actual del ray marcher.

**Constante precomputada por frame:**

$$\lambda_{cone} = \tan\!\left(\frac{\text{fov}_y}{2 \cdot res_y}\right)$$

Entonces en el shader, por cada punto de evaluación a profundidad $z$:

$$\delta_{px}(z) = z \cdot \lambda_{cone}$$

**Declaración de uniforme en shader:**

```glsl
uniform float u_cone_lambda;   // precomputado en CPU: tan(fov_y / (2 * res_y))

// En el ray marcher, al evaluar select_closer_blended:
float delta_px = z_current * u_cone_lambda;
InternalState result = select_closer_blended(s1, s2, shading_ctx, delta_px, k_blend);
```

**Declaración en fichero de escena:**

```json
{ "fov_y_degrees": 60.0, "res_y": 1080 }
```

El motor calcula `u_cone_lambda = tan(radians(fov_y_degrees) / (2.0 * res_y))` en CPU una vez por frame y lo sube como uniform.

---

### 15.6 Modelo de Transporte de Luz: Emisión, Fuentes y Ecuación de Renderizado

#### Problema

El modelo define geometría, materiales y BRDF, pero no especifica cómo la luz existe en el campo ni cómo se resuelve la integral de iluminación incidente $L_i(\omega)$ requerida por la ecuación de renderizado de Kajiya.

#### Contrato Formal: Extensión del Estado Interno con Emisión

El estado interno $\mathcal{R}(\mathbf{p})$ se extiende con un canal de emisión:

$$\mathcal{R}(\mathbf{p}) = (d, m, \mathbf{c}, A, P, \mathbf{E})$$

donde $\mathbf{E} = [E_r, E_g, E_b] \in [0, \infty)^3$ es la radianza emitida por el punto $\mathbf{p}$ en todas las direcciones (emisión isotrópica). Para la mayoría de los materiales, $\mathbf{E} = \mathbf{0}$. Para fuentes de luz, $\mathbf{E} > \mathbf{0}$.

**Contrato del `material_table` extendido:**

```json
{
  "sun_light": {
    "id": "sun_light",
    "base_color": [1.0, 0.98, 0.9],
    "roughness": 1.0,
    "metallic": 0.0,
    "emission": [10.0, 9.8, 9.0],
    "emission_mode": "isotropic"
  }
}
```

Campos nuevos en `MaterialTable`:
- `emission: [Er, Eg, Eb]` — radianza emitida en $W/m^2/sr$. Por defecto `[0,0,0]`.
- `emission_mode: "isotropic" | "directional"` — si directional, el material incluye `emission_direction: [dx, dy, dz]`.

**Extensión de `collapse()`:**

```
function collapse(internal, material_table, shading_ctx):
    if internal.m == null or internal.A <= 0: return [0,0,0,0]
    mat = material_table[internal.m]
    if mat == null: return [0,0,0,0]

    // emisión propia
    emitted = mat.emission or [0,0,0]

    // iluminación: seleccionar modo
    if shading_ctx.mode == 'direct':
        reflected = evaluate_direct_illumination(mat, internal, shading_ctx)
    elif shading_ctx.mode == 'ao':
        ao = evaluate_ambient_occlusion(internal.p, shading_ctx)
        reflected = mat.base_color * ao * shading_ctx.ambient_radiance
    elif shading_ctx.mode == 'path_trace':
        reflected = evaluate_path_trace_sample(mat, internal, shading_ctx)

    color = emitted + reflected
    alpha = internal.A * (mat.shader_params.opacity if present else 1.0)
    return [color.r, color.g, color.b, alpha]
```

#### Tres Modos de Iluminación Soportados

**Modo 1 — Iluminación Directa (`direct`):**

Itera sobre todas las fuentes de luz registradas como nodos con `emission > 0` en la escena. Para cada fuente $l$ visible desde el punto $\mathbf{p}$ (sin oclusión):

$$L_{direct}(\mathbf{p}) = \sum_{l \in \text{luces}} f_r(\omega_l, \omega_o) \cdot L_l \cdot (\omega_l \cdot \hat{n}) \cdot V(\mathbf{p}, l)$$

donde $V(\mathbf{p}, l)$ es el término de visibilidad calculado con un shadow ray (sphere_trace hacia la luz).

**Modo 2 — Ambient Occlusion (`ao`):**

Aproximación rápida que mide qué fracción del hemisferio está libre de geometría:

$$\text{ao}(\mathbf{p}) = \frac{1}{N} \sum_{k=1}^{N} V(\mathbf{p}, \mathbf{p} + r \cdot \omega_k)$$

con $N$ muestras hemisféricas distribuidas por quasi-random sampling (Halton sequence). El coste es $N$ shadow rays adicionales, típicamente $N \in [8, 32]$.

**Modo 3 — Path Tracing (`path_trace`):**

Implementa la ecuación de renderizado completa mediante muestreo de Monte Carlo:

$$L_o(\mathbf{p}, \omega_o) = \mathbf{E}(\mathbf{p}) + \int_\Omega f_r(\omega_i, \omega_o) \cdot L_i(\mathbf{p}, \omega_i) \cdot (\omega_i \cdot \hat{n}) \, d\omega_i$$

El integrador lanza recursivamente rayos secundarios (hasta profundidad `max_bounces`). Este modo es el único que produce iluminación global correcta (rebotes, caustics, GI). Se recomienda para offline rendering; para tiempo real, usar `direct` o `ao`.

**Declaración en fichero de escena:**

```json
{
  "lighting": {
    "mode": "direct",
    "ambient_radiance": [0.1, 0.1, 0.12],
    "ao_samples": 16,
    "max_bounces": 4,
    "shadow_eps": 1e-3
  }
}
```

#### Registro de Fuentes de Luz en el Grafo Biológico

Las fuentes de luz son **nodos regulares del grafo** con un material de emisión asignado:

```json
{
  "id": "sun",
  "parent": null,
  "local_transform": [1,0,0,0, 0,1,0,0, 0,0,1,5, 0,0,0,1],
  "geometry": { "type": "sphere", "radius": 0.5 },
  "material_id": "sun_light",
  "mode": "solid"
}
```

El sistema detecta automáticamente los nodos cuyo material tiene `emission > [0,0,0]` y los registra en la `LightList` que usa el integrador de iluminación directa. No existe un tipo especial de nodo "luz": **la emisión es una propiedad del material**, consistente con la arquitectura de separación de capas del modelo.

---

### 15.7 Tabla de Invariantes Actualizada: Modelo v3.0

Con la incorporación de esta sección, los invariantes del sistema se amplían:

| Invariante | Garantía Formal | Mecanismo |
|------------|----------------|-----------|
| **Convergencia de sphere_trace** | $N_{pasos} \le \lceil \|\Phi(p_0)\| / \epsilon \rceil$ | Jacobian Scaling + `h_min` estable (14.1, 15.2) |
| **Determinismo temporal** | $\mathcal{C}(p, W)$ estático para todo $W$ | Bakeado offline + delta-encoding (14.2, 15.4) |
| **Topología por intervalo** | $\mathcal{G}(W)$ reconstruible en $O(N_{snap})$ deltas | Delta-encoding con snapshots (15.4) |
| **Consistencia energética del BRDF** | $\int_\Omega f_r \le 1$ garantizado | Blending en espacio de parámetros (15.3) |
| **Gradiente numérico estable en frontera** | $h \ge h_{min} > 0$ siempre | `h_min` escena-dependiente (15.2) |
| **`delta_px` evaluable en shader** | $\delta_{px}(z) = z \cdot \lambda_{cone}$ en $O(1)$ | Cone footprint uniform (15.5) |
| **Transporte de luz formalizado** | $L_o$ calculable en tres modos | Emisión en estado interno + LightList (15.6) |
| **Ausencia de stalls en GPU** | `SSBO_front` inmutable durante render | Doble buffer atómico (14.3) |
| **Memoria proporcional a cambios** | $O(n_0 + \sum |\Delta_k|)$ | Delta-encoding + snapshots (15.4) |
| **Contratos del simulador externo** | Keyframes validados en frecuencia, energía y jerarquía | Importador con schema JSON (15.1) |

---

### 15.8 Checklist de Solidez del Modelo v3.0

- [ ] El importador de simulación valida frecuencia mínima ($f \ge 4f_{max}$), energía y jerarquía
- [ ] El fichero de escena declara `h_min`, `scene_scale`, `fov_y_degrees`, `res_y` y `lighting.mode`
- [ ] El blending de frontera opera sobre `roughness`, `metallic` y `base_color`, no sobre colores de salida
- [ ] El grafo biológico usa delta-encoding con snapshots cada `N_snap = 64` eventos
- [ ] `u_cone_lambda` se calcula en CPU una vez por frame y se sube como uniform al shader
- [ ] Las fuentes de luz son nodos ordinarios del grafo con `material.emission > [0,0,0]`
- [ ] `collapse()` respeta el modo de iluminación declarado en la escena
- [ ] Los shadow rays usan `shadow_eps` separado de `eps` del sphere tracer principal

---

## 16. Refinamiento Filosófico y Óptica Avanzada (Ronda 3)

Esta sección aborda las correcciones derivadas de la tercera ronda de análisis adversarial, corrigiendo violaciones a la separación de capas de la pirámide y formalizando el comportamiento óptico en espacios continuos complejos.

---

### 16.1 Corrección Filosófica: La Luz Pertenece a la Física

#### Problema

La Sec. 15.6 asumió implícitamente que la luz y el transporte electromagnético eran entidades organizadas por la capa Biológica, rompiendo el contrato fundamental donde la luz, como fenómeno espacial, debe resolverse en la capa Física.

#### Contrato Formal

La radianza emitida $\mathbf{E}$ es una propiedad fundamental del punto en el espacio-tiempo evaluado por la capa Física. El estado físico se extiende:

$$\mathcal{F}(\mathbf{p}) = (d, A, T_{local}, \text{bbox}, \text{motion}, \mathbf{E})$$

La capa Biológica organiza nodos que pueden tener un `material_id` con propiedades emisivas, pero el transporte de luz (trazado de rayos, evaluación de visibilidad $V(\mathbf{p}, l)$) es de competencia exclusiva del resolvedor Físico. La `LightList` es mantenida y consultada por el motor de física óptica, no por el grafo de escena.

---

### 16.2 Normal Geométrica en Zona de Blending

#### Problema

El blending subpíxel (Sec. 14.4) mezclaba propiedades químicas pero dejaba indefinida la normal geométrica $\hat{n}$ en la transición, impidiendo evaluar correctamente el BRDF.

#### Contrato Formal

La normal se interpola utilizando el mismo factor $\alpha_{blend}$ empleado para las propiedades del material, seguido de una normalización estricta:

$$\hat{n}_{blend} = \text{normalize}\!\left(\text{LERP}(\hat{n}_1, \hat{n}_2, \; \alpha_{blend})\right)$$

**Justificación Física:** Esta interpolación representa la macro-superficie suavizada que resultaría de una micro-geometría de transición entre ambos medios en la zona de contacto, unificando la respuesta del BRDF sin introducir discontinuidades angulares.

---

### 16.3 Travesía Multi-Medio: El Media Stack

#### Problema

Un rayo que penetra en volúmenes transparentes (ej. agua, cristal) carecía de memoria sobre el índice de refracción ($\eta$) activo y la absorción espectral del medio, impidiendo calcular correctamente la refracción de Snell y la atenuación diferencial de color de Beer-Lambert.

#### Contrato Formal

Cada rayo mantiene un **Media Stack** que registra los medios en los que está inmerso actualmente:

```
struct RayState:
    origin: vec3
    direction: vec3
    depth: int
    media_stack: MediumLayer[]
```

**Material extendido:**

```json
{
  "glass_01": {
    "eta": 1.5,
    "sigma_a": [0.01, 0.005, 0.002],  // Absorción espectral por metro
    "sigma_s": [0.0, 0.0, 0.0]        // Scattering por metro
  }
}
```

**Transiciones de Medio:**

Al cruzar una superficie con $A > 0$:
1. Si el rayo entra ($\hat{n} \cdot \vec{\omega_i} < 0$): `push` del nuevo medio al `media_stack`.
2. Si el rayo sale ($\hat{n} \cdot \vec{\omega_i} > 0$): `pop` del medio del `media_stack`.

La dirección de refracción se computa usando $\eta_{out} / \eta_{in}$, donde $\eta_{in}$ es el medio actual en el top del stack, y $\eta_{out}$ el nuevo medio.

La transmitancia se actualiza con la absorción del medio activo:

$$T_{spectral}(z) = \exp\!\left(-\int_{z_0}^{z} \boldsymbol{\sigma}_a^{medium} \, dz'\right)$$

---

### 16.4 Parametrización UV para Geometría Implícita (SDF)

#### Problema

Las SDFs no poseen coordenadas UV intrínsecas, imposibilitando el uso de texturas sin una proyección explícita.

#### Contrato Formal

Cada nodo define un `uv_mode` que determina cómo se computan las coordenadas UV $(u,v)$ a partir del punto de intersección $\mathbf{p}$ y la normal $\hat{n}$:

| `uv_mode` | Mapeo | Fórmula Computacional |
|---|---|---|
| `triplanar` | Proyección ponderada por $\hat{n}$ | Mezcla suave de proyecciones planas $XY, YZ, XZ$ |
| `spherical` | Esférico al centro del AABB | $\phi, \theta$ a partir del vector relativo al centro |
| `box` | Proyección planar (cubo) | Cara dominante según componente mayor de $\hat{n}$ |
| `none` | Color procedimental | N/A |

```json
{
  "id": "cushion",
  "geometry": { "type": "box", "extents": [2, 0.2, 2], "uv_mode": "triplanar" }
}
```

---

### 16.5 Manifest Unificado de Escena (Scene JSON Schema)

#### Problema

Configuraciones cruciales del resolvedor y constantes matemáticas estaban dispersas. Faltaba una fuente de verdad validable.

#### Contrato Formal

Toda la configuración paramétrica requerida por el motor debe compilarse en un único archivo manifiesto `scene.json` que precede a la evaluación.

```json
{
  "$schema": "hermetic-scene-v3.1",
  "world": {
    "scene_scale": 10.0,
    "z_max": 100.0
  },
  "renderer": {
    "eps": 1e-4,
    "max_steps": 128,
    "h_min": 0.001,
    "fov_y_degrees": 60.0,
    "res_y": 1080,
    "k_blend": 2.0,
    "shadow_eps": 1e-3
  },
  "lighting": {
    "mode": "path_trace",
    "ambient_radiance": [0.05, 0.05, 0.08],
    "ao_samples": 32,
    "max_bounces": 6
  },
  "temporal": {
    "N_snap": 64,
    "density_scale": 5.0
  },
  "graph": "scene_graph.json",
  "materials": "materials.json"
}
```

Este esquema provee las constantes base que rigen las tolerancias numéricas, estrategias de culling y comportamientos del integrador de forma centralizada.

---

### 16.6 Checklist Final de Solidez (v3.1)

- [ ] $\mathbf{E}$ está definido como propiedad del campo en la capa Física
- [ ] La normal $\hat{n}$ es interpolada y normalizada dentro de `select_closer_blended`
- [ ] El integrador de rayos gestiona un `media_stack` con operaciones push/pop en transiciones de superficie
- [ ] La atenuación volumétrica utiliza $\boldsymbol{\sigma}_a$ del medio activo actual del stack
- [ ] Cada primitiva en el grafo incluye una directiva `uv_mode` explícita
- [ ] El motor carga y valida los parámetros operativos de tolerancia y resolución desde `scene.json`

---

## 17. Los Axiomas Matemáticos de la Creación (Bases Funcionales)

Para garantizar la hermeticidad matemática del modelo y evitar depender de abstracciones externas (como "mallas" o "modelos 3D"), esta sección define las **fórmulas axiomáticas** que gobiernan la generación de materia y espacio. Estas funciones son el equivalente al "código genético" del motor.

---

### 17.1 Axiomas de la Materia Base (Primitivas Puras)

La geometría no se almacena como vértices, sino que se evalúa analíticamente en cualquier punto $\mathbf{p} \in \mathbb{R}^3$. El motor garantiza el soporte nativo de las siguientes Funciones de Distancia Signada exactas:

**A. Esfera:** Centro en origen, radio $r$.
$$\Phi_{esfera}(\mathbf{p}, r) = \|\mathbf{p}\| - r$$

**B. Caja (Box):** Centro en origen, dimensiones $2\mathbf{e} = (2e_x, 2e_y, 2e_z)$.
$$\mathbf{q} = |\mathbf{p}| - \mathbf{e}$$
$$\Phi_{caja}(\mathbf{p}, \mathbf{e}) = \|\max(\mathbf{q}, \mathbf{0})\| + \min(\max(q_x, \max(q_y, q_z)), 0)$$

**C. Cilindro:** Altura $2h$, radio $r$, alineado al eje Y.
$$\mathbf{d} = (\|\mathbf{p}_{xz}\| - r, \; |p_y| - h)$$
$$\Phi_{cilindro}(\mathbf{p}, h, r) = \min(\max(d_x, d_y), 0.0) + \|\max(\mathbf{d}, \mathbf{0})\|$$

**D. Plano:** Vector normal $\hat{n}$, distancia al origen $h$.
$$\Phi_{plano}(\mathbf{p}, \hat{n}, h) = \mathbf{p} \cdot \hat{n} + h$$

---

### 17.2 Axiomas de Deformación Espacial (Moduladores)

No modificamos el objeto; modificamos el espacio antes de que el objeto lo evalúe.

**A. Transformación Afín Local:**
Para evaluar un nodo con transformación $T_{local}$ (matriz $4\times 4$), el punto de mundo $\mathbf{p}_{world}$ se mapea al espacio del objeto:
$$\mathbf{p}_{local} = (T_{local})^{-1} \cdot \mathbf{p}_{world}$$
$$\Phi_{nodo}(\mathbf{p}_{world}) = \Phi_{base}(\mathbf{p}_{local}) \cdot \text{scale}_{local}$$
*(Nota: la distancia debe re-escalarse si $T_{local}$ contiene escala uniforme para mantener la métrica Lipschitz-1).*

**B. Repetición Espacial (Domain Repetition):**
Permite crear bosques o grillas infinitas con coste computacional $O(1)$. Para repetir el espacio cada vector $\mathbf{c}$:
$$\mathbf{p}_{mod} = \mathbf{p} - \mathbf{c} \cdot \text{round}\left(\frac{\mathbf{p}}{\mathbf{c}}\right)$$
$$\Phi_{bosque}(\mathbf{p}) = \Phi_{arbol}(\mathbf{p}_{mod})$$

---

### 17.3 Axiomas de Crecimiento Orgánico (Proceduralismo)

Para evitar que la naturaleza parezca un "ensamblaje de cajas", introducimos operadores de perturbación continua.

**A. Ruido Volumétrico (Simplex / FBM):**
Sea $N(\mathbf{p})$ una función de ruido continuo (ej. Fractional Brownian Motion) en $[-1, 1]$. La deformación de la superficie se logra perturbando el campo de distancias:
$$\Phi_{organico}(\mathbf{p}) = \Phi_{base}(\mathbf{p}) + \text{amplitud} \cdot N(\mathbf{p} \cdot \text{frecuencia})$$
*Precaución:* La adición de ruido rompe la métrica Lipschitz estricta. El resolutor (`sphere_trace`) compensa esto usando un paso seguro menor, modulado por la derivada máxima teórica del ruido.

**B. Volúmenes de Densidad Heterogénea:**
Para follaje o nubes, no se usa superficie sólida ($\Phi=0$). Se evalúa una densidad espacial $A(\mathbf{p})$:
$$A_{nube}(\mathbf{p}) = \text{clamp}(N_{fbm}(\mathbf{p}) - \text{threshold}, 0, 1) \cdot \text{density\_multiplier}$$

---

### 17.4 La Ley de Composición Jerárquica (El Algoritmo de Grafo)

¿Cómo colapsa un grafo con cientos de primitivas y modificaciones orgánicas en un solo estado $\mathcal{R}(\mathbf{p})$? Mediante la función recursiva matemática $\text{EvalGraph}$.

Sea $\mathcal{G}$ un grafo biológico. La evaluación en el punto $\mathbf{p}$ sigue este axioma:

```
function EvalGraph(nodo, p_parent, shading_ctx):
    // 1. Deformación espacial (Axioma 17.2.A)
    p_local = inverse(nodo.T_local) * p_parent
    
    // 2. Aplicar repetición o ruido si el nodo lo requiere (17.2.B, 17.3)
    p_eval = apply_domain_distortion(p_local, nodo.distortion)
    
    // 3. Evaluar primitiva propia (si la tiene)
    S_current = NULL
    if nodo.geometry != NULL:
        d = evaluate_primitive(nodo.geometry, p_eval)
        d += evaluate_displacement(nodo.displacement, p_eval)
        S_current = State(d, nodo.material, p_eval)
        
    // 4. Composición recursiva con hijos
    for child in nodo.children:
        if not bbox_intersect(child.bbox_local, p_local):
            continue // Optimization
            
        S_child = EvalGraph(child, p_local, shading_ctx)
        
        // Operador de Unión Suavizada (Smooth Min)
        if S_current == NULL:
            S_current = S_child
        else:
            S_current = select_closer_blended(S_current, S_child, shading_ctx, delta_px)
            
    return S_current
```

```

Este algoritmo es la **verdad matemática absoluta** del motor. Transforma un diseño semántico ("un árbol que repite ramas y tiene corteza rugosa") en un valor numérico escalar evaluable instantáneamente en cualquier coordenada de la simulación.

---

## 18. Fe de Erratas y Estabilidad Numérica (Rigor Matemático Final - Ronda 4)

El análisis adversarial profundo (Ronda 4) reveló supuestos numéricos optimistas en versiones previas. Un modelo verdaderamente hermético no puede depender del "caso feliz". Esta sección parchea las inestabilidades numéricas formales y depreca los algoritmos ingenuos.

---

### 18.1 Deprecación de la Afirmación de Blending Lineal

**Fe de Erratas:** La Sección 14.4 propuso una mezcla lineal de colores (`c_out = LERP(c1, c2)`). Esta afirmación es **físicamente incorrecta** y viola la conservación de energía.

**Resolución Oficial:** La ecuación de la Sec. 14.4 queda estrictamente deprecada. El motor delega el blending exclusivamente a la interpolación del espacio de parámetros de material definida en la **Sección 15.3**. Además, el motor restringe su soporte a un único modelo de BRDF (Microfacetado Cook-Torrance con distribución GGX) para garantizar matemáticamente que el LERP paramétrico conserve la cota $\int f_r \le 1$.

---

### 18.2 Corrección de Convergencia del Sphere Tracing (Relaxation Factor)

**Fe de Erratas:** La cota de pasos teórica $N \le \lceil|\Phi(p_0)|/\epsilon\rceil$ asume un campo SDF perfecto ($L=1$). Las operaciones de ruido orgánico y geometría procedural elevan la constante de Lipschitz ($L > 1$), lo que puede causar overshooting (atravesar paredes).

**Resolución Oficial (Multiplicador de Seguridad):**
El avance del rayo no confía ciegamente en $d$. Introduce un factor de relajación $k_{seguridad} \in (0, 1]$ que escala el avance:

$$\mathbf{p}_{k+1} = \mathbf{p}_k + \vec{\omega} \cdot \left( \Phi_{global}(\mathbf{p}_k) \cdot k_{seguridad} \right)$$

Para geometría pura $k_{seguridad} = 1.0$. Si un nodo introduce ruido fractal, impone localmente un $k_{seguridad} = 0.5$ (por ejemplo). Esto requiere más iteraciones para converger ($\mathcal{O}(\log(1/\epsilon))$ en lugar de decaimiento lineal), pero garantiza matemáticamente la no-penetración de superficies.

---

### 18.3 Manejo de Reflexión Total Interna (TIR) en Medios

**Fe de Erratas:** La Sec. 16.3 usa la ley de Snell ingenuamente, ignorando el escenario donde el cálculo de refracción arroja raíces imaginarias (cuando un rayo intenta salir de un medio denso a un ángulo agudo).

**Resolución Oficial:**
Al calcular el cambio de dirección refractada con $\mu = \eta_{in} / \eta_{out}$, se evalúa primero el discriminante del TIR:

$$k_{TIR} = 1.0 - \mu^2 \cdot (1.0 - (\hat{n} \cdot \vec{\omega}_{in})^2)$$

Si $k_{TIR} < 0.0$:
- La refracción es imposible.
- El rayo sufre **Reflexión Total Interna**: $\vec{\omega}_{out} = \text{reflect}(\vec{\omega}_{in}, \hat{n})$.
- No se realiza `pop` del `media_stack` (el rayo rebota hacia adentro del mismo medio).

---

### 18.4 Corrección de la Repetición de Dominio Espacial

**Fe de Erratas:** La repetición de dominio mediante módulo (Sec. 17.2.B) produce un campo no-Lipschitz en las fronteras de la celda de repetición $\mathbf{c}$, causando artefactos de rayo.

**Resolución Oficial:**
La función de repetición acota el valor máximo de distancia que puede retornar al límite de la celda, garantizando que el rayo deba detenerse exactamente en la frontera antes de continuar evaluando la celda vecina:

$$d_{local} = \Phi_{arbol}(\mathbf{p}_{mod})$$
$$d_{frontera} = \min_{xyz}\left( \frac{\mathbf{c}}{2} - |\mathbf{p}_{mod}| \right)$$
$$d_{final} = \min(d_{local}, d_{frontera})$$

Esto asegura que L no supere nunca 1.0 artificialmente, y la métrica de distancia se mantiene euclidiana incluso entre copias infinitas de la geometría.

---

---

## 19. Contrato de Compilación: El Protocolo hacia GTE

Para garantizar que este nuevo modelo matemático (La Representación Hermética) no quede aislado de la arquitectura existente del proyecto, se establece su integración estricta con el **MLIR de Campo Tensorial**, específicamente con **GTE** (*Generador de Tensores por Expresiones*).

### 19.1 El Cambio de Paradigma: GTE como Backend

En la arquitectura original, GTE (Nivel P3) era el lenguaje de alto nivel donde el usuario declaraba la intención. Bajo el nuevo paradigma hermético, la Representación Hermética y su **Resolvedor** asumen el rol de lenguaje de nivel superior. 

GTE se convierte en el **backend de compilación**. El usuario ya no escribirá `.gte` manualmente para escenas complejas. El usuario escribirá (o generará) axiomas y perturbaciones (el Grafo Biológico y la Escena), y el Resolvedor evaluará este universo matemáticamente.

### 19.2 El Protocolo de Traducción (El "Cable")

El Resolvedor evalúa la ecuación continua y total del universo para un intervalo de tiempo $W$. Dado que GTE es una función sin memoria ni iteración compleja, el Resolvedor debe "hornear" (bake) la evaluación analítica en **slots GTE cerrados**.

El protocolo de salida hacia `.gte` se rige por las siguientes reglas:

1. **Evaluación de Caja Bounding (AABB a Slot):**
   Para cada nodo u objeto delimitado en el espacio, el Resolvedor calcula su AABB en el lienzo 2D tras la proyección de cámara. Este rectángulo se declara como un `slot` en GTE.
   ```gte
   slot s_objeto1 {
     en: y = Y0..Y1, x = X0..X1;
     // ...
   }
   ```

2. **Inyección de Expresiones Analíticas:**
   Si el Resolvedor encuentra que la función de color e iluminación para ese píxel puede ser expresada como un polinomio o una fórmula simple (ej. un gradiente de fondo), inyectará esa expresión matemática pura dentro del bloque `relleno` del slot GTE.

3. **Colapso a Tensores Constantes (Rasterización Temprana):**
   Para geometría orgánica, rebotes de path-tracing, o BRDFs microfacetados (cuyas ecuaciones son demasiado pesadas o imposibles de expresar en las directivas matemáticas limitadas de GTE original), el Resolvedor realiza el *ray-marching* internamente. El resultado (el color de cada píxel) se inyecta en el slot GTE como datos estáticos absolutos, o se mapea directamente al formato **PTR** (Nivel P2) si GTE no es necesario para datos puramente pre-calculados.

4. **El Eje Temporal ($W$ a $t$):**
   Si la escena tiene movimiento analítico simple (ej. traslación lineal), el Resolvedor puede exportar el eje $W$ como la variable de tiempo libre en la expresión GTE (`t`). Para cinemáticas complejas, el Resolvedor debe emitir fotogramas estáticos individuales o slots condicionales.

### 19.3 El Contrato de Aislamiento

El diseño de la geometría, los materiales (Química) y el transporte de luz (Física) queda estrictamente confinado dentro del Resolvedor de la Representación Hermética. **GTE no sabe qué es una normal, un índice de refracción o una Función de Distancia Signada**. GTE solo recibe, como producto final, la matriz `[R, G, B, A]` calculada para sus coordenadas `[X, Y, Z, W]`. 

Con este puente, todo el conocimiento previo de compresión (CTP, TB) y manejo de píxeles se reutiliza íntegramente. El Resolvedor es la "mente" física, y GTE/PTR/CTP es la tubería de empaquetado y entrega de la imagen final.

