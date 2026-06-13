---
name: agent
description: Graph-first agent workflow for making focused Zero changes with CLI feedback.
---

# Zero Agent Workflow

Use this when editing Zero code, examples, tests, docs, or a package. `zero.graph` is the package compiler input; `.0` files are the human-readable projection. Command text output is written for agents. Use JSON only when another tool must parse stable fields.

## Edit Through Patch

Anchored small edits win. Never retype a function to change one line, and never rewrite a whole `.0` file for one declaration.

1. `--replace-in-fn`: Edit semantics on one function's canonical body text.

```sh
zero patch . --replace-in-fn handleLine --old 'limit + 1' --new 'limit + 2'
```

`--old` must match the text `zero view --fn <name>` prints exactly once; misses fail with the occurrence count.

2. `--replace-fn` with a heredoc for one whole body:

```sh
zero patch . --replace-fn greet --body-file - <<'EOF'
check world.out.write("hello agent\n")
EOF
```

3. Declaration work stays in ops, call sites updated for you:

```sh
zero patch . --op 'setConst name="limit" value="64"'
zero patch . --op 'addParamTo fn="scan" name="bias" type="i32" default="0"'  # updates every call site
zero patch . --op 'setReturnType fn="scan" type="i64"'
```

A successful patch prints `validated: check-equivalent`: it already validated and saved the graph, so do not run `zero check` to confirm it; go straight to `zero run . -- <args>` / `zero test`. Repeat `--op` to batch edits into one patch with a single revalidation. For expression-level cross-cutting swaps and node-addressed micro-edits (handles from `zero view --fn <name> --handles` or `zero query --fn <name> --handles`), see `zero skills get graph`.

Scoped reads; never read a whole `.0` file for one function:

- `zero view --fn <name>`: one function's source; misses fail with close matches.
- `zero view --fn <name> --around <text>`: only the enclosing block containing the text.
- `zero view --outline <module-or-file>`: signatures plus one-line docs, no bodies.

For a new agent-authored package: `zero init`, then `zero patch --op 'addMain'`.

## zero query

```text
zero query [--json] [--fn <name>] [--find <text>] [--refs <name>] [--calls <name>]
           [--node <id>] [--depth <n>] [--full] [--handles] [graph-input|name]
```

- bare name that is not an existing path: runs `--find` against the current package
- `--find <text>`: search names, ids, types, values, and node kinds; prints matches with spans
- `--calls <name>` / `--refs <name>`: resolved calls and semantic references
- `--node <id>`: one node's span, parents, and children; short handles resolve here too

Import/export, identity recovery, structural rewrites, and merge live in the `graph` topic. Direct `.0` edits are a last resort; never delete `zero.graph`.

## Verify Before Done

After a fix works on the path you changed, exercise the paths you did not. Zero inserts runtime checks (indexing is bounds-checked), so code that passes `zero check` and one probe run can still trap on another input; a trap exits with a signal status and no output.

```sh
zero run . -- <typical input>
zero run . -- <empty or boundary input>
zero test
```

If behavior changed, add or update a `test` block. On a diagnostic, run `zero explain <code>` before broad refactors.

## Rules

- Treat effects as capabilities, not ambient globals: `World`, `std.fs`, `std.args`, `std.env`.
- Use `Maybe<T>`, explicit `raises` / `raises [...]`, and `check` / `rescue` instead of hidden failure.
- Do not invent syntax or CLI fields; load `language` when unsure.
- Do not hand-write parsing or validation before checking the `stdlib` topic: it ships validators such as `std.time` (RFC 3339), `std.inet`, `std.regex`, and `std.unicode`. Fetch one module's signatures with `zero skills get stdlib --topic std.time`.
