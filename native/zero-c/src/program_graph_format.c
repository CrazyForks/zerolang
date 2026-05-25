#include "program_graph_format.h"

static void graph_format_append_quoted(ZBuf *buf, const char *text) {
  zbuf_append_char(buf, '"');
  for (const char *p = text ? text : ""; *p; p++) {
    unsigned char ch = (unsigned char)*p;
    switch (ch) {
      case '\\': zbuf_append(buf, "\\\\"); break;
      case '"': zbuf_append(buf, "\\\""); break;
      case '\n': zbuf_append(buf, "\\n"); break;
      case '\r': zbuf_append(buf, "\\r"); break;
      case '\t': zbuf_append(buf, "\\t"); break;
      default:
        if (ch < 0x20) {
          const char *hex = "0123456789abcdef";
          char escape[7] = {'\\', 'u', '0', '0', hex[ch >> 4], hex[ch & 0x0f], 0};
          zbuf_append(buf, escape);
        } else {
          zbuf_append_char(buf, (char)ch);
        }
        break;
    }
  }
  zbuf_append_char(buf, '"');
}

static void graph_format_append_validation_json(ZBuf *buf, const ZProgramGraphValidation *validation) {
  bool ok = !validation || validation->ok;
  zbuf_append(buf, "{\"state\":");
  graph_format_append_quoted(buf, z_program_graph_validation_state_name(validation ? validation->state : Z_PROGRAM_GRAPH_VALIDATION_SHAPE_VALID));
  zbuf_appendf(buf, ",\"ok\":%s,\"diagnostics\":[", ok ? "true" : "false");
  if (!ok) {
    zbuf_append(buf, "{\"code\":");
    graph_format_append_quoted(buf, validation->code);
    zbuf_append(buf, ",\"message\":");
    graph_format_append_quoted(buf, validation->message);
    zbuf_append(buf, ",\"node\":");
    graph_format_append_quoted(buf, validation->node_id);
    zbuf_append(buf, ",\"edge\":{\"from\":");
    graph_format_append_quoted(buf, validation->edge_from);
    zbuf_append(buf, ",\"to\":");
    graph_format_append_quoted(buf, validation->edge_to);
    zbuf_append(buf, ",\"target\":");
    graph_format_append_quoted(buf, validation->edge_target);
    zbuf_append(buf, "}}");
  }
  zbuf_append(buf, "]}");
}

static void graph_format_append_validation_dump(ZBuf *buf, const ZProgramGraphValidation *validation) {
  bool ok = !validation || validation->ok;
  zbuf_append(buf, "validation ");
  graph_format_append_quoted(buf, z_program_graph_validation_state_name(validation ? validation->state : Z_PROGRAM_GRAPH_VALIDATION_SHAPE_VALID));
  zbuf_appendf(buf, " %s\n", ok ? "ok" : "failed");
  if (!ok) {
    zbuf_append(buf, "diagnostic code=");
    graph_format_append_quoted(buf, validation->code);
    zbuf_append(buf, " message=");
    graph_format_append_quoted(buf, validation->message);
    if (validation->node_id[0]) {
      zbuf_append(buf, " node=");
      graph_format_append_quoted(buf, validation->node_id);
    }
    if (validation->edge_from[0] || validation->edge_to[0]) {
      zbuf_append(buf, " edgeFrom=");
      graph_format_append_quoted(buf, validation->edge_from);
      zbuf_append(buf, " edgeTo=");
      graph_format_append_quoted(buf, validation->edge_to);
      zbuf_append(buf, " edgeTarget=");
      graph_format_append_quoted(buf, validation->edge_target);
    }
    zbuf_append_char(buf, '\n');
  }
}

static void graph_format_append_failed_json(ZBuf *buf) {
  zbuf_append(buf, "{\"schemaVersion\":1,\"canonicalSource\":false,\"idStrategy\":\"deterministic-traversal-r0\",\"graphHash\":\"\",\"validation\":{\"state\":\"decoded\",\"ok\":false,\"diagnostics\":[{\"code\":\"GRF001\",\"message\":\"program graph construction failed\"}]},\"counts\":{\"nodes\":0,\"edges\":0},\"nodes\":[],\"edges\":[]}");
}

static void graph_format_append_failed_dump(ZBuf *buf) {
  zbuf_append(buf, "zero-program-graph v1\n");
  zbuf_append(buf, "canonicalSource false\n");
  zbuf_append(buf, "idStrategy \"deterministic-traversal-r0\"\n");
  zbuf_append(buf, "graphHash \"\"\n");
  zbuf_append(buf, "validation \"decoded\" failed\n");
  zbuf_append(buf, "diagnostic code=\"GRF001\" message=\"program graph construction failed\"\n");
  zbuf_append(buf, "counts nodes=0 edges=0\n");
}

void z_program_graph_append_json(ZBuf *buf, const ZProgramGraph *graph, const ZProgramGraphValidation *validation) {
  zbuf_appendf(buf, "{\"schemaVersion\":%u,\"canonicalSource\":false,\"idStrategy\":", graph ? graph->schema_version : 1);
  graph_format_append_quoted(buf, graph ? graph->id_strategy : "deterministic-traversal-r0");
  zbuf_append(buf, ",\"graphHash\":");
  graph_format_append_quoted(buf, graph ? graph->graph_hash : NULL);
  zbuf_append(buf, ",\"validation\":");
  graph_format_append_validation_json(buf, validation);
  zbuf_appendf(buf, ",\"counts\":{\"nodes\":%zu,\"edges\":%zu},\"nodes\":[", graph ? graph->node_len : 0, graph ? graph->edge_len : 0);
  for (size_t i = 0; graph && i < graph->node_len; i++) {
    const ZProgramGraphNode *node = &graph->nodes[i];
    if (i > 0) zbuf_append(buf, ",");
    zbuf_append(buf, "{\"id\":");
    graph_format_append_quoted(buf, node->id);
    zbuf_append(buf, ",\"kind\":");
    graph_format_append_quoted(buf, z_program_graph_node_kind_name(node->kind));
    zbuf_append(buf, ",\"name\":");
    graph_format_append_quoted(buf, node->name);
    zbuf_append(buf, ",\"type\":");
    graph_format_append_quoted(buf, node->type);
    zbuf_append(buf, ",\"value\":");
    graph_format_append_quoted(buf, node->value);
    zbuf_append(buf, ",\"symbolId\":");
    graph_format_append_quoted(buf, node->symbol_id);
    zbuf_append(buf, ",\"typeId\":");
    graph_format_append_quoted(buf, node->type_id);
    zbuf_append(buf, ",\"effectId\":");
    graph_format_append_quoted(buf, node->effect_id);
    zbuf_append(buf, ",\"nodeHash\":");
    graph_format_append_quoted(buf, node->node_hash);
    zbuf_append(buf, ",\"path\":");
    graph_format_append_quoted(buf, node->path);
    zbuf_appendf(buf, ",\"line\":%d,\"column\":%d,\"public\":%s,\"mutable\":%s,\"static\":%s,\"fallible\":%s}",
                 node->line, node->column, node->is_public ? "true" : "false", node->is_mutable ? "true" : "false", node->is_static ? "true" : "false", node->fallible ? "true" : "false");
  }
  zbuf_append(buf, "],\"edges\":[");
  for (size_t i = 0; graph && i < graph->edge_len; i++) {
    const ZProgramGraphEdge *edge = &graph->edges[i];
    if (i > 0) zbuf_append(buf, ",");
    zbuf_append(buf, "{\"from\":");
    graph_format_append_quoted(buf, edge->from);
    zbuf_append(buf, ",\"to\":");
    graph_format_append_quoted(buf, edge->to);
    zbuf_append(buf, ",\"kind\":");
    graph_format_append_quoted(buf, edge->kind);
    zbuf_append(buf, ",\"target\":");
    graph_format_append_quoted(buf, z_program_graph_edge_target_name(edge->target));
    zbuf_appendf(buf, ",\"order\":%zu}", edge->order);
  }
  zbuf_append(buf, "]}");
}

void z_program_graph_append_dump(ZBuf *buf, const ZProgramGraph *graph, const ZProgramGraphValidation *validation) {
  zbuf_appendf(buf, "zero-program-graph v%u\n", graph ? graph->schema_version : 1);
  zbuf_append(buf, "canonicalSource false\n");
  zbuf_append(buf, "idStrategy ");
  graph_format_append_quoted(buf, graph ? graph->id_strategy : "deterministic-traversal-r0");
  zbuf_append_char(buf, '\n');
  zbuf_append(buf, "graphHash ");
  graph_format_append_quoted(buf, graph ? graph->graph_hash : NULL);
  zbuf_append_char(buf, '\n');
  graph_format_append_validation_dump(buf, validation);
  zbuf_appendf(buf, "counts nodes=%zu edges=%zu\n", graph ? graph->node_len : 0, graph ? graph->edge_len : 0);
  for (size_t i = 0; graph && i < graph->node_len; i++) {
    const ZProgramGraphNode *node = &graph->nodes[i];
    zbuf_append(buf, "node id=");
    graph_format_append_quoted(buf, node->id);
    zbuf_append(buf, " kind=");
    graph_format_append_quoted(buf, z_program_graph_node_kind_name(node->kind));
    zbuf_append(buf, " name=");
    graph_format_append_quoted(buf, node->name);
    zbuf_append(buf, " type=");
    graph_format_append_quoted(buf, node->type);
    zbuf_append(buf, " value=");
    graph_format_append_quoted(buf, node->value);
    zbuf_append(buf, " symbolId=");
    graph_format_append_quoted(buf, node->symbol_id);
    zbuf_append(buf, " typeId=");
    graph_format_append_quoted(buf, node->type_id);
    zbuf_append(buf, " effectId=");
    graph_format_append_quoted(buf, node->effect_id);
    zbuf_append(buf, " nodeHash=");
    graph_format_append_quoted(buf, node->node_hash);
    zbuf_append(buf, " path=");
    graph_format_append_quoted(buf, node->path);
    zbuf_appendf(buf, " line=%d column=%d public=%s mutable=%s static=%s fallible=%s\n",
                 node->line, node->column, node->is_public ? "true" : "false", node->is_mutable ? "true" : "false", node->is_static ? "true" : "false", node->fallible ? "true" : "false");
  }
  for (size_t i = 0; graph && i < graph->edge_len; i++) {
    const ZProgramGraphEdge *edge = &graph->edges[i];
    zbuf_append(buf, "edge from=");
    graph_format_append_quoted(buf, edge->from);
    zbuf_append(buf, " to=");
    graph_format_append_quoted(buf, edge->to);
    zbuf_append(buf, " kind=");
    graph_format_append_quoted(buf, edge->kind);
    zbuf_append(buf, " target=");
    graph_format_append_quoted(buf, z_program_graph_edge_target_name(edge->target));
    zbuf_appendf(buf, " order=%zu\n", edge->order);
  }
}

void z_append_program_graph_json(ZBuf *buf, const SourceInput *input, const Program *program) {
  ZProgramGraph graph;
  if (!z_program_graph_from_program(input, program, &graph)) {
    graph_format_append_failed_json(buf);
    return;
  }
  ZProgramGraphValidation validation = {0};
  z_program_graph_validate(&graph, &validation);
  z_program_graph_append_json(buf, &graph, &validation);
  z_program_graph_free(&graph);
}

void z_append_program_graph_dump(ZBuf *buf, const SourceInput *input, const Program *program, bool json) {
  ZProgramGraph graph;
  if (!z_program_graph_from_program(input, program, &graph)) {
    if (json) graph_format_append_failed_json(buf);
    else graph_format_append_failed_dump(buf);
    return;
  }
  ZProgramGraphValidation validation = {0};
  z_program_graph_validate(&graph, &validation);
  if (json) z_program_graph_append_json(buf, &graph, &validation);
  else z_program_graph_append_dump(buf, &graph, &validation);
  z_program_graph_free(&graph);
}
