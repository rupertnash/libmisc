// -*- mode: c++; -*-
// 
// Copyright (c) 2019, Rupert Nash
// 
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.

// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
#ifndef LIBMISC_REJECTION_SAMPLE_H
#define LIBMISC_REJECTION_SAMPLE_H

#include <random>

// Perform rejection sampling to get values distributed according
// to a target distribution
// 
// ProposalSampler must be invokable with a Generator arg to return a
// floating type drawn from some known distribution.
// 
// ProposalDist must be invokable with a floating arg, to return the
// probability density of that value from the distribution.
// 
// TargetDist must be invokable with a floating arg, to return the
// probability density of that value from the distribution.

// Ideally this would model RandomNumberDistribution
// https://en.cppreference.com/w/cpp/named_req/RandomNumberDistribution
// But that's a lot of complexity and when you want to have a general
// solution like this with a valid default constructor!
template< typename ProposalSampler, typename ProposalDist, typename TargetDist, typename Floating = double >
class RejectionSampler {
public:
  using result_type = Floating;

  // The final factor arg is a scaling factor for the proposal distribution such that
  // target(x) < factor * proposal(x) for all x in the support of target
  RejectionSampler(ProposalSampler sampler, ProposalDist proposal, TargetDist target, result_type factor = 1.0)
    : m_unit_rand(),
      m_factor(factor),
      m_proposed_sampler(sampler),
      m_proposed(proposal),
      m_target(target)
  {
  }

  template< class Generator >
  result_type operator()(Generator& g) {
    while (true) {
      auto x = sampler(g);
      auto acceptance_prob = m_target(x) / (m_factor * m_proposed(x));
      if (m_unit_rand(g) < acceptance_prob)
	return x;
    }
  }
  
private:
  using uniform_dist = std::uniform_real_distribution<result_type>;
  
  uniform_dist m_unit_rand;
  results_type m_factor;
  ProposalSampler m_proposed_sampler;
  ProposalDist m_proposed;
  TargetDist m_target;
};

#endif // include guard
