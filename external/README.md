# external/

Generated, not committed:

- `shadercross/` — the SDL_shadercross CLI, built on first configure from
  `tools/shadercross` (pinned SDL3, SPIRV-Cross, SDL_shadercross, and prebuilt
  DXC). Delete it to force a rebuild.
- `_build/` — scratch build tree for the above.

Engine libraries (SDL3, GLM, Catch2, and the optional stack) are fetched by
`cmake/Dependencies.cmake` into each build directory's `_deps/`.
