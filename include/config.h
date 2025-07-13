#ifndef CONFIG_H
#define CONFIG_H

#include <cstdint>
#include <set>
#include <string>
#include <vector>

// U_THRESHOLD*insert_count number of inserts must be made before Updates
// may take place (applicable when an empty database is being populated)
#define U_THRESHOLD 1

// PD_THRESHOLD*insert_count number of inserts must be made before Point
// Deletes may take place (applicable when an empty database is being
// populated)
#define PD_THRESHOLD 0.1

// RD_THRESHOLD*insert_count number of inserts must be made before Range
// Deletes may take place (applicable when an empty database is being
// populated)
#define RD_THRESHOLD 0.1

// PQ_THRESHOLD*insert_count number of inserts must be made before Point
// Queries may take place (applicable when an empty database is being
// populated)
#define PQ_THRESHOLD 0.1

// RQ_THRESHOLD*insert_count number of inserts must be made before Range
// Queries may take place (applicable when an empty database is being
// populated)
#define RQ_THRESHOLD 1

#define STRING_KEY_ENABLED true
#define FILENAME "workload.txt"

struct Config {
  std::string file_path;
  std::string output_file_name;

  uint32_t entry_size = 8; // in bytes
  uint32_t key_size = 4;
  float lambda = -1.0f; // Computed as: key_size / (key_size + value_size)

  bool load_from_existing_workload = false;

  long insert_count = 0;
  long update_count = 0;
  long point_query_count = 0;
  long range_query_count = 0;
  long point_delete_count = 0;
  long range_delete_count = 0;
  long overlapping_range_query_count = 0;

  float range_query_selectivity = 0.0f;
  float range_delete_selectivity = 0.0f;
  float range_query_overlapping_percentage = 1.0f;
  bool enable_ycsb_capped_range_queries = false;

  float zero_result_point_lookup_proportion = 0.0f;
  float zero_result_point_delete_proportion = 0.0f;

  long existing_point_query_count = 0;
  long non_existing_point_query_count = 0;
  long max_unique_existing_point_query_count = 0;
  long max_unique_non_existing_point_query_count = 0;

  // insert distribution
  int insert_dist = 0;
  float insert_norm_mean_percentile = 0.0f;
  float insert_norm_stddev = 0.1f;
  float insert_beta_alpha = 1.0f;
  float insert_beta_beta = 1.0f;
  float insert_zipf_alpha = 1.0f;

  // update distribution
  int update_dist = 0;
  float update_norm_mean_percentile = 0.0f;
  float update_norm_stddev = 0.1f;
  float update_beta_alpha = 1.0f;
  float update_beta_beta = 1.0f;
  float update_zipf_alpha = 1.0f;

  // existing point lookup distribution
  int existing_point_lookup_dist = 0;
  float existing_point_lookup_norm_mean_percentile = 0.0f;
  float existing_point_lookup_norm_stddev = 0.1f;
  float existing_point_lookup_beta_alpha = 1.0f;
  float existing_point_lookup_beta_beta = 1.0f;
  float existing_point_lookup_zipf_alpha = 1.0f;

  // non-existing point lookup distribution
  int non_existing_point_lookup_dist = 0;
  float non_existing_point_lookup_norm_mean_percentile = 0.0f;
  float non_existing_point_lookup_norm_stddev = 0.1f;
  float non_existing_point_lookup_beta_alpha = 1.0f;
  float non_existing_point_lookup_beta_beta = 1.0f;
  float non_existing_point_lookup_zipf_alpha = 1.0f;

  void print() const {
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "========== Workload Configuration ==========" << std::endl;
    std::cout << "file_path = " << file_path << ",";
    std::cout << "output_file_name = " << output_file_name << ",";

    std::cout << "entry_size = " << entry_size << " bytes" << ",";
    std::cout << "key_size = " << key_size << " bytes" << ",";
    std::cout << "lambda = " << lambda << ",";

    std::cout << "load_from_existing_workload = " << std::boolalpha
              << load_from_existing_workload << ",";

    std::cout << "insert_count = " << insert_count << ",";
    std::cout << "update_count = " << update_count << ",";
    std::cout << "point_query_count = " << point_query_count << ",";
    std::cout << "range_query_count = " << range_query_count << ",";
    std::cout << "point_delete_count = " << point_delete_count << ",";
    std::cout << "range_delete_count = " << range_delete_count << ",";
    std::cout << "overlapping_range_query_count = "
              << overlapping_range_query_count << ",";

    std::cout << "range_query_selectivity = " << range_query_selectivity << ",";
    std::cout << "range_delete_selectivity = " << range_delete_selectivity
              << ",";
    std::cout << "range_query_overlapping_percentage = "
              << range_query_overlapping_percentage << ",";
    std::cout << "enable_ycsb_capped_range_queries = "
              << enable_ycsb_capped_range_queries << ",";

    std::cout << "zero_result_point_lookup_proportion = "
              << zero_result_point_lookup_proportion << ",";
    std::cout << "zero_result_point_delete_proportion = "
              << zero_result_point_delete_proportion << ",";

    std::cout << "existing_point_query_count = " << existing_point_query_count
              << ",";
    std::cout << "non_existing_point_query_count = "
              << non_existing_point_query_count << ",";
    std::cout << "max_unique_existing_point_query_count = "
              << max_unique_existing_point_query_count << ",";
    std::cout << "max_unique_non_existing_point_query_count = "
              << max_unique_non_existing_point_query_count << std::endl;

    std::cout << "\n-- Insert Distribution --" << std::endl;
    std::cout << "insert_dist = " << insert_dist << ",";
    std::cout << "insert_norm_mean_percentile = " << insert_norm_mean_percentile
              << ",";
    std::cout << "insert_norm_stddev = " << insert_norm_stddev << ",";
    std::cout << "insert_beta_alpha = " << insert_beta_alpha << ",";
    std::cout << "insert_beta_beta = " << insert_beta_beta << ",";
    std::cout << "insert_zipf_alpha = " << insert_zipf_alpha << std::endl;

    std::cout << "\n-- Update Distribution --" << std::endl;
    std::cout << "update_dist = " << update_dist << ",";
    std::cout << "update_norm_mean_percentile = " << update_norm_mean_percentile
              << ",";
    std::cout << "update_norm_stddev = " << update_norm_stddev << ",";
    std::cout << "update_beta_alpha = " << update_beta_alpha << ",";
    std::cout << "update_beta_beta = " << update_beta_beta << ",";
    std::cout << "update_zipf_alpha = " << update_zipf_alpha << std::endl;

    std::cout << "\n-- Existing Point Lookup Distribution --" << std::endl;
    std::cout << "existing_point_lookup_dist = " << existing_point_lookup_dist
              << ",";
    std::cout << "existing_point_lookup_norm_mean_percentile = "
              << existing_point_lookup_norm_mean_percentile << ",";
    std::cout << "existing_point_lookup_norm_stddev = "
              << existing_point_lookup_norm_stddev << ",";
    std::cout << "existing_point_lookup_beta_alpha = "
              << existing_point_lookup_beta_alpha << ",";
    std::cout << "existing_point_lookup_beta_beta = "
              << existing_point_lookup_beta_beta << ",";
    std::cout << "existing_point_lookup_zipf_alpha = "
              << existing_point_lookup_zipf_alpha << std::endl;

    std::cout << "\n-- Non-existing Point Lookup Distribution --" << std::endl;
    std::cout << "non_existing_point_lookup_dist = "
              << non_existing_point_lookup_dist << ",";
    std::cout << "non_existing_point_lookup_norm_mean_percentile = "
              << non_existing_point_lookup_norm_mean_percentile << ",";
    std::cout << "non_existing_point_lookup_norm_stddev = "
              << non_existing_point_lookup_norm_stddev << ",";
    std::cout << "non_existing_point_lookup_beta_alpha = "
              << non_existing_point_lookup_beta_alpha << ",";
    std::cout << "non_existing_point_lookup_beta_beta = "
              << non_existing_point_lookup_beta_beta << ",";
    std::cout << "non_existing_point_lookup_zipf_alpha = "
              << non_existing_point_lookup_zipf_alpha << std::endl;
    std::cout << "============================================" << std::endl;
  }
};

#endif // CONFIG_H