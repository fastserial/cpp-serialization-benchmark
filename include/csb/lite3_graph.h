#pragma once

#include "csb/generate_graph.h"

#include "graph_constants.h"


namespace csb {

struct lite3_graph {
  struct edge {
    uint16_t from_;
    uint16_t to_;
    uint16_t weight_;
  };

  struct node {
    uint16_t id_;
    std::string name_;
    std::vector<edge> out_;
    std::vector<edge> in_;
  };

  using node_t = node const*;
  using edge_t = edge;
  using string_t = std::string;

  void make_node(std::string name) {
    nodes_.emplace_back(node{next_node_id_++, std::move(name)});
  }

  void make_edge(uint16_t const from_id, uint16_t const to_id,
                 uint16_t const weight) {
    edge e{from_id, to_id, weight};
    nodes_[from_id].out_.push_back(e);
    nodes_[to_id].in_.push_back(e);
  }

  template <typename Fn>
  static void for_each_out_edge(node_t n, Fn&& f) {
    for (auto const& e : n->out_) f(e);
  }

  template <typename Fn>
  static void for_each_in_edge(node_t n, Fn&& f) {
    for (auto const& e : n->in_) f(e);
  }

  node_t get_target(edge_t e) const { return &nodes_[e.to_]; }
  node_t get_node(unsigned node_id) const { return &nodes_[node_id]; }

  std::vector<node> nodes_;
  uint16_t next_node_id_{0};
};

static lite3_graph LITE3_GRAPH =
    generate_graph<lite3_graph>(GRAPH_SIZE, CONNECTEDNESS);

} // namespace csb 