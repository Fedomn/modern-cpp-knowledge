#include <gtest/gtest.h>

#include <boost/graph/adj_list_serialize.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/reverse_graph.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/json.hpp>

#include "boost/graph/graphviz.hpp"

// https://www.boost.org/doc/libs/1_77_0/libs/graph/doc/table_of_contents.html
// https://www.boost.org/doc/libs/1_80_0/libs/graph/doc/adjacency_list.html
// http://www.uml.org.cn/c++/201303064.asp
// https://www.boost.org/doc/libs/1_83_0/libs/graph/doc/file_dependency_example.html#sec:cycles
// https://www.boost.org/doc/libs/1_77_0/libs/graph/doc/bundles.html
struct Vertex {
  uint64_t id = 0;
  uint64_t in_degree = 0;
};
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, Vertex> Graph;
typedef boost::graph_traits<Graph>::vertex_descriptor GraphVertexDescriptor;
typedef boost::graph_traits<Graph>::edge_descriptor GraphEdgeDescriptor;
typedef std::unordered_map<uint64_t, GraphVertexDescriptor> GraphIndexMap;  // Vertex.id -> GraphVertexDescriptor
typedef boost::graph_traits<Graph>::degree_size_type GraphDegreeSizeType;
class CycleDetector : public boost::dfs_visitor<> {
 public:
  CycleDetector(bool& has_cycle) : _has_cycle(has_cycle) {
  }

  template<class Edge, class Graph>
  void back_edge(Edge e, Graph& g) {
    _has_cycle = true;
    std::cout << "CycleDetector find a cycle: " << e << std::endl;
  }

 protected:
  bool& _has_cycle;
};

bool check_cycle(Graph& g) {
  bool has_cycle = false;
  CycleDetector vis(has_cycle);
  boost::depth_first_search(g, visitor(vis));
  std::cout << "The graph has a cycle? " << has_cycle << std::endl;
  return has_cycle;
}

template<class BidirectionalGraph>
std::string to_json(BidirectionalGraph& g) {
  boost::json::object json_obj;
  for (auto [i, end] = boost::vertices(g); i != end; ++i) {
    auto v = boost::get(boost::vertex_bundle, g, *i);
    json_obj[std::to_string(v.id)] = boost::json::array{};
    for (auto [ai, a_end] = boost::adjacent_vertices(*i, g); ai != a_end; ++ai) {
      auto av = boost::get(boost::vertex_bundle, g, *ai);
      json_obj[std::to_string(v.id)].as_array().push_back(av.id);
    }
  }
  std::string json_str = boost::json::serialize(json_obj);
  return json_str;
}

template<class BidirectionalGraph>
void from_json(BidirectionalGraph& g, std::string& json_str, std::map<uint64_t, Vertex>& vertex_map) {
  boost::json::object json_obj = boost::json::parse(json_str).as_object();
  std::map<uint64_t, GraphVertexDescriptor> index_map;
  for (const auto& [key, value] : json_obj) {
    uint64_t k = std::strtoull(key.data(), nullptr, 10);
    if (index_map.find(k) == index_map.end()) {
      index_map[k] = boost::add_vertex(vertex_map[k], g);
    }
    if (value.is_array()) {
      for (const auto& item : value.as_array()) {
        uint64_t k2 = item.as_int64();
        if (index_map.find(k2) == index_map.end()) {
          index_map[k2] = boost::add_vertex(vertex_map[k2], g);
        }
        boost::add_edge(index_map[k], index_map[k2], g);
      }
    }
  }
  check_cycle(g);
}

class label_writer {
 public:
  label_writer(Graph* g) : g_(g) {
  }

  void operator()(std::ostream& out, const GraphVertexDescriptor& v) const {
    auto v_bundle = boost::get(boost::vertex_bundle, *g_, v);
    out << "[shape=record label=\"{" << v_bundle.id << "|TaskId:" << v_bundle.id << "}\"]";
  }

 private:
  Graph* g_;
};

void init_degree(Graph& g) {
  for (auto [i, end] = boost::vertices(g); i != end; ++i) {
    GraphDegreeSizeType in_degree = boost::in_degree(*i, g);
    boost::get(boost::vertex_bundle, g, *i).in_degree = in_degree;
  }
}

void print_degree(Graph& g) {
  for (auto [i, end] = boost::vertices(g); i != end; ++i) {
    auto v = boost::get(boost::vertex_bundle, g, *i);
    std::cout << "id: " << v.id;
    GraphDegreeSizeType in_degree = boost::in_degree(*i, g);
    std::cout << " in_degree " << in_degree << ",";
    std::cout << " custom_degree " << v.in_degree << ",";
    GraphDegreeSizeType out_degree = boost::out_degree(*i, g);
    std::cout << " out_degree " << out_degree << std::endl;
  }
  std::cout << std::endl;
}

void print_childeren(Graph& g) {
  for (auto [i, end] = boost::vertices(g); i != end; ++i) {
    auto v = boost::get(boost::vertex_bundle, g, *i);
    std::cout << "id: " << v.id;
    auto [ai, a_end] = boost::adjacent_vertices(*i, g);
    if (ai == a_end) {
      std::cout << " has no children";
    } else {
      std::cout << " has children: ";
    }
    for (; ai != a_end; ++ai) {
      std::cout << boost::get(boost::vertex_bundle, g, *ai).id;
      if (boost::next(ai) != a_end)
        std::cout << ", ";
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

void reverse_graph(Graph& from_g, Graph& to_g, std::map<uint64_t, Vertex>& vertex_map) {
  boost::reverse_graph<Graph> g3 = boost::make_reverse_graph(from_g);
  std::string out_json = to_json(g3);
  from_json(to_g, out_json, vertex_map);
}

void visualize(Graph& g) {
  std::stringstream ss;
  boost::write_graphviz(ss, g, label_writer(&g));
  std::cout << ss.str() << std::endl;
  std::cout << std::endl;
}

void build_graph_index_map(Graph& g, std::unordered_map<uint64_t, GraphVertexDescriptor>& index_map) {
  for (auto [i, end] = boost::vertices(g); i != end; ++i) {
    auto v = boost::get(boost::vertex_bundle, g, *i);
    index_map[v.id] = *i;
  }
  for (auto& [k, v] : index_map) {
    std::cout << "index_map: " << k << ", " << v << std::endl;
  }
}

void print_predecessors(Graph& g, GraphVertexDescriptor v) {
  std::cout << boost::get(boost::vertex_bundle, g, v).id << " predecessors: ";
  for (auto [i, end] = boost::in_edges(v, g); i != end; ++i) {
    auto s = boost::source(*i, g);
    auto v = boost::get(boost::vertex_bundle, g, s);
    std::cout << v.id << " ";
  }
  std::cout << std::endl;
}

void print_successors(Graph& g, GraphVertexDescriptor v) {
  std::cout << boost::get(boost::vertex_bundle, g, v).id << " successors: ";
  for (auto [i, end] = boost::out_edges(v, g); i != end; ++i) {
    auto t = boost::target(*i, g);
    auto v = boost::get(boost::vertex_bundle, g, t);
    std::cout << v.id << " ";
  }
  std::cout << std::endl;
}

TEST(DAGTest, Test1) {
  Graph g;
  Vertex d1{ 1, 0 };
  Vertex d2{ 2, 0 };
  Vertex d3{ 3, 0 };
  Vertex d4{ 4, 0 };
  std::map<uint64_t, Vertex> vertex_map;
  vertex_map[d1.id] = d1;
  vertex_map[d2.id] = d2;
  vertex_map[d3.id] = d3;
  vertex_map[d4.id] = d4;

  GraphVertexDescriptor v1 = boost::add_vertex(d1, g);
  GraphVertexDescriptor v2 = boost::add_vertex(d2, g);
  GraphVertexDescriptor v3 = boost::add_vertex(d3, g);
  GraphVertexDescriptor v4 = boost::add_vertex(d4, g);
  auto [e1, is_add1] = boost::add_edge(v1, v2, g);
  auto [e2, is_add2] = boost::add_edge(v1, v3, g);
  auto [e3, is_add3] = boost::add_edge(v3, v4, g);
  auto [e4, is_add4] = boost::add_edge(v2, v4, g);
  std::cout << "edge1 " << e1 << ", is_add1: " << is_add1 << std::endl;
  std::cout << "edge2 " << e2 << ", is_add2: " << is_add2 << std::endl;
  std::cout << "edge3 " << e3 << ", is_add3: " << is_add3 << std::endl;
  std::cout << "edge4 " << e4 << ", is_add4: " << is_add4 << std::endl;
  std::cout << std::endl;

  // degree
  // print_degree(g);
  init_degree(g);
  print_degree(g);

  // children
  print_childeren(g);

  // print graph
  // boost::print_graph(g);
  // std::cout << std::endl;

  // check cycle
  check_cycle(g);
  boost::add_edge(v2, v1, g);
  boost::add_edge(v4, v3, g);
  check_cycle(g);
  boost::remove_edge(v2, v1, g);
  boost::remove_edge(v4, v3, g);
  check_cycle(g);
  std::cout << std::endl;

  // visualize
  visualize(g);

  // serialize/deserialize adjacency list
  std::string out_json = to_json(g);
  std::cout << "json: " << out_json << std::endl;
  Graph g2;
  from_json(g2, out_json, vertex_map);
  visualize(g2);

  // reverse graph
  Graph g3;
  reverse_graph(g2, g3, vertex_map);
  visualize(g3);

  GraphIndexMap index_map;
  build_graph_index_map(g, index_map);
  // predecessors
  print_predecessors(g, index_map[1]);
  print_predecessors(g, index_map[2]);
  print_predecessors(g, index_map[3]);
  print_predecessors(g, index_map[4]);
  // successors
  print_successors(g, index_map[1]);
  print_successors(g, index_map[2]);
  print_successors(g, index_map[3]);
  print_successors(g, index_map[4]);
}

class Scheduler {
 public:
  void init(Graph& g) {
    zero_in_degree_vertices.clear();
    non_zero_in_degree_vertices.clear();
    for (auto [i, end] = boost::vertices(g); i != end; ++i) {
      auto v = boost::get(boost::vertex_bundle, g, *i);
      if (v.in_degree == 0) {
        zero_in_degree_vertices.push_back(*i);
      } else {
        non_zero_in_degree_vertices.push_back(*i);
      }
    }
    build_graph_index_map(g, index_map);
  }

  void debug() {
    // debug zero_in_degree_vertices
    std::cout << "zero_in_degree_vertices: ";
    for (auto v : zero_in_degree_vertices) {
      std::cout << v << " ";
    }
    std::cout << std::endl;
    // debug non_zero_in_degree_vertices
    std::cout << "non_zero_in_degree_vertices: ";
    for (auto v : non_zero_in_degree_vertices) {
      std::cout << v << " ";
    }
    std::cout << std::endl;
  }

  Vertex poll(Graph& g) {
    auto v = zero_in_degree_vertices.front();
    zero_in_degree_vertices.erase(zero_in_degree_vertices.begin());
    auto v2 = boost::get(boost::vertex_bundle, g, v);
    return v2;
  }

  void poll_batch(Graph& g, std::vector<Vertex>& res) {
    res.clear();
    while (!zero_in_degree_vertices.empty()) {
      auto v = poll(g);
      res.push_back(v);
    }
    // debug();
  }

  void mark_done(Graph& g, GraphVertexDescriptor vd) {
    for (auto [i, end] = boost::out_edges(vd, g); i != end; ++i) {
      auto& target = boost::get(boost::vertex_bundle, g, boost::target(*i, g));
      target.in_degree--;
      if (target.in_degree == 0) {
        zero_in_degree_vertices.push_back(index_map[target.id]);
        non_zero_in_degree_vertices.erase(
            std::find(non_zero_in_degree_vertices.begin(), non_zero_in_degree_vertices.end(), index_map[target.id]));
      }
    }
  }

  void print_batch(Graph& g) {
    std::vector<Vertex> batch;
    uint64_t cnt = 1;
    while (true) {
      poll_batch(g, batch);
      if (batch.empty()) {
        break;
      }
      std::cout << "batch " << cnt << " got id: ";
      for (auto v : batch) {
        std::cout << v.id << " ";
        mark_done(g, index_map[v.id]);
      }
      std::cout << std::endl;
      batch.clear();
      cnt++;
    }
  }

  std::vector<GraphVertexDescriptor> zero_in_degree_vertices;
  std::vector<GraphVertexDescriptor> non_zero_in_degree_vertices;
  GraphIndexMap index_map;
};

TEST(DAGTest, Test2) {
  Graph g;
  Vertex d1{ 1, 0 };
  Vertex d2{ 2,0 };
  Vertex d3{ 3,0 };
  Vertex d4{ 4,0 };
  GraphVertexDescriptor v1 = boost::add_vertex(d1, g);
  GraphVertexDescriptor v2 = boost::add_vertex(d2, g);
  GraphVertexDescriptor v3 = boost::add_vertex(d3, g);
  GraphVertexDescriptor v4 = boost::add_vertex(d4, g);
  boost::add_edge(v1, v2, g);
  boost::add_edge(v1, v3, g);
  boost::add_edge(v3, v4, g);
  boost::add_edge(v2, v4, g);
  init_degree(g);
  visualize(g);

  Scheduler s{};
  s.init(g);

  s.print_batch(g);
}