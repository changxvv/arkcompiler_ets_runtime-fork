/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cstdio>
#include <fstream>
#include <fcntl.h>

#include "ecmascript/dfx/hprof/heap_profiler_interface.h"
#include "ecmascript/dfx/hprof/heap_profiler.h"
#include "ecmascript/dfx/hprof/heap_snapshot_json_serializer.h"
#include "ecmascript/dfx/hprof/heap_snapshot.h"
#include "ecmascript/ecma_string.h"
#include "ecmascript/global_env.h"

#include "ecmascript/js_tagged_value.h"
#include "ecmascript/js_thread.h"
#include "ecmascript/mem/heap.h"
#include "ecmascript/tests/test_helper.h"
#include "ecmascript/dfx/hprof/file_stream.h"

using namespace panda::ecmascript;

namespace panda::ecmascript {
class TestProgress : public Progress {
public:
    TestProgress() = default;
    ~TestProgress() = default;

    void ReportProgress([[maybe_unused]] int32_t done, [[maybe_unused]] int32_t total) override {}
};

class TestStream : public Stream {
public:
    TestStream() = default;
    ~TestStream() = default;

    void EndOfStream() override {}
    int GetSize() override
    {
        static const int HEAP_PROFILER_CHUNK_SIZE = 100_KB;
        return HEAP_PROFILER_CHUNK_SIZE;
    }
    bool WriteChunk([[maybe_unused]] char *data, [[maybe_unused]] int32_t size) override
    {
        return true;
    }
    bool Good() override
    {
        return testStream_.good();
    }

    void UpdateHeapStats([[maybe_unused]] HeapStat* updateData, [[maybe_unused]] int32_t count) override
    {
    }

    void UpdateLastSeenObjectId([[maybe_unused]] int32_t lastSeenObjectId, [[maybe_unused]]int64_t timeStampUs) override
    {
    }

    void Clear()
    {
        testStream_.clear(std::ios::badbit);
    }

private:
    std::fstream testStream_;
};
}

namespace panda::test {
class HeapTrackerTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        GTEST_LOG_(INFO) << "SetUpTestCase";
    }

    static void TearDownTestCase()
    {
        GTEST_LOG_(INFO) << "TearDownCase";
    }

    void SetUp() override
    {
        TestHelper::CreateEcmaVMWithScope(instance, thread, scope);
        instance->SetEnableForceGC(false);
    }

    void TearDown() override
    {
        TestHelper::DestroyEcmaVMWithScope(instance, scope);
    }

    EcmaVM *instance {nullptr};
    EcmaHandleScope *scope {nullptr};
    JSThread *thread {nullptr};
};

HWTEST_F_L0(HeapTrackerTest, GenDumpFileName_001)
{
    [[maybe_unused]] EcmaHandleScope handleScope(thread);
    HeapProfilerInterface *heapProfile = HeapProfilerInterface::GetInstance(instance);

    sleep(1);
    int count = 100;
    while (count-- > 0) {
        instance->GetFactory()->NewJSAsyncFuncObject();
    }
    sleep(1);
    count = 100;
    while (count-- > 0) {
        instance->GetFactory()->NewJSSymbol();
    }
    sleep(1);
    count = 100;
    while (count-- > 0) {
        JSHandle<JSTaggedValue> undefined = instance->GetJSThread()->GlobalConstants()->GetHandledUndefined();
        JSHandle<EcmaString> string = instance->GetFactory()->NewFromASCII("Hello World");
        instance->GetFactory()->NewJSString(JSHandle<JSTaggedValue>(string), undefined);
    }

    TestStream stream;
    stream.Clear();
    EXPECT_TRUE(!stream.Good());
    TestProgress testProgress;
    heapProfile->DumpHeapSnapshot(DumpFormat::JSON, &stream, &testProgress, true, true, false);
    HeapProfilerInterface::Destroy(instance);
}

HWTEST_F_L0(HeapTrackerTest, GenDumpFileName_002)
{
    [[maybe_unused]] EcmaHandleScope handleScope(thread);
    HeapProfilerInterface *heapProfile = HeapProfilerInterface::GetInstance(instance);

    sleep(1);
    int count = 100;
    while (count-- > 0) {
        instance->GetFactory()->NewJSAsyncFuncObject();
    }
    sleep(1);
    count = 100;
    while (count-- > 0) {
        instance->GetFactory()->NewJSSymbol();
    }
    sleep(1);
    count = 100;
    while (count-- > 0) {
        JSHandle<JSTaggedValue> undefined = instance->GetJSThread()->GlobalConstants()->GetHandledUndefined();
        JSHandle<EcmaString> string = instance->GetFactory()->NewFromASCII("Hello World");
        instance->GetFactory()->NewJSString(JSHandle<JSTaggedValue>(string), undefined);
    }

    TestStream stream;
    stream.Clear();
    EXPECT_TRUE(!stream.Good());
    TestProgress testProgress;
    heapProfile->DumpHeapSnapshot(DumpFormat::BINARY, &stream, &testProgress, true, true, false);
    HeapProfilerInterface::Destroy(instance);
}

HWTEST_F_L0(HeapTrackerTest, GenDumpFileName_003)
{
    [[maybe_unused]] EcmaHandleScope handleScope(thread);
    HeapProfilerInterface *heapProfile = HeapProfilerInterface::GetInstance(instance);

    sleep(1);
    int count = 100;
    while (count-- > 0) {
        instance->GetFactory()->NewJSAsyncFuncObject();
    }
    sleep(1);
    count = 100;
    while (count-- > 0) {
        instance->GetFactory()->NewJSSymbol();
    }
    sleep(1);
    count = 100;
    while (count-- > 0) {
        JSHandle<JSTaggedValue> undefined = instance->GetJSThread()->GlobalConstants()->GetHandledUndefined();
        JSHandle<EcmaString> string = instance->GetFactory()->NewFromASCII("Hello World");
        instance->GetFactory()->NewJSString(JSHandle<JSTaggedValue>(string), undefined);
    }

    TestStream stream;
    stream.Clear();
    EXPECT_TRUE(!stream.Good());
    TestProgress testProgress;
    heapProfile->DumpHeapSnapshot(DumpFormat::OTHER, &stream, &testProgress, true, true, false);
    HeapProfilerInterface::Destroy(instance);
}

HWTEST_F_L0(HeapTrackerTest, GenDumpFileName_004)
{
    [[maybe_unused]] EcmaHandleScope handleScope(thread);
    HeapProfilerInterface *heapProfile = HeapProfilerInterface::GetInstance(instance);

    sleep(1);
    int count = 100;
    while (count-- > 0) {
        instance->GetFactory()->NewJSAsyncFuncObject();
    }
    sleep(1);
    count = 100;
    while (count-- > 0) {
        instance->GetFactory()->NewJSSymbol();
    }
    sleep(1);
    count = 100;
    while (count-- > 0) {
        JSHandle<JSTaggedValue> undefined = instance->GetJSThread()->GlobalConstants()->GetHandledUndefined();
        JSHandle<EcmaString> string = instance->GetFactory()->NewFromASCII("Hello World");
        instance->GetFactory()->NewJSString(JSHandle<JSTaggedValue>(string), undefined);
    }

    TestStream stream;
    stream.Clear();
    EXPECT_TRUE(!stream.Good());
    TestProgress testProgress;
    DumpFormat dumFormat = static_cast<DumpFormat>(5);
    heapProfile->DumpHeapSnapshot(dumFormat, &stream, &testProgress, true, true, false);
    HeapProfilerInterface::Destroy(instance);
}

HWTEST_F_L0(HeapTrackerTest, FileDescriptorStreamEndOfStream)
{
    int fd = 3;
    FileDescriptorStream fileStream(fd);
    EXPECT_TRUE(fileStream.Good());
    fileStream.EndOfStream();
}
} // namespace panda::test