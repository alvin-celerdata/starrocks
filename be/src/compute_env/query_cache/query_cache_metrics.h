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

#include <functional>
#include <utility>

#include "base/metrics.h"
#include "compute_env/query_cache/cache_manager_fwd.h"

namespace starrocks::query_cache {

class QueryCacheMetrics {
public:
    using CacheManagerProvider = std::function<CacheManagerRawPtr()>;

    QueryCacheMetrics() = default;
    QueryCacheMetrics(MetricRegistry* registry, CacheManagerProvider provider) {
        install(registry, std::move(provider));
    }
    ~QueryCacheMetrics() = default;

    static QueryCacheMetrics* instance();

    void install(MetricRegistry* registry, CacheManagerProvider provider);
    void update();

    METRIC_DEFINE_INT_GAUGE(query_cache_capacity, MetricUnit::BYTES);
    METRIC_DEFINE_INT_GAUGE(query_cache_usage, MetricUnit::BYTES);
    METRIC_DEFINE_DOUBLE_GAUGE(query_cache_usage_ratio, MetricUnit::PERCENT);
    METRIC_DEFINE_INT_GAUGE(query_cache_lookup_count, MetricUnit::NOUNIT);
    METRIC_DEFINE_INT_GAUGE(query_cache_hit_count, MetricUnit::NOUNIT);
    METRIC_DEFINE_DOUBLE_GAUGE(query_cache_hit_ratio, MetricUnit::PERCENT);

private:
    MetricRegistry* _registry = nullptr;
    CacheManagerProvider _cache_manager_provider;
};

} // namespace starrocks::query_cache
