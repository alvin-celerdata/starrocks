---
name: gensrc-compatibility
description: Ensure schema compatibility when editing gensrc IDL files (protobuf, thrift, enums) in StarRocks. Use when adding or modifying fields or enum values so changes remain backward-compatible and safe for generated sources.
---

# Gensrc Compatibility

## Overview

Apply compatibility rules when changing gensrc schema definitions so generated code keeps wire compatibility and downstream consumers do not break.

## Compatibility Rules

- Add new enum values at the end of the enum. Do not reorder or renumber existing values.
- Do not use `required` fields. Use `optional` (or the default non-required behavior) for new fields.
- Add new fields at the end of message/struct definitions. Do not reorder existing fields.
- Use a new, larger field ID for protobuf and thrift fields. Never reuse or renumber existing IDs.
- Avoid changing the type or meaning of an existing field. Add a new field instead if semantics change.

## Quick Checklist

- New enum value appended at the end and assigned a new numeric value.
- New fields appended at the end with a new, larger field ID.
- No `required` fields used in protobuf or thrift.
- No renumbering, reordering, or type changes for existing fields.
