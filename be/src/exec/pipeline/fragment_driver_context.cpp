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

#include "exec/pipeline/fragment_driver_context.h"

#include "base/time/time.h"
#include "common/logging.h"
#include "compute_env/pipeline/pipeline_timer.h"
#include "exec/pipeline/query_context.h"
#include "exec/pipeline/schedule/event_scheduler.h"
#include "exec/pipeline/schedule/timeout_tasks.h"
#include "gutil/casts.h"
#include "runtime/runtime_state.h"

namespace starrocks::pipeline {

FragmentDriverContext::~FragmentDriverContext() {
    clear_pipeline_timer();
}

void FragmentDriverContext::close_runtime_filters() {
    _runtime_filter_hub.close_all_in_filters(_runtime_state.get());
}

Status FragmentDriverContext::set_pipeline_timer(PipelineTimer* timer) {
    _pipeline_timer = timer;
    _timeout_task = std::make_shared<CheckFragmentTimeout>(this);
    timespec tm = butil::seconds_from_now(runtime_state()->query_ctx()->get_query_expire_seconds());
    RETURN_IF_ERROR(_pipeline_timer->schedule(_timeout_task.get(), tm));
    return Status::OK();
}

void FragmentDriverContext::clear_pipeline_timer() {
    if (_pipeline_timer) {
        if (!_rf_timeout_tasks.empty()) {
            for (auto& [ignore, task] : _rf_timeout_tasks) {
                if (task) {
                    task->unschedule_and_join(_pipeline_timer);
                    task.reset();
                }
            }
            _rf_timeout_tasks.clear();
        }
        if (_timeout_task) {
            _timeout_task->unschedule_and_join(_pipeline_timer);
            _timeout_task.reset();
        }
    }
}

void FragmentDriverContext::init_event_scheduler() {
    _event_scheduler = std::make_unique<EventScheduler>();
    runtime_state()->runtime_profile()->add_info_string("EnableEventScheduler",
                                                        enable_event_scheduler() ? "true" : "false");
}

void FragmentDriverContext::add_timer_observer(PipelineObserver* observer, uint64_t timeout) {
    RFScanWaitTimeout* task;
    if (auto iter = _rf_timeout_tasks.find(timeout); iter != _rf_timeout_tasks.end()) {
        task = down_cast<RFScanWaitTimeout*>(iter->second.get());
    } else {
        auto timeout_task = std::make_shared<RFScanWaitTimeout>();
        task = timeout_task.get();
        _rf_timeout_tasks.emplace(timeout, timeout_task);
    }
    task->add_observer(_runtime_state.get(), observer);
}

Status FragmentDriverContext::submit_all_timer() {
    timespec tm = butil::microseconds_to_timespec(butil::gettimeofday_us());
    for (const auto& [delta_ns, task] : _rf_timeout_tasks) {
        timespec abstime = tm;
        abstime.tv_nsec += delta_ns;
        butil::timespec_normalize(&abstime);
        RETURN_IF_ERROR(_pipeline_timer->schedule(task.get(), abstime));
    }
    return Status::OK();
}

bool FragmentDriverContext::is_canceled() const {
    return _runtime_state != nullptr && _runtime_state->is_cancelled();
}

Status FragmentDriverContext::final_status() const {
    return _final_status_provider ? _final_status_provider() : Status::OK();
}

void FragmentDriverContext::cancel(const Status& status, bool cancelled_by_fe) {
    if (_cancel_handler) {
        _cancel_handler(status, cancelled_by_fe);
    }
}

bool FragmentDriverContext::need_report_exec_state() const {
    return _need_report_provider != nullptr && _need_report_provider();
}

void FragmentDriverContext::report_exec_state_if_necessary() const {
    if (_report_handler) {
        _report_handler();
    }
}

void FragmentDriverContext::notify_timeout() const {
    if (_timeout_handler) {
        _timeout_handler();
    }
}

} // namespace starrocks::pipeline
