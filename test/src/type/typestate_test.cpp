// NOLINTBEGIN
#include <gtest/gtest.h>

#include "protenc.h"

using HTTPConnection = std::tuple<std::vector<std::string>, std::string>;

enum class HTTPBuilderState { START, HEADERS, BODY };

template<HTTPBuilderState>
class HTTPConnectionBuilderWrapper;

class HTTPConnectionBuilder {
 public:
  void add_header(std::string header) {
    headers_.emplace_back(std::move(header));
  }

  void add_body(std::string body) {
    body_ = std::move(body);
  }

  size_t num_headers() const {
    return headers_.size();
  }

  HTTPConnection build() && {
    return HTTPConnection(std::move(headers_), std::move(body_));
  }

  HTTPConnectionBuilder() = default;

 private:
  template<HTTPBuilderState>
  friend class ::HTTPConnectionBuilderWrapper;

  std::vector<std::string> headers_;
  std::string body_;
};

using prot_enc::FinalTransition;
using prot_enc::FinalTransitions;
using prot_enc::InitialStates;
using prot_enc::Transition;
using prot_enc::Transitions;
using prot_enc::ValidQueries;
using prot_enc::ValidQuery;

using MyInitialStates = InitialStates<HTTPBuilderState::START>;

using MyTransitions = Transitions<
    Transition<HTTPBuilderState::START, HTTPBuilderState::HEADERS, &HTTPConnectionBuilder::add_header>,
    Transition<HTTPBuilderState::HEADERS, HTTPBuilderState::HEADERS, &HTTPConnectionBuilder::add_header>,
    Transition<HTTPBuilderState::HEADERS, HTTPBuilderState::BODY, &HTTPConnectionBuilder::add_body>>;

using MyFinalTransitions = FinalTransitions<FinalTransition<HTTPBuilderState::BODY, &HTTPConnectionBuilder::build>>;

using MyValidQueries = ValidQueries<ValidQuery<HTTPBuilderState::BODY, &HTTPConnectionBuilder::num_headers>>;

PROTENC_START_WRAPPER(
    HTTPConnectionBuilderWrapper,
    HTTPConnectionBuilder,
    HTTPBuilderState,
    MyInitialStates,
    MyTransitions,
    MyFinalTransitions,
    MyValidQueries);
PROTENC_DECLARE_TRANSITION(add_header);
PROTENC_DECLARE_TRANSITION(add_body);
PROTENC_DECLARE_FINAL_TRANSITION(build);
PROTENC_DECLARE_QUERY_METHOD(num_headers);
PROTENC_END_WRAPPER;

static HTTPConnectionBuilderWrapper<HTTPBuilderState::START> GetConnectionBuilder() {
  return {};
}

TEST(TypeState, HelloWorld) {
  auto builder = GetConnectionBuilder();
  auto builder2 = std::move(builder).add_header("h1").add_header("h2");
  auto res = std::move(builder2).add_body("b1").build();
  std::cout << "body is: " << std::get<1>(res) << '\n';

  // This doesn't compile:
  // GetConnectionBuilder().add_body("Body");
  // GetConnectionBuilder().add_header("First header").add_body("Body").add_header("Second header");
  // GetConnectionBuilder().add_header("Header").build();
}
// NOLINTEND