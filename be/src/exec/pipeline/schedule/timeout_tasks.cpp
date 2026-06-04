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

#include "exec/pipeline/schedule/timeout_tasks.h"

#include <fmt/format.h>

#include "common/status.h"
#include "exec/pipeline/fragment_driver_context.h"
#include "exec/pipeline/query_context.h"
#include "exec/pipeline/schedule/common.h"
#include "runtime/logconfig.h"
#include "runtime/runtime_state.h"

namespace starrocks::pipeline {
void CheckFragmentTimeout::Run() {
    auto query_ctx = _driver_ctx->runtime_state()->query_ctx();
    size_t expire_seconds = query_ctx->get_query_expire_seconds();
    TRACE_SCHEDULE_LOG << "fragment_instance_id:" << print_id(_driver_ctx->fragment_instance_id());
    auto query_id = query_ctx->query_id();
    hook_on_query_timeout(query_id, expire_seconds);
    _driver_ctx->cancel(Status::TimedOut(fmt::format("Query reached its timeout of {} seconds", expire_seconds)));
    _driver_ctx->notify_timeout();
}

void RFScanWaitTimeout::Run() {
    if (_all_rf_timeout) {
        _timeout.notify_runtime_filter_timeout();
    } else {
        _timeout.notify_source_observers();
    }
}

} // namespace starrocks::pipeline
