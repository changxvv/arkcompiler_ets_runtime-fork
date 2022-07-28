/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "ecmascript/containers/containers_private.h"
#include "ecmascript/ecma_vm.h"
#include "ecmascript/ecma_runtime_call_info.h"
#include "ecmascript/js_tagged_value.h"
#include "ecmascript/js_api/js_api_arraylist.h"
#include "ecmascript/global_env.h"
#include "ecmascript/object_factory.h"
#include "ecmascript/tests/test_helper.h"

using namespace panda;
using namespace panda::ecmascript;

namespace panda::test {
class JSAPIArrayListTest : public testing::Test {
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
    }

    void TearDown() override
    {
        TestHelper::DestroyEcmaVMWithScope(instance, scope);
    }

    EcmaVM *instance {nullptr};
    EcmaHandleScope *scope {nullptr};
    JSThread *thread {nullptr};

    class TestClass : public base::BuiltinsBase {
    public:
        static JSTaggedValue TestForEachAndReplaceAllFunc(EcmaRuntimeCallInfo *argv)
        {
            JSHandle<JSTaggedValue> value = GetCallArg(argv, 0);
            JSHandle<JSTaggedValue> key = GetCallArg(argv, 1);
            JSHandle<JSTaggedValue> arrayList = GetCallArg(argv, 2); // 2 means the secode arg
            if (!arrayList->IsUndefined()) {
                if (value->IsNumber()) {
                    TaggedArray *elements = TaggedArray::Cast(JSAPIArrayList::Cast(arrayList.GetTaggedValue().
                                            GetTaggedObject())->GetElements().GetTaggedObject());
                    JSTaggedValue result = elements->Get(key->GetInt());
                    EXPECT_EQ(result, value.GetTaggedValue());
                }
            }
            return JSTaggedValue::True();
        }
    };
protected:
    JSAPIArrayList *CreateArrayList()
    {
        ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
        JSHandle<GlobalEnv> env = thread->GetEcmaVM()->GetGlobalEnv();

        JSHandle<JSTaggedValue> globalObject = env->GetJSGlobalObject();
        JSHandle<JSTaggedValue> key(factory->NewFromASCII("ArkPrivate"));
        JSHandle<JSTaggedValue> value =
            JSObject::GetProperty(thread, JSHandle<JSTaggedValue>(globalObject), key).GetValue();

        auto objCallInfo =
            TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 6); // 6 means the value
        objCallInfo->SetFunction(JSTaggedValue::Undefined());
        objCallInfo->SetThis(value.GetTaggedValue());
        objCallInfo->SetCallArg(0, JSTaggedValue(static_cast<int>(containers::ContainerTag::ArrayList)));

        [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, objCallInfo);
        JSTaggedValue result = containers::ContainersPrivate::Load(objCallInfo);
        TestHelper::TearDownFrame(thread, prev);

        JSHandle<JSTaggedValue> constructor(thread, result);
        JSHandle<JSAPIArrayList> arrayList(
            factory->NewJSObjectByConstructor(JSHandle<JSFunction>(constructor), constructor));
        JSHandle<TaggedArray> taggedArray = factory->NewTaggedArray(JSAPIArrayList::DEFAULT_CAPACITY_LENGTH);
        arrayList->SetElements(thread, taggedArray);
        return *arrayList;
    }
};

HWTEST_F_L0(JSAPIArrayListTest, CreateArrayList)
{
    JSAPIArrayList *arrayList = CreateArrayList();
    EXPECT_TRUE(arrayList != nullptr);
}

/**
 * @tc.name: Add
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F_L0(JSAPIArrayListTest, Add)
{
    uint32_t increasedLength = 5;
    JSHandle<JSAPIArrayList> arrayList(thread, CreateArrayList());
    for (uint32_t i = 0; i < increasedLength; i++) {
        JSHandle<JSTaggedValue> value(thread, JSTaggedValue(i * 10));
        JSAPIArrayList::Add(thread, arrayList, value);
    }
    JSHandle<TaggedArray> elements(thread, arrayList->GetElements());
    for (uint32_t i = 0; i < increasedLength; i++) {
        EXPECT_EQ(elements->Get(i), JSTaggedValue(i * 10));
    }
}

/**
 * @tc.name: Insert
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F_L0(JSAPIArrayListTest, Insert)
{
    uint32_t basicLength = 5;
    JSHandle<JSAPIArrayList> arrayList(thread, CreateArrayList());
    for (uint32_t i = 0; i < basicLength; i++) {
        JSHandle<JSTaggedValue> value(thread, JSTaggedValue(i * 10));
        JSAPIArrayList::Add(thread, arrayList, value);
    }
    uint32_t insertStartFrom = 2;
    uint32_t insertNums = 3;
    for (uint32_t i = 0; i < insertNums; i++) {
        JSHandle<JSTaggedValue> insertValue(thread, JSTaggedValue(99 + i));
        JSAPIArrayList::Insert(thread, arrayList, insertValue, insertStartFrom + i);
    }
    JSHandle<TaggedArray> elements(thread, arrayList->GetElements());
    for (uint32_t i = 0; i < basicLength + insertNums; i++) {
        if (i < insertStartFrom) {
            EXPECT_EQ(elements->Get(i), JSTaggedValue(i * 10));
        } else if (i >= insertStartFrom && i < insertStartFrom + insertNums) {
            EXPECT_EQ(elements->Get(i), JSTaggedValue(99 + i - insertStartFrom));
        } else if (i >= insertStartFrom + insertNums) {
            EXPECT_EQ(elements->Get(i), JSTaggedValue((i - insertNums) * 10));
        }
    }
}

/**
 * @tc.name: Clear & IsEmpty
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F_L0(JSAPIArrayListTest, Clear)
{
    JSHandle<JSAPIArrayList> arrayList(thread, CreateArrayList());
    EXPECT_TRUE(JSAPIArrayList::IsEmpty(arrayList));
    EXPECT_EQ(arrayList->GetLength(), JSTaggedValue(0));

    JSHandle<JSTaggedValue> value(thread, JSTaggedValue(99));
    JSAPIArrayList::Add(thread, arrayList, value);
    EXPECT_FALSE(JSAPIArrayList::IsEmpty(arrayList));
    EXPECT_EQ(arrayList->GetLength(), JSTaggedValue(1));

    JSAPIArrayList::Clear(thread, arrayList);
    EXPECT_TRUE(JSAPIArrayList::IsEmpty(arrayList));
    EXPECT_EQ(arrayList->GetLength(), JSTaggedValue(0));
}

/**
 * @tc.name: Clone
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F_L0(JSAPIArrayListTest, Clone)
{
    JSHandle<JSAPIArrayList> arrayList(thread, CreateArrayList());
    uint32_t length = 10;
    for (uint32_t i = 0; i < length; i++) {
        JSHandle<JSTaggedValue> value(thread, JSTaggedValue(i));
        JSAPIArrayList::Add(thread, arrayList, value);
    }
    JSHandle<JSAPIArrayList> newArrayList = JSAPIArrayList::Clone(thread, arrayList);
    JSHandle<TaggedArray> elements(thread, arrayList->GetElements());
    JSHandle<TaggedArray> newElements(thread, newArrayList->GetElements());
    for (uint32_t i = 0; i < length; i++) {
        EXPECT_EQ(elements->Get(i), JSTaggedValue(i));
        EXPECT_EQ(newElements->Get(i), JSTaggedValue(i));
    }
}

/**
 * @tc.name: GetCapacity & IncreaseCapacityTo
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F_L0(JSAPIArrayListTest, GetCapacity_IncreaseCapacityTo)
{
    JSHandle<JSAPIArrayList> arrayList(thread, CreateArrayList());
    uint32_t oldCapacity = JSAPIArrayList::GetCapacity(thread, arrayList);
    EXPECT_EQ(oldCapacity, JSAPIArrayList::DEFAULT_CAPACITY_LENGTH);

    uint32_t addElementNums = 256;
    uint32_t growCapacityTimes = 0;
    uint32_t currentCapacity = JSAPIArrayList::DEFAULT_CAPACITY_LENGTH;
    for (uint32_t i = 0; i < addElementNums; i++) {
        JSAPIArrayList::Add(thread, arrayList, JSHandle<JSTaggedValue>(thread, JSTaggedValue::Undefined()));

        // After capacity expansion, the capacity will be about 1.5 times that of the original.
        currentCapacity = JSAPIArrayList::DEFAULT_CAPACITY_LENGTH;
        for (uint32_t j = 0; j < growCapacityTimes; j++) {
            currentCapacity = static_cast<uint32_t>(currentCapacity * 1.5);
        }
        EXPECT_EQ(JSAPIArrayList::GetCapacity(thread, arrayList), currentCapacity);

        // When an element is added to the end of the current list, dynamic capacity expansion will be triggered.
        if (i == (currentCapacity - 2U)) {
            growCapacityTimes++;
        }
    }
    
    // Expand capacity to a specified capacity value
    uint32_t newCapacity = JSAPIArrayList::GetCapacity(thread, arrayList);
    EXPECT_EQ(newCapacity, currentCapacity);

    JSAPIArrayList::IncreaseCapacityTo(thread, arrayList, currentCapacity + 123U);
    newCapacity = JSAPIArrayList::GetCapacity(thread, arrayList);
    EXPECT_EQ(newCapacity, currentCapacity + 123U);
}

/**
 * @tc.name: TrimToCurrentLength
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F_L0(JSAPIArrayListTest, TrimToCurrentLength)
{
    JSHandle<JSAPIArrayList> arrayList(thread, CreateArrayList());

    uint32_t addElementNums = 256;
    uint32_t growCapacityTimes = 0;
    uint32_t currentCapacity = JSAPIArrayList::DEFAULT_CAPACITY_LENGTH;
    for (uint32_t i = 0; i < addElementNums; i++) {
        JSAPIArrayList::Add(thread, arrayList, JSHandle<JSTaggedValue>(thread, JSTaggedValue::Undefined()));
        currentCapacity = JSAPIArrayList::DEFAULT_CAPACITY_LENGTH;
        for (uint32_t j = 0; j < growCapacityTimes; j++) {
            currentCapacity = static_cast<uint32_t>(currentCapacity * 1.5);
        }
        EXPECT_EQ(JSAPIArrayList::GetCapacity(thread, arrayList), currentCapacity);
        if (i == (currentCapacity - 2U)) {
            growCapacityTimes++;
        }
    }
    EXPECT_EQ(JSAPIArrayList::GetCapacity(thread, arrayList), currentCapacity);

    // Cut the excess length to the actual number of elements
    JSAPIArrayList::TrimToCurrentLength(thread, arrayList);
    EXPECT_EQ(JSAPIArrayList::GetCapacity(thread, arrayList), addElementNums);
}

/**
 * @tc.name: GetIndexOf & GetLastIndexOf
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F_L0(JSAPIArrayListTest, GetIndexOf_GetLastIndexOf)
{
    JSHandle<JSAPIArrayList> arrayList(thread, CreateArrayList());
    uint32_t addElementNums = 100;
    for (uint32_t i = 0; i < addElementNums; i++) {
        JSHandle<JSTaggedValue> value(thread, JSTaggedValue(i));
        JSAPIArrayList::Add(thread, arrayList, value);
    }
    for (uint32_t i = 0; i < JSAPIArrayList::GetCapacity(thread, arrayList); i++) {
        if (i < addElementNums) {
            int index =
                JSAPIArrayList::GetIndexOf(thread, arrayList, JSHandle<JSTaggedValue>(thread, JSTaggedValue(i)));
            EXPECT_EQ(index, static_cast<int>(i));
        } else {
            int index =
                JSAPIArrayList::GetIndexOf(thread, arrayList, JSHandle<JSTaggedValue>(thread, JSTaggedValue(i)));
            EXPECT_EQ(index, -1);
        }
    }

    JSHandle<JSTaggedValue> value(thread, JSTaggedValue(99));
    JSAPIArrayList::Add(thread, arrayList, value);
    int firstIndex =
        JSAPIArrayList::GetIndexOf(thread, arrayList, JSHandle<JSTaggedValue>(thread, JSTaggedValue(99)));
    EXPECT_EQ(firstIndex, 99);

    int lastIndex =
        JSAPIArrayList::GetLastIndexOf(thread, arrayList, JSHandle<JSTaggedValue>(thread, JSTaggedValue(99)));
    EXPECT_EQ(lastIndex, 99 + 1);
}

/**
 * @tc.name: RemoveByIndex & Remove & RemoveByRange
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F_L0(JSAPIArrayListTest, RemoveByIndex_Remove_RemoveByRange)
{
    JSHandle<JSAPIArrayList> arrayList(thread, CreateArrayList());
    uint32_t addElementNums = 256;
    uint32_t removeElementNums = 56;
    for (uint32_t i = 0; i < addElementNums; i++) {
        JSHandle<JSTaggedValue> value(thread, JSTaggedValue(i));
        JSAPIArrayList::Add(thread, arrayList, value);
    }

    // RemoveByIndex
    {
        for (uint32_t i = 0; i < removeElementNums; i++) {
            // Delete elements with indexes between [0, 55].
            JSAPIArrayList::RemoveByIndex(thread, arrayList, 0);
        }
        JSHandle<TaggedArray> elements(thread, arrayList->GetElements());
        for (uint32_t i = 0; i < addElementNums - removeElementNums; i++) {
            // The value of the corresponding index [0, 199] is [56, 255].
            EXPECT_EQ(elements->Get(i), JSTaggedValue(i + removeElementNums));
        }
    }

    // Remove
    {
        for (uint32_t i = removeElementNums; i < 100; i++) {
            JSHandle<JSTaggedValue> value(thread, JSTaggedValue(i));

            // Delete 44 elements whose element values are in [56, 99].
            JSAPIArrayList::Remove(thread, arrayList, value);
        }
        JSHandle<TaggedArray> elements(thread, arrayList->GetElements());
        for (uint32_t i = 0; i < addElementNums - 100 - 1; i++) {

            // The value of the corresponding index [0, 155] is [100, 255].
            EXPECT_EQ(elements->Get(i), JSTaggedValue(i + 100));
        }
    }

    // RemoveByRange
    {
        uint32_t formIndex = 50;
        uint32_t toIndex = 100;
        JSHandle<JSTaggedValue> fromIndexValue(thread, JSTaggedValue(formIndex));
        JSHandle<JSTaggedValue> toIndexValue(thread, JSTaggedValue(toIndex));

        // Remove the value between 50 and 100 of the index element.
        JSAPIArrayList::RemoveByRange(thread, arrayList, fromIndexValue, toIndexValue);
        uint32_t length = arrayList->GetLength().GetArrayLength();
        JSHandle<TaggedArray> elements(thread, arrayList->GetElements());
        for (uint32_t i = 0; i < length - (toIndex - formIndex); i++) {

            // The value of the corresponding index [0, 105] is [100, 150] ∪ [201, 255].
            if (i >= 0 && i < 50) {
                EXPECT_EQ(elements->Get(i), JSTaggedValue(i + 100));
            } else if (i > 50) {
                EXPECT_EQ(elements->Get(i), JSTaggedValue(i + 150));
            }
        }
    }
}

/**
 * @tc.name: ReplaceAllElements
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F_L0(JSAPIArrayListTest, ReplaceAllElements)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<JSAPIArrayList> arrayList(thread, CreateArrayList());
    JSHandle<GlobalEnv> env = thread->GetEcmaVM()->GetGlobalEnv();
    JSHandle<JSFunction> func =
        factory->NewJSFunction(env, reinterpret_cast<void *>(TestClass::TestForEachAndReplaceAllFunc));
    auto callInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 6);
    callInfo->SetFunction(JSTaggedValue::Undefined());
    callInfo->SetThis(arrayList.GetTaggedValue());
    callInfo->SetCallArg(0, func.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, callInfo);
    JSHandle<JSTaggedValue> result(thread, JSAPIArrayList::ReplaceAllElements(thread,
        callInfo->GetThis(), callInfo->GetFunction(), callInfo->GetCallArg(0)));
    EXPECT_EQ(result.GetTaggedValue(), JSTaggedValue::Undefined());
    TestHelper::TearDownFrame(thread, prev);

    // Recheck the results after replace.
    uint32_t length = arrayList->GetLength().GetArrayLength();
    JSHandle<TaggedArray> elements(thread, arrayList->GetElements());
    for (uint32_t i = 0; i < length; i++) {
        EXPECT_EQ(elements->Get(i), JSTaggedValue(i));
    }
}

/**
 * @tc.name: SubArrayList
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F_L0(JSAPIArrayListTest, SubArrayList)
{
    JSHandle<JSAPIArrayList> arrayList(thread, CreateArrayList());
    uint32_t addElementNums = 256;
    for (uint32_t i = 0; i < addElementNums; i++) {
        JSHandle<JSTaggedValue> value(thread, JSTaggedValue(i));
        JSAPIArrayList::Add(thread, arrayList, value);
    }
    uint32_t formIndex = 50;
    uint32_t toIndex = 100;
    JSHandle<JSTaggedValue> fromIndexValue(thread, JSTaggedValue(formIndex));
    JSHandle<JSTaggedValue> toIndexValue(thread, JSTaggedValue(toIndex));
    JSHandle<JSAPIArrayList> subArrayList =
        JSAPIArrayList::SubArrayList(thread, arrayList, fromIndexValue, toIndexValue);
    JSHandle<TaggedArray> subElements(thread, subArrayList->GetElements());
    for (uint32_t i = 0; i < subArrayList->GetLength().GetArrayLength(); i++) {
        // The element value interval of substring is [50, 100]
        EXPECT_EQ(subElements->Get(i), JSTaggedValue(i + formIndex));
    }
}

/**
 * @tc.name: ForEach
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F_L0(JSAPIArrayListTest, ForEach)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<JSAPIArrayList> arrayList(thread, CreateArrayList());
    JSHandle<GlobalEnv> env = thread->GetEcmaVM()->GetGlobalEnv();
    JSHandle<JSFunction> func =
        factory->NewJSFunction(env, reinterpret_cast<void *>(TestClass::TestForEachAndReplaceAllFunc));
    auto callInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 6);
    callInfo->SetFunction(JSTaggedValue::Undefined());
    callInfo->SetThis(arrayList.GetTaggedValue());
    callInfo->SetCallArg(0, func.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, callInfo);
    JSHandle<JSTaggedValue> result(thread,
        JSAPIArrayList::ForEach(thread, callInfo->GetThis(), callInfo->GetFunction(), callInfo->GetCallArg(0)));
    EXPECT_EQ(result.GetTaggedValue(), JSTaggedValue::Undefined());
    TestHelper::TearDownFrame(thread, prev);
}

/**
 * @tc.name: GetIteratorObj
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F_L0(JSAPIArrayListTest, GetIteratorObj)
{
    JSHandle<JSAPIArrayList> arrayList(thread, CreateArrayList());
    JSHandle<JSTaggedValue> iteratorObj(thread, JSAPIArrayList::GetIteratorObj(thread, arrayList));
    EXPECT_TRUE(iteratorObj->IsJSAPIArrayListIterator());
}

/**
 * @tc.name: Get & Set & Has
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F_L0(JSAPIArrayListTest, Get_Set_Has)
{
    JSHandle<JSAPIArrayList> arrayList(thread, CreateArrayList());
    uint32_t elementsNum = 256;
    for (uint32_t i = 0; i < elementsNum; i++) {
        JSHandle<JSTaggedValue> value(thread, JSTaggedValue(i));
        JSAPIArrayList::Add(thread, arrayList, value);
        arrayList->Set(thread, i, JSTaggedValue(i * 10));

        JSHandle<JSTaggedValue> getValue(thread, arrayList->Get(thread, i));
        EXPECT_EQ(getValue.GetTaggedValue(), JSTaggedValue(i * 10));

        bool isHas = arrayList->Has(JSTaggedValue(i * 10));
        EXPECT_EQ(isHas, true);
    }
}

/**
 * @tc.name: OwnKeys
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F_L0(JSAPIArrayListTest, OwnKeys)
{
    JSHandle<JSAPIArrayList> arrayList(thread, CreateArrayList());
    uint32_t elementsNum = 256;
    for (uint32_t i = 0; i < elementsNum; i++) {
        JSHandle<JSTaggedValue> value(thread, JSTaggedValue(i));
        JSAPIArrayList::Add(thread, arrayList, value);
    }
    JSHandle<TaggedArray> keys = JSAPIArrayList::OwnKeys(thread, arrayList);
    uint32_t length = arrayList->GetLength().GetArrayLength();
    for (uint32_t i = 0; i < length; i++) {
        EXPECT_EQ(keys->Get(i), JSTaggedValue(i));
    }
}

/**
 * @tc.name: GetOwnProperty
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F_L0(JSAPIArrayListTest, GetOwnProperty)
{
    JSHandle<JSAPIArrayList> arrayList(thread, CreateArrayList());
    uint32_t elementsNums = 256;
    for (uint32_t i = 0; i < elementsNums; i++) {
        JSHandle<JSTaggedValue> value(thread, JSTaggedValue(i));
        JSAPIArrayList::Add(thread, arrayList, value);
    }
    for (uint32_t i = 0; i < elementsNums; i++) {
        JSHandle<JSTaggedValue> key(thread, JSTaggedValue(i));
        bool getOwnPropertyRes = JSAPIArrayList::GetOwnProperty(thread, arrayList, key);
        EXPECT_EQ(getOwnPropertyRes, true);
    }
}
} // namespace panda::test