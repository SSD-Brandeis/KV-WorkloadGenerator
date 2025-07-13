#include "utils.h"
#include "args.hxx"
#include <iostream>
#include <cmath>

inline void showProgress(uint32_t workload_size, uint32_t counter)
{
    if (workload_size == 0) return; // avoid division by zero

    constexpr int barWidth = 50;
    float progress = static_cast<float>(counter) / workload_size;
    int pos = static_cast<int>(barWidth * progress);
    int percent = static_cast<int>(progress * 100);

    std::cout << "\r[";
    for (int i = 0; i < barWidth; ++i)
    {
        if (i < pos) std::cout << '=';
        else if (i == pos) std::cout << '>';
        else std::cout << ' ';
    }
    std::cout << "] " << std::setw(3) << percent << "%";
    std::cout.flush();

    if (counter >= workload_size)
        std::cout << std::endl;
}


int parse_args(int argc, char *argv[], std::unique_ptr<Config>& config)
{
    args::ArgumentParser parser("workload_gen_parser", "");
    args::Group group(parser, "This group is all exclusive:", args::Group::Validators::DontCare);
    
    args::ValueFlag<std::string> output_filename_cmd(group, "OP", "output path [def: 0]", {"OP", "output-path"});

    args::ValueFlag<uint32_t> entry_size_cmd(group, "E", "Entry size (in bytes) [def: 8]", {'E', "entry_size"});
    args::ValueFlag<float> lambda_cmd(group, "L", "lambda = key_size / (key_size + value_size) [def: 0.5]", {'L', "lambda"});

    args::Flag load_from_existing_workload_cmd(group, "Preload", "preload from workload.txt", {"PL", "preloading"});

    args::ValueFlag<long> insert_cmd(group, "I", "Number of inserts [def: 0]", {'I', "insert"});
    args::ValueFlag<long> update_cmd(group, "U", "Number of updates [def: 0]", {'U', "update"});
    args::ValueFlag<long> point_query_cmd(group, "Q", "Number of point queries [def: 0]", {'Q', "point_query"});
    args::ValueFlag<long> range_query_cmd(group, "S", "Number of range queries [def: 0]", {'S', "range_query"});
    args::ValueFlag<long> point_delete_cmd(group, "D", "Number of point deletes [def: 0]", {'D', "point_delete"});
    args::ValueFlag<long> range_delete_cmd(group, "R", "Number of range deletes [def: 0]", {'R', "range_delete"});
    args::ValueFlag<long> overlapping_range_query_count_cmd(group, "O", "Number of overlapping range queries [def: 0]", {'O', "overlapping_range_query_count"});

    args::ValueFlag<float> range_query_selectivity_cmd(group, "Y", "Range query selectivity [def: 0]", {'Y', "range_query_selectivity"});
    args::ValueFlag<float> range_delete_selectivity_cmd(group, "y", "Range delete selectivity [def: 0]", {'y', "range_delete_selectivity"});
    args::ValueFlag<float> range_query_overlap_percent_cmd(group, "PO", "Range query overlap percent [def: 100%]", {"PO", "range_query_overlap_percent"});
    args::ValueFlag<bool> ycsb_capped_range_queries_cmd(group, "YCSB", "Range Query selectivity is random and capped by given selectivity (like YCSB)", {"YCSB", "ycsb_range_queries"});

    args::ValueFlag<float> zero_result_point_delete_proportion_cmd(group, "z", "Proportion of zero-result point deletes [def: 0]", {'z', "zero_result_point_delete_proportion"});
    args::ValueFlag<float> zero_result_point_lookup_proportion_cmd(group, "Z", "Proportion of zero-result point lookups [def: 0]", {'Z', "zero_result_point_lookup_proportion"});
    args::ValueFlag<float> unique_zero_result_point_lookup_proportion_cmd(group, "UZ", "Proportion of maximum unique zero-result point lookups [def: 0.5]", {"UZ", "unique_zero_result_point_lookup_proportion"});
    args::ValueFlag<float> maximum_unique_existing_point_lookup_proportion_cmd(group, "UE", "Proportion of maximum unique exising point lookups [def: 0.5]", {"UE", "maximum_unique_existing_point_lookup_proportion"});

    // insert distribution params
    args::ValueFlag<uint32_t> insert_dist_cmd(group, "ID", "Insert Distribution [0: uniform, 1:normal, 2:beta, 3:zipf, def: 0]", {"ID", "insert_distribution"});
    args::ValueFlag<float> insert_dist_norm_mean_percentile_cmd(group, "ID_Norm_Mean_Percentile", ", def: 0.5]", {"ID_NMP", "insert_distribution_norm_mean_percentile"});
    args::ValueFlag<float> insert_dist_norm_stddev_cmd(group, "ID_Norm_Stddev", ", def: 1]", {"ID_NDEV", "insert_distribution_norm_standard_deviation"});
    args::ValueFlag<float> insert_dist_beta_alpha_cmd(group, "ID_Beta_Alpha", ", def: 1.0]", {"ID_BALPHA", "insert_distribution_beta_alpha"});
    args::ValueFlag<float> insert_dist_beta_beta_cmd(group, "ID_Beta_Beta", ", def: 1.0]", {"ID_BBETA", "insert_distribution_beta_beta"});
    args::ValueFlag<float> insert_dist_zipf_alpha_cmd(group, "ID_Zipf_Alpha", ", def: 1.0]", {"ID_ZALPHA", "insert_distribution_zipf_alpha"});

    // update distribution params
    args::ValueFlag<uint32_t> update_dist_cmd(group, "UD", "Update Distribution [0: uniform, 1:normal, 2:beta, 3:zipf, def: 0]", {"UD", "update_distribution"});
    args::ValueFlag<float> update_dist_norm_mean_percentile_cmd(group, "UD_Norm_Mean_Percentile", ", def: 0.5]", {"UD_NMP", "update_distribution_norm_mean_percentile"});
    args::ValueFlag<float> update_dist_norm_stddev_cmd(group, "UD_Norm_Stddev", ", def: 1]", {"UD_NDEV", "update_distribution_norm_standard_deviation"});
    args::ValueFlag<float> update_dist_beta_alpha_cmd(group, "UD_Beta_Alpha", ", def: 1.0]", {"UD_BALPHA", "update_distribution_beta_alpha"});
    args::ValueFlag<float> update_dist_beta_beta_cmd(group, "UD_Beta_Beta", ", def: 1.0]", {"UD_BBETA", "update_distribution_beta_beta"});
    args::ValueFlag<float> update_dist_zipf_alpha_cmd(group, "UD_Zipf_Alpha", ", def: 1.0]", {"UD_ZALPHA", "update_distribution_zipf_alpha"});

    // existing point lookup distribution params
    args::ValueFlag<uint32_t> existing_point_lookup_dist_cmd(group, "ED", "Existing Point Lookup Distribution [0: uniform, 1:normal, 2:beta, 3:zipf, def: 0]", {"ED", "existing_point_lookup_distribution"});
    args::ValueFlag<float> existing_point_lookup_dist_norm_mean_percentile_cmd(group, "ED_Norm_Mean_Percentile", ", def: 0.5]", {"ED_NMP", "existing_point_lookup_distribution_norm_mean_percentile"});
    args::ValueFlag<float> existing_point_lookup_dist_norm_stddev_cmd(group, "ED_Norm_Stddev", ", def: 1]", {"ED_NDEV", "existing_point_lookup_distribution_norm_standard_deviation"});
    args::ValueFlag<float> existing_point_lookup_dist_beta_alpha_cmd(group, "ED_Beta_Alpha", ", def: 1.0]", {"ED_BALPHA", "existing_point_lookup_distribution_beta_alpha"});
    args::ValueFlag<float> existing_point_lookup_dist_beta_beta_cmd(group, "ED_Beta_Beta", ", def: 1.0]", {"ED_BBETA", "existing_point_lookup_distribution_beta_beta"});
    args::ValueFlag<float> existing_point_lookup_dist_zipf_alpha_cmd(group, "ED_Zipf_Alpha", ", def: 1.0]", {"ED_ZALPHA", "existing_point_lookup_distribution_zipf_alpha"});

    // non-existing point lookup distribution params
    args::ValueFlag<uint32_t> non_existing_point_lookup_dist_cmd(group, "ZD", "Zero-result Point Lookup Distribution [0: uniform, 1:normal, 2:beta, 3:zipf, def: 0]", {"ZD", "non_existing_point_lookup_distribution"});
    args::ValueFlag<float> non_existing_point_lookup_dist_norm_mean_percentile_cmd(group, "ZD_Norm_Mean_Percentile", ", def: 0.5]", {"ZD_NMP", "non_existing_point_lookup_distribution_norm_mean_percentile"});
    args::ValueFlag<float> non_existing_point_lookup_dist_norm_stddev_cmd(group, "ZD_Norm_Stddev", ", def: 1]", {"ZD_NDEV", "non_existing_point_lookup_distribution_norm_standard_deviation"});
    args::ValueFlag<float> non_existing_point_lookup_dist_beta_alpha_cmd(group, "ZD_Beta_Alpha", ", def: 1.0]", {"ZD_BALPHA", "non_existing_point_lookup_distribution_beta_alpha"});
    args::ValueFlag<float> non_existing_point_lookup_dist_beta_beta_cmd(group, "ZD_Beta_Beta", ", def: 1.0]", {"ZD_BBETA", "non_existing_point_lookup_distribution_beta_beta"});
    args::ValueFlag<float> non_existing_point_lookup_dist_zipf_alpha_cmd(group, "ZD_Zipf_Alpha", ", def: 1.0]", {"ZD_ZALPHA", "non_existing_point_lookup_distribution_zipf_alpha"});

    try
    {
        parser.ParseCLI(argc, argv);
    }
    catch (args::Help &)
    {
        std::cout << parser;
        exit(0);
    }
    catch (args::ParseError &e)
    {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    }
    catch (args::ValidationError &e)
    {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    }

    // setting config
    config->output_file_name = output_filename_cmd ? args::get(output_filename_cmd) : "";

    config->entry_size = entry_size_cmd ? args::get(entry_size_cmd) : 8;

    if (!config->entry_size)
    {
        std::cerr << "\033[1;31m ERROR:\033[0m entry_size = 0" << std::endl;
        return 1;
    }

    config->lambda = lambda_cmd ? args::get(lambda_cmd) : 0.5;
    if (config->lambda <= 0 || config->lambda > 1)
    {
        std::cerr << "\033[0;31m ERROR:\033[0m Lambda should be set between 0 and 1" << std::endl;
        return 1;
    }
    if (!STRING_KEY_ENABLED && (config->lambda > 0 && config->lambda < 1))
    {
        config->key_size = sizeof(uint32_t);
    }
    else if (config->lambda > 0 && config->lambda < 1)
    {
        config->key_size = config->lambda * config->entry_size;
    }

    config->load_from_existing_workload = load_from_existing_workload_cmd ? true : false;

    config->insert_count = insert_cmd ? args::get(insert_cmd) : 0;
    config->update_count = update_cmd ? args::get(update_cmd) : 0;
    config->point_query_count = point_query_cmd ? args::get(point_query_cmd) : 0;
    config->range_query_count = range_query_cmd ? args::get(range_query_cmd) : 0;
    config->point_delete_count = point_delete_cmd ? args::get(point_delete_cmd) : 0;
    config->range_delete_count = range_delete_cmd ? args::get(range_delete_cmd) : 0;
    config->overlapping_range_query_count = overlapping_range_query_count_cmd ? args::get(overlapping_range_query_count_cmd) : 0;

    config->range_query_selectivity = range_query_selectivity_cmd ? args::get(range_query_selectivity_cmd) : 0;
    config->range_delete_selectivity = range_delete_selectivity_cmd ? args::get(range_delete_selectivity_cmd) : 0;
    config->range_query_overlapping_percentage = range_query_overlap_percent_cmd ? args::get(range_query_overlap_percent_cmd) : 1;
    config->enable_ycsb_capped_range_queries = ycsb_capped_range_queries_cmd ? args::get(ycsb_capped_range_queries_cmd): 0;

    config->zero_result_point_delete_proportion = zero_result_point_delete_proportion_cmd ? args::get(zero_result_point_delete_proportion_cmd) : 0;
    config->zero_result_point_lookup_proportion = zero_result_point_lookup_proportion_cmd ? args::get(zero_result_point_lookup_proportion_cmd) : 0;
    if (config->point_query_count != 0 && (config->zero_result_point_lookup_proportion < 0 || config->zero_result_point_lookup_proportion > 1))
    {
        std::cerr << "\033[0;31m Error: \033[0m The proportion of zero-result point lookups should be set between 0 and 1" << std::endl;
        return 1;
    }
    config->non_existing_point_query_count = floor(config->point_query_count * config->zero_result_point_lookup_proportion);
    config->existing_point_query_count = config->point_query_count - config->non_existing_point_query_count;

    float maximum_unique_non_existing_point_query_proportion = unique_zero_result_point_lookup_proportion_cmd ? args::get(unique_zero_result_point_lookup_proportion_cmd) : 0.5;
    config->max_unique_non_existing_point_query_count = round(config->non_existing_point_query_count * maximum_unique_non_existing_point_query_proportion);
    float maximum_unique_existing_point_query_proportion = maximum_unique_existing_point_lookup_proportion_cmd ? args::get(maximum_unique_existing_point_lookup_proportion_cmd) : 0.5;
    config->max_unique_existing_point_query_count = round(config->existing_point_query_count * maximum_unique_existing_point_query_proportion);

    // distribution
    config->insert_dist = insert_dist_cmd ? args::get(insert_dist_cmd) : 0;
    config->insert_norm_mean_percentile = insert_dist_norm_mean_percentile_cmd ? args::get(insert_dist_norm_mean_percentile_cmd) : 0.5;
    config->insert_norm_stddev = insert_dist_norm_stddev_cmd ? args::get(insert_dist_norm_stddev_cmd) : 1.0;
    config->insert_beta_alpha = insert_dist_beta_alpha_cmd ? args::get(insert_dist_beta_alpha_cmd) : 1.0;
    config->insert_beta_beta = insert_dist_beta_beta_cmd ? args::get(insert_dist_beta_beta_cmd) : 1.0;
    config->insert_zipf_alpha = insert_dist_zipf_alpha_cmd ? args::get(insert_dist_zipf_alpha_cmd) : 1.0;

    config->update_dist = update_dist_cmd ? args::get(update_dist_cmd) : 0;
    config->update_norm_mean_percentile = update_dist_norm_mean_percentile_cmd ? args::get(update_dist_norm_mean_percentile_cmd) : 0.5;
    config->update_norm_stddev = update_dist_norm_stddev_cmd ? args::get(update_dist_norm_stddev_cmd) : 1.0;
    config->update_beta_alpha = update_dist_beta_alpha_cmd ? args::get(update_dist_beta_alpha_cmd) : 1.0;
    config->update_beta_beta = update_dist_beta_beta_cmd ? args::get(update_dist_beta_beta_cmd) : 1.0;
    config->update_zipf_alpha = update_dist_zipf_alpha_cmd ? args::get(update_dist_zipf_alpha_cmd) : 1.0;

    config->existing_point_lookup_dist = existing_point_lookup_dist_cmd ? args::get(existing_point_lookup_dist_cmd) : 0;
    config->existing_point_lookup_norm_mean_percentile = existing_point_lookup_dist_norm_mean_percentile_cmd ? args::get(existing_point_lookup_dist_norm_mean_percentile_cmd) : 0.5;
    config->existing_point_lookup_norm_stddev = existing_point_lookup_dist_norm_stddev_cmd ? args::get(existing_point_lookup_dist_norm_stddev_cmd) : 1.0;
    config->existing_point_lookup_beta_alpha = existing_point_lookup_dist_beta_alpha_cmd ? args::get(existing_point_lookup_dist_beta_alpha_cmd) : 1.0;
    config->existing_point_lookup_beta_beta = existing_point_lookup_dist_beta_beta_cmd ? args::get(existing_point_lookup_dist_beta_beta_cmd) : 1.0;
    config->existing_point_lookup_zipf_alpha = existing_point_lookup_dist_zipf_alpha_cmd ? args::get(existing_point_lookup_dist_zipf_alpha_cmd) : 1.0;

    config->non_existing_point_lookup_dist = non_existing_point_lookup_dist_cmd ? args::get(non_existing_point_lookup_dist_cmd) : 0;
    config->non_existing_point_lookup_norm_mean_percentile = non_existing_point_lookup_dist_norm_mean_percentile_cmd ? args::get(non_existing_point_lookup_dist_norm_mean_percentile_cmd) : 0.5;
    config->non_existing_point_lookup_norm_stddev = non_existing_point_lookup_dist_norm_stddev_cmd ? args::get(non_existing_point_lookup_dist_norm_stddev_cmd) : 1.0;
    config->non_existing_point_lookup_beta_alpha = non_existing_point_lookup_dist_beta_alpha_cmd ? args::get(non_existing_point_lookup_dist_beta_alpha_cmd) : 1.0;
    config->non_existing_point_lookup_beta_beta = non_existing_point_lookup_dist_beta_beta_cmd ? args::get(non_existing_point_lookup_dist_beta_beta_cmd) : 1.0;
    config->non_existing_point_lookup_zipf_alpha = non_existing_point_lookup_dist_zipf_alpha_cmd ? args::get(non_existing_point_lookup_dist_zipf_alpha_cmd) : 1.0;

    if (config->insert_norm_mean_percentile <= 0 || config->insert_norm_mean_percentile > 1 || config->update_norm_mean_percentile <= 0 || config->update_norm_mean_percentile > 1 ||
        config->non_existing_point_lookup_norm_mean_percentile <= 0 || config->non_existing_point_lookup_norm_mean_percentile > 1 ||
        config->existing_point_lookup_norm_mean_percentile <= 0 || config->existing_point_lookup_norm_mean_percentile > 1)
    {
        std::cerr << "\033[0;31m ERROR:\033[0m The percentile of mean in normal distribution should be set between 0 and 1" << std::endl;
        return 1;
    }

    return 0;
}