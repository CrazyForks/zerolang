---
name: agent
description: Graph-first agent workflow for making focused Zero changes with CLI feedback.
---

# Zero Agent Workflow

Use this when editing Zero code. `zero.graph` is compiler input; `.0` is the human projection. Use JSON only when another tool must parse stable fields.

## Edit Through Patch

Anchored edits win. Do not retype a function for one line or rewrite `.0` for one declaration.

1. `--replace-in-fn`: edit one function's canonical body text.

```sh
zero patch . --replace-in-fn handleLine --old 'limit + 1' --new 'limit + 2'
```

`--old` must match `zero view --fn <name>` output exactly once.

2. `--replace-fn` for one whole body:

```sh
zero patch . --replace-fn greet --body-file - <<'EOF'
check world.out.write("hello agent\n")
EOF
```

3. Declaration work stays in ops; call sites update:

```sh
zero patch . --op 'setConst name="limit" value="64"'
zero patch . --op 'addParamTo fn="scan" name="bias" type="i32" default="0"'  # updates every call site
zero patch . --op 'setReturnType fn="scan" type="i64"'
```

4. New helpers stay graph-native:

```text
zero-program-graph-patch v1
upsertFunction handle
fn handle(request: Span<u8>, response: MutSpan<u8>) -> Maybe<Span<u8>> {
    return null
}
end
```

Pass a patch file, or stream full `zero-program-graph-patch v1` text with `zero patch . --patch-text -`.

Use `addReturnExpr fn="maybe" expr="null"` for non-identifier returns, `appendStmt fn="main" stmt="check std.http.listen(world, 3000_u16)"` for one statement, and `addTestBody name="api add" ... end` for tests. If a test shape is unsupported, use `deleteTest name="api add"` or `renameTest name="api add" value="api add route"`.

For runnable CLIs, keep `World` on `pub fn main`; helpers should be value-based. HTTP servers use `handle(request, response)` helpers instead of `World`.

A successful patch prints `validated: check-equivalent`: it validated and saved the graph. Run `zero run . -- <args>` / `zero test`. Repeat `--op` to batch edits. For rewrites and handle edits, see `zero skills get graph`.

Scoped reads; never read a whole `.0` file for one function:

- `zero view --fn <name>`: one function's source; misses show close matches.
- `zero view --fn <name> --around <text>`: enclosing block only.
- `zero view --outline <module-or-file>`: signatures plus one-line docs.

For a new agent-authored package: `zero init`, then `zero patch --op 'addMain'`.

## zero query

```text
zero query [--json] [--fn <name>] [--find <text>] [--refs <name>] [--calls <name>]
           [--node <id>] [--depth <n>] [--full] [--handles] [--no-help] [graph-input|name]
```

- bare name that is not an existing path: runs `--find` against the current package
- `zero query --fn <name> --handles`: patch handles for one function
- add `--no-help` when you need handles without the patch-operation footer
- `--find <text>`: search names, ids, types, values, and node kinds; prints matches with spans
- `--calls <name>` / `--refs <name>`: resolved calls and semantic references
- `--node <id>`: one node's span, parents, and children; short handles resolve here too

Import/export, identity recovery, structural edits, and merge live in `graph`. Direct `.0` edits are a last resort; never delete `zero.graph`.

## Verify Before Done

After a fix works, exercise typical and boundary inputs.

```sh
zero run . -- <typical input>
zero run . -- <empty or boundary input>
zero test
```

If behavior changed, add or update a `test` block. On a diagnostic, run `zero explain <code>`.

## Rules

- Treat effects as capabilities, not ambient globals: `World`, `std.fs`, `std.args`, `std.env`.
- Use `Maybe<T>`, explicit `raises` / `raises [...]`, and `check` / `rescue` instead of hidden failure.
- Do not invent syntax or CLI fields; load `language` when unsure.
- Check `stdlib` before hand-writing parsing or validation; it ships validators such as `std.time`, `std.inet`, `std.regex`, and `std.unicode`. Fetch one module with `zero skills get stdlib --topic std.time`.
