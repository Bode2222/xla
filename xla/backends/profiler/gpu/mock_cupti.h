/* Copyright 2021 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#ifndef TENSORFLOW_COMPILER_XLA_BACKENDS_PROFILER_GPU_MOCK_CUPTI_H_
#define TENSORFLOW_COMPILER_XLA_BACKENDS_PROFILER_GPU_MOCK_CUPTI_H_

#include <stddef.h>
#include <stdint.h>

#include <cstdint>

#include "xla/backends/profiler/gpu/cupti_interface.h"
#include "third_party/tsl/platform/test.h"

namespace xla {
namespace profiler {

// A mock object automatically generated by gmock_gen.py.
class MockCupti : public xla::profiler::CuptiInterface {
 public:
  MOCK_METHOD(CUptiResult, ActivityDisable, (CUpti_ActivityKind kind),
              (override));
  MOCK_METHOD(CUptiResult, ActivityEnable, (CUpti_ActivityKind kind),
              (override));
  MOCK_METHOD(CUptiResult, ActivityFlushAll, (uint32_t flag), (override));
  MOCK_METHOD(CUptiResult, ActivityGetNextRecord,
              (uint8_t * buffer, size_t valid_buffer_size_bytes,
               CUpti_Activity** record),
              (override));
  MOCK_METHOD(CUptiResult, ActivityGetNumDroppedRecords,
              (CUcontext context, uint32_t stream_id, size_t* dropped),
              (override));
  MOCK_METHOD(CUptiResult, ActivityConfigureUnifiedMemoryCounter,
              (CUpti_ActivityUnifiedMemoryCounterConfig * config,
               uint32_t count),
              (override));
  MOCK_METHOD(CUptiResult, ActivityRegisterCallbacks,
              (CUpti_BuffersCallbackRequestFunc func_buffer_requested,
               CUpti_BuffersCallbackCompleteFunc func_buffer_completed),
              (override));
  MOCK_METHOD(CUptiResult, GetDeviceId, (CUcontext context, uint32_t* deviceId),
              (override));
  MOCK_METHOD(CUptiResult, GetTimestamp, (uint64_t * timestamp), (override));
  MOCK_METHOD(CUptiResult, Finalize, (), (override));
  MOCK_METHOD(CUptiResult, EnableCallback,
              (uint32_t enable, CUpti_SubscriberHandle subscriber,
               CUpti_CallbackDomain domain, CUpti_CallbackId cbid),
              (override));
  MOCK_METHOD(CUptiResult, EnableDomain,
              (uint32_t enable, CUpti_SubscriberHandle subscriber,
               CUpti_CallbackDomain domain),
              (override));
  MOCK_METHOD(CUptiResult, Subscribe,
              (CUpti_SubscriberHandle * subscriber, CUpti_CallbackFunc callback,
               void* userdata),
              (override));
  MOCK_METHOD(CUptiResult, Unsubscribe, (CUpti_SubscriberHandle subscriber),
              (override));
  MOCK_METHOD(CUptiResult, DeviceEnumEventDomains,
              (CUdevice device, size_t* array_size_bytes,
               CUpti_EventDomainID* domain_array),
              (override));
  MOCK_METHOD(CUptiResult, DeviceGetEventDomainAttribute,
              (CUdevice device, CUpti_EventDomainID event_domain,
               CUpti_EventDomainAttribute attrib, size_t* value_size,
               void* value),
              (override));
  MOCK_METHOD(CUptiResult, DisableKernelReplayMode, (CUcontext context),
              (override));
  MOCK_METHOD(CUptiResult, EnableKernelReplayMode, (CUcontext context),
              (override));
  MOCK_METHOD(CUptiResult, DeviceGetNumEventDomains,
              (CUdevice device, uint32_t* num_domains), (override));
  MOCK_METHOD(CUptiResult, EventDomainEnumEvents,
              (CUpti_EventDomainID event_domain, size_t* array_size_bytes,
               CUpti_EventID* event_array),
              (override));
  MOCK_METHOD(CUptiResult, EventDomainGetNumEvents,
              (CUpti_EventDomainID event_domain, uint32_t* num_events),
              (override));
  MOCK_METHOD(CUptiResult, EventGetAttribute,
              (CUpti_EventID event, CUpti_EventAttribute attrib,
               size_t* value_size, void* value),
              (override));
  MOCK_METHOD(CUptiResult, EventGetIdFromName,
              (CUdevice device, const char* event_name, CUpti_EventID* event),
              (override));
  MOCK_METHOD(CUptiResult, EventGroupDisable, (CUpti_EventGroup event_group),
              (override));
  MOCK_METHOD(CUptiResult, EventGroupEnable, (CUpti_EventGroup event_group),
              (override));
  MOCK_METHOD(CUptiResult, EventGroupGetAttribute,
              (CUpti_EventGroup event_group, CUpti_EventGroupAttribute attrib,
               size_t* value_size, void* value),
              (override));
  MOCK_METHOD(CUptiResult, EventGroupReadEvent,
              (CUpti_EventGroup event_group, CUpti_ReadEventFlags flags,
               CUpti_EventID event, size_t* event_value_buffer_size_bytes,
               uint64_t* eventValueBuffer),
              (override));
  MOCK_METHOD(CUptiResult, EventGroupSetAttribute,
              (CUpti_EventGroup event_group, CUpti_EventGroupAttribute attrib,
               size_t value_size, void* value),
              (override));
  MOCK_METHOD(CUptiResult, EventGroupSetsCreate,
              (CUcontext context, size_t event_id_array_size_bytes,
               CUpti_EventID* event_id_array,
               CUpti_EventGroupSets** event_group_passes),
              (override));
  MOCK_METHOD(CUptiResult, EventGroupSetsDestroy,
              (CUpti_EventGroupSets * event_group_sets), (override));
  MOCK_METHOD(CUptiResult, DeviceEnumMetrics,
              (CUdevice device, size_t* arraySizeBytes,
               CUpti_MetricID* metricArray),
              (override));
  MOCK_METHOD(CUptiResult, DeviceGetNumMetrics,
              (CUdevice device, uint32_t* num_metrics), (override));
  MOCK_METHOD(CUptiResult, MetricGetIdFromName,
              (CUdevice device, const char* metric_name,
               CUpti_MetricID* metric),
              (override));
  MOCK_METHOD(CUptiResult, MetricGetNumEvents,
              (CUpti_MetricID metric, uint32_t* num_events), (override));
  MOCK_METHOD(CUptiResult, MetricEnumEvents,
              (CUpti_MetricID metric, size_t* event_id_array_size_bytes,
               CUpti_EventID* event_id_array),
              (override));
  MOCK_METHOD(CUptiResult, MetricGetAttribute,
              (CUpti_MetricID metric, CUpti_MetricAttribute attrib,
               size_t* value_size, void* value),
              (override));
  MOCK_METHOD(CUptiResult, MetricGetValue,
              (CUdevice device, CUpti_MetricID metric,
               size_t event_id_array_size_bytes, CUpti_EventID* event_id_array,
               size_t event_value_array_size_bytes, uint64_t* event_value_array,
               uint64_t time_duration, CUpti_MetricValue* metric_value),
              (override));
  MOCK_METHOD(CUptiResult, GetResultString,
              (CUptiResult result, const char** str), (override));

  MOCK_METHOD(CUptiResult, GetContextId,
              (CUcontext context, uint32_t* context_id), (override));

  MOCK_METHOD(CUptiResult, GetStreamIdEx,
              (CUcontext context, CUstream stream, uint8_t per_thread_stream,
               uint32_t* stream_id),
              (override));

  MOCK_METHOD(void, CleanUp, (), (override));
  MOCK_METHOD(bool, Disabled, (), (const, override));
};

}  // namespace profiler
}  // namespace xla

#endif  // TENSORFLOW_COMPILER_XLA_BACKENDS_PROFILER_GPU_MOCK_CUPTI_H_
