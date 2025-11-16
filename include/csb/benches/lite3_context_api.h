#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include <errno.h>

#include "csb/traverse_graph.h"
#include "csb/lite3_graph.h"

#include "../../../external_lib/lite3/include/lite3_context_api.h"


namespace csb {

struct lite3_context_api_bench {
  lite3_ctx *ctx;
  lite3_graph deserialized_;

  lite3_context_api_bench() : ctx(NULL) {
    ctx = lite3_ctx_create_with_size(48*1024*1024);
    if (!ctx)
      throw std::bad_alloc();
  }
  ~lite3_context_api_bench() {
    lite3_ctx_destroy(ctx);
    ctx = NULL;
  }

  void serialize() {
    if (lite3_ctx_init_arr(ctx) < 0)
      throw std::runtime_error(std::string("Could not initialize Lite3 array: ") + std::strerror(errno));

    for (auto const& n : LITE3_GRAPH.nodes_) {
      size_t arr_ofs;
      if (lite3_ctx_arr_append_arr(ctx, 0, &arr_ofs) < 0)
        goto error;
      
      if (lite3_ctx_arr_append_i64(ctx, arr_ofs, (int64_t)n.id_) < 0)
        goto error;
      if (lite3_ctx_arr_append_str_n(ctx, arr_ofs, n.name_.data(), n.name_.size()) < 0)
        goto error;

      size_t out_size = n.out_.size() * sizeof(lite3_graph::edge);
      if (lite3_ctx_arr_append_bytes(ctx, arr_ofs, (const unsigned char *)n.out_.data(), out_size) < 0)
        goto error;
      
      size_t in_size = n.in_.size() * sizeof(lite3_graph::edge);
      if (lite3_ctx_arr_append_bytes(ctx, arr_ofs, (const unsigned char *)n.in_.data(), in_size) < 0)
        goto error;
    }
    return;
error:
    throw std::runtime_error(std::string("Lite3 error: ") + std::strerror(errno));
  }

  void deserialize() {
    deserialized_.nodes_.clear();
    deserialized_.next_node_id_ = 0;

    lite3_iter iter;
    if (lite3_ctx_iter_create(ctx, 0, &iter) < 0)
      goto error;

    size_t arr_ofs;
    while (lite3_ctx_iter_next(ctx, &iter, NULL, &arr_ofs) == LITE3_ITER_ITEM) {
      deserialized_.nodes_.emplace_back();
      auto& n = deserialized_.nodes_.back();

      int64_t id;
      if (lite3_ctx_arr_get_i64(ctx, arr_ofs, 0, &id) < 0)
        goto error;
      n.id_ = (uint16_t)id;

      lite3_str str;
      if (lite3_ctx_arr_get_str(ctx, arr_ofs, 1, &str) < 0)
        goto error;
      n.name_ = std::string(str.ptr, str.len);

      lite3_bytes out_bytes;
      if (lite3_ctx_arr_get_bytes(ctx, arr_ofs, 2, &out_bytes) < 0)
        goto error;
      n.out_.resize(out_bytes.len / sizeof(lite3_graph::edge));
      memcpy(n.out_.data(), out_bytes.ptr, out_bytes.len);
      
      lite3_bytes in_bytes;
      if (lite3_ctx_arr_get_bytes(ctx, arr_ofs, 3, &in_bytes) < 0)
        goto error;
      n.in_.resize(in_bytes.len / sizeof(lite3_graph::edge));
      memcpy(n.in_.data(), in_bytes.ptr, in_bytes.len);
      
      if (n.id_ + 1 > deserialized_.next_node_id_)
        deserialized_.next_node_id_ = n.id_ + 1;
    }
    return;
error:
    throw std::runtime_error(std::string("Lite3 error: ") + std::strerror(errno));
  }

  void deserialize_fast() {
    deserialize();
  }

  unsigned traverse() { return traverse_forward(deserialized_); }

  size_t serialized_size() const { return ctx->buflen; }

  void backup() {}
  void restore() {}
};

} // namespace csb