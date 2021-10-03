/******************************************************************************
 * Copyright (c) 2015-2016, TigerGraph Inc.
 * All rights reserved.
 * Project: TigerGraph Query Language
 * udf.hpp: a library of user defined functions used in queries.
 *
 * - This library should only define functions that will be used in
 *   TigerGraph Query scripts. Other logics, such as structs and helper
 *   functions that will not be directly called in the GQuery scripts,
 *   must be put into "ExprUtil.hpp" under the same directory where
 *   this file is located.
 *
 * - Supported type of return value and parameters
 *     - int
 *     - float
 *     - double
 *     - bool
 *     - string (don't use std::string)
 *     - accumulators
 *
 * - Function names are case sensitive, unique, and can't be conflict with
 *   built-in math functions and reserve keywords.
 *
 * - Please don't remove necessary codes in this file
 *
 * - A backup of this file can be retrieved at
 *     <tigergraph_root_path>/dev_<backup_time>/gdk/gsql/src/QueryUdf/ExprFunctions.hpp
 *   after upgrading the system.
 *
 ******************************************************************************/

#ifndef EXPRFUNCTIONS_HPP_
#define EXPRFUNCTIONS_HPP_

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <gle/engine/cpplib/headers.hpp>

/**     XXX Warning!! Put self-defined struct in ExprUtil.hpp **
 *  No user defined struct, helper functions (that will not be directly called
 *  in the GQuery scripts) etc. are allowed in this file. This file only
 *  contains user-defined expression function's signature and body.
 *  Please put user defined structs, helper functions etc. in ExprUtil.hpp
 */
#include "ExprUtil.hpp"

namespace UDIMPL {
  typedef std::string string; //XXX DON'T REMOVE

  /****** BIULT-IN FUNCTIONS **************/
  /****** XXX DON'T REMOVE ****************/
  inline int64_t str_to_int (string str) {
    return atoll(str.c_str());
  }

  inline int64_t float_to_int (float val) {
    return (int64_t) val;
  }

  inline string to_string (double val) {
    char result[200];
    sprintf(result, "%g", val);
    return string(result);
  }

  inline void tg_node2vec_sub(int dimension, string input_file, string output_file){
    Model model(dimension);
    model.sample_ = 0;
    // model.window = 10;
    int n_workers = 4;
    std::vector<SentenceP> sentences;

    size_t count =0;
    const size_t max_sentence_len = 200;

    SentenceP sentence(new Sentence);
    std::ifstream in(input_file);
    while (true) {
        std::string s;
        in >> s;
        if (s.empty()) break;
        ++count;
        sentence->tokens_.push_back(std::move(s));
        if (count == max_sentence_len) {
            count = 0;
            sentences.push_back(std::move(sentence));
            sentence.reset(new Sentence);
        }
    }

    if (!sentence->tokens_.empty())
        sentences.push_back(std::move(sentence));

    model.build_vocab(sentences);
    model.train(sentences, n_workers);
    model.save(output_file);
  }

  inline MapAccum<int, ListAccum<double>> fastRP(MapAccum<int, int> degree_diagonal, ListAccum<ListAccum<int>> edge_list, int m, int n, int k, int s, int d, double beta, string input_weights) {
  // parameters
  std::ofstream foutput("/home/tigergraph/parameters.txt");
  foutput << "|E|:" << m << std::endl;
  foutput << "|V|:" << n << std::endl;
  foutput << "K:" << k << std::endl;
  foutput << "Random Projection S:" << s << std::endl;
  foutput << "Embedding Dimension:" << d << std::endl;
  foutput << "Normalization Strength:" << beta << std::endl;
  foutput << "Weights:" << std::endl;

  // get weights
  // weights should be formatted as a single string seperated by a comma
  std::stringstream s_stream(input_weights);
  std::vector<double> weights;
  string current_weight;
  while (s_stream.good()) {
    std::getline(s_stream, current_weight, ',');
    foutput << "\t" << current_weight << std::endl;
    weights.push_back(std::stod(current_weight));
  }
  foutput.close();

  // random number generation
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<double> distribution(0.0, 1.0);

  // number of non zeros for matrix R
  size_t nnz_R = (size_t)(n * d * 1.0 / s);

  // R matrix fill version 2
  std::vector<Eigen::Triplet<double>> triplets_R;
  triplets_R.reserve(nnz_R);
  double p1 = 0.5 / s, p2 = p1, p3 = 1 - 1.0 / s;
  double v1 = sqrt(s), v2 = -v1, v3 = 0.0;
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < d; j++) {
      double random_value = distribution(gen);
      double triplet_value;
      if (random_value <= p1)
        triplet_value = v1;
      else if (random_value <= p1 + p2)
        triplet_value = v2;
      else
        triplet_value = v3;

      triplets_R.push_back(Eigen::Triplet<double>(i, j, triplet_value));
    }
  }
  Eigen::SparseMatrix<double> R(n, d);
  R.setFromTriplets(std::begin(triplets_R), std::end(triplets_R));

  // create D,L,S and similar matrices
  std::vector<Eigen::Triplet<double>> triplets_L;
  triplets_L.reserve(n);

  std::vector<Eigen::Triplet<double>> triplets_Dinv;
  triplets_Dinv.reserve(n);

  std::vector<Eigen::Triplet<double>> triplets_S;
  triplets_S.reserve(m);

  for (auto it = std::begin(degree_diagonal); it != std::end(degree_diagonal); it++) {
    int value = it->second, i = it->first;
    triplets_L.push_back(Eigen::Triplet<double>(i, i, pow((.5 / m) * value, beta)));
    triplets_Dinv.push_back(Eigen::Triplet<double>(i, i, 1.0 / value));
  }

  for (int e = 0; e < m; e++) {
    int source = edge_list.get(e).get(0);
    int target = edge_list.get(e).get(1);
    triplets_S.push_back(Eigen::Triplet<double>(source, target, 1));
  }
  Eigen::SparseMatrix<double> L(n, n);
  L.setFromTriplets(std::begin(triplets_L), std::end(triplets_L));

  Eigen::SparseMatrix<double> Dinv(n, n);
  Dinv.setFromTriplets(std::begin(triplets_Dinv), std::end(triplets_Dinv));

  Eigen::SparseMatrix<double> S(n, n);
  S.setFromTriplets(std::begin(triplets_S), std::end(triplets_S));

  Eigen::SparseMatrix<double> A = Dinv * S;

  // store embeddings
  Eigen::SparseMatrix<double> N_1 = L * R;

  // apply weights and compute N
  Eigen::SparseMatrix<double> N(n, d);
  for (int i = 0; i < k; i++){
    N_1 = (A*N_1);
    N += weights[i] * N_1;
  }
  // return n x d MapAccum
  MapAccum<int, ListAccum<double>> res;
  for (auto it = std::begin(degree_diagonal); it != std::end(degree_diagonal); it++) {
    int i = it->first;
    for (int j = 0; j < d; j++) {
      MapAccum<int, ListAccum<double>> temp(i, ListAccum<double>(N.coeff(i, j)));
      res += temp;
    }
  }
  return res;
  }

  // random function, generate a random value between 0 and 1
  inline float random(){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0, 1);
    return dis(gen);
  }

  // generate a int random value given a range
  inline int random_range(int start, int end){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(start, end);
    return dis(gen);
  }

  // generate a random value based on probability distribution
  // For example: given {0.5,0.3,0.2}, this function will generate {0,1,2} based on its probability
  inline int random_distribution(ListAccum<float> p){
    std::vector<float> a;
    for (auto it : p.data_){
        a.push_back(it);
    }
    std::random_device rd;
    std::mt19937 gen(rd());
    std::discrete_distribution<> dis(a.begin(), a.end());
    return dis(gen);
  }
}
/****************************************/

#endif /* EXPRFUNCTIONS_HPP_ */
