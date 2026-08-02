# Entity Data Schema

## Status

This document defines the canonical schema for entity definition data stored in `data/entity/`.

The schema is intentionally documented before the dataset grows so entity files remain predictable, validatable, and compatible with engine tooling.

## File format

Entity definitions are UTF-8 text files containing a single XML-like `<entity>` document.

The canonical extension is `.txt` for compatibility with the current dataset. The content is structured as XML-like markup, but is currently treated as an application-defined schema rather than formally validated XML.

Each entity file must contain exactly one root `<entity>` element.

```xml
<entity>
   <base>
      ...
   </base>
   ... optional sections ...
</entity>
```

## Top-level sections

| Section | Required | Purpose |
|---|---|---|
| `<base>` | Yes | Identity and classification |
| `<animation>` | No | Animation identifiers |
| `<audio>` | No | Audio asset references |
| `<graphics>` | No | Visual representation and rendering data |
| `<physics>` | No | Physical simulation properties |

Sections may be omitted when the entity does not use that subsystem.

## 1. Base

```xml
<base>
   <entity_name>barrel_001</entity_name>
   <entity_type>4</entity_type>
</base>
```

| Field | Type | Required | Description |
|---|---|---|---|
| `entity_name` | string | Yes | Unique identifier for the entity definition. |
| `entity_type` | integer | Yes | Numeric entity classification used by the engine. |

Rules:

- `entity_name` must be unique within the entity dataset.
- `entity_name` should use lowercase `snake_case` or the established project naming convention.
- `entity_type` must be a valid value defined by the engine's entity-type enumeration.
- Entity type values must not be assigned ad hoc in data files. The engine enumeration is authoritative.

## 2. Animation

```xml
<animation>
   <animation_idle>-1</animation_idle>
   <animation_walk>0</animation_walk>
</animation>
```

| Field | Type | Required | Description |
|---|---|---|---|
| `animation_idle` | integer | No | Animation index for idle. `-1` means none assigned. |
| `animation_walk` | integer | No | Animation index for walking. |

Rules:

- Animation indices are model-specific and must refer to valid animations when non-negative.
- `-1` may explicitly indicate that an animation is unavailable.
- Additional states should use the `animation_<state>` naming convention.
- Animation names and indices should be documented by the asset pipeline for multi-animation models.

## 3. Audio

```xml
<audio>
   <audio_spawn>sound_001.ogg</audio_spawn>
</audio>
```

| Field | Type | Required | Description |
|---|---|---|---|
| `audio_spawn` | asset path | No | Audio played when the entity is spawned or created. |

Rules:

- Audio paths are relative to the configured asset root.
- Referenced assets should exist and use a supported format.
- Additional event-specific fields should use `audio_<event>`.

## 4. Graphics

```xml
<graphics>
   <graphics_model>barrel_001.obj</graphics_model>
   <graphics_scale>0.5,0.5,0.5</graphics_scale>
   <material>
      <texture_diffuse>barrel_001_d.png</texture_diffuse>
      <texture_normal>barrel_001_n.png</texture_normal>
      <texture_specular>barrel_001_s.png</texture_specular>
   </material>
</graphics>
```

| Field | Type | Required | Description |
|---|---|---|---|
| `graphics_model` | asset path | No | Model file used to render the entity. |
| `graphics_scale` | vec3 | No | X, Y, and Z scale applied to the model. |
| `<material>` | object | No | Material and texture references. |
| `mesh_name` | string | No | Named mesh within a model to use or configure. |
| `mesh_enabled` | boolean/integer | No | Enables or disables the referenced mesh. Current data uses `1` for enabled. |

### Material fields

| Field | Type | Required | Description |
|---|---|---|---|
| `texture_diffuse` | asset path | No | Diffuse/albedo texture. |
| `texture_normal` | asset path | No | Normal map texture. |
| `texture_specular` | asset path | No | Specular texture. |

Rules:

- `graphics_model` must reference a supported model format.
- `graphics_scale` is a three-component floating-point vector.
- Material texture paths are relative to the configured asset root.
- Texture references should resolve to existing assets.
- `mesh_name` and `mesh_enabled` may be grouped when a model exposes multiple named meshes.
- Boolean fields should eventually use one canonical representation. Until the loader is standardized, existing `0`/`1` values are accepted.

## 5. Physics

```xml
<physics>
   <physics_shape>circle</physics_shape>
   <physics_radius>0.5</physics_radius>
   <physics_body_type>dynamic</physics_body_type>
   <physics_mass>5.0</physics_mass>
   <physics_restitution>0.6</physics_restitution>
   <physics_friction>0.4</physics_friction>
   <physics_position>10.0,0.0,10.0</physics_position>
   <physics_velocity>0.0,0.0,0.0</physics_velocity>
</physics>
```

| Field | Type | Required | Description |
|---|---|---|---|
| `physics_shape` | enum/string | Yes when physics is present | Collision shape. |
| `physics_body_type` | enum/string | Yes when physics is present | Physical body behavior, such as `static` or `dynamic`. |
| `physics_mass` | float | Dynamic bodies | Mass of the body. |
| `physics_restitution` | float | No | Bounciness coefficient. |
| `physics_friction` | float | No | Friction coefficient. |
| `physics_radius` | float | Shape-dependent | Radius or size parameter for supported shapes. |
| `physics_angle` | float | No | Initial orientation angle. |
| `physics_angular_velocity` | float | No | Initial angular velocity. |
| `physics_direction` | vec3 | No | Initial movement or facing direction, depending on engine semantics. |
| `physics_position` | vec3 | No | Initial world position. |
| `physics_velocity` | vec3 | No | Initial velocity. |
| `physics_acceleration` | vec3 | No | Initial acceleration. |
| `physics_deceleration` | vec3 | No | Deceleration limits or values. |
| `physics_max_velocity` | vec3 | No | Maximum velocity per axis. |
| `physics_max_acceleration` | vec3 | No | Maximum acceleration per axis. |
| `physics_max_deceleration` | vec3 | No | Maximum deceleration per axis. |

### Supported body types

The current dataset uses:

- `static`: body does not participate in dynamic motion.
- `dynamic`: body participates in dynamic simulation.

Additional body types must be added to the engine's authoritative enumeration and documented here before use.

### Supported shapes

The current dataset demonstrates:

- `circle`
- `aabb`

Additional shapes must be documented here and validated against the collision system before use.

### Rules

- Dynamic bodies must have a positive mass.
- Static bodies do not require dynamic-only properties such as mass, velocity, or acceleration.
- `physics_restitution` should normally be in `[0, 1]` unless the physics engine explicitly supports other values.
- `physics_friction` must not be negative.
- Shape-specific fields are required only when applicable to the selected shape.
- Vector fields must contain exactly three numeric components in the current representation.
- Physics values must use consistent world units defined by the engine.

## Data conventions

### Vectors

Vectors are comma-separated numeric values with three components:

```text
x,y,z
```

Whitespace around values is permitted by current examples but should be normalized by tooling.

### Numeric values

Floating-point values use decimal notation, for example `0.5`, `10.0`, and `-1.0`.

### Asset paths

Asset references are relative paths or filenames. They must not contain machine-specific absolute paths.

## Validation requirements

A future entity validator should enforce at least:

1. Exactly one `<entity>` root element.
2. A `<base>` section is present.
3. `entity_name` is present and unique.
4. `entity_type` is present and valid.
5. All numeric fields contain valid numeric values.
6. All vectors contain exactly three components.
7. Dynamic physics bodies have positive mass.
8. Friction and restitution values are within accepted physical ranges.
9. Shape-specific fields are present when required.
10. Referenced model, texture, and audio assets exist.
11. Animation indices are valid for the referenced model where metadata is available.
12. Boolean fields use the canonical representation.
13. Unknown fields and sections are rejected or reported as warnings according to validator strictness.

## Canonical example

```xml
<entity>
   <base>
      <entity_name>butcher</entity_name>
      <entity_type>2</entity_type>
   </base>

   <animation>
      <animation_idle>-1</animation_idle>
      <animation_walk>0</animation_walk>
   </animation>

   <audio>
      <audio_spawn>sound_001.ogg</audio_spawn>
   </audio>

   <graphics>
      <graphics_model>butcher_001.gltf</graphics_model>
      <graphics_scale>1.5,1.5,1.5</graphics_scale>
      <material>
         <texture_diffuse>butcher_001_d.png</texture_diffuse>
         <texture_normal>butcher_001_n.png</texture_normal>
         <texture_specular>butcher_001_s.png</texture_specular>
      </material>
      <mesh>
         <mesh_name>butcher</mesh_name>
         <mesh_enabled>1</mesh_enabled>
      </mesh>
   </graphics>

   <physics>
      <physics_shape>circle</physics_shape>
      <physics_radius>0.5</physics_radius>
      <physics_body_type>dynamic</physics_body_type>
      <physics_mass>10.0</physics_mass>
      <physics_restitution>0.8</physics_restitution>
      <physics_friction>0.5</physics_friction>
      <physics_angle>0.0</physics_angle>
      <physics_angular_velocity>0.0</physics_angular_velocity>
      <physics_direction>8.0,0.0,0.0</physics_direction>
      <physics_position>16.0,0.0,16.0</physics_position>
      <physics_velocity>0.0,0.0,0.0</physics_velocity>
      <physics_acceleration>0.5,0.0,0.5</physics_acceleration>
      <physics_deceleration>5.0,0.0,5.0</physics_deceleration>
      <physics_max_velocity>2.5,2.5,2.5</physics_max_velocity>
      <physics_max_acceleration>1.25,1.25,1.25</physics_max_acceleration>
      <physics_max_deceleration>20.0,20.0,20.0</physics_max_deceleration>
   </physics>
</entity>
```

## Schema evolution

Changes to the entity schema should follow these rules:

1. Update this document first or in the same change as the data format change.
2. Update the entity loader/parser in the same change when required.
3. Update validation tooling and tests.
4. Document breaking changes clearly.
5. Avoid silently changing the meaning of an existing field.
6. Prefer adding new optional fields over repurposing existing fields.

The schema should remain backward-compatible where practical. When compatibility cannot be maintained, define the schema versioning strategy before introducing the breaking format change.
