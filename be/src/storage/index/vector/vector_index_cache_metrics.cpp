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

#include "storage/index/vector/vector_index_cache_metrics.h"

#include <utility>

#include "gutil/macros.h"
#include "storage/index/vector/vector_index_cache.h"
#include "storage/storage_env.h"

namespace starrocks {

namespace {

const char* const kUpdateVectorIndexCacheMetricsHookName = "update_vector_index_cache_metrics";

VectorIndexCache* process_vector_index_cache() {
    return StorageEnv::GetInstance()->vector_index_cache();
}

} // namespace

VectorIndexCacheMetrics* VectorIndexCacheMetrics::instance() {
    // Process-lifetime singleton: registered Metric objects keep back-pointers
    // to MetricRegistry, so avoid exit-time destruction after registry teardown.
    static auto* instance = new VectorIndexCacheMetrics();
    return instance;
}

void VectorIndexCacheMetrics::install(MetricRegistry* registry) {
    install(registry, process_vector_index_cache);
}

void VectorIndexCacheMetrics::install(MetricRegistry* registry, VectorIndexCacheProvider provider) {
    if (_registry != nullptr) {
        DCHECK_EQ(_registry, registry);
        return;
    }
    _registry = registry;
    _vector_index_cache_provider = std::move(provider);

    registry->register_metric("vector_index_cache_capacity", &vector_index_cache_capacity);
    registry->register_metric("vector_index_cache_usage", &vector_index_cache_usage);
    registry->register_metric("vector_index_cache_usage_ratio", &vector_index_cache_usage_ratio);
    registry->register_metric("vector_index_cache_lookup_count", &vector_index_cache_lookup_count);
    registry->register_metric("vector_index_cache_hit_count", &vector_index_cache_hit_count);
    registry->register_metric("vector_index_cache_hit_ratio", &vector_index_cache_hit_ratio);
    registry->register_metric("vector_index_cache_dynamic_lookup_count", &vector_index_cache_dynamic_lookup_count);
    registry->register_metric("vector_index_cache_dynamic_hit_count", &vector_index_cache_dynamic_hit_count);
    registry->register_metric("vector_index_cache_dynamic_hit_ratio", &vector_index_cache_dynamic_hit_ratio);
    registry->register_hook(kUpdateVectorIndexCacheMetricsHookName, [this] { update(); });
}

void VectorIndexCacheMetrics::update() {
    if (!_vector_index_cache_provider) {
        return;
    }
    auto* index_cache = _vector_index_cache_provider();
    if (index_cache == nullptr) {
        return;
    }

    auto capacity = index_cache->capacity();
    auto usage = index_cache->memory_usage();
    auto lookup_count = index_cache->lookup_count();
    auto hit_count = index_cache->hit_count();
    auto usage_ratio = (capacity == 0L) ? 0.0 : double(usage) / double(capacity);
    auto hit_ratio = (lookup_count == 0L) ? 0.0 : double(hit_count) / double(lookup_count);
    auto dynamic_lookup_count = lookup_count - _previous_lookup_count;
    auto dynamic_hit_count = hit_count - _previous_hit_count;
    auto dynamic_hit_ratio =
            (dynamic_lookup_count == 0) ? 0.0 : double(dynamic_hit_count) / double(dynamic_lookup_count);
    vector_index_cache_capacity.set_value(capacity);
    vector_index_cache_usage.set_value(usage);
    vector_index_cache_usage_ratio.set_value(usage_ratio);
    vector_index_cache_lookup_count.set_value(lookup_count);
    vector_index_cache_hit_count.set_value(hit_count);
    vector_index_cache_hit_ratio.set_value(hit_ratio);
    vector_index_cache_dynamic_lookup_count.set_value(dynamic_lookup_count);
    vector_index_cache_dynamic_hit_count.set_value(dynamic_hit_count);
    vector_index_cache_dynamic_hit_ratio.set_value(dynamic_hit_ratio);

    _previous_lookup_count = lookup_count;
    _previous_hit_count = hit_count;
}

} // namespace starrocks
