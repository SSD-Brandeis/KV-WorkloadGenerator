#include "generator.h"
#include <algorithm>
#include <chrono>
#include <cmath>

namespace {
constexpr int kZipfianThreshold = 1000;
constexpr int kZipfSmallAreaSize = 50000;
} // namespace

Generator::Generator() : Generator(DistributionType::Uniform, 0, 1) {}

Generator::Generator(DistributionType dist, uint32_t lb, uint32_t ub,
                     double norm_mean, double norm_stddev, double beta_alpha,
                     double beta_beta, double zipf_alpha, int zipf_size,
                     const std::vector<int> &_index_mapping)
    : dist_(dist), lb_(lb), ub_(ub), norm_mean_(norm_mean),
      norm_stddev_(norm_stddev), beta_alpha_(beta_alpha), beta_beta_(beta_beta),
      zipf_alpha_(zipf_alpha), zipf_size_(zipf_size),
      gen_(std::chrono::system_clock::now().time_since_epoch().count()),
      uniform_int_(lb, ub), normal_dist_(norm_mean, norm_stddev),
      gamma_dist_x_(beta_alpha, 1.0), gamma_dist_y_(beta_beta, 1.0),
      zipf_small_area_dist_(0, kZipfSmallAreaSize - 1),
      uniform_real_(0.0, 1.0) {
  if (dist_ == DistributionType::Zipf) {
    initializeZipf(_index_mapping);
  }
}

uint32_t Generator::getNext() {
  switch (dist_) {
  case DistributionType::Uniform: {
    uint32_t value;
    do {
      value = uniform_int_(gen_);
    } while (value < lb_ || value > ub_);
    return value;
  }
  case DistributionType::Normal: {
    uint32_t value;
    do {
      value = static_cast<uint32_t>(std::round(normal_dist_(gen_)));
    } while (value < lb_ || value > ub_);
    return value;
  }
  case DistributionType::Beta: {
    double X = gamma_dist_x_(gen_);
    double Y = gamma_dist_y_(gen_);
    return lb_ + static_cast<uint32_t>(std::round((ub_ - lb_) * (X / (X + Y))));
  }
  case DistributionType::Zipf: {
    double p = uniform_real_(gen_);
    while (p == 0.0 || p == 1.0)
      p = uniform_real_(gen_);

    if (p < zipf_searching_threshold_) {
      return index_mapping_[zipf_small_area_[zipf_small_area_dist_(gen_)]];
    } else if (p == zipf_searching_threshold_) {
      return index_mapping_[kZipfianThreshold];
    }

    int low = 0, high = zipf_size_ - 1, step = 1;
    int start = 0;
    for (; start < zipf_size_; start += step) {
      if (cumulative_probs_[start] >= p) {
        low = std::max(0, start - step + 1);
        high = start;
        break;
      }
      step *= 2;
    }

    if (start > zipf_size_) {
      low = start - step;
      high = zipf_size_ - 1;
    }

    return index_mapping_[binarySearch(p, low, high, cumulative_probs_)];
  }
  default:
    throw std::runtime_error("Invalid distribution type");
  }
}

int Generator::binarySearch(double p, int low, int high,
                            const std::vector<double> &cdf) const {
  int mid;
  while (high - low > 1) {
    mid = (low + high) / 2;
    if (p < cdf[mid])
      high = mid;
    else if (p >= cdf[mid + 1])
      low = mid;
    else
      return mid;
  }
  return (low + high) / 2;
}

void Generator::initializeZipf(const std::vector<int> &mapping) {
  std::vector<double> small_cdf(kZipfianThreshold + 1, 0.0);
  cumulative_probs_ = std::vector<double>(zipf_size_ + 1, 0.0);
  zipf_small_area_.resize(kZipfSmallAreaSize);

  double norm_const = 0.0, small_norm_const = 0.0;
  for (int i = 1; i < zipf_size_; ++i) {
    norm_const += 1.0 / std::pow(i, zipf_alpha_);
    if (i == kZipfianThreshold)
      small_norm_const = norm_const;
  }
  zipf_normalize_constant_ = norm_const;

  index_mapping_.resize(zipf_size_);
  if (mapping.empty()) {
    for (int i = 0; i < zipf_size_; ++i)
      index_mapping_[i] = i;
  } else {
    index_mapping_ = mapping;
    while (index_mapping_.size() < static_cast<size_t>(zipf_size_)) {
      int pos = std::rand() % index_mapping_.size();
      index_mapping_.insert(index_mapping_.begin() + pos,
                            index_mapping_.size());
    }
  }
  std::shuffle(index_mapping_.begin(), index_mapping_.end(), gen_);

  for (int i = 1; i < zipf_size_; ++i) {
    cumulative_probs_[i] = cumulative_probs_[i - 1] +
                           1.0 / (std::pow(i, zipf_alpha_) * norm_const);
    if (i <= kZipfianThreshold) {
      small_cdf[i] = small_cdf[i - 1] +
                     1.0 / (std::pow(i, zipf_alpha_) * small_norm_const);
    }
  }
  zipf_searching_threshold_ = cumulative_probs_[kZipfianThreshold];
  cumulative_probs_[zipf_size_] = 1.0;
  small_cdf[kZipfianThreshold] = 1.0;

  for (int i = 0; i < kZipfSmallAreaSize; ++i) {
    double p = uniform_real_(gen_);
    while (p == 0.0 || p == 1.0)
      p = uniform_real_(gen_);
    zipf_small_area_[i] = binarySearch(p, 0, kZipfianThreshold - 1, small_cdf);
  }
}