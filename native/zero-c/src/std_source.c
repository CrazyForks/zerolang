#include "std_source.h"

#include "embedded_stdlib.inc"

#include <stdlib.h>
#include <string.h>

typedef struct {
  const char *public_name;
  const char *target_name;
  const char *module;
} ZStdSourceCall;

static const ZStdSourceModule std_source_modules[] = {
  {"std.collections", "std/collections.0", zero_embedded_stdlib_std_collections_0_chunks},
  {"std.math", "std/math.0", zero_embedded_stdlib_std_math_0_chunks},
  {"std.mem", "std/mem.0", zero_embedded_stdlib_std_mem_0_chunks},
  {"std.path", "std/path.0", zero_embedded_stdlib_std_path_0_chunks},
  {"std.search", "std/search.0", zero_embedded_stdlib_std_search_0_chunks},
  {"std.sort", "std/sort.0", zero_embedded_stdlib_std_sort_0_chunks},
  {"std.str", "std/str.0", zero_embedded_stdlib_std_str_0_chunks},
};

static const ZStdSourceCall std_source_calls[] = {
  {"std.collections.append", "__zero_std_collections_append", "std.collections"},
  {"std.collections.contains", "__zero_std_collections_contains", "std.collections"},
  {"std.collections.count", "__zero_std_collections_count", "std.collections"},
  {"std.collections.moveToFront", "__zero_std_collections_move_to_front", "std.collections"},
  {"std.collections.push", "__zero_std_collections_push", "std.collections"},
  {"std.collections.removeSwap", "__zero_std_collections_remove_swap", "std.collections"},
  {"std.collections.view", "__zero_std_collections_view", "std.collections"},
  {"std.math.clampU32", "__zero_std_math_clamp_u32", "std.math"},
  {"std.math.divisorCountU32", "__zero_std_math_divisor_count_u32", "std.math"},
  {"std.math.gcdU32", "__zero_std_math_gcd_u32", "std.math"},
  {"std.math.isPrimeU32", "__zero_std_math_is_prime_u32", "std.math"},
  {"std.math.lcmU32", "__zero_std_math_lcm_u32", "std.math"},
  {"std.math.maxU32", "__zero_std_math_max_u32", "std.math"},
  {"std.math.minU32", "__zero_std_math_min_u32", "std.math"},
  {"std.math.modPowU32", "__zero_std_math_mod_pow_u32", "std.math"},
  {"std.math.powU32", "__zero_std_math_pow_u32", "std.math"},
  {"std.math.properDivisorSumU32", "__zero_std_math_proper_divisor_sum_u32", "std.math"},
  {"std.mem.contains", "__zero_std_mem_contains", "std.mem"},
  {"std.mem.copyItems", "__zero_std_mem_copy_items", "std.mem"},
  {"std.mem.dropPrefix", "__zero_std_mem_drop_prefix", "std.mem"},
  {"std.mem.fillItems", "__zero_std_mem_fill_items", "std.mem"},
  {"std.mem.isEmpty", "__zero_std_mem_is_empty", "std.mem"},
  {"std.mem.prefix", "__zero_std_mem_prefix", "std.mem"},
  {"std.path.basename", "__zero_std_path_basename", "std.path"},
  {"std.path.dirname", "__zero_std_path_dirname", "std.path"},
  {"std.path.extension", "__zero_std_path_extension", "std.path"},
  {"std.path.join", "__zero_std_path_join", "std.path"},
  {"std.path.normalize", "__zero_std_path_normalize", "std.path"},
  {"std.path.relative", "__zero_std_path_relative", "std.path"},
  {"std.search.binaryI32", "__zero_std_search_binary_i32", "std.search"},
  {"std.search.binaryU32", "__zero_std_search_binary_u32", "std.search"},
  {"std.search.binaryUsize", "__zero_std_search_binary_usize", "std.search"},
  {"std.search.indexOf", "__zero_std_search_index_of", "std.search"},
  {"std.search.lastIndexOf", "__zero_std_search_last_index_of", "std.search"},
  {"std.search.lowerBoundI32", "__zero_std_search_lower_bound_i32", "std.search"},
  {"std.search.lowerBoundU32", "__zero_std_search_lower_bound_u32", "std.search"},
  {"std.search.lowerBoundUsize", "__zero_std_search_lower_bound_usize", "std.search"},
  {"std.sort.insertionI32", "__zero_std_sort_insertion_i32", "std.sort"},
  {"std.sort.insertionU32", "__zero_std_sort_insertion_u32", "std.sort"},
  {"std.sort.insertionUsize", "__zero_std_sort_insertion_usize", "std.sort"},
  {"std.sort.isSortedI32", "__zero_std_sort_is_sorted_i32", "std.sort"},
  {"std.sort.isSortedU32", "__zero_std_sort_is_sorted_u32", "std.sort"},
  {"std.sort.isSortedUsize", "__zero_std_sort_is_sorted_usize", "std.sort"},
  {"std.str.contains", "__zero_std_str_contains", "std.str"},
  {"std.str.countByte", "__zero_std_str_count_byte", "std.str"},
  {"std.str.endsWith", "__zero_std_str_ends_with", "std.str"},
  {"std.str.reverse", "__zero_std_str_reverse", "std.str"},
  {"std.str.startsWith", "__zero_std_str_starts_with", "std.str"},
  {"std.str.trimAscii", "__zero_std_str_trim_ascii", "std.str"},
  {"std.str.wordCountAscii", "__zero_std_str_word_count_ascii", "std.str"},
};

size_t z_std_source_module_count(void) {
  return sizeof(std_source_modules) / sizeof(std_source_modules[0]);
}

const ZStdSourceModule *z_std_source_module_at(size_t index) {
  return index < z_std_source_module_count() ? &std_source_modules[index] : NULL;
}

const ZStdSourceModule *z_std_source_module_for_name(const char *module) {
  for (size_t i = 0; i < sizeof(std_source_modules) / sizeof(std_source_modules[0]); i++) {
    if (strcmp(std_source_modules[i].module, module ? module : "") == 0) return &std_source_modules[i];
  }
  return NULL;
}

static const ZStdSourceCall *std_source_call_for_name(const char *qualified_name) {
  for (size_t i = 0; i < sizeof(std_source_calls) / sizeof(std_source_calls[0]); i++) {
    if (strcmp(std_source_calls[i].public_name, qualified_name ? qualified_name : "") == 0) return &std_source_calls[i];
  }
  return NULL;
}

const ZStdSourceModule *z_std_source_module_for_public_call(const char *qualified_name) {
  const ZStdSourceCall *call = std_source_call_for_name(qualified_name);
  return call ? z_std_source_module_for_name(call->module) : NULL;
}

const char *z_std_source_target_for_public_call(const char *qualified_name) {
  const ZStdSourceCall *call = std_source_call_for_name(qualified_name);
  return call ? call->target_name : NULL;
}

char *z_std_source_module_copy_source(const ZStdSourceModule *module) {
  if (!module || !module->chunks) return z_strdup("");
  ZBuf source;
  zbuf_init(&source);
  for (size_t i = 0; module->chunks[i]; i++) zbuf_append(&source, module->chunks[i]);
  return source.data ? source.data : z_strdup("");
}
