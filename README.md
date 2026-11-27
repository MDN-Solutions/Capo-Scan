# Capô — Scanner automotivo 3D

[![CI](https://github.com/MDN-Solutions/MDN-Scan/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/MDN-Solutions/MDN-Scan/actions/workflows/ci.yml)
[![Security](https://github.com/MDN-Solutions/MDN-Scan/actions/workflows/security.yml/badge.svg?branch=main)](https://github.com/MDN-Solutions/MDN-Scan/actions/workflows/security.yml)
[![OpenSSF Scorecard](https://api.scorecard.dev/projects/github.com/MDN-Solutions/MDN-Scan/badge)](https://scorecard.dev/viewer/?uri=github.com/MDN-Solutions/MDN-Scan)

Diagnóstico automotivo com visualização 3D do veículo. Roda local, offline.

| Pasta | Conteúdo |
|---|---|
| `godot/` | Interface + 3D (Godot 4, GDScript) |
| `core/` | Núcleo de diagnóstico (C++20) |
| `ext-diag/` | GDExtension C++ |
| `ext-data/` | GDExtension Rust (dados, licença, pacotes) |
| `bridge-j2534/` | Auxiliar 32 bits para VCI J2534 |
| `app/` | Executável de linha de comando `mdn-scan` (provisório, até o export do Godot) |
| `tools/` | Ferramentas Python (Blender, FIPE, DTCs, simulador) |
| `cmake/`, `packaging/`, `scripts/` | Build, empacotamento e verificações do CI |
| `docs/` | Documentação |

## Desenvolvimento

```bash
cmake --preset dev && cmake --build --preset dev && ctest --preset dev
cargo fmt --all && cargo clippy --workspace --all-targets -- -D warnings
```

Os títulos de PR seguem [Conventional Commits](https://www.conventionalcommits.org/pt-br/)
(`feat(elm327): ...`, `fix(dtc): ...`): eles viram o commit no `main` e geram a
versão e o changelog. Detalhes da pipeline (CI, segurança, releases) em
[docs/PIPELINE.md](docs/PIPELINE.md).
