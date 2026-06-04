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

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <unordered_map>
#include <utility>

#include "common/status.h"
#include "compute_env/workgroup/work_group_fwd.h"
#include "exec/pipeline/pipeline_fwd.h"
#include "exec/pipeline/runtime_filter_hub.h"
#include "gen_cpp/Types_types.h"
#include "runtime/runtime_state_fwd.h"

namespace starrocks::pipeline {

class FragmentDriverContext {
public:
    using CancelHandler = std::function<void(const Status&, bool)>;
    using FinalStatusProvider = std::function<Status()>;
    using NeedReportProvider = std::function<bool()>;
    using ReportHandler = std::function<void()>;
    using TimeoutHandler = std::function<void()>;

    FragmentDriverContext() = default;
    ~FragmentDriverContext();

    const TUniqueId& fragment_instance_id() const { return _fragment_instance_id; }
    void set_fragment_instance_id(const TUniqueId& fragment_instance_id) {
        _fragment_instance_id = fragment_instance_id;
    }

    RuntimeState* runtime_state() const { return _runtime_state.get(); }
    std::shared_ptr<RuntimeState> runtime_state_ptr() const { return _runtime_state; }
    void set_runtime_state(const std::shared_ptr<RuntimeState>& runtime_state) { _runtime_state = runtime_state; }

    void set_workgroup(workgroup::WorkGroupPtr wg) { _workgroup = std::move(wg); }
    const workgroup::WorkGroupPtr& workgroup() const { return _workgroup; }
    bool enable_resource_group() const { return _workgroup != nullptr; }

    void set_enable_cache(bool flag) { _enable_cache = flag; }
    bool enable_cache() const { return _enable_cache; }

    size_t next_driver_id() { return _next_driver_id++; }

    size_t expired_log_count() const { return _expired_log_count; }
    void set_expired_log_count(size_t val) { _expired_log_count = val; }

    RuntimeFilterHub* runtime_filter_hub() { return &_runtime_filter_hub; }
    void close_runtime_filters();

    Status set_pipeline_timer(PipelineTimer* pipeline_timer);
    void clear_pipeline_timer();
    PipelineTimer* pipeline_timer() { return _pipeline_timer; }

    bool enable_event_scheduler() const { return event_scheduler() != nullptr; }
    EventScheduler* event_scheduler() const { return _event_scheduler.get(); }
    void init_event_scheduler();
    void add_timer_observer(PipelineObserver* observer, uint64_t timeout);
    Status submit_all_timer();

    bool is_canceled() const;
    Status final_status() const;
    void cancel(const Status& status, bool cancelled_by_fe = false);
    bool need_report_exec_state() const;
    void report_exec_state_if_necessary() const;
    void notify_timeout() const;

    void set_cancel_handler(CancelHandler handler) { _cancel_handler = std::move(handler); }
    void set_final_status_provider(FinalStatusProvider provider) { _final_status_provider = std::move(provider); }
    void set_need_report_provider(NeedReportProvider provider) { _need_report_provider = std::move(provider); }
    void set_report_handler(ReportHandler handler) { _report_handler = std::move(handler); }
    void set_timeout_handler(TimeoutHandler handler) { _timeout_handler = std::move(handler); }

private:
    TUniqueId _fragment_instance_id;
    std::shared_ptr<RuntimeState> _runtime_state = nullptr;
    workgroup::WorkGroupPtr _workgroup = nullptr;
    bool _enable_cache = false;
    size_t _next_driver_id = 0;
    size_t _expired_log_count = 0;

    RuntimeFilterHub _runtime_filter_hub;

    std::unique_ptr<EventScheduler> _event_scheduler;
    PipelineTimer* _pipeline_timer = nullptr;
    std::shared_ptr<PipelineTimerTask> _timeout_task = nullptr;
    std::unordered_map<uint64_t, std::shared_ptr<PipelineTimerTask>> _rf_timeout_tasks;

    CancelHandler _cancel_handler;
    FinalStatusProvider _final_status_provider;
    NeedReportProvider _need_report_provider;
    ReportHandler _report_handler;
    TimeoutHandler _timeout_handler;
};

} // namespace starrocks::pipeline
