#include "program_graph_roundtrip.h"
#include "program_graph_format.h"
#include "program_graph_import.h"
#include "program_graph_lower.h"

#include <stdio.h>
#include <string.h>

bool z_program_graph_direct_roundtrip_file(const char *artifact_path, const char *out_path, ZProgramGraphDirectRoundtrip *result, ZDiag *diag) {
  if (!result) return false;
  *result = (ZProgramGraphDirectRoundtrip){0};

  Program lowered_program = {0};
  SourceInput lowered_input = {0};
  bool ok = z_program_graph_load(artifact_path, &result->original, diag) &&
            z_program_graph_lower_to_program_for_roundtrip(&result->original, artifact_path, &lowered_program, &lowered_input, diag) &&
            z_program_graph_from_program(&lowered_input, &lowered_program, &result->roundtrip);

  if (ok && out_path && !z_program_graph_save(out_path, &result->roundtrip, diag)) ok = false;
  if (ok) z_program_graph_semantic_compare(&result->original, &result->roundtrip, &result->comparison);
  else if (diag && diag->code == 0) {
    diag->code = 2002;
    diag->path = artifact_path;
    diag->line = 1;
    diag->column = 1;
    diag->length = 1;
    snprintf(diag->message, sizeof(diag->message), "failed to rebuild program graph after direct lowering");
  }

  z_free_program(&lowered_program);
  z_free_source(&lowered_input);
  return ok;
}

void z_program_graph_direct_roundtrip_free(ZProgramGraphDirectRoundtrip *result) {
  if (!result) return;
  z_program_graph_free(&result->roundtrip);
  z_program_graph_free(&result->original);
  memset(result, 0, sizeof(*result));
}
