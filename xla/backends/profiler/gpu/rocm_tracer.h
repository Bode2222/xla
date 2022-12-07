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

#ifndef TENSORFLOW_COMPILER_XLA_BACKENDS_PROFILER_GPU_ROCM_TRACER_H_
#define TENSORFLOW_COMPILER_XLA_BACKENDS_PROFILER_GPU_ROCM_TRACER_H_

#include "absl/container/fixed_array.h"
#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/container/node_hash_set.h"
#include "absl/types/optional.h"
#include "xla/stream_executor/rocm/roctracer_wrapper.h"
#include "third_party/tsl/platform/errors.h"
#include "third_party/tsl/platform/macros.h"
#include "third_party/tsl/platform/status.h"
#include "third_party/tsl/platform/types.h"

namespace xla {
namespace profiler {

struct MemcpyDetails {
  // The amount of data copied for memcpy events.
  size_t num_bytes;
  // The destination device for peer-2-peer communication (memcpy). The source
  // device is implicit: it's the current device.
  uint32_t destination;
  // Whether or not the memcpy is asynchronous.
  bool async;
};

struct MemsetDetails {
  // The number of memory elements getting set
  size_t num_bytes;
  // Whether or not the memset is asynchronous.
  bool async;
};

struct MemAllocDetails {
  // The amount of data requested for cudaMalloc events.
  uint64_t num_bytes;
};

struct KernelDetails {
  // The number of registers used in this kernel.
  uint32_t registers_per_thread;
  // The amount of shared memory space used by a thread block.
  uint32_t static_shared_memory_usage;
  // The amount of dynamic memory space used by a thread block.
  uint32_t dynamic_shared_memory_usage;
  // X-dimension of a thread block.
  uint32_t block_x;
  // Y-dimension of a thread block.
  uint32_t block_y;
  // Z-dimension of a thread block.
  uint32_t block_z;
  // X-dimension of a grid.
  uint32_t grid_x;
  // Y-dimension of a grid.
  uint32_t grid_y;
  // Z-dimension of a grid.
  uint32_t grid_z;

  // kernel address. Used for calculating core occupancy
  void* func_ptr;
};

// RocmTracerSyncTypes forward decleration
enum class RocmTracerSyncTypes;
struct SynchronizationDetails {
  RocmTracerSyncTypes sync_type;
};

enum class RocmTracerEventType {
  Unsupported = 0,
  Kernel,
  MemcpyH2D,
  MemcpyD2H,
  MemcpyD2D,
  MemcpyP2P,
  MemcpyOther,
  MemoryAlloc,
  MemoryFree,
  Memset,
  Synchronization,
  Generic,
};

const char* GetRocmTracerEventTypeName(const RocmTracerEventType& type);

enum class RocmTracerEventSource {
  Invalid = 0,
  ApiCallback,
  Activity,
};

const char* GetRocmTracerEventSourceName(const RocmTracerEventSource& source);

enum class RocmTracerEventDomain {
  InvalidDomain = 0,
  HIP_API,
  HCC_OPS,  // TODO(rocm-profiler): renme this to HIP_OPS
};
enum class RocmTracerSyncTypes {
  InvalidSync = 0,
  StreamSynchronize,  // caller thread wait stream to become empty
  EventSynchronize,   // caller thread will block until event happens
  StreamWait          // compute stream will wait for event to happen
};

const char* GetRocmTracerEventDomainName(const RocmTracerEventDomain& domain);

struct RocmTracerEvent {
  static constexpr uint32_t kInvalidDeviceId =
      std::numeric_limits<uint32_t>::max();
  static constexpr uint32_t kInvalidThreadId =
      std::numeric_limits<uint32_t>::max();
  static constexpr uint32_t kInvalidCorrelationId =
      std::numeric_limits<uint32_t>::max();
  static constexpr uint64_t kInvalidStreamId =
      std::numeric_limits<uint64_t>::max();
  RocmTracerEventType type;
  RocmTracerEventSource source = RocmTracerEventSource::Invalid;
  RocmTracerEventDomain domain;
  std::string name;
  // This points to strings in AnnotationMap, which should outlive the point
  // where serialization happens.
  absl::string_view annotation;
  absl::string_view roctx_range;
  uint64_t start_time_ns = 0;
  uint64_t end_time_ns = 0;
  uint32_t device_id = kInvalidDeviceId;
  uint32_t correlation_id = kInvalidCorrelationId;
  uint32_t thread_id = kInvalidThreadId;
  int64_t stream_id = kInvalidStreamId;
  union {
    MemcpyDetails memcpy_info;                    // If type == Memcpy*
    MemsetDetails memset_info;                    // If type == Memset*
    MemAllocDetails memalloc_info;                // If type == MemoryAlloc
    KernelDetails kernel_info;                    // If type == Kernel
    SynchronizationDetails synchronization_info;  // If type == Synchronization
  };
};

void DumpRocmTracerEvent(const RocmTracerEvent& event,
                         uint64_t start_walltime_ns, uint64_t start_gputime_ns,
                         const std::string& message);

struct RocmTracerOptions {
  std::set<uint32_t> api_tracking_set;  // actual api set we want to profile

  // map of domain --> ops for which we need to enable the API callbacks
  // If the ops vector is empty, then enable API callbacks for entire domain
  absl::flat_hash_map<activity_domain_t, std::vector<uint32_t> > api_callbacks;

  // map of domain --> ops for which we need to enable the Activity records
  // If the ops vector is empty, then enable Activity records for entire domain
  absl::flat_hash_map<activity_domain_t, std::vector<uint32_t> >
      activity_tracing;
};

struct RocmTraceCollectorOptions {
  // Maximum number of events to collect from callback API; if -1, no limit.
  // if 0, the callback API is enabled to build a correlation map, but no
  // events are collected.
  uint64_t max_callback_api_events;
  // Maximum number of events to collect from activity API; if -1, no limit.
  uint64_t max_activity_api_events;
  // Maximum number of annotation strings that we can accommodate.
  uint64_t max_annotation_strings;
  // Number of GPUs involved.
  uint32_t num_gpus;
};

class AnnotationMap {
 public:
  explicit AnnotationMap(uint64_t max_size) : max_size_(max_size) {}
  void Add(uint32_t correlation_id, const std::string& annotation);
  absl::string_view LookUp(uint32_t correlation_id);

 private:
  struct AnnotationMapImpl {
    // The population/consumption of annotations might happen from multiple
    // callback/activity api related threads.
    absl::Mutex mutex;
    // Annotation tends to be repetitive, use a hash_set to store the strings,
    // an use the reference to the string in the map.
    absl::node_hash_set<std::string> annotations;
    absl::flat_hash_map<uint32_t, absl::string_view> correlation_map;
  };
  const uint64_t max_size_;
  AnnotationMapImpl map_;

 public:
  // Disable copy and move.
  AnnotationMap(const AnnotationMap&) = delete;
  AnnotationMap& operator=(const AnnotationMap&) = delete;
};

class RocmTraceCollector {
 public:
  explicit RocmTraceCollector(const RocmTraceCollectorOptions& options)
      : options_(options), annotation_map_(options.max_annotation_strings) {}
  virtual ~RocmTraceCollector() {}

  virtual void AddEvent(RocmTracerEvent&& event, bool is_auxiliary) = 0;
  virtual void OnEventsDropped(const std::string& reason,
                               uint32_t num_events) = 0;
  virtual void Flush() = 0;

  AnnotationMap* annotation_map() { return &annotation_map_; }

 protected:
  RocmTraceCollectorOptions options_;

 private:
  AnnotationMap annotation_map_;

 public:
  // Disable copy and move.
  RocmTraceCollector(const RocmTraceCollector&) = delete;
  RocmTraceCollector& operator=(const RocmTraceCollector&) = delete;
};

class RocmTracer;

class RocmApiCallbackImpl {
 public:
  RocmApiCallbackImpl(const RocmTracerOptions& options, RocmTracer* tracer,
                      RocmTraceCollector* collector)
      : options_(options), tracer_(tracer), collector_(collector) {}

  tsl::Status operator()(uint32_t domain, uint32_t cbid, const void* cbdata);

 private:
  void AddKernelEventUponApiExit(uint32_t cbid, const hip_api_data_t* data,
                                 uint64_t enter_time, uint64_t exit_time);
  void AddNormalMemcpyEventUponApiExit(uint32_t cbid,
                                       const hip_api_data_t* data,
                                       uint64_t enter_time, uint64_t exit_time);
  void AddMemcpyPeerEventUponApiExit(uint32_t cbid, const hip_api_data_t* data,
                                     uint64_t enter_time, uint64_t exit_time);
  void AddMemsetEventUponApiExit(uint32_t cbid, const hip_api_data_t* data,
                                 uint64_t enter_time, uint64_t exit_time);
  void AddMallocFreeEventUponApiExit(uint32_t cbid, const hip_api_data_t* data,
                                     uint32_t device_id, uint64_t enter_time,
                                     uint64_t exit_time);
  void AddStreamSynchronizeEventUponApiExit(uint32_t cbid,
                                            const hip_api_data_t* data,
                                            uint64_t enter_time,
                                            uint64_t exit_time);
  void AddSynchronizeEventUponApiExit(uint32_t cbid, const hip_api_data_t* data,
                                      uint64_t enter_time, uint64_t exit_time);

  RocmTracerOptions options_;
  RocmTracer* tracer_ = nullptr;
  RocmTraceCollector* collector_ = nullptr;
  tsl::mutex api_call_start_mutex_;
  // TODO(rocm-profiler): replace this with absl hashmap
  // keep a map from the corr. id to enter time for API callbacks.
  std::map<uint32_t, uint64_t> api_call_start_time_
      TF_GUARDED_BY(api_call_start_mutex_);
};

class RocmActivityCallbackImpl {
 public:
  RocmActivityCallbackImpl(const RocmTracerOptions& options, RocmTracer* tracer,
                           RocmTraceCollector* collector)
      : options_(options), tracer_(tracer), collector_(collector) {}

  tsl::Status operator()(const char* begin, const char* end);

 private:
  void AddHipKernelActivityEvent(const roctracer_record_t* record);
  void AddNormalHipMemcpyActivityEvent(const roctracer_record_t* record);
  void AddHipMemsetActivityEvent(const roctracer_record_t* record);
  void AddHipMallocActivityEvent(const roctracer_record_t* record);
  void AddHipStreamSynchronizeActivityEvent(const roctracer_record_t* record);
  void AddHccKernelActivityEvent(const roctracer_record_t* record);
  void AddNormalHipOpsMemcpyActivityEvent(const roctracer_record_t* record);
  void AddHipOpsMemsetActivityEvent(const roctracer_record_t* record);
  RocmTracerOptions options_;
  RocmTracer* tracer_ = nullptr;
  RocmTraceCollector* collector_ = nullptr;
};

// The class use to enable cupti callback/activity API and forward the collected
// trace events to RocmTraceCollector. There should be only one RocmTracer
// per process.
class RocmTracer {
 public:
  // Returns a pointer to singleton RocmTracer.
  static RocmTracer* GetRocmTracerSingleton();

  // Only one profile session can be live in the same time.
  bool IsAvailable() const;

  void Enable(const RocmTracerOptions& options, RocmTraceCollector* collector);
  void Disable();

  tsl::Status ApiCallbackHandler(uint32_t domain, uint32_t cbid,
                                 const void* cbdata);
  tsl::Status ActivityCallbackHandler(const char* begin, const char* end);

  static uint64_t GetTimestamp();
  static int NumGpus();

  void AddToPendingActivityRecords(uint32_t correlation_id) {
    pending_activity_records_.Add(correlation_id);
  }

  void RemoveFromPendingActivityRecords(uint32_t correlation_id) {
    pending_activity_records_.Remove(correlation_id);
  }

  void ClearPendingActivityRecordsCount() { pending_activity_records_.Clear(); }

  size_t GetPendingActivityRecordsCount() {
    return pending_activity_records_.Count();
  }

 protected:
  // protected constructor for injecting mock cupti interface for testing.
  explicit RocmTracer() : num_gpus_(NumGpus()) {}

 private:
  tsl::Status EnableApiTracing();
  tsl::Status DisableApiTracing();

  tsl::Status EnableActivityTracing();
  tsl::Status DisableActivityTracing();

  int num_gpus_;
  std::optional<RocmTracerOptions> options_;
  RocmTraceCollector* collector_ = nullptr;

  bool api_tracing_enabled_ = false;
  bool activity_tracing_enabled_ = false;

  RocmApiCallbackImpl* api_cb_impl_;
  RocmActivityCallbackImpl* activity_cb_impl_;

  class PendingActivityRecords {
   public:
    // add a correlation id to the pending set
    void Add(uint32_t correlation_id) {
      absl::MutexLock lock(&mutex);
      pending_set.insert(correlation_id);
    }
    // remove a correlation id from the pending set
    void Remove(uint32_t correlation_id) {
      absl::MutexLock lock(&mutex);
      pending_set.erase(correlation_id);
    }
    // clear the pending set
    void Clear() {
      absl::MutexLock lock(&mutex);
      pending_set.clear();
    }
    // count the number of correlation ids in the pending set
    size_t Count() {
      absl::MutexLock lock(&mutex);
      return pending_set.size();
    }

   private:
    // set of co-relation ids for which the hcc activity record is pending
    absl::flat_hash_set<uint32_t> pending_set;
    // the callback which processes the activity records (and consequently
    // removes items from the pending set) is called in a separate thread
    // from the one that adds item to the list.
    absl::Mutex mutex;
  };
  PendingActivityRecords pending_activity_records_;

 public:
  // Disable copy and move.
  RocmTracer(const RocmTracer&) = delete;
  RocmTracer& operator=(const RocmTracer&) = delete;
};

}  // namespace profiler
}  // namespace xla
#endif  // TENSORFLOW_COMPILER_XLA_BACKENDS_PROFILER_GPU_ROCM_TRACER_H_
