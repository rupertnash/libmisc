#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <iostream>

#include "../ndarray.hpp"

template<typename... Ints>
auto mk_idx(Ints... ints) -> std::array<int, sizeof...(Ints)> {
  return std::array<int, sizeof...(Ints)>{{ints...}};
}

TEST_CASE("Some stuff with 1D arrays works") {
  using vec = ndarray<double, 1>;

  SECTION("arrays can be default constructed") {
    vec x;
    REQUIRE(x.shape() == mk_idx(0));
    REQUIRE(x.size() == 0);
    REQUIRE(x.strides() == mk_idx(1));
  }

  SECTION("uninitialised arrays can be constructed") {
    constexpr int N = 10;
    auto x = vec{{N}};
    REQUIRE(x.size() == N);
    REQUIRE(x.shape() == mk_idx(N));
    REQUIRE(x.strides() == mk_idx(1));
  }

  SECTION("initialised arrays can be constructed") {
    constexpr int N = 3;
    constexpr double VAL = 2.0;
    auto x = vec{{N}, VAL};
    REQUIRE(x.size() == N);
    REQUIRE(x.shape() == mk_idx(N));
    REQUIRE(x.strides() == mk_idx(1));

    for (int i = 0; i < N; ++i) {
      REQUIRE(x(i) == VAL);
    }
  }

  SECTION("array can move") {
    vec x{{5}};
    // move assign
    x = vec{{10}};
    REQUIRE(x.size() == 10);

    vec y{std::move(x)};
    // y should hold old x
    // x in some minimal state
    REQUIRE(y.size() == 10);
  }
}

TEST_CASE("Some stuff with 2D arrays work") {
  using mat = ndarray<double, 2>;

  SECTION("can be default constructed") {
    mat x;
    REQUIRE(x.shape() == mk_idx(0,0));
    REQUIRE(x.size() == 0);
    REQUIRE(x.strides() == mk_idx(0, 1));
  }

  SECTION("uninitialised arrays can be constructed") {
    constexpr int M = 5;
    constexpr int N = 10;
    auto x = mat{{M, N}};
    REQUIRE(x.size() == M*N);
    REQUIRE(x.shape() == mk_idx(M, N));
    REQUIRE(x.strides() == mk_idx(N, 1));
  }

   SECTION("initialised arrays can be constructed") {
    constexpr double VAL = 2.0;
    constexpr int M = 2;
    constexpr int N = 3;
    auto x = mat{{M, N}, VAL};
    REQUIRE(x.size() == M*N);
    REQUIRE(x.shape() == mk_idx(M, N));
    REQUIRE(x.strides() == mk_idx(N, 1));
    for (int i = 0; i < M; ++i) {
      for (int j = 0; j < N; ++j) {
	REQUIRE(x(i, j) == VAL);
      }
    }
  }
}
  
TEST_CASE("Higher dimensionality arrays work") {
  SECTION("5D") {
    auto x = ndarray<double,5>{{2,3,4,5,6}};
    REQUIRE(x.strides() == mk_idx(360, 120, 30, 6, 1));
    REQUIRE(x.size() == 720);
  }
}

TEST_CASE("Flat iterators") {
  using mat = ndarray<int, 2>;

  SECTION("mutable iterators work explicitly") {
    constexpr int VAL = 2;
    constexpr int M = 2;
    constexpr int N = 3;
    auto x = mat{{M, N}};
    for (mat::iterator i = x.begin(); i != x.end(); ++i) {
      *i = VAL;
    }
    
    for (int i = 0; i < M; ++i) {
      for (int j = 0; j < N; ++j) {
	REQUIRE(x(i, j) == VAL);
      }
    }
  }

  SECTION("mutable iterators work in range-for") {
    constexpr int VAL = 2;
    constexpr int M = 2;
    constexpr int N = 3;
    auto x = mat{{M, N}};
    for (int& el: x)
      el = VAL;

    for (int i = 0; i < M; ++i) {
      for (int j = 0; j < N; ++j) {
	REQUIRE(x(i, j) == VAL);
      }
    }
  }

  SECTION("explicitly const iterators work") {
    constexpr int VAL = 2;    
    constexpr int M = 2;
    constexpr int N = 3;
    auto x = mat{{M, N}, VAL};

    for (mat::const_iterator i = x.cbegin(); i != x.cend(); ++i) {
      REQUIRE(*i == VAL);
    }
  }

  SECTION("implicitly const iterators work") {
    constexpr int VAL = 2;    
    constexpr int M = 2;
    constexpr int N = 3;
    const auto x = mat{{M, N}, VAL};

    for (mat::const_iterator i = x.begin(); i != x.end(); ++i) {
      REQUIRE(*i == VAL);
    }
  }
}

TEST_CASE("ND iterators") {
  using mat = ndarray<int, 2>;
  SECTION("ND iterators work") {
    constexpr int M = 2;
    constexpr int N = 3;
    auto x = mat{{M, N}};

    for (mat::nd_iterator it = x.nd_begin(); it != x.nd_end(); ++it) {
      auto ind = it.index();
      *it = ind[0] + ind[1];
    }

    for (int i = 0; i < M; ++i) {
      for (int j = 0; j < N; ++j) {
	REQUIRE(x(i, j) == i+j);
      }
    }
  }
}

TEST_CASE("ND enumerator") {
  using mat = ndarray<int, 2>;
  using enumerator = nd_enumerator<mat>;

  SECTION("ND enumeration works") {
    constexpr int M = 2;
    constexpr int N = 3;
    auto x = mat{{M, N}};

    enumerator e{&x};
    auto it = e.begin();
    auto end = e.end();
    for (; it != end; ++it) {
      std::pair<std::array<int,2>, int&> p = *it;
      p.second = p.first[0] + p.first[1];
    }

    for (int i = 0; i < M; ++i) {
      for (int j = 0; j < N; ++j) {
  	REQUIRE(x(i, j) == i+j);
      }
    }
  }
  SECTION("ND enumeration works in a range for") {
    constexpr int M = 2;
    constexpr int N = 3;
    auto x = mat{{M, N}};

    using ret = std::pair<mat::index_type, int&>;
    for (auto stuff: nd_enumerate(x)) {
      std::array<int,2> ind = stuff.first;
      stuff.second = ind[0] + ind[1];
    }
    for (int i = 0; i < M; ++i) {
      for (int j = 0; j < N; ++j) {
  	REQUIRE(x(i, j) == i+j);
      }
    }
  }

#ifdef __cpp_structured_bindings
  SECTION("ND enum works with structured binding range for") {
    constexpr int M = 2;
    constexpr int N = 3;
    auto x = mat{{M, N}};

    for (auto [ind, val]: nd_enumerate(x)) {
      val = ind[0] + ind[1];
    }

    for (int i = 0; i < M; ++i) {
      for (int j = 0; j < N; ++j) {
  	REQUIRE(x(i, j) == i+j);
      }
    }
  }
#endif

}
