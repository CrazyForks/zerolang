#include "program_graph_std_deps.h"

#include "std_source.h"

#include <stdlib.h>
#include <string.h>

static bool std_deps_text_eq(const char *left, const char *right) {
  return strcmp(left ? left : "", right ? right : "") == 0;
}

static const ZProgramGraphNode *std_deps_find_node(const ZProgramGraph *graph, const char *id) {
  for (size_t i = 0; graph && id && i < graph->node_len; i++) {
    if (std_deps_text_eq(graph->nodes[i].id, id)) return &graph->nodes[i];
  }
  return NULL;
}

static const ZProgramGraphNode *std_deps_ordered_node(const ZProgramGraph *graph, const char *from, const char *kind, size_t order) {
  for (size_t i = 0; graph && from && kind && i < graph->edge_len; i++) {
    const ZProgramGraphEdge *edge = &graph->edges[i];
    if (edge->target == Z_PROGRAM_GRAPH_EDGE_TARGET_NODE && edge->order == order &&
        std_deps_text_eq(edge->from, from) && std_deps_text_eq(edge->kind, kind)) {
      return std_deps_find_node(graph, edge->to);
    }
  }
  return NULL;
}

static bool std_deps_expr_name_into(const ZProgramGraph *graph, const ZProgramGraphNode *node, ZBuf *out) {
  if (!node || !out) return false;
  if (node->kind == Z_PROGRAM_GRAPH_NODE_IDENTIFIER || node->kind == Z_PROGRAM_GRAPH_NODE_CALL) {
    zbuf_append(out, node->name ? node->name : "");
    return true;
  }
  if (node->kind == Z_PROGRAM_GRAPH_NODE_FIELD_ACCESS || node->kind == Z_PROGRAM_GRAPH_NODE_METHOD_CALL) {
    const ZProgramGraphNode *left = std_deps_ordered_node(graph, node->id, "left", 0);
    if (!std_deps_expr_name_into(graph, left, out)) return false;
    if (node->kind == Z_PROGRAM_GRAPH_NODE_FIELD_ACCESS ||
        !left || left->kind != Z_PROGRAM_GRAPH_NODE_FIELD_ACCESS || !std_deps_text_eq(left->name, node->name)) {
      zbuf_append_char(out, '.');
      zbuf_append(out, node->name ? node->name : "");
    }
    return true;
  }
  return false;
}

static char *std_deps_expr_name(const ZProgramGraph *graph, const ZProgramGraphNode *node) {
  ZBuf name;
  zbuf_init(&name);
  if (!std_deps_expr_name_into(graph, node, &name)) {
    zbuf_free(&name);
    return NULL;
  }
  return name.data;
}

bool z_program_graph_references_std_module(const ZProgramGraph *graph, const char *module) {
  for (size_t i = 0; graph && module && i < graph->node_len; i++) {
    const ZProgramGraphNode *node = &graph->nodes[i];
    if (node->kind != Z_PROGRAM_GRAPH_NODE_CALL && node->kind != Z_PROGRAM_GRAPH_NODE_METHOD_CALL) continue;
    char *qualified = std_deps_expr_name(graph, node);
    const ZStdSourceModule *call_module = z_std_source_module_for_public_call(qualified && qualified[0] ? qualified : node->name);
    bool matches = call_module && std_deps_text_eq(call_module->module, module);
    free(qualified);
    if (matches) return true;
  }
  return false;
}
