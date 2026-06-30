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

#include "cache/cache_memory_tracker_updater.h"

#include "cache/datacache.h"
#include "cache/mem_cache/page_cache.h"
#include "common/config_cache_fwd.h"
#include "runtime/mem_tracker.h"

#ifdef USE_STAROS
#include "fslib/star_cache_handler.h"
#endif

namespace starrocks {

void CacheMemoryTrackerUpdater::update(MemTracker* datacache_mem_tracker, MemTracker* page_cache_mem_tracker) {
    if (datacache_mem_tracker != nullptr) {
        int64_t datacache_mem_bytes = 0;
        LocalMemCacheEngine* local_cache = DataCache::GetInstance()->local_mem_cache();
        if (local_cache != nullptr && local_cache->is_initialized()) {
            auto datacache_metrics = local_cache->cache_metrics();
            datacache_mem_bytes = datacache_metrics.mem_used_bytes;
        }
#ifdef USE_STAROS
        if (!config::datacache_unified_instance_enable) {
            datacache_mem_bytes += staros::starlet::fslib::star_cache_get_memory_usage();
        }
#endif
        datacache_mem_tracker->set(datacache_mem_bytes);
    }

    auto* page_cache = StoragePageCache::instance();
    if (page_cache_mem_tracker != nullptr && page_cache != nullptr && page_cache->is_initialized()) {
        page_cache_mem_tracker->set(page_cache->memory_usage());
    }
}

} // namespace starrocks
