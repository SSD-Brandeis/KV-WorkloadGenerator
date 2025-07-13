#include "Generator.h"
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

// #include "Generator.h"
// #include "math.h"
// #include <algorithm>
// #include "time.h"

// #define ZIPFIAN_THRESHOLD_UNIQ_ELEMENTS 1000
// #define ZIPFIAN_GENERATOR_SMALL_AREA_SIZE 50000

// std::random_device rd;
// std::mt19937 g(rd());

// inline int binary_search(double p, int low, int high, std::vector<double>
// &cdf)
// {
// 	// std::cout << "p : " << p << "\tlow : " << low << "\t high : " << high
// << std::endl; 	int mid = (low + high) / 2; 	while (high - low > 1)
// 	{
// 		// std::cout << "p: " << p << "\tmid: " <<
// cumulative_probabilities[mid] << "\tmid+1: " <<
// cumulative_probabilities[mid+1] << std::endl; 		if (p < cdf[mid])
// 		{
// 			high = mid;
// 		}
// 		else if (p >= cdf[mid + 1])
// 		{
// 			low = mid;
// 		}
// 		else
// 		{
// 			break;
// 		}
// 		mid = (low + high) / 2;
// 	}
// 	return mid;
// }

// Generator::Generator() {}
// Generator::Generator(int dist, uint32_t lb, uint32_t ub, double norm_mean,
// double norm_stddev, double beta_alpha, double beta_beta, double zipf_alpha,
// int zipf_size, std::vector<int> _index_mapping) : dist_(dist), lb_(lb),
// ub_(ub), norm_mean_(norm_mean), norm_stddev_(norm_stddev),
// beta_alpha_(beta_alpha), beta_beta_(beta_beta), zipf_alpha_(zipf_alpha),
// zipf_size_(zipf_size)
// {
// 	gen.seed(std::chrono::system_clock::now().time_since_epoch().count());
// 	distribution0 = std::uniform_int_distribution<int>(lb_, ub_);
// 	distribution1 = std::normal_distribution<double>(norm_mean_,
// norm_stddev_); 	distribution2_x =
// std::gamma_distribution<double>(beta_alpha_, 1.0); 	distribution2_y =
// std::gamma_distribution<double>(beta_beta_, 1.0); 	distribution3 =
// std::uniform_int_distribution<int>(0, ZIPFIAN_GENERATOR_SMALL_AREA_SIZE - 1);

// 	// for zipfian distribution
// 	if (dist == 3)
// 	{
// 		double zipf_small_normalize_constant = 0;
// 		zipf_generator_small_area =
// std::vector<uint32_t>(ZIPFIAN_GENERATOR_SMALL_AREA_SIZE, 0);
// 		std::vector<double> *small_cumulative_probabilities = new
// std::vector<double>(ZIPFIAN_THRESHOLD_UNIQ_ELEMENTS + 1, 0);
// 		zipf_normalize_constant = 0;
// 		for (int i = 1; i < zipf_size_; i++)
// 		{
// 			zipf_normalize_constant += 1 / (pow((double)i,
// zipf_alpha_)); 			if (i == ZIPFIAN_THRESHOLD_UNIQ_ELEMENTS)
// 			{
// 				zipf_small_normalize_constant =
// zipf_normalize_constant;
// 			}
// 		}
// 		if (_index_mapping.size() == 0)
// 		{
// 			index_mapping = std::vector<int>(zipf_size_, 0);

// 			for (int i = 1; i < zipf_size_; i++)
// 			{
// 				index_mapping[i] = i;
// 			}
// 		}
// 		else
// 		{
// 			index_mapping = _index_mapping;
// 			int size = _index_mapping.size();
// 			while (size < zipf_size_)
// 			{
// 				int pos = rand() % size;
// 				index_mapping.insert(index_mapping.begin() +
// pos, size); 				size++;
// 			}
// 		}
// 		cumulative_probabilities = std::vector<double>(zipf_size + 1,
// 0.0); 		cumulative_probabilities[0] = 0.0;
// 		small_cumulative_probabilities->at(0) = 0.0;
// 		for (int i = 1; i < zipf_size_; i++)
// 		{
// 			cumulative_probabilities[i] = cumulative_probabilities[i
// - 1] + 1 / (pow((double)i, zipf_alpha_) * zipf_normalize_constant); 			if (i <=
// ZIPFIAN_THRESHOLD_UNIQ_ELEMENTS)
// 			{
// 				small_cumulative_probabilities->at(i) =
// small_cumulative_probabilities->at(i - 1) + 1.0 / (pow((double)i,
// zipf_alpha_) * zipf_small_normalize_constant);
// 			}
// 		}
// 		zipf_searching_threshold_ =
// cumulative_probabilities[ZIPFIAN_THRESHOLD_UNIQ_ELEMENTS];
// 		cumulative_probabilities[zipf_size_] = 1.0;
// 		small_cumulative_probabilities->at(ZIPFIAN_THRESHOLD_UNIQ_ELEMENTS)
// = 1.0; 		std::shuffle(index_mapping.begin(), index_mapping.end(), g);

// 		double p;
// 		for (uint32_t k = 0; k < ZIPFIAN_GENERATOR_SMALL_AREA_SIZE; k++)
// 		{
// 			p = uniform_standard_distribution(gen);
// 			while (p == 0 || p == 1)
// 				p = uniform_standard_distribution(gen);
// 			zipf_generator_small_area[k] = binary_search(p, 0,
// ZIPFIAN_THRESHOLD_UNIQ_ELEMENTS - 1, *small_cumulative_probabilities);
// 		}
// 		std::cout << std::endl;
// 		small_cumulative_probabilities->clear();
// 		delete small_cumulative_probabilities;
// 	}
// 	else
// 	{
// 		/*
// 		cumulative_probabilities = std::vector<double> ( zipf_size+1,
// 0.0); 		index_mapping = std::vector<int> (zipf_size_, 0); 		if(zipf_size_ == 0){
// 			zipf_size_ = 1;
// 		}
// 		double unit = 1/zipf_size_;
// 		for(int i = 0; i < zipf_size; i++){
// 			cumulative_probabilities[i+1] =
// cumulative_probabilities[i] + unit; 	index_mapping[i] = i;
// 		}*/
// 		cumulative_probabilities = std::vector<double>();
// 	}
// 	uniform_standard_distribution =
// std::uniform_real_distribution<double>(0.0, 1.0);
// }

// uint32_t Generator::getNext()
// {
// 	switch (dist_)
// 	{ // 0 -> uniform; 1 -> norm; 2 -> beta; 3-> Zipf
// 	case 0:
// 	{
// 		uint32_t number = (uint32_t)distribution0(gen);
// 		while (number < lb_ || number > ub_)
// 		{
// 			number = distribution0(gen);
// 		}
// 		return number;
// 	}
// 	case 1:
// 	{
// 		uint32_t number = (uint32_t)round(distribution1(gen));
// 		while (number < lb_ || number > ub_)
// 		{
// 			number = (uint32_t)round(distribution1(gen));
// 		}
// 		return number;
// 	}
// 	case 2:
// 	{
// 		double X = distribution2_x(gen);
// 		double Y = distribution2_y(gen);
// 		return lb_ + round((ub_ - lb_) * (X / (X + Y)));
// 	}
// 	case 3:
// 	{
// 		double p = uniform_standard_distribution(gen);
// 		while (p == 0 || p == 1)
// 			p = uniform_standard_distribution(gen);

// 		if (p < zipf_searching_threshold_)
// 		{
// 			return
// index_mapping[zipf_generator_small_area[distribution3(gen)]];
// 		}
// 		else if (p == zipf_searching_threshold_)
// 		{
// 			return index_mapping[ZIPFIAN_THRESHOLD_UNIQ_ELEMENTS];
// 		}
// 		int step = 1;
// 		int start = 0;
// 		int low = 0;
// 		int high = zipf_size_ - 1;
// 		for (; start < zipf_size_; start += step)
// 		{
// 			if (cumulative_probabilities[start] >= p)
// 			{
// 				low = start > step ? (start - step + 1) : 0;
// 				high = start;
// 				break;
// 			}
// 			step *= 2;
// 		}

// 		if (start > zipf_size_)
// 		{
// 			low = start - step;
// 			high = zipf_size_ - 1;
// 		}

// 		return index_mapping[binary_search(p, low, high,
// cumulative_probabilities)];
// 	}
// 	default:
// 	{
// 		std::cout << "Unexpected case" << std::endl;
// 		return 0;
// 	}
// 	}
// }
