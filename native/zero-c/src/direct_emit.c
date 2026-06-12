#include "zero.h"

#include <stdlib.h>

bool z_direct_trap_branches_record(ZDirectTrapBranchList *list, size_t patch_offset) {
  if (!list) return false;
  if (list->len == list->cap) {
    size_t cap = list->cap ? list->cap * 2 : 8;
    size_t *items = realloc(list->items, cap * sizeof(size_t));
    if (!items) return false;
    list->items = items;
    list->cap = cap;
  }
  list->items[list->len++] = patch_offset;
  return true;
}

void z_direct_trap_branches_free(ZDirectTrapBranchList *lists, size_t count) {
  for (size_t i = 0; lists && i < count; i++) {
    free(lists[i].items);
    lists[i] = (ZDirectTrapBranchList){0};
  }
}

const char *z_direct_trap_message(ZDirectTrapKind kind) {
  switch (kind) {
    case Z_DIRECT_TRAP_INDEX_BOUNDS: return "trap: index out of bounds\n";
    case Z_DIRECT_TRAP_VALUE_BOUNDS: return "trap: value out of bounds\n";
    case Z_DIRECT_TRAP_WRITE_FAILED: return "trap: write failed\n";
    case Z_DIRECT_TRAP_KIND_COUNT: break;
  }
  return "trap\n";
}

bool z_direct_loop_frame_add_break(ZDirectLoopFrame *frame, size_t patch_offset) {
  if (!frame) return false;
  if (frame->break_len == frame->break_cap) {
    size_t cap = frame->break_cap ? frame->break_cap * 2 : 4;
    size_t *items = realloc(frame->break_patches, cap * sizeof(size_t));
    if (!items) return false;
    frame->break_patches = items;
    frame->break_cap = cap;
  }
  frame->break_patches[frame->break_len++] = patch_offset;
  return true;
}

bool z_emit_direct_object_from_ir(ZDirectBackend backend, const IrProgram *program, ZBuf *out, ZDiag *diag) {
  switch (backend) {
    case Z_DIRECT_BACKEND_ELF64: return z_emit_elf64_object_from_ir(program, out, diag);
    case Z_DIRECT_BACKEND_ELF_AARCH64: return z_emit_elf_aarch64_object_from_ir(program, out, diag);
    case Z_DIRECT_BACKEND_MACHO64: return z_emit_macho64_object_from_ir(program, out, diag);
    case Z_DIRECT_BACKEND_MACHO_X64: return z_emit_macho_x64_object_from_ir(program, out, diag);
    case Z_DIRECT_BACKEND_COFF_X64: return z_emit_coff_x64_object_from_ir(program, out, diag);
    case Z_DIRECT_BACKEND_COFF_AARCH64: return z_emit_coff_aarch64_object_from_ir(program, out, diag);
    case Z_DIRECT_BACKEND_NONE: return false;
  }
  return false;
}

bool z_emit_direct_executable_from_ir(ZDirectBackend backend, const IrProgram *program, ZBuf *out, ZDiag *diag) {
  switch (backend) {
    case Z_DIRECT_BACKEND_ELF64: return z_emit_elf64_exe_from_ir(program, out, diag);
    case Z_DIRECT_BACKEND_ELF_AARCH64: return z_emit_elf_aarch64_exe_from_ir(program, out, diag);
    case Z_DIRECT_BACKEND_MACHO64: return z_emit_macho64_exe_from_ir(program, out, diag);
    case Z_DIRECT_BACKEND_MACHO_X64: return z_emit_macho_x64_exe_from_ir(program, out, diag);
    case Z_DIRECT_BACKEND_COFF_X64: return z_emit_coff_x64_exe_from_ir(program, out, diag);
    case Z_DIRECT_BACKEND_COFF_AARCH64: return z_emit_coff_aarch64_exe_from_ir(program, out, diag);
    case Z_DIRECT_BACKEND_NONE: return false;
  }
  return false;
}
