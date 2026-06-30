// Copyright 2021-present StarRocks, Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <cstdint>
#include <functional>
#include <utility>

#include "base/metrics.h"

namespace starrocks {

class VectorIndexCache;

class VectorIndexCacheMetrics {
public:
    using VectorIndexCacheProvider = std::function<VectorIndexCache*()>;

    VectorIndexCacheMetrics() = default;
    explicit VectorIndexCacheMetrics(MetricRegistry* registry) { install(registry); }
    VectorIndexCacheMetrics(MetricRegistry* registry, VectorIndexCacheProvider provider) {
        install(registry, std::move(provider));
    }
    ~VectorIndexCacheMetrics() = default;

    static VectorIndexCacheMetrics* instance();

    void install(MetricRegistry* registry);
    void install(MetricRegistry* registry, VectorIndexCacheProvider provider);
    void update();

    METRIC_DEFINE_INT_GAUGE(vector_index_cache_capacity, MetricUnit::BYTES);
    METRIC_DEFINE_INT_GAUGE(vector_index_cache_usage, MetricUnit::BYTES);
    METRIC_DEFINE_DOUBLE_GAUGE(vector_index_cache_usage_ratio, MetricUnit::PERCENT);
    METRIC_DEFINE_INT_GAUGE(vector_index_cache_lookup_count, MetricUnit::NOUNIT);
    METRIC_DEFINE_INT_GAUGE(vector_index_cache_hit_count, MetricUnit::NOUNIT);
    METRIC_DEFINE_DOUBLE_GAUGE(vector_index_cache_hit_ratio, MetricUnit::PERCENT);
    METRIC_DEFINE_INT_GAUGE(vector_index_cache_dynamic_lookup_count, MetricUnit::NOUNIT);
    METRIC_DEFINE_INT_GAUGE(vector_index_cache_dynamic_hit_count, MetricUnit::NOUNIT);
    METRIC_DEFINE_DOUBLE_GAUGE(vector_index_cache_dynamic_hit_ratio, MetricUnit::PERCENT);

private:
    MetricRegistry* _registry = nullptr;
    VectorIndexCacheProvider _vector_index_cache_provider;
    uint64_t _previous_lookup_count = 0;
    uint64_t _previous_hit_count = 0;
};

} // namespace starrocks
