# Map Data Schema

## Status

This document defines the canonical schema for map definition data stored in data/map/.

The schema is intentionally documented before the dataset grows so map files remain predictable, validatable, and compatible with engine tooling.

## File format

Map definitions are UTF‑8 text files containing a single XML‑like <map> document.

The canonical extension is .txt for compatibility with the current dataset. The content is structured as XML‑like markup, but is currently treated as an application‑defined schema rather than formally validated XML.

Each map file must contain exactly one root <map> element.

```xml
<map>
   <map_name>...</map_name>
   <map_size>...</map_size>
   <map_tiles>...</map_tiles>
   ...
</map>
```

## Top‑level sections

Tag	Required	Purpose
<map_name>	Yes	Unique identifier for the map.
<map_size>	Yes	Dimensions of the tile grid (width,height).
<map_tiles>	Yes	One or more rows of tile data.
<playerStartTile>	Yes	Initial spawn position for the player.
<map_music>	No	Background music asset.
<map_texture_atlas_diffuse>	Yes	Diffuse texture atlas.
<map_texture_atlas_normal>	Yes	Normal map atlas.
<map_texture_atlas_specular>	Yes	Specular map atlas.
<map_texture_atlas_size>	Yes	Number of tiles in the atlas (columns,rows).
<map_biome>	No	Numeric biome identifier (reserved, not yet used).
<entity_wall>	Yes	Entity definition file for wall tiles.
<portal_tile>	No	Tile coordinate of a portal (if any).
<boss_alert_tile>	No	Tile coordinate that triggers a boss alert.

## 1. Map name

```xml
<map_name>test</map_name>
```

Field	Type	Required	Description
map_name	string	Yes	Unique identifier for the map.
Rules:

Must be unique within the map dataset.

Should use lowercase snake_case or the established project naming convention.

## 2. Map size

```xml
<map_size>30,30</map_size>
```

Field	Type	Required	Description
map_size	vec2	Yes	Width and height of the tile grid, in tiles.
The value is a comma‑separated pair of positive integers: width,height.

Rules:

Both values must be greater than zero.

The total number of tiles must match the product of width and height.

Each <map_tiles> row must contain exactly width tile values.

There must be exactly height rows of <map_tiles>.

## 3. Map tiles

```xml
<map_tiles>2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2</map_tiles>
<map_tiles>2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2</map_tiles>
```
...
Field	Type	Required	Description
map_tiles	list of integers	Yes	One row of tile data.
Each <map_tiles> element contains a comma‑separated list of integer tile identifiers. There must be exactly height such elements, each with exactly width integers.

The tile identifiers correspond to the engine’s eMapTileType enumeration, defined in map_define.hpp:

Value	Enumeration	Meaning
0	eMapTileType::none	Empty / unassigned tile.
1	eMapTileType::floor	Walkable floor terrain.
2	eMapTileType::wall	Solid obstacle; typically blocks pathfinding and collision.
3	eMapTileType::path	Walkable path tile (may be used for special navigation or decoration).
Rules:

Every <map_tiles> row must contain exactly width integers.

There must be exactly height rows.

Tile values must be one of the above enum values.

The map should normally be enclosed by walls (value 2) unless a specific design requires otherwise.

## 4. Player start tile

```xml
<playerStartTile>15,5</playerStartTile>
```

Field	Type	Required	Description
playerStartTile	vec2	Yes	Tile coordinates (x,y) where the player initially spawns.
Coordinates are zero‑based, with (0,0) at the top‑left corner of the map.

Rules:

The coordinates must be within the map bounds (0 ≤ x < width, 0 ≤ y < height).

The tile at that position must be walkable (i.e., not a wall) unless the engine allows spawning on any tile.

## 5. Map music

xml
<map_music>music_001.ogg</map_music>
Field	Type	Required	Description
map_music	asset path	No	Audio file played as background music for this map.
Asset paths are relative to the configured asset root and should use a supported format.

6. Texture atlas references

```xml
<map_texture_atlas_diffuse>map_atlas_diffuse.png</map_texture_atlas_diffuse>
<map_texture_atlas_normal>map_atlas_normal.png</map_texture_atlas_normal>
<map_texture_atlas_specular>map_atlas_specular.png</map_texture_atlas_specular>
<map_texture_atlas_size>2,2</map_texture_atlas_size>
```

Field	Type	Required	Description
map_texture_atlas_diffuse	asset path	Yes	Diffuse/albedo texture atlas.
map_texture_atlas_normal	asset path	Yes	Normal map atlas.
map_texture_atlas_specular	asset path	Yes	Specular map atlas.
map_texture_atlas_size	vec2	Yes	Number of tiles in the atlas (columns,rows).
The atlas size defines how the tile indices map to texture coordinates. For example, 2,2 indicates a 2×2 grid of tile textures in each atlas image.

Rules:

All referenced texture files must exist.

The atlas size must be consistent across all three textures.

Tile identifiers used in <map_tiles> must be within the range 0 … (columns × rows - 1) if the engine maps them directly to atlas indices.

## 7. Map biome

```xml
<map_biome>0</map_biome>
```

Field	Type	Required	Description
map_biome	integer	No	Numeric identifier for the biome.
Biome values correspond to the engine’s eMapBiome enumeration (defined in map_define.hpp):

Value	Enumeration
0	eMapBiome::plains
1	eMapBiome::desert
2	eMapBiome::forrest
3	eMapBiome::tundra
This field is currently parsed but not used by the engine; it is reserved for future extensions (e.g., lighting, weather, entity sets). If omitted, a default biome is assumed.

## 8. Wall entity

```xml
<entity_wall>wall_001.txt</entity_wall>
```

Field	Type	Required	Description
entity_wall	asset path	Yes	Entity definition file used for wall tiles.
This reference points to an entity file (see entity_schema.md) that defines the visual and physical properties of wall tiles. The engine instantiates wall entities for tiles with value 2 (or another designated wall tile type).

Rules:

The referenced entity file must exist and be valid according to the entity schema.

The entity must be compatible with wall tile placement (e.g., static physics, appropriate graphics).

## 9. Portal tile

```xml
<portal_tile>0,9</portal_tile>
```

Field	Type	Required	Description
portal_tile	vec2	No	Tile coordinate where a portal is located.
When the player steps on or interacts with this tile, the engine fires an eMapEventType::portal event. The tile value at that coordinate may be overridden or interpreted by the engine.

Rules:

If present, the coordinates must be within the map bounds.

The tile at that position must exist; the engine may treat it as a portal regardless of its tile value.

## 10. Boss alert tile

```xml
<boss_alert_tile>15,15</boss_alert_tile>
```

Field	Type	Required	Description
boss_alert_tile	vec2	No	Tile coordinate that triggers a boss alert when the player enters or approaches.
When triggered, the engine fires an eMapEventType::bossAlert event. This can be used to signal the start of a boss encounter.

Rules:

If present, the coordinates must be within the map bounds.

## Data conventions

Vectors
Vectors are comma‑separated numeric values:

vec2: x,y (two components) – used for positions, sizes, and atlas dimensions.

Whitespace around values is permitted but should be normalized by tooling.

## Numeric values

Integers are used for tile IDs, sizes, biome indices, etc. Floating‑point values are not used in the current map schema.

Asset paths
Asset references are relative paths or filenames. They must not contain machine‑specific absolute paths.

## Validation requirements

A future map validator should enforce at least:

Exactly one <map> root element.

<map_name> is present and unique within the map dataset.

<map_size> is present and contains exactly two positive integers.

Exactly height <map_tiles> elements exist, each containing exactly width integers.

All tile values are within the valid enumeration range (0–3) defined by eMapTileType.

<playerStartTile> is present and contains exactly two integers within map bounds.

All referenced texture and entity files exist.

<map_texture_atlas_diffuse>, <map_texture_atlas_normal>, <map_texture_atlas_specular>, and <map_texture_atlas_size> are present; size contains exactly two positive integers.

Optional fields, if present, contain valid data (e.g., coordinates within bounds, biome integer 0–3).

Unknown fields or sections are rejected or reported as warnings according to validator strictness.

The entity_wall file, if referenced, must be a valid entity definition and its type compatible with walls.

## Canonical example

```xml
<map>
   <map_name>test</map_name>
   <map_size>30,30</map_size>
   <map_tiles>2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2</map_tiles>
   <map_tiles>2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2</map_tiles>
   <map_tiles>2,1,3,3,3,1,3,1,3,1,3,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2</map_tiles>
   <map_tiles>2,1,3,1,3,1,3,1,3,1,3,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2</map_tiles>
   <map_tiles>2,1,3,3,3,1,3,1,3,1,3,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2</map_tiles>
   <map_tiles>2,1,3,1,1,1,3,1,3,1,3,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2</map_tiles>
   <map_tiles>2,1,3,1,1,1,3,3,3,3,3,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2</map_tiles>
   <map_tiles>2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2</map_tiles>
   <map_tiles>2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2</map_tiles>
   <map_tiles>1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2</map_tiles>
   <map_tiles>2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2</map_tiles>
   <map_tiles>2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2</map_tiles>
   <map_tiles>2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2</map_tiles>
   <map_tiles>2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2</map_tiles>
   <map_tiles>2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2</map_tiles>
   <map_tiles>2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2</map_tiles>
   <map_tiles>2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2</map_tiles>
   <map_tiles>2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2</map_tiles>
   <map_tiles>2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2</map_tiles>
   <map_tiles>2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2</map_tiles>
   <map_tiles>2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2</map_tiles>
   <map_tiles>2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2</map_tiles>
   <map_tiles>2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2</map_tiles>
   <map_tiles>2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2</map_tiles>
   <map_tiles>2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2</map_tiles>
   <map_tiles>2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2</map_tiles>
   <map_tiles>2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2</map_tiles>
   <map_tiles>2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2</map_tiles>
   <map_tiles>2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2</map_tiles>
   <map_tiles>2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2</map_tiles>

   <playerStartTile>15,5</playerStartTile>
   <map_music>music_001.ogg</map_music>
   <map_texture_atlas_diffuse>map_atlas_diffuse.png</map_texture_atlas_diffuse>
   <map_texture_atlas_normal>map_atlas_normal.png</map_texture_atlas_normal>
   <map_texture_atlas_specular>map_atlas_specular.png</map_texture_atlas_specular>
   <map_texture_atlas_size>2,2</map_texture_atlas_size>

   <map_biome>0</map_biome>
   <entity_wall>wall_001.txt</entity_wall>

   <portal_tile>0,9</portal_tile>
   <boss_alert_tile>15,15</boss_alert_tile>
</map>
```

## Event types

The engine defines the following event types in map_event_define.hpp (eMapEventType) that may be triggered by map interactions:

Value	Enumeration	Description
0	none	No event.
1	tileClicked	Player clicked on a tile (used in play mode).
2	pathChanged	Path changed (reserved).
3	portal	Player triggered a portal tile.
4	bossAlert	Player triggered a boss alert tile.
The portal_tile and boss_alert_tile fields set the corresponding event on those tiles.

## Schema evolution

Changes to the map schema should follow these rules:

Update this document first or in the same change as the data format change.

Update the map loader/parser in the same change when required.

Update validation tooling and tests.

Document breaking changes clearly.

Avoid silently changing the meaning of an existing field.

Prefer adding new optional fields over repurposing existing fields.

When a new field is added, specify its required/optional status and default behavior if omitted.

The schema should remain backward‑compatible where practical. When compatibility cannot be maintained, define the schema versioning strategy before introducing the breaking format change.


