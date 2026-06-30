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

#include "compute_env/query_cache/query_cache_metrics.h"

#include <utility>

#include "compute_env/query_cache/cache_manager.h"
#include "gutil/macros.h"

namespace starrocks::query_cache {

namespace {

const char* const kUpdateQueryCacheMetricsHookName = "update_query_cache_metrics";

} // namespace

QueryCacheMetrics* QueryCacheMetrics::instance() {
    // Process-lifetime singleton: registered Metric objects keep back-pointers
    // to MetricRegistry, so avoid exit-time destruction after registry teardown.
    static auto* instance = new QueryCacheMetrics();
    return instance;
}

void QueryCacheMetrics::install(MetricRegistry* registry, CacheManagerProvider provider) {
    if (_registry != nullptr) {
        DCHECK_EQ(_registry, registry);
        return;
    }
    _registry = registry;
    _cache_manager_provider = std::move(provider);

    registry->register_metric("query_cache_capacity", &query_cache_capacity);
    registry->register_metric("query_cache_usage", &query_cache_usage);
    registry->register_metric("query_cache_usage_ratio", &query_cache_usage_ratio);
    registry->register_metric("query_cache_lookup_count", &query_cache_lookup_count);
    registry->register_metric("query_cache_hit_count", &query_cache_hit_count);
    registry->register_metric("query_cache_hit_ratio", &query_cache_hit_ratio);
    registry->register_hook(kUpdateQueryCacheMetricsHookName, [this] { update(); });
}

void QueryCacheMetrics::update() {
    if (!_cache_manager_provider) {
        return;
    }
    auto* cache_mgr = _cache_manager_provider();
    if (cache_mgr == nullptr) {
        return;
    }

    auto capacity = cache_mgr->capacity();
    auto usage = cache_mgr->memory_usage();
    auto lookup_count = cache_mgr->lookup_count();
    auto hit_count = cache_mgr->hit_count();
    auto usage_ratio = (capacity == 0L) ? 0.0 : double(usage) / double(capacity);
    auto hit_ratio = (lookup_count == 0L) ? 0.0 : double(hit_count) / double(lookup_count);
    query_cache_capacity.set_value(capacity);
    query_cache_usage.set_value(usage);
    query_cache_usage_ratio.set_value(usage_ratio);
    query_cache_lookup_count.set_value(lookup_count);
    query_cache_hit_count.set_value(hit_count);
    query_cache_hit_ratio.set_value(hit_ratio);
}

} // namespace starrocks::query_cache
