#ifndef FLB_API_WRAPPER_H
#define FLB_API_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

// 構造体として定義せず、マクロで void に置き換える
// これにより、flb_ctx_t* は暗黙的に void* となり、衝突が起こらなくなる
#ifndef flb_ctx_t
#define flb_ctx_t void
#endif

// FluentBit.cppで使用している関数だけを定義
void* wrap_flb_create();
int wrap_flb_service_set(flb_ctx_t* ctx, const char* key, const char* val);
int wrap_flb_input(flb_ctx_t* ctx, const char* name, void* data);
int wrap_flb_output(flb_ctx_t* ctx, const char* name, void* data);
int wrap_flb_input_set(flb_ctx_t* ctx, int f_id, const char* key, const char* val);
int wrap_flb_output_set(flb_ctx_t* ctx, int f_id, const char* key, const char* val);
int wrap_flb_start(flb_ctx_t* ctx);
int wrap_flb_stop(flb_ctx_t* ctx);
void wrap_flb_destroy(flb_ctx_t* ctx);
int wrap_flb_lib_push(flb_ctx_t* ctx, int f_id, const void* data, size_t len);

#ifdef __cplusplus
}
#endif

#endif
