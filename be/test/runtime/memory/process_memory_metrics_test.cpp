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

#include "runtime/memory/process_memory_metrics.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

namespace starrocks {

namespace {

void assert_metric_registered(MetricRegistry* registry, const std::string& name) {
    ASSERT_NE(nullptr, registry->get_metric(name)) << name;
}

} // namespace

TEST(ProcessMemoryMetricsTest, InstallRegistersMemoryMetrics) {
    ProcessMemoryMetrics metrics;
    MetricRegistry registry("test_registry");
    metrics.install(&registry);

    const std::vector<std::string> metric_names = {
            "jemalloc_allocated_bytes",
            "jemalloc_active_bytes",
            "jemalloc_metadata_bytes",
            "jemalloc_metadata_thp",
            "jemalloc_resident_bytes",
            "jemalloc_mapped_bytes",
            "jemalloc_retained_bytes",
            "process_mem_bytes",
            "query_mem_bytes",
            "connector_scan_pool_mem_bytes",
            "load_mem_bytes",
            "metadata_mem_bytes",
            "tablet_metadata_mem_bytes",
            "rowset_metadata_mem_bytes",
            "segment_metadata_mem_bytes",
            "column_metadata_mem_bytes",
            "tablet_schema_mem_bytes",
            "column_zonemap_index_mem_bytes",
            "ordinal_index_mem_bytes",
            "bitmap_index_mem_bytes",
            "bloom_filter_index_mem_bytes",
            "builtin_inverted_index_mem_bytes",
            "segment_zonemap_mem_bytes",
            "short_key_index_mem_bytes",
            "compaction_mem_bytes",
            "schema_change_mem_bytes",
            "storage_page_cache_mem_bytes",
            "jit_cache_mem_bytes",
            "update_mem_bytes",
            "clone_mem_bytes",
            "consistency_mem_bytes",
            "datacache_mem_bytes",
            "vector_index_mem_bytes",
    };

    for (const auto& name : metric_names) {
        assert_metric_registered(&registry, name);
    }
}

TEST(ProcessMemoryMetricsTest, TriggerHookRunsInjectedUpdater) {
    ProcessMemoryMetrics metrics;
    MetricRegistry registry("test_registry");
    int before_update_count = 0;
    metrics.install(&registry, [&] { ++before_update_count; });

    registry.trigger_hook();

    ASSERT_EQ(1, before_update_count);
}

} // namespace starrocks
