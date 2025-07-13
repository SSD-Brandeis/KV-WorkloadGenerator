#ifndef GENERATOR_H
#define GENERATOR_H

#include <cstdint>
#include <random>
#include <vector>

enum class DistributionType { Uniform = 0, Normal = 1, Beta = 2, Zipf = 3 };

class Generator {
public:
  Generator();
  Generator(DistributionType dist, uint32_t lower_bound, uint32_t upper_bound,
            double norm_mean = 0.0, double norm_stddev = 1.0,
            double beta_alpha = 2.0, double beta_beta = 5.0,
            double zipf_alpha = 1.0, int zipf_size = 1000,
            const std::vector<int> &index_mapping = {});

  uint32_t getNext();

private:
  DistributionType dist_;
  uint32_t lb_;
  uint32_t ub_;

  // Normal distribution params
  double norm_mean_;
  double norm_stddev_;

  // Beta distribution params
  double beta_alpha_;
  double beta_beta_;

  // Zipfian distribution params
  double zipf_alpha_;
  int zipf_size_;
  double zipf_searching_threshold_;
  double zipf_normalize_constant_;

  std::default_random_engine gen_;

  // Distributions
  std::uniform_int_distribution<int> uniform_int_;
  std::normal_distribution<double> normal_dist_;
  std::gamma_distribution<double> gamma_dist_x_;
  std::gamma_distribution<double> gamma_dist_y_;
  std::uniform_int_distribution<int> zipf_small_area_dist_;
  std::uniform_real_distribution<double> uniform_real_;

  std::vector<double> cumulative_probs_;
  std::vector<uint32_t> zipf_small_area_;
  std::vector<int> index_mapping_;

  int binarySearch(double p, int low, int high,
                   const std::vector<double> &cdf) const;
  void initializeZipf(const std::vector<int> &index_mapping);
};

#endif // GENERATOR_H

// #ifndef GENERATOR_H
// #define GENERATOR_H

// #include <random>
// #include <iostream>
// #include <chrono>
// #include <vector>

// class Generator
// {
// 	int dist_; // 0 -> unifrom; 1 -> norm; 2 -> beta; 3 -> Zipf
// 	uint32_t lb_;
// 	uint32_t ub_;
// 	double norm_mean_;
// 	double norm_stddev_;
// 	double beta_alpha_;
// 	double beta_beta_;
// 	double zipf_alpha_;
// 	int zipf_size_;
// 	double zipf_searching_threshold_;
// 	double zipf_normalize_constant;
// 	std::default_random_engine gen;
// 	std::uniform_int_distribution<int> distribution0;
// 	std::normal_distribution<double> distribution1;
// 	std::gamma_distribution<double> distribution2_x;
// 	std::gamma_distribution<double> distribution2_y;
// 	std::uniform_int_distribution<int> distribution3;

// 	// for zipf distribution
// 	std::uniform_real_distribution<double> uniform_standard_distribution;
// 	std::vector<double> cumulative_probabilities;
// 	std::vector<uint32_t> zipf_generator_small_area;

// public:
// 	std::vector<int> index_mapping;
// 	Generator();
// 	Generator(int dist, uint32_t lb, uint32_t ub, double norm_mean, double
// norm_stddev, double beta_alpha, double beta_beta, double zipf_alpha, int
// zipf_size, std::vector<int> _index_mapping = {}); 	uint32_t getNext();
// };
// #endif
