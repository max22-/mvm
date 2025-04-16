#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <imgui.h>
#define MVM_IMPLEMENTATION
#include <mvm.h>
#include "gui.h"

static mvm vm;
static uint8_t *ram = nullptr;
static char load_error[1024] = {0};
static bool gui_is_init = false;

void gui_init(int argc, char *argv[]) {
    if(argc != 2) {
        snprintf(load_error, sizeof(load_error), "usage: %s file.rom", argv[0]);
        return;
    }
    const char *rom_path = argv[1];
    FILE *f = fopen(rom_path, "rb");
    ram = (uint8_t *)malloc(MVM_RAM_SIZE);
    if(!ram) {
        strncpy(load_error, "failed to allocate memory", sizeof(load_error));
        return;
    }
    if(!f) {
        snprintf(load_error, sizeof(load_error), "failed to open %s", rom_path);
        return;
    }

    fseek(f, 0, SEEK_END);
    const long rom_size = ftell(f);
    if(rom_size > MVM_RAM_SIZE) {
        fclose(f);
        strncpy(load_error, "rom file is too big to fit in ram", sizeof(load_error));
        return;
    }
    fseek(f, 0, SEEK_SET);
    size_t n = fread(ram, rom_size, 1, f);
    fclose(f);
    if(n != 1) {
        strncpy(load_error, "failed to load the rom file", sizeof(load_error));
        return;
    }

    mvm_init(&vm, ram);
    gui_is_init = true;
}

void gui_deinit() {
    if(ram)
        free(vm.ram);
}

#define TABLE_ROW(label, format, value) \
    do { \
        ImGui::TableNextRow(); \
        ImGui::TableNextColumn(); \
        ImGui::Text(label); \
        ImGui::TableNextColumn(); \
        ImGui::Text(format, value); \
    } while(0)

static void vm_state() {
    ImGui::Begin("mvm");
    if(*load_error) {
        ImGui::Text(load_error);
        ImGui::End();
        return;
    }
    const ImGuiTableFlags flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders;
    if(ImGui::BeginTable("vm state", 2, flags, ImVec2(300, 0))) {
        TABLE_ROW("pc", "0x%08x", vm.pc);
        TABLE_ROW("instruction", "%s", mvm_current_instruction_name(&vm));
        TABLE_ROW("sp", "0x%08x", vm.sp);
        TABLE_ROW("rsp", "0x%08x", vm.rsp);
        TABLE_ROW("status", "%s", mvm_status_name[vm.status]);
        ImGui::EndTable();
    }
    if(ImGui::Button("step"))
        mvm_run(&vm, 1);
    ImGui::End();
}

static void stack_display(const char *name, const uint32_t *stk, const uint32_t ptr) {
    const unsigned int n_columns = 8;
    if(ImGui::CollapsingHeader(name, ImGuiTreeNodeFlags_DefaultOpen)) {
        const ImGuiTableFlags flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders;
        if(ptr != 0) {
            if(ImGui::BeginTable(name, n_columns, flags)) {
                ImGui::TableNextRow();
                for(uint32_t i = 0; i < ptr; i++) {
                    ImGui::TableNextColumn();
                    ImGui::Text("0x%x", stk[i]);
                    if((i + 1) % n_columns == 0)
                        ImGui::TableNextRow();

                }
                for(uint32_t i = ptr; i % n_columns != 0; i++) {
                    ImGui::TableNextColumn();
                    //ImGui::Text("column %u", i);
                }
                ImGui::EndTable();
            }
        }
    }

}

static void vm_memory() {
    if(*load_error) return;
    ImGui::Begin("memory");
    stack_display("stack", vm.stk, vm.sp);
    stack_display("return stack", vm.rstk, vm.rsp);
    ImGui::End();
}

void gui() {
    vm_state();
    vm_memory();
}