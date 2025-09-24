# Copilot Instructions for tp-scaffold (UTN SO TP)

## Project Overview
- This is a multi-component C project scaffold for UTN FRBA Operating Systems course TPs.
- Main modules: `master`, `worker`, `storage`, `query_control`, `utils`. Each is a separate service with its own `Makefile`, config, and source files.
- All modules depend on [so-commons-library](https://github.com/sisoputnfrba/so-commons-library) for shared utilities and abstractions.

## Architecture & Data Flow
- Each module is compiled and run independently. Communication between modules is typically via sockets (see `conexiones.c/h` in `utils/src/utils/`).
- Config files for each module are in `configs/` subfolders, e.g. `worker/configs/worker.config`.
- Shared helpers and abstractions are in `utils/src/utils/` and `lib/libutils.a`.
- Build artifacts: object files in `obj/`, executables in `bin/` (where present).

## Build & Run Workflow
- Build each module by running `make` in its directory (e.g. `cd worker && make`).
- Executables are placed in `bin/` (if defined in the module).
- To build all modules, run `make` in each main folder: `master`, `worker`, `storage`, `query_control`, `utils`.
- The project expects `so-commons-library` to be installed system-wide (see README for install steps).

## Project Conventions
- Each module has its own `settings.mk` and `Makefile` for build configuration.
- Source code is organized under `src/` per module. Helpers are named `helpers-<module>.c/h`.
- Use `configs/<module>.config` for runtime configuration.
- Tag checkpoints in git as `checkpoint-<n>` (see README for details).
- For deployment, use the [so-deploy](https://github.com/sisoputnfrba/so-deploy) script as described in the README.

## Patterns & Integration
- Socket communication is abstracted in `utils/src/utils/conexiones.c/h`.
- Common buffer and memory utilities are in `utils/src/utils/buffer.c/h` and `memoria_interna.c/h` (worker).
- Each module's `main.c` is the entry point; helpers are used for modular logic.
- External dependencies: Only `so-commons-library` is required; no other third-party libraries are used.

## Example: Adding a New Module
1. Create a new folder with `Makefile`, `settings.mk`, `configs/`, and `src/`.
2. Implement helpers as `<module>/src/helpers-<module>.c/h`.
3. Use `utils` for shared logic and socket abstractions.
4. Add build rules to the new `Makefile`.

## Key Files & Directories
- `master/`, `worker/`, `storage/`, `query_control/`, `utils/`: Main modules
- `utils/src/utils/conexiones.c/h`: Socket abstractions
- `utils/lib/libutils.a`: Compiled shared library
- `README.md`: Developer workflow, build, and deployment instructions
- `tp.code-workspace`: VS Code workspace config

---

For questions about unclear conventions or missing documentation, ask the user for clarification or refer to the README and module Makefiles for canonical workflows.
