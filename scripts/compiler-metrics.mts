import { readdir, readFile } from "node:fs/promises";

const LARGE_FUNCTION_REPORT_THRESHOLD = 80;
const NEW_LARGE_FUNCTION_LIMIT = 120;

const sourceFileDirs = [
  "native/zero-c/include",
  "native/zero-c/src",
];

const fileBudgets = {
  "native/zero-c/include/zero.h": { maxLines: 900, maxStrcmpCalls: 0 },
  "native/zero-c/include/zero_runtime.h": { maxLines: 100, maxStrcmpCalls: 0 },
  "native/zero-c/src/checker.c": { maxLines: 9800, maxStrcmpCalls: 687 },
  "native/zero-c/src/main.c": { maxLines: 10300, maxStrcmpCalls: 546 },
  "native/zero-c/src/ir.c": { maxLines: 3700, maxStrcmpCalls: 224 },
  "native/zero-c/src/row_syntax.c": { maxLines: 2150, maxStrcmpCalls: 11 },
  "native/zero-c/src/ast.c": { maxLines: 250, maxStrcmpCalls: 0 },
  "native/zero-c/src/call_resolve.c": { maxLines: 200, maxStrcmpCalls: 2 },
  "native/zero-c/src/call_resolve.h": { maxLines: 100, maxStrcmpCalls: 0 },
  "native/zero-c/src/emit_macho64.c": { maxLines: 2600, maxStrcmpCalls: 2 },
  "native/zero-c/src/emit_elf64.c": { maxLines: 3300, maxStrcmpCalls: 3 },
  "native/zero-c/src/emit_elf_aarch64.c": { maxLines: 400, maxStrcmpCalls: 1 },
  "native/zero-c/src/emit_coff.c": { maxLines: 1500, maxStrcmpCalls: 1 },
  "native/zero-c/src/fs.c": { maxLines: 1250, maxStrcmpCalls: 32 },
  "native/zero-c/src/mir_verify.c": { maxLines: 1300, maxStrcmpCalls: 0 },
  "native/zero-c/src/mir_verify.h": { maxLines: 50, maxStrcmpCalls: 0 },
  "native/zero-c/src/specialize.c": { maxLines: 150, maxStrcmpCalls: 2 },
  "native/zero-c/src/specialize.h": { maxLines: 50, maxStrcmpCalls: 0 },
  "native/zero-c/src/target.c": { maxLines: 550, maxStrcmpCalls: 48 },
  "native/zero-c/src/type_core.c": { maxLines: 900, maxStrcmpCalls: 8 },
  "native/zero-c/src/type_core.h": { maxLines: 150, maxStrcmpCalls: 0 },
  "native/zero-c/src/unify.c": { maxLines: 500, maxStrcmpCalls: 14 },
  "native/zero-c/src/unify.h": { maxLines: 75, maxStrcmpCalls: 0 },
};

const knownLargeFunctionLimits = new Map([
  ["native/zero-c/src/ir.c|static bool ir_lower_expr(const Program *program, IrProgram *ir, const IrFunction *fun, const Expr *expr, IrValue **out) {", 1484],
  ["native/zero-c/src/checker.c|static bool check_expr_expected(CheckContext *ctx, const Program *program, const Expr *expr, Scope *scope, ZDiag *diag, const char *expected) {", 1170],
  ["native/zero-c/src/emit_elf64.c|static bool elf_emit_value(ZBuf *code, const IrFunction *fun, const IrValue *value, ElfEmitContext *ctx, ZDiag *diag) {", 1085],
  ["native/zero-c/src/emit_macho64.c|bool z_emit_macho64_object_from_ir(const IrProgram *program, ZBuf *out, ZDiag *diag) {", 481],
  ["native/zero-c/src/emit_elf64.c|bool z_emit_elf64_object_from_ir(const IrProgram *ir, ZBuf *out, ZDiag *diag) {", 405],
  ["native/zero-c/src/emit_macho64.c|bool z_emit_macho64_exe_from_ir(const IrProgram *program, ZBuf *out, ZDiag *diag) {", 318],
  ["native/zero-c/src/emit_elf64.c|static bool elf_emit_instr(ZBuf *text, const IrFunction *fun, const IrInstr *instr, ElfEmitContext *ctx, ZDiag *diag) {", 300],
  ["native/zero-c/src/emit_macho64.c|static bool macho_emit_value_to_reg_at(ZBuf *text, const IrFunction *fun, const IrValue *value, unsigned reg, unsigned frame_size, unsigned scratch_slot, MachOEmitContext *ctx, ZDiag *diag) {", 295],
  ["native/zero-c/src/checker.c|static bool check_stmt(CheckContext *ctx, const Program *program, const Function *fun, const Stmt *stmt, Scope *scope, ZDiag *diag, int loop_depth) {", 259],
  ["native/zero-c/src/checker.c|bool z_check_program(const Program *program, ZDiag *diag) {", 213],
  ["native/zero-c/src/emit_coff.c|bool z_emit_coff_x64_exe_from_ir(const IrProgram *program, ZBuf *out, ZDiag *diag) {", 213],
  ["native/zero-c/src/checker.c|static const char *expr_type(CheckContext *ctx, const Program *program, const Expr *expr, Scope *scope) {", 205],
  ["native/zero-c/src/emit_macho64.c|static bool macho_emit_instr(ZBuf *text, const IrFunction *fun, const IrInstr *instr, unsigned frame_size, bool restore_process_args, MachOEmitContext *ctx, ZDiag *diag) {", 193],
  ["native/zero-c/src/checker.c|static bool collect_return_value_provenance_from_stmt_vec(CheckContext *ctx, const Program *program, const Function *fun, const StmtVec *body, Scope *scope, GenericBinding *bindings, size_t binding_len, ValueProvenance *out, bool *may_return, bool *complete) {", 192],
  ["native/zero-c/src/emit_coff.c|static bool coff_emit_value(ZBuf *text, const IrFunction *fun, const IrValue *value, CoffEmitContext *ctx, ZDiag *diag) {", 191],
  ["native/zero-c/src/row_syntax.c|ZRowTokenVec z_row_tokenize(const char *source, ZDiag *diag) {", 177],
  ["native/zero-c/src/ir.c|static bool ir_lower_stmt_to_vec(const Program *program, IrProgram *ir, IrFunction *mir_fun, const Stmt *stmt, IrInstr **out_items, size_t *out_len, size_t *out_cap, bool *saw_return) {", 172],
  ["native/zero-c/src/emit_coff.c|bool z_emit_coff_x64_object_from_ir(const IrProgram *program, ZBuf *out, ZDiag *diag) {", 171],
  ["native/zero-c/src/emit_coff.c|static bool coff_emit_instr(ZBuf *text, const IrFunction *fun, const IrInstr *instr, CoffEmitContext *ctx, ZDiag *diag) {", 165],
  ["native/zero-c/src/emit_elf64.c|bool z_emit_elf64_exe_from_ir(const IrProgram *ir, ZBuf *out, ZDiag *diag) {", 158],
  ["native/zero-c/src/checker.c|static bool expr_reference_provenance(CheckContext *ctx, const Program *program, const Expr *expr, Scope *scope, ValueProvenance *origins) {", 152],
  ["native/zero-c/src/checker.c|static const char *std_call_return_type(const Expr *callee) {", 146],
  ["native/zero-c/src/emit_elf64.c|static bool elf_emit_read_all_or_raise_to_local(ZBuf *text, const IrFunction *fun, const IrInstr *instr, ElfEmitContext *ctx, ZDiag *diag) {", 145],
  ["native/zero-c/src/ast.c|void z_free_program(Program *program) {", 143],
  ["native/zero-c/src/checker.c|static int std_call_arg_count(const char *name) {", 141],
  ["native/zero-c/src/checker.c|static const char *std_call_arg_type(const char *name, size_t index) {", 139],
  ["native/zero-c/src/emit_elf_aarch64.c|bool z_emit_elf_aarch64_object_from_ir(const IrProgram *ir, ZBuf *out, ZDiag *diag) {", 134],
  ["native/zero-c/src/mir_verify.c|static bool mir_verify_direct_value_kind_contract(IrProgram *ir, const IrFunction *fun, const MirVerifierState *state, const IrValue *value, MirHelperRequirements *requirements) {", 134],
  ["native/zero-c/src/row_syntax.c|static Stmt *row_parse_statement(const ZRowTokenVec *tokens, const ZRowTree *tree, size_t row_index, ZDiag *diag) {", 132],
  ["native/zero-c/src/row_syntax.c|Program z_parse_row(const ZRowTokenVec *tokens, const ZRowTree *tree, ZDiag *diag) {", 130],
  ["native/zero-c/src/ir.c|static bool ir_lower_byte_view(const Program *program, IrProgram *ir, const IrFunction *fun, const Expr *expr, IrValue **out) {", 124],
  ["native/zero-c/src/row_syntax.c|bool z_row_parse_layout(const ZRowTokenVec *tokens, ZRowTree *tree, ZDiag *diag) {", 123],
]);

const knownReturnTypeDivergences = new Map([
  ["std.mem.get", {
    helperReturnType: "Maybe<T>",
    checkerReturnType: "Unknown",
    reason: "checker resolves the element-specific Maybe<T> return in a dedicated std.mem.get path",
  }],
]);

const allowedHelpersWithSpecialArgTypeChecks = [
  "std.mem.eqlBytes",
  "std.mem.len",
];

async function nativeSourceFiles() {
  const groups = await Promise.all(sourceFileDirs.map(async (dir) => {
    const entries = await readdir(dir, { withFileTypes: true });
    return entries
      .filter((entry) => entry.isFile() && /\.[ch]$/.test(entry.name))
      .map((entry) => `${dir}/${entry.name}`);
  }));
  return groups.flat().sort((a, b) => a.localeCompare(b));
}

function countMatches(text, pattern) {
  return [...text.matchAll(pattern)].length;
}

function lineCount(text) {
  if (text.length === 0) return 0;
  return text.endsWith("\n") ? text.split("\n").length - 1 : text.split("\n").length;
}

function updateBraceDepth(line, depth) {
  for (const ch of line) {
    if (ch === "{") depth++;
    else if (ch === "}") depth--;
  }
  return depth;
}

function largeFunctions(path, text) {
  const lines = text.split("\n");
  const results = [];
  let depth = 0;
  let current = null;
  const functionStart = /^([A-Za-z_][A-Za-z0-9_]*|static)[A-Za-z0-9_ \t*]+[A-Za-z_][A-Za-z0-9_]*\([^;]*\)[ \t]*\{/;
  for (let index = 0; index < lines.length; index++) {
    const line = lines[index];
    if (!current && depth === 0 && functionStart.test(line)) {
      current = { path, line: index + 1, signature: line.trim() };
    }
    depth = updateBraceDepth(line, depth);
    if (current && depth === 0) {
      const size = index + 1 - current.line + 1;
      if (size >= LARGE_FUNCTION_REPORT_THRESHOLD) results.push({ ...current, lines: size });
      current = null;
    }
  }
  return results;
}

function namesFromRegex(text, pattern) {
  return [...text.matchAll(pattern)].map((match) => match[1]).sort();
}

function sortedMapKeys(map) {
  return [...map.keys()].sort((a, b) => a.localeCompare(b));
}

function duplicates(items) {
  const counts = new Map();
  for (const item of items) counts.set(item, (counts.get(item) ?? 0) + 1);
  return [...counts.entries()]
    .filter(([, count]) => count > 1)
    .map(([name, count]) => ({ name, count }))
    .sort((a, b) => a.name.localeCompare(b.name));
}

function missingFrom(left, right) {
  const rightSet = new Set(right);
  return [...new Set(left)].filter((item) => !rightSet.has(item)).sort();
}

function largeFunctionKey(item) {
  return `${item.path}|${item.signature}`;
}

function cBlock(text, marker) {
  const markerIndex = text.indexOf(marker);
  if (markerIndex < 0) return "";
  const openIndex = text.indexOf("{", markerIndex);
  if (openIndex < 0) return "";
  let depth = 0;
  for (let index = openIndex; index < text.length; index++) {
    const ch = text[index];
    if (ch === "{") depth++;
    else if (ch === "}") {
      depth--;
      if (depth === 0) return text.slice(openIndex + 1, index);
    }
  }
  return "";
}

function parseStdHelpers(text) {
  const block = cBlock(text, "static const StdHelperInfo std_helpers[] =");
  return [...block.matchAll(/\{\s*"([^"]+)"\s*,\s*"([^"]+)"\s*,\s*(-?\d+)\s*,/g)]
    .map((match) => ({
      name: match[1],
      returnType: match[2],
      argCount: Number(match[3]),
    }))
    .sort((a, b) => a.name.localeCompare(b.name));
}

function parseStdHttpErrorNames(text) {
  const block = cBlock(text, "static int std_http_error_code");
  return namesFromRegex(block, /strcmp\(name,\s+"(std\.[^"]+)"\)\s*==\s*0\)\s*return\s*\d+/g);
}

function parseCheckerReturnTypes(text) {
  const result = new Map();
  const block = cBlock(text, "static const char *std_call_return_type");
  for (const match of block.matchAll(/strcmp\(name\.data,\s+"(std\.[^"]+)"\)\s*==\s*0\)\s*result\s*=\s*"([^"]+)"/g)) {
    result.set(match[1], match[2]);
  }
  for (const name of parseStdHttpErrorNames(text)) {
    result.set(name, "HttpError");
  }
  return result;
}

function parseCheckerArgCounts(text) {
  const result = new Map();
  const block = cBlock(text, "static int std_call_arg_count");
  for (const match of block.matchAll(/strcmp\(name,\s+"(std\.[^"]+)"\)\s*==\s*0\)\s*return\s*(-?\d+)/g)) {
    result.set(match[1], Number(match[2]));
  }
  for (const name of parseStdHttpErrorNames(text)) {
    result.set(name, 0);
  }
  return result;
}

function parseCheckerArgTypeNames(text) {
  const block = cBlock(text, "static const char *std_call_arg_type");
  return namesFromRegex(block, /strcmp\(name,\s+"(std\.[^"]+)"/g);
}

function helperReturnTypeMismatches(helpers, checkerReturnTypes) {
  return helpers
    .filter((helper) => checkerReturnTypes.has(helper.name) && checkerReturnTypes.get(helper.name) !== helper.returnType)
    .map((helper) => ({
      name: helper.name,
      helperReturnType: helper.returnType,
      checkerReturnType: checkerReturnTypes.get(helper.name),
    }))
    .sort((a, b) => a.name.localeCompare(b.name));
}

function helperArgCountMismatches(helpers, checkerArgCounts) {
  return helpers
    .filter((helper) => checkerArgCounts.has(helper.name) && checkerArgCounts.get(helper.name) !== helper.argCount)
    .map((helper) => ({
      name: helper.name,
      helperArgCount: helper.argCount,
      checkerArgCount: checkerArgCounts.get(helper.name),
    }))
    .sort((a, b) => a.name.localeCompare(b.name));
}

function knownReturnTypeDivergenceMatches(mismatch) {
  const known = knownReturnTypeDivergences.get(mismatch.name);
  return known &&
    known.helperReturnType === mismatch.helperReturnType &&
    known.checkerReturnType === mismatch.checkerReturnType;
}

function budgetViolations(files, allLargeFunctions, stdlib) {
  const violations = [];
  for (const path of Object.keys(files).sort()) {
    if (!fileBudgets[path]) {
      violations.push({ kind: "missing-file-budget", path });
    }
  }
  for (const [path, budget] of Object.entries(fileBudgets)) {
    const metrics = files[path];
    if (!metrics) {
      violations.push({ kind: "missing-file-metrics", path });
      continue;
    }
    if (metrics.lines > budget.maxLines) {
      violations.push({
        kind: "file-line-budget",
        path,
        actual: metrics.lines,
        limit: budget.maxLines,
      });
    }
    if (metrics.strcmpCalls > budget.maxStrcmpCalls) {
      violations.push({
        kind: "strcmp-budget",
        path,
        actual: metrics.strcmpCalls,
        limit: budget.maxStrcmpCalls,
      });
    }
  }
  for (const item of allLargeFunctions) {
    const key = largeFunctionKey(item);
    const knownLimit = knownLargeFunctionLimits.get(key);
    if (knownLimit !== undefined) {
      if (item.lines > knownLimit) {
        violations.push({
          kind: "known-large-function-growth",
          path: item.path,
          signature: item.signature,
          actual: item.lines,
          limit: knownLimit,
        });
      }
    } else if (item.lines > NEW_LARGE_FUNCTION_LIMIT) {
      violations.push({
        kind: "new-large-function",
        path: item.path,
        signature: item.signature,
        actual: item.lines,
        limit: NEW_LARGE_FUNCTION_LIMIT,
      });
    }
  }
  if (stdlib.duplicateMainHelpers.length > 0) {
    violations.push({
      kind: "duplicate-stdlib-helper",
      helpers: stdlib.duplicateMainHelpers,
    });
  }
  if (stdlib.checkerReturnsMissingFromMainHelpers.length > 0) {
    violations.push({
      kind: "stdlib-checker-return-extra",
      names: stdlib.checkerReturnsMissingFromMainHelpers,
    });
  }
  if (stdlib.mainHelpersMissingFromCheckerReturns.length > 0) {
    violations.push({
      kind: "stdlib-helper-return-missing",
      names: stdlib.mainHelpersMissingFromCheckerReturns,
    });
  }
  const unexpectedReturnTypeMismatches = stdlib.returnTypeMismatches.filter((mismatch) => !knownReturnTypeDivergenceMatches(mismatch));
  if (unexpectedReturnTypeMismatches.length > 0) {
    violations.push({
      kind: "stdlib-helper-return-type-mismatch",
      mismatches: unexpectedReturnTypeMismatches,
    });
  }
  const staleReturnTypeDivergences = [...knownReturnTypeDivergences.keys()]
    .filter((name) => !stdlib.returnTypeMismatches.some((mismatch) => knownReturnTypeDivergenceMatches(mismatch) && mismatch.name === name))
    .sort((a, b) => a.localeCompare(b));
  if (staleReturnTypeDivergences.length > 0) {
    violations.push({
      kind: "stale-stdlib-return-type-divergence-allowlist",
      names: staleReturnTypeDivergences,
    });
  }
  if (stdlib.checkerArgCountsMissingFromMainHelpers.length > 0) {
    violations.push({
      kind: "stdlib-checker-arg-count-extra",
      names: stdlib.checkerArgCountsMissingFromMainHelpers,
    });
  }
  if (stdlib.mainHelpersMissingFromCheckerArgCounts.length > 0) {
    violations.push({
      kind: "stdlib-helper-arg-count-missing",
      names: stdlib.mainHelpersMissingFromCheckerArgCounts,
    });
  }
  if (stdlib.argCountMismatches.length > 0) {
    violations.push({
      kind: "stdlib-helper-arg-count-mismatch",
      mismatches: stdlib.argCountMismatches,
    });
  }
  if (stdlib.checkerArgTypesMissingFromMainHelpers.length > 0) {
    violations.push({
      kind: "stdlib-checker-arg-type-extra",
      names: stdlib.checkerArgTypesMissingFromMainHelpers,
    });
  }
  const unexpectedArgTypeGaps = missingFrom(
    stdlib.nonzeroArgHelpersMissingFromCheckerArgTypes,
    allowedHelpersWithSpecialArgTypeChecks,
  );
  if (unexpectedArgTypeGaps.length > 0) {
    violations.push({
      kind: "stdlib-helper-arg-type-missing",
      names: unexpectedArgTypeGaps,
    });
  }
  const staleArgTypeAllowlist = missingFrom(
    allowedHelpersWithSpecialArgTypeChecks,
    stdlib.nonzeroArgHelpersMissingFromCheckerArgTypes,
  );
  if (staleArgTypeAllowlist.length > 0) {
    violations.push({
      kind: "stale-stdlib-helper-arg-type-allowlist",
      names: staleArgTypeAllowlist,
    });
  }
  return violations;
}

const sourceFiles = await nativeSourceFiles();
const texts = new Map();
for (const path of sourceFiles) {
  texts.set(path, await readFile(path, "utf8"));
}

const files = Object.fromEntries([...texts.entries()].map(([path, text]) => [path, {
  lines: lineCount(text),
  strcmpCalls: countMatches(text, /strcmp\(/g),
  unsupportedMarkers: countMatches(text, /Unknown|unsupported|currently|MVP|direct backend/g),
}]));

const checker = texts.get("native/zero-c/src/checker.c") ?? "";
const main = texts.get("native/zero-c/src/main.c") ?? "";
const ir = texts.get("native/zero-c/src/ir.c") ?? "";

const stdHelpers = parseStdHelpers(main);
const checkerReturnTypes = parseCheckerReturnTypes(checker);
const checkerArgCounts = parseCheckerArgCounts(checker);
const checkerArgTypeNames = parseCheckerArgTypeNames(checker);
const checkerKnownStdNames = namesFromRegex(checker, /"(std\.[^"]+)"/g);
const checkerReturnNames = sortedMapKeys(checkerReturnTypes);
const checkerArgCountNames = sortedMapKeys(checkerArgCounts);
const mainHelperNames = stdHelpers.map((helper) => helper.name);
const irStdNames = namesFromRegex(ir, /strcmp\(callee_name,\s+"(std\.[^"]+)"/g);
const nonzeroArgHelperNames = stdHelpers
  .filter((helper) => helper.argCount > 0)
  .map((helper) => helper.name);

const allLargeFunctions = [...texts.entries()]
  .flatMap(([path, text]) => largeFunctions(path, text))
  .sort((a, b) => b.lines - a.lines);

const stdlib = {
  checkerReturnCount: new Set(checkerReturnNames).size,
  checkerKnownStdNameCount: new Set(checkerKnownStdNames).size,
  checkerArgCountCount: new Set(checkerArgCountNames).size,
  checkerArgTypeCount: new Set(checkerArgTypeNames).size,
  mainHelperCount: new Set(mainHelperNames).size,
  irDirectStdCallCount: new Set(irStdNames).size,
  duplicateMainHelpers: duplicates(mainHelperNames),
  returnNamesMissingFromMainHelpers: missingFrom(checkerReturnNames, mainHelperNames),
  checkerReturnsMissingFromMainHelpers: missingFrom(checkerReturnNames, mainHelperNames),
  mainHelpersMissingFromCheckerReturns: missingFrom(mainHelperNames, checkerReturnNames),
  returnTypeMismatches: helperReturnTypeMismatches(stdHelpers, checkerReturnTypes),
  checkerArgCountsMissingFromMainHelpers: missingFrom(checkerArgCountNames, mainHelperNames),
  mainHelpersMissingFromCheckerArgCounts: missingFrom(mainHelperNames, checkerArgCountNames),
  argCountMismatches: helperArgCountMismatches(stdHelpers, checkerArgCounts),
  checkerArgTypesMissingFromMainHelpers: missingFrom(checkerArgTypeNames, mainHelperNames),
  nonzeroArgHelpersMissingFromCheckerArgTypes: missingFrom(nonzeroArgHelperNames, checkerArgTypeNames),
  mainHelpersMissingFromCheckerKnownNames: missingFrom(mainHelperNames, checkerKnownStdNames),
};
const violations = budgetViolations(files, allLargeFunctions, stdlib);

const report = {
  schema: 1,
  files,
  largeFunctions: allLargeFunctions.slice(0, 25),
  stdlib,
  budget: {
    ok: violations.length === 0,
    newLargeFunctionLimit: NEW_LARGE_FUNCTION_LIMIT,
    reportThreshold: LARGE_FUNCTION_REPORT_THRESHOLD,
    sourceFileCount: sourceFiles.length,
    knownReturnTypeDivergences: Object.fromEntries(knownReturnTypeDivergences),
    allowedHelpersWithSpecialArgTypeChecks,
    violations,
  },
};

console.log(JSON.stringify(report, null, 2));
if (violations.length > 0) {
  process.exitCode = 1;
}
