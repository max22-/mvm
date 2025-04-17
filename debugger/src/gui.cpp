#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <imgui.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL_opengles2.h>
#else
#include <SDL_opengl.h>
#endif
#define MVM_IMPLEMENTATION
#include <mvm.h>
#include "gui.h"

static mvm vm;
static uint8_t *ram = nullptr;
static char load_error[1024] = {0};
static bool gui_is_init = false;

bool run = false;

GLuint fb_texture;
uint16_t *frame_buffer = nullptr;
bool dirty = false;

uint32_t port_in(uint32_t port) {
    return 0;
}

void port_out(uint32_t port, uint32_t value) {
    if(port == 0 && value == 1) {
        dirty = true;
    }
}

uint32_t mmio_read8(mvm *vm, uint32_t addr) {
    vm->status = MVM_SEGMENTATION_FAULT;
    return 0;
}

uint32_t mmio_read16(mvm *vm, uint32_t addr) {
    vm->status = MVM_SEGMENTATION_FAULT;
    return 0;
}

uint32_t mmio_read32(mvm *vm, uint32_t addr) {
    vm->status = MVM_SEGMENTATION_FAULT;
    return 0;
}

void mmio_write8(mvm *vm, uint32_t addr, uint8_t value) {
    vm->status = MVM_SEGMENTATION_FAULT;
}

void mmio_write16(mvm *vm, uint32_t addr, uint16_t value) {
    const uint32_t base_fb_addr = 0x80000000;
    if(addr >= base_fb_addr && addr < base_fb_addr + FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT * sizeof(uint16_t)) {
        frame_buffer[addr - base_fb_addr] = value;
    } else {
        vm->status = MVM_SEGMENTATION_FAULT;
    }
}

void mmio_write32(mvm *vm, uint32_t addr, uint32_t value) {
    vm->status = MVM_SEGMENTATION_FAULT;
}

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
        strncpy(load_error, "rom file is too big to fit in ram",
                sizeof(load_error));
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

    frame_buffer = (uint16_t*)calloc(1, FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT * sizeof(uint16_t));
    if(!frame_buffer) {
        free(ram);
        ram = nullptr;
        strncpy(load_error, "failed to allocate memory for the frame buffer", sizeof(load_error));
        return;
    }

    glGenTextures(1, &fb_texture);
    glBindTexture(GL_TEXTURE_2D, fb_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, nullptr);


    gui_is_init = true;
}

void gui_deinit() {
    if(ram)
        free(vm.ram);
    if(gui_is_init)
        glDeleteTextures(1, &fb_texture);
}

#define TABLE_ROW(label, format, value)                                        \
    do {                                                                       \
        ImGui::TableNextRow();                                                 \
        ImGui::TableNextColumn();                                              \
        ImGui::Text(label);                                                    \
        ImGui::TableNextColumn();                                              \
        ImGui::Text(format, value);                                            \
    } while(0)

static void vm_state() {
    ImGui::Begin("mvm");
    if(*load_error) {
        ImGui::Text(load_error);
        ImGui::End();
        return;
    }
    const ImGuiTableFlags flags =
        ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders;
    if(ImGui::BeginTable("vm state", 2, flags, ImVec2(300, 0))) {
        TABLE_ROW("pc", "0x%08x", vm.pc);
        TABLE_ROW("instruction", "%s", mvm_current_instruction_name(&vm));
        TABLE_ROW("sp", "0x%08x", vm.sp);
        TABLE_ROW("rsp", "0x%08x", vm.rsp);
        TABLE_ROW("status", "%s", mvm_status_name[vm.status]);
        ImGui::EndTable();
    }
    if(run) 
        ImGui::BeginDisabled();
    if(ImGui::Button("step"))
        mvm_run(&vm, 1);
    if(run) 
        ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::Checkbox("run", &run);
    ImGui::End();
}

static void stack_display(const char *name, const uint32_t *stk,
                          const uint32_t ptr) {
    const unsigned int n_columns = 8;
    if(ImGui::CollapsingHeader(name, ImGuiTreeNodeFlags_DefaultOpen)) {
        const ImGuiTableFlags flags =
            ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders;
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
                    // ImGui::Text("column %u", i);
                }
                ImGui::EndTable();
            }
        }
    }
}

static void ram_display() {
    const unsigned int n_columns = 16;
    const uint32_t bytes_to_display = 256;
    if(ImGui::CollapsingHeader("ram", ImGuiTreeNodeFlags_DefaultOpen)) {
        const ImGuiTableFlags flags =
            ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders;
        if(ImGui::BeginTable("ram", n_columns, flags)) {
            ImGui::TableNextRow();
            for(uint32_t i = 0; i < bytes_to_display; i++) {
                ImGui::TableNextColumn();
                if(vm.pc == i)
                    ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "0x%02x",
                                       vm.ram[i]);
                else
                    ImGui::Text("0x%02x", vm.ram[i]);
                if((i + 1) % n_columns == 0)
                    ImGui::TableNextRow();
            }
            ImGui::EndTable();
        }
    }
}

static void vm_memory() {
    if(*load_error)
        return;
    ImGui::Begin("memory");
    stack_display("stack", vm.stk, vm.sp);
    stack_display("return stack", vm.rstk, vm.rsp);
    ram_display();
    ImGui::End();
}

static void vm_screen() {
    if(*load_error)
        return;
    if(dirty) {
        glBindTexture(GL_TEXTURE_2D, fb_texture);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, frame_buffer);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    ImGui::Begin("Screen");
    ImGui::Image((ImTextureID)fb_texture, ImVec2(FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT));
    ImGui::End();
}

void gui() {
    if(!*load_error && run)
        mvm_run(&vm, 1000000);
    vm_state();
    vm_memory();
    vm_screen();

    ImGuiIO& io = ImGui::GetIO();
    ImGui::Begin("FPS");
    ImGui::Text("%.1f FPS", io.Framerate);
    ImGui::End();
}
