#include "program_graph_size.h"

#include <stdlib.h>
#include <string.h>

static void graph_size_push_string(char ***items, size_t *len, const char *value) {
  *items = z_checked_reallocarray(*items, *len + 1, sizeof(char *));
  (*items)[(*len)++] = z_strdup(value ? value : "");
}

static const char *graph_size_source_path_for_line(const SourceInput *input, int line) {
  if (input && line > 0 && (size_t)line <= input->source_line_count && input->source_line_paths[line - 1]) return input->source_line_paths[line - 1];
  return input && input->source_file ? input->source_file : "";
}

static int graph_size_original_line_for_line(const SourceInput *input, int line) {
  if (input && line > 0 && (size_t)line <= input->source_line_count) return input->source_line_numbers[line - 1] > 0 ? input->source_line_numbers[line - 1] : 1;
  return line > 0 ? line : 1;
}

static const char *graph_size_module_for_line(const SourceInput *input, int line) {
  const char *path = graph_size_source_path_for_line(input, line);
  for (size_t i = 0; input && i < input->module_count; i++) {
    if (input->module_paths[i] && strcmp(input->module_paths[i], path) == 0) return input->module_names[i];
  }
  return input && input->module_count == 1 ? input->module_names[0] : "";
}

static const char *graph_size_module_path_for_name(const SourceInput *input, const char *name) {
  for (size_t i = 0; input && name && i < input->module_count; i++) {
    if (input->module_names[i] && strcmp(input->module_names[i], name) == 0) return input->module_paths[i];
  }
  return "";
}

static void graph_size_record_import(SourceInput *input, const char *from, const char *to, const char *path, const char *source_path, int line, int column, int length) {
  const char *import_name = to ? to : "";
  graph_size_push_string(&input->imports, &input->import_count, import_name);
  input->import_from = z_checked_reallocarray(input->import_from, input->import_edge_count + 1, sizeof(char *));
  input->import_to = z_checked_reallocarray(input->import_to, input->import_edge_count + 1, sizeof(char *));
  input->import_paths = z_checked_reallocarray(input->import_paths, input->import_edge_count + 1, sizeof(char *));
  input->import_source_paths = z_checked_reallocarray(input->import_source_paths, input->import_edge_count + 1, sizeof(char *));
  input->import_lines = z_checked_reallocarray(input->import_lines, input->import_edge_count + 1, sizeof(int));
  input->import_columns = z_checked_reallocarray(input->import_columns, input->import_edge_count + 1, sizeof(int));
  input->import_lengths = z_checked_reallocarray(input->import_lengths, input->import_edge_count + 1, sizeof(int));
  input->import_from[input->import_edge_count] = z_strdup(from ? from : "");
  input->import_to[input->import_edge_count] = z_strdup(import_name);
  input->import_paths[input->import_edge_count] = z_strdup(path ? path : "");
  input->import_source_paths[input->import_edge_count] = z_strdup(source_path ? source_path : "");
  input->import_lines[input->import_edge_count] = line > 0 ? line : 1;
  input->import_columns[input->import_edge_count] = column > 0 ? column : 1;
  input->import_lengths[input->import_edge_count] = length > 0 ? length : (import_name[0] ? (int)strlen(import_name) : 1);
  input->import_edge_count++;
}

static void graph_size_record_symbol(SourceInput *input, const char *module, const char *kind, const char *name, bool is_public) {
  input->symbol_names = z_checked_reallocarray(input->symbol_names, input->symbol_count + 1, sizeof(char *));
  input->symbol_modules = z_checked_reallocarray(input->symbol_modules, input->symbol_count + 1, sizeof(char *));
  input->symbol_kinds = z_checked_reallocarray(input->symbol_kinds, input->symbol_count + 1, sizeof(char *));
  input->symbol_public = z_checked_reallocarray(input->symbol_public, input->symbol_count + 1, sizeof(bool));
  input->symbol_names[input->symbol_count] = z_strdup(name ? name : "");
  input->symbol_modules[input->symbol_count] = z_strdup(module ? module : "");
  input->symbol_kinds[input->symbol_count] = z_strdup(kind ? kind : "");
  input->symbol_public[input->symbol_count++] = is_public;
}

static void graph_size_seed_imports(SourceInput *input, const Program *program) {
  for (size_t i = 0; program && i < program->use_imports.len; i++) {
    const UseImport *item = &program->use_imports.items[i];
    const char *name = item->module ? item->module : "";
    if (strncmp(name, "std.", 4) == 0) continue;
    graph_size_record_import(input,
                             graph_size_module_for_line(input, item->line),
                             name,
                             graph_size_module_path_for_name(input, name),
                             graph_size_source_path_for_line(input, item->line),
                             graph_size_original_line_for_line(input, item->line),
                             item->column,
                             item->end_column > item->column ? item->end_column - item->column : (int)strlen(name));
  }
}

static void graph_size_seed_symbols(SourceInput *input, const Program *program) {
  for (size_t i = 0; program && i < program->consts.len; i++) graph_size_record_symbol(input, graph_size_module_for_line(input, program->consts.items[i].line), "const", program->consts.items[i].name, program->consts.items[i].is_public);
  for (size_t i = 0; program && i < program->aliases.len; i++) graph_size_record_symbol(input, graph_size_module_for_line(input, program->aliases.items[i].line), "type-alias", program->aliases.items[i].name, program->aliases.items[i].is_public);
  for (size_t i = 0; program && i < program->shapes.len; i++) graph_size_record_symbol(input, graph_size_module_for_line(input, program->shapes.items[i].line), "shape", program->shapes.items[i].name, program->shapes.items[i].is_public);
  for (size_t i = 0; program && i < program->interfaces.len; i++) graph_size_record_symbol(input, graph_size_module_for_line(input, program->interfaces.items[i].line), "interface", program->interfaces.items[i].name, program->interfaces.items[i].is_public);
  for (size_t i = 0; program && i < program->enums.len; i++) graph_size_record_symbol(input, graph_size_module_for_line(input, program->enums.items[i].line), "enum", program->enums.items[i].name, false);
  for (size_t i = 0; program && i < program->choices.len; i++) graph_size_record_symbol(input, graph_size_module_for_line(input, program->choices.items[i].line), "choice", program->choices.items[i].name, false);
  for (size_t i = 0; program && i < program->functions.len; i++) graph_size_record_symbol(input, graph_size_module_for_line(input, program->functions.items[i].line), "function", program->functions.items[i].name, program->functions.items[i].is_public);
}

void z_program_graph_seed_size_source_metadata(SourceInput *input, const ZProgramGraph *graph, const Program *program) {
  if (!input || !graph) return;
  free(input->source);
  input->source = z_strdup(graph->graph_hash ? graph->graph_hash : "");
  graph_size_seed_imports(input, program);
  graph_size_seed_symbols(input, program);
}
