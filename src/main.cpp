#include <cstddef>
#include <dlfcn.h>
#include <libhat.hpp>
#include <link.h>
#include <span>

extern "C" [[gnu::visibility("default")]] void mod_preinit() {}

extern "C" [[gnu::visibility("default")]] void mod_init() {
    using namespace hat::literals::signature_literals;

    auto mcLib = dlopen("libminecraftpe.so", 0);

    std::span<std::byte> range1, range2;

    auto callback = [&](const dl_phdr_info& info) {
        if (auto h = dlopen(info.dlpi_name, RTLD_NOLOAD); dlclose(h), h != mcLib)
            return 0;
        range1 = {reinterpret_cast<std::byte*>(info.dlpi_addr + info.dlpi_phdr[1].p_vaddr), info.dlpi_phdr[1].p_memsz};
        range2 = {reinterpret_cast<std::byte*>(info.dlpi_addr + info.dlpi_phdr[2].p_vaddr), info.dlpi_phdr[2].p_memsz};
        return 1;
    };

    dl_iterate_phdr(
        [](dl_phdr_info* info, size_t, void* data) {
            return (*static_cast<decltype(callback)*>(data))(*info);
        },
        &callback);

    auto Options_typeinfo_name       = hat::find_pattern(range1, hat::object_to_signature("7Options")).get();
    auto Options_typeinfo            = hat::find_pattern(range2, hat::object_to_signature(Options_typeinfo_name)).get() - sizeof(void*);
    auto Options_vtable              = hat::find_pattern(range2, hat::object_to_signature(Options_typeinfo)).get() + sizeof(void*);
    auto Options_getScreenAnimations = reinterpret_cast<bool (**)(void*)>(Options_vtable) + 137;

    auto SplashTextRenderer_typeinfo_name = hat::find_pattern(range1, hat::object_to_signature("18SplashTextRenderer")).get();
    auto SplashTextRenderer_typeinfo      = hat::find_pattern(range2, hat::object_to_signature(SplashTextRenderer_typeinfo_name)).get() - sizeof(void*);
    auto SplashTextRenderer_vtable        = hat::find_pattern(range2, hat::object_to_signature(SplashTextRenderer_typeinfo)).get() + sizeof(void*);
    auto SplashTextRenderer_render        = reinterpret_cast<void (**)(void*, void*, void*, void*, int, void*)>(SplashTextRenderer_vtable) + 17;

    static auto Options_getScreenAnimations_orig = *Options_getScreenAnimations;
    static auto SplashTextRenderer_render_orig   = *SplashTextRenderer_render;

    static bool screenAnimations = false;

    *Options_getScreenAnimations = [](void* self) {
        return screenAnimations || Options_getScreenAnimations_orig(self);
    };

    *SplashTextRenderer_render = [](void* self, void* a, void* b, void* c, int d, void* e) {
        screenAnimations = true;
        SplashTextRenderer_render_orig(self, a, b, c, d, e);
        screenAnimations = false;
    };

    if (auto addr = hat::find_pattern(range1, "FF 91 48 04 00 00 84 C0 74 24"_sig).get()) {
        addr[8] = std::byte{0x90};
        addr[9] = std::byte{0x90};
    }
}
