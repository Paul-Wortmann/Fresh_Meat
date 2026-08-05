# Map Validator Documentation

This document describes the **map_validator** command‑line tool used to validate map definition files.

### Purpose

The map validator checks map data against the schema documented in:

`documentation/map_schema.md`

It scans all `.txt` files under `./data/map/`, parses them in a simple XML‑like format, and enforces syntactic and semantic rules. The tool helps catch malformed or inconsistent map definitions before they are consumed by the engine or added to the dataset.

### Compiling

```bash
g++ -std=c++20 -Wall -o map_validator map_validator.cpp
```

### Running

Run the validator from the root directory of the application:

```bash
map_validator
```

The validator automatically scans all map definitions under:

```text
./data/map/
```

### Validation Checks

The validator reports errors for any violation of the schema. The following checks are performed:

##Syntactic (Parsing) Checks

The root element must be <map> and appear exactly once.

All tags must be properly closed and correctly nested.

Tags must be well‑formed (no missing >).

Self‑closing tags (<tag/>) are not supported and will cause errors.

Leaf tags (<name>value</name>) must appear entirely on one line.

### Required Fields

map_name: must be present and not empty. It must not contain spaces.

map_size: must be present and consist of two positive integers (width,height).

map_tiles: at least one row is required. The number of rows must match the height defined in map_size.

Each row must contain exactly width integers.

Each tile value must be in the range 0..3.

playerStartTile: must be present and consist of two integers (x,y). The coordinates must be within map bounds and the tile must not be a wall (value 2).

Texture atlases (all three required):

map_texture_atlas_diffuse

map_texture_atlas_normal

map_texture_atlas_specular

Each must point to an existing file (relative path).

map_texture_atlas_size: must be present and consist of two positive integers (columns,rows).

entity_wall: must be present and point to an existing entity definition file in ./data/entity/.

### Optional Fields

map_music: if present, the referenced audio file must exist.

map_biome: if present, must be an integer in 0..3.

portal_tile: if present, must be two integers within map bounds.

boss_alert_tile: if present, must be two integers within map bounds.

### Global Consistency

Duplicate map_name values across all map files are reported as errors.

### Output

For each valid file, the tool prints:

```text
OK: ./data/map/example.txt
```

For each error, a message is written to stderr in the format:

```text
ERROR: file_path: field_name: description
```

At the end of the run, a summary is printed:

```text
Validated N map file(s): M passed, E error(s).
```

### Exit Status

The validator uses the following exit‑status convention:

## Exit status	Meaning

0	All map definitions passed validation.
Non‑zero	One or more validation errors were detected, or the tool could not complete validation (e.g., directory not found).
This makes the tool suitable for use in build pipelines or continuous integration (CI).

### Scope

The validator is read‑only. It does not modify any files. Automatic repair or normalisation is intentionally excluded so that validation can be safely used in CI without unintended side effects.

### Schema Reference

The authoritative map data schema is documented in:

documentation/map_schema.md

Any changes to the map format should update both the schema documentation and this validator.

