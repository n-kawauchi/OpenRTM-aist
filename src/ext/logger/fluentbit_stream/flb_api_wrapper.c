#include <fluent-bit.h>
#include "flb_api_wrapper.h"

// 実際のFluent Bit APIを呼び出すだけの仲介
void* wrap_flb_create() { return flb_create(); }

int wrap_flb_service_set(void* ctx, const char* key, const char* val) { return flb_service_set(ctx, key, val, NULL); }
int wrap_flb_input(void* ctx, const char* name, void* data) { return flb_input(ctx, name, data); }
int wrap_flb_output(void* ctx, const char* name, void* data) { return flb_output(ctx, name, data); }
int wrap_flb_input_set(void* ctx, int f_id, const char* key, const char* val) {
    return flb_input_set(ctx, f_id, key, val, NULL);
}
int wrap_flb_output_set(void* ctx, int f_id, const char* key, const char* val) {
    return flb_output_set(ctx, f_id, key, val, NULL);
}
int wrap_flb_start(void* ctx) { return flb_start((flb_ctx_t *)ctx); }
int wrap_flb_stop(void* ctx) { return flb_stop((flb_ctx_t *)ctx); }
void wrap_flb_destroy(void* ctx) { return flb_destroy((flb_ctx_t *)ctx); }
int wrap_flb_lib_push(void* ctx, int f_id, const void* data, size_t len) { return flb_lib_push(ctx, f_id, data, len); }
