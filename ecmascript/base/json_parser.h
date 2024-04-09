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

#ifndef ECMASCRIPT_BASE_JSON_PARSE_INL_H
#define ECMASCRIPT_BASE_JSON_PARSE_INL_H

#include <cerrno>

#include "ecmascript/base/json_helper.h"
#include "ecmascript/base/json_parser.h"
#include "ecmascript/base/builtins_base.h"
#include "ecmascript/base/number_helper.h"
#include "ecmascript/base/string_helper.h"
#include "ecmascript/base/utf_helper.h"
#include "ecmascript/ecma_string-inl.h"
#include "ecmascript/ecma_string.h"
#include "ecmascript/js_array.h"
#include "ecmascript/js_function.h"
#include "ecmascript/js_handle.h"
#include "ecmascript/global_env.h"
#include "ecmascript/js_tagged_value.h"
#include "ecmascript/message_string.h"
#include "ecmascript/object_factory.h"
#include "ecmascript/object_fast_operator-inl.h"

namespace panda::ecmascript::base {
constexpr unsigned int UNICODE_DIGIT_LENGTH = 4;
constexpr unsigned int NUMBER_TEN = 10;
constexpr unsigned int NUMBER_SIXTEEN = 16;
constexpr unsigned int INTEGER_MAX_LEN = 9;

constexpr unsigned char CODE_SPACE = 0x20;
constexpr unsigned char ASCII_END = 0X7F;
enum class Tokens : uint8_t {
        // six structural tokens
        OBJECT = 0,
        ARRAY,
        NUMBER,
        STRING,
        LITERAL_TRUE,
        LITERAL_FALSE,
        LITERAL_NULL,
        TOKEN_ILLEGAL,
    };

struct JsonContinuation {
    enum class ContinuationType : uint8_t {
        RETURN = 0,
        ARRAY,
        OBJECT,
    };
    JsonContinuation(ContinuationType type, size_t index) : type_(type), index_(index) {}

    ContinuationType type_ {ContinuationType::RETURN};
    size_t index_ {0};
};

template<typename T>
class JsonParser {
protected:
    using TransformType = base::JsonHelper::TransformType;
    using Text = const T *;
    using ContType = JsonContinuation::ContinuationType;
    // Instantiation of the class is prohibited
    JsonParser() = default;
    JsonParser(JSThread *thread, TransformType transformType)
        : thread_(thread), transformType_(transformType)
    {
    }
    ~JsonParser() = default;
    NO_COPY_SEMANTIC(JsonParser);
    NO_MOVE_SEMANTIC(JsonParser);

    JSHandle<JSTaggedValue> Launch(Text begin, Text end)
    {
        // check empty
        if (UNLIKELY(begin == end)) {
            return JSHandle<JSTaggedValue>(thread_, [&]() -> JSTaggedValue {
                THROW_SYNTAX_ERROR_AND_RETURN(thread_, "Unexpected Text in JSON", JSTaggedValue::Exception());
            }());
        }
        end_ = end - 1;
        current_ = begin;
        SkipEndWhiteSpace();
        range_ = end_;

        auto vm = thread_->GetEcmaVM();
        factory_ = vm->GetFactory();
        env_ = *vm->GetGlobalEnv();

        // For Json, we do not support ElementsKind
        auto index = static_cast<size_t>(ConstantIndex::ELEMENT_HOLE_TAGGED_HCLASS_INDEX);
        auto globalConstant = const_cast<GlobalEnvConstants *>(thread_->GlobalConstants());
        auto hclassVal = globalConstant->GetGlobalConstantObject(index);
        initialJSArrayClass_ = JSHandle<JSHClass>(thread_, hclassVal);
        JSHandle<JSFunction> objectFunc(env_->GetObjectFunction());
        initialJSObjectClass_ =
            JSHandle<JSHClass>(thread_, JSFunction::GetOrCreateInitialJSHClass(thread_, objectFunc));

        JSTaggedValue result = ParseJSONText();
        RETURN_HANDLE_IF_ABRUPT_COMPLETION(JSTaggedValue, thread_);
        return JSHandle<JSTaggedValue>(thread_, result);
    }

    inline bool IsInObjOrArray(ContType type)
    {
        return type == ContType::ARRAY || type == ContType::OBJECT;
    }

    inline bool EmptyArrayCheck()
    {
        GetNextNonSpaceChar();
        return *current_ == ']';
    }

    inline bool EmptyObjectCheck()
    {
        GetNextNonSpaceChar();
        return *current_ == '}';
    }

    JSHandle<JSTaggedValue> GetSJsonPrototype()
    {
        JSHandle<JSFunction> sObjFunction(thread_->GetEcmaVM()->GetGlobalEnv()->GetSObjectFunction());
        JSHandle<JSTaggedValue> jsonPrototype = JSHandle<JSTaggedValue>(thread_, sObjFunction->GetFunctionPrototype());
        return jsonPrototype;
    }

    JSTaggedValue ParseJSONText()
    {
        JSHandle<JSTaggedValue> parseValue;
        std::vector<JsonContinuation> continuationList;
        std::vector<JSHandle<JSTaggedValue>> elementsList;
        std::vector<JSHandle<JSTaggedValue>> propertyList;
        continuationList.reserve(16); // 16: initial capacity
        elementsList.reserve(16); // 16: initial capacity
        propertyList.reserve(16); // 16: initial capacity
        JsonContinuation continuation(ContType::RETURN, 0);
        while (true) {
            while (true) {
                SkipStartWhiteSpace();
                Tokens token = ParseToken();
                switch (token) {
                    case Tokens::OBJECT:
                        if (EmptyObjectCheck()) {
                            if (transformType_ == TransformType::SENDABLE) {
                                JSHandle<JSHClass> sHclass;
                                JSHandle<JSTaggedValue> sJsonPrototype = GetSJsonPrototype();
                                JSHandle<LayoutInfo> sLayout = factory_->CreateSLayoutInfo(0);
                                sHclass = factory_->NewSEcmaHClass(JSSharedObject::SIZE, 0, JSType::JS_SHARED_OBJECT,
                                                                   JSHandle<JSTaggedValue>(sJsonPrototype),
                                                                   JSHandle<JSTaggedValue>(sLayout));
                                parseValue = JSHandle<JSTaggedValue>(factory_->NewSharedOldSpaceJSObject(sHclass));
                            } else {
                                parseValue = JSHandle<JSTaggedValue>(factory_->NewJSObject(initialJSObjectClass_));
                            }
                            GetNextNonSpaceChar();
                            break;
                        }
                        continuationList.emplace_back(std::move(continuation));
                        continuation = JsonContinuation(ContType::OBJECT, propertyList.size());

                        SkipStartWhiteSpace();
                        if (UNLIKELY(*current_ != '"')) {
                            THROW_SYNTAX_ERROR_AND_RETURN(thread_, "Unexpected Object Prop in JSON",
                                                          JSTaggedValue::Exception());
                        }
                        propertyList.emplace_back(ParseString(true));
                        RETURN_EXCEPTION_IF_ABRUPT_COMPLETION(thread_);
                        SkipStartWhiteSpace();
                        if (UNLIKELY(*current_ != ':')) {
                            THROW_SYNTAX_ERROR_AND_RETURN(thread_, "Unexpected Object in JSON",
                                                          JSTaggedValue::Exception());
                        }
                        Advance();
                        continue;
                    case Tokens::ARRAY:
                        // sendable array not support now
                        if (UNLIKELY(transformType_ == TransformType::SENDABLE)) {
                            THROW_SYNTAX_ERROR_AND_RETURN(thread_, GET_MESSAGE_STRING(SendableArrayForJson),
                                                          JSTaggedValue::Exception());
                        }
                        if (EmptyArrayCheck()) {
                            parseValue = JSHandle<JSTaggedValue>(factory_->NewJSArray(0, initialJSArrayClass_));
                            GetNextNonSpaceChar();
                            break;
                        }
                        continuationList.emplace_back(std::move(continuation));
                        continuation = JsonContinuation(ContType::ARRAY, elementsList.size());
                        continue;
                    case Tokens::LITERAL_TRUE:
                        parseValue = JSHandle<JSTaggedValue>(thread_, ParseLiteralTrue());
                        RETURN_EXCEPTION_IF_ABRUPT_COMPLETION(thread_);
                        break;
                    case Tokens::LITERAL_FALSE:
                        parseValue = JSHandle<JSTaggedValue>(thread_, ParseLiteralFalse());
                        RETURN_EXCEPTION_IF_ABRUPT_COMPLETION(thread_);
                        break;
                    case Tokens::LITERAL_NULL:
                        parseValue = JSHandle<JSTaggedValue>(thread_, ParseLiteralNull());
                        RETURN_EXCEPTION_IF_ABRUPT_COMPLETION(thread_);
                        break;
                    case Tokens::NUMBER:
                        parseValue = JSHandle<JSTaggedValue>(thread_, ParseNumber(IsInObjOrArray(continuation.type_)));
                        RETURN_EXCEPTION_IF_ABRUPT_COMPLETION(thread_);
                        break;
                    case Tokens::STRING:
                        parseValue = ParseString(IsInObjOrArray(continuation.type_));
                        RETURN_EXCEPTION_IF_ABRUPT_COMPLETION(thread_);
                        break;
                    default:
                        THROW_SYNTAX_ERROR_AND_RETURN(thread_, "Unexpected Text in JSON", JSTaggedValue::Exception());
                }
                break;
            }

            while (true) {
                switch (continuation.type_) {
                    case ContType::RETURN:
                        ASSERT(continuationList.empty());
                        ASSERT(elementsList.empty());
                        ASSERT(propertyList.empty());
                        if (current_ <= range_) {
                            THROW_SYNTAX_ERROR_AND_RETURN(thread_, "Unexpected Text in JSON",
                                                          JSTaggedValue::Exception());
                        }
                        return parseValue.GetTaggedValue();
                    case ContType::ARRAY: {
                        elementsList.emplace_back(parseValue);
                        SkipStartWhiteSpace();
                        if (*current_ == ',') {
                            Advance();
                            break;
                        }
                        parseValue = CreateJsonArray(continuation, elementsList);
                        if (*current_ != ']') {
                            THROW_SYNTAX_ERROR_AND_RETURN(thread_, "Unexpected Array in JSON",
                                                          JSTaggedValue::Exception());
                        }
                        Advance();
                        elementsList.resize(continuation.index_);
                        continuation = std::move(continuationList.back());
                        continuationList.pop_back();
                        continue;
                    }
                    case ContType::OBJECT: {
                        propertyList.emplace_back(parseValue);
                        SkipStartWhiteSpace();
                        if (*current_ == ',') {
                            GetNextNonSpaceChar();
                            if (UNLIKELY(*current_ != '"')) {
                                THROW_SYNTAX_ERROR_AND_RETURN(thread_, "Unexpected Object Prop in JSON",
                                                              JSTaggedValue::Exception());
                            }
                            propertyList.emplace_back(ParseString(true));
                            RETURN_EXCEPTION_IF_ABRUPT_COMPLETION(thread_);
                            SkipStartWhiteSpace();
                            if (UNLIKELY(*current_ != ':')) {
                                THROW_SYNTAX_ERROR_AND_RETURN(thread_, "Unexpected Object in JSON",
                                                              JSTaggedValue::Exception());
                            }
                            Advance();
                            break;
                        }
                        if (UNLIKELY(transformType_ == TransformType::SENDABLE)) {
                            parseValue = CreateSJsonObject(continuation, propertyList);
                        } else {
                            parseValue = CreateJsonObject(continuation, propertyList);
                        }
                        if (UNLIKELY(*current_ != '}')) {
                            THROW_SYNTAX_ERROR_AND_RETURN(thread_, "Unexpected Object in JSON",
                                                          JSTaggedValue::Exception());
                        }
                        Advance();
                        propertyList.resize(continuation.index_);
                        continuation = std::move(continuationList.back());
                        continuationList.pop_back();
                        continue;
                    }
                }
                break;
            }
        }
    }

    JSHandle<JSTaggedValue> CreateJsonArray(JsonContinuation continuation,
                                            std::vector<JSHandle<JSTaggedValue>> &elementsList)
    {
        size_t start = continuation.index_;
        size_t size = elementsList.size() - start;
        JSHandle<JSArray> array = factory_->NewJSArray(size, initialJSArrayClass_);
        JSHandle<TaggedArray> elements = factory_->NewJsonFixedArray(start, size, elementsList);
        JSHandle<JSObject> obj(array);
        obj->SetElements(thread_, elements);
        return JSHandle<JSTaggedValue>(array);
    }

    JSHandle<JSTaggedValue> CreateJsonObject(JsonContinuation continuation,
                                             std::vector<JSHandle<JSTaggedValue>> &propertyList)
    {
        size_t start = continuation.index_;
        size_t size = propertyList.size() - start;
        auto obj = JSHandle<JSTaggedValue>(factory_->NewJSObject(initialJSObjectClass_));
        for (size_t i = 0; i < size; i += 2) { // 2: prop name and value
            auto &keyHandle = propertyList[start + i];
            auto &valueHandle = propertyList[start + i + 1];
            auto res = SetPropertyByValue(obj, keyHandle, valueHandle);
            RETURN_HANDLE_IF_ABRUPT_COMPLETION(JSTaggedValue, thread_);
            if (res.IsHole()) {
                // slow path
                JSTaggedValue::SetProperty(thread_, obj, keyHandle, valueHandle, true);
                RETURN_HANDLE_IF_ABRUPT_COMPLETION(JSTaggedValue, thread_);
            }
        }
        return obj;
    }

    JSHandle<JSTaggedValue> CreateSJsonObject(JsonContinuation continuation,
                                              std::vector<JSHandle<JSTaggedValue>> &propertyList)
    {
        size_t start = continuation.index_;
        size_t size = propertyList.size() - start;
        uint32_t fieldNum = size / 2; // 2: key-value pair
        JSHandle<JSHClass> hclass;
        JSHandle<LayoutInfo> layout;
        JSHandle<JSTaggedValue> jsonPrototype = GetSJsonPrototype();
        if (fieldNum == 0) {
            layout = factory_->CreateSLayoutInfo(fieldNum);
            hclass = factory_->NewSEcmaHClass(JSSharedObject::SIZE, fieldNum, JSType::JS_SHARED_OBJECT,
                                              JSHandle<JSTaggedValue>(jsonPrototype), JSHandle<JSTaggedValue>(layout));
            JSHandle<JSObject> obj = factory_->NewSharedOldSpaceJSObject(hclass);
            return JSHandle<JSTaggedValue>(obj);
        } else if (LIKELY(fieldNum <= JSSharedObject::MAX_INLINE)) {
            layout = factory_->CreateSLayoutInfo(fieldNum);
            JSHandle<TaggedArray> propertyArray = factory_->NewSTaggedArray(size);
            for (size_t i = 0; i < size; i += 2) { // 2: prop name and value
                JSHandle<JSTaggedValue> keyHandle = propertyList[start + i];
                propertyArray->Set(thread_, i, keyHandle);
                propertyArray->Set(thread_, i + 1, JSTaggedValue(int(FieldType::NONE)));
            }
            hclass = factory_->NewSEcmaHClass(JSSharedObject::SIZE, fieldNum, JSType::JS_SHARED_OBJECT,
                                              JSHandle<JSTaggedValue>(jsonPrototype), JSHandle<JSTaggedValue>(layout));
            SendableClassDefiner::AddFieldTypeToHClass(thread_, propertyArray, size, layout, hclass);
            JSHandle<JSObject> obj = factory_->NewSharedOldSpaceJSObject(hclass);
            uint32_t index = 0;
            for (size_t i = 0; i < size; i += 2) { // 2: prop name and value
                obj->SetPropertyInlinedProps(thread_, index++, propertyList[start + i + 1].GetTaggedValue());
            }
            return JSHandle<JSTaggedValue>(obj);
        }
        // build in dict mode
        JSMutableHandle<NameDictionary> dict(
            thread_, NameDictionary::CreateInSharedHeap(thread_, NameDictionary::ComputeHashTableSize(fieldNum)));
        JSMutableHandle<JSTaggedValue> propKey(thread_, JSTaggedValue::Undefined());
        JSMutableHandle<JSTaggedValue> propValue(thread_, JSTaggedValue::Undefined());
        // create dict and set key value
        for (size_t i = 0; i < size; i += 2) { // 2: prop name and value
            PropertyAttributes attributes = PropertyAttributes::Default(false, false, false);
            propKey.Update(propertyList[start + i]);
            propValue.Update(propertyList[start + i + 1]);
            JSHandle<NameDictionary> newDict = NameDictionary::PutIfAbsent(thread_, dict, propKey,
                                                                           propValue, attributes);
            dict.Update(newDict);
        }
        hclass = factory_->NewSEcmaHClassDictMode(JSSharedObject::SIZE, fieldNum, JSType::JS_SHARED_OBJECT,
                                                  JSHandle<JSTaggedValue>(jsonPrototype));
        JSHandle<JSObject> obj = factory_->NewSharedOldSpaceJSObject(hclass);
        obj->SetProperties(thread_, dict);
        return JSHandle<JSTaggedValue>(obj);
    }

    JSTaggedValue SetPropertyByValue(const JSHandle<JSTaggedValue> &receiver, const JSHandle<JSTaggedValue> &key,
                                     const JSHandle<JSTaggedValue> &value)
    {
        ASSERT(key->IsString());
        auto newKey = key.GetTaggedValue();
        auto stringAccessor = EcmaStringAccessor(newKey);
        if (!stringAccessor.IsLineString() || IsNumberCharacter(*stringAccessor.GetDataUtf8())) {
            uint32_t index = 0;
            if (stringAccessor.ToElementIndex(&index)) {
                return ObjectFastOperator::SetPropertyByIndex<ObjectFastOperator::Status::UseOwn>(thread_,
                    receiver.GetTaggedValue(), index, value.GetTaggedValue());
            }
        }
        if (!stringAccessor.IsInternString()) {
            newKey = JSTaggedValue(thread_->GetEcmaVM()->GetFactory()->InternString(key));
        }
        return ObjectFastOperator::SetPropertyByName<ObjectFastOperator::Status::UseOwn>(thread_,
            receiver.GetTaggedValue(), newKey, value.GetTaggedValue());
    }

    JSTaggedValue ParseNumber(bool inObjorArr = false)
    {
        if (inObjorArr) {
            bool isFast = true;
            int32_t fastInteger = 0;
            bool isNumber = ReadNumberRange(isFast, fastInteger);
            if (!isNumber) {
                THROW_SYNTAX_ERROR_AND_RETURN(thread_, "Unexpected Number in JSON Array Or Object",
                                              JSTaggedValue::Exception());
            }
            if (isFast) {
                return JSTaggedValue(fastInteger);
            }
        }

        Text current = current_;
        bool hasExponent = false;
        if (*current_ == '-') {
            if (UNLIKELY(current_++ == end_)) {
                THROW_SYNTAX_ERROR_AND_RETURN(thread_, "Unexpected Number in JSON", JSTaggedValue::Exception());
            }
        }
        if (*current_ == '0') {
            if (!CheckZeroBeginNumber(hasExponent)) {
                THROW_SYNTAX_ERROR_AND_RETURN(thread_, "Unexpected Number in JSON", JSTaggedValue::Exception());
            }
        } else if (*current_ >= '1' && *current_ <= '9') {
            if (!CheckNonZeroBeginNumber(hasExponent)) {
                THROW_SYNTAX_ERROR_AND_RETURN(thread_, "Unexpected Number in JSON", JSTaggedValue::Exception());
            }
        } else {
            THROW_SYNTAX_ERROR_AND_RETURN(thread_, "Unexpected Number in JSON", JSTaggedValue::Exception());
        }

        std::string strNum(current, end_ + 1);
        current_ = end_ + 1;
        errno = 0; // reset errno to 0 to avoid errno has been changed
        double v = std::strtod(strNum.c_str(), nullptr);
        if (errno == ERANGE) {
            errno = 0;
            return v > 0 ? JSTaggedValue(base::POSITIVE_INFINITY): JSTaggedValue(-base::POSITIVE_INFINITY);
        }
        errno = 0;
        return JSTaggedValue::TryCastDoubleToInt32(v);
    }

    bool ParseBackslash(std::u16string &res)
    {
        if (current_ == end_) {
            return false;
        }
        Advance();
        switch (*current_) {
            case '\"':
                res += '\"';
                break;
            case '\\':
                res += '\\';
                break;
            case '/':
                res += '/';
                break;
            case 'b':
                res += '\b';
                break;
            case 'f':
                res += '\f';
                break;
            case 'n':
                res += '\n';
                break;
            case 'r':
                res += '\r';
                break;
            case 't':
                res += '\t';
                break;
            case 'u': {
                std::u16string u16Str;
                if (UNLIKELY(!ConvertStringUnicode(u16Str))) {
                    return false;
                }
                res += u16Str;
                break;
            }
            default:
                return false;
        }
        return true;
    }

    JSHandle<JSTaggedValue> SlowParseString()
    {
        end_--;
        std::u16string res;
        res.reserve(end_ - current_);
        while (current_ <= end_) {
            if (*current_ == '\\') {
                if (UNLIKELY(!ParseBackslash(res))) {
                    THROW_SYNTAX_ERROR_AND_RETURN(thread_, "Unexpected string in JSON",
                        JSHandle<JSTaggedValue>(thread_, JSTaggedValue::Exception()));
                }
                Advance();
            } else {
                Text nextCurrent = current_;
                while (nextCurrent <= end_ && *nextCurrent != '\\') {
                    ++nextCurrent;
                }
                res += std::u16string(current_, nextCurrent);
                current_ = nextCurrent;
            }
        }
        ASSERT(res.size() <= static_cast<size_t>(UINT32_MAX));
        Advance();
        return JSHandle<JSTaggedValue>::Cast(factory_->NewFromUtf16(
            reinterpret_cast<const uint16_t *>(res.data()), res.size()));
    }

    virtual void ParticalParseString(std::string& str, Text current, Text nextCurrent) = 0;

    virtual JSHandle<JSTaggedValue> ParseString(bool inObjorArr = false) = 0;

    void SkipEndWhiteSpace()
    {
        while (current_ != end_) {
            if (*end_ == ' ' || *end_ == '\r' || *end_ == '\n' || *end_ == '\t') {
                end_--;
            } else {
                break;
            }
        }
    }

    void SkipStartWhiteSpace()
    {
        while (current_ != end_) {
            if (*current_ == ' ' || *current_ == '\r' || *current_ == '\n' || *current_ == '\t') {
                Advance();
            } else {
                break;
            }
        }
    }

    void GetNextNonSpaceChar()
    {
        Advance();
        SkipStartWhiteSpace();
    }

    Tokens ParseToken()
    {
        switch (*current_) {
            case '{':
                return Tokens::OBJECT;
            case '[':
                return Tokens::ARRAY;
            case '"':
                return Tokens::STRING;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case '-':
                return Tokens::NUMBER;
            case 't':
                return Tokens::LITERAL_TRUE;
            case 'f':
                return Tokens::LITERAL_FALSE;
            case 'n':
                return Tokens::LITERAL_NULL;
            default:
                return Tokens::TOKEN_ILLEGAL;
        }
    }

    JSTaggedValue ParseLiteralTrue()
    {
        static const char literalTrue[] = "true";
        uint32_t remainingLength = range_ - current_;
        if (UNLIKELY(remainingLength < 3)) { // 3: literalTrue length - 1
            THROW_SYNTAX_ERROR_AND_RETURN(thread_, "Unexpected Text in JSON", JSTaggedValue::Exception());
        }
        bool isMatch = MatchText(literalTrue, 4); // 4: literalTrue length
        if (LIKELY(isMatch)) {
            return JSTaggedValue::True();
        }
        THROW_SYNTAX_ERROR_AND_RETURN(thread_, "Unexpected Text in JSON", JSTaggedValue::Exception());
    }

    JSTaggedValue ParseLiteralFalse()
    {
        static const char literalFalse[] = "false";
        uint32_t remainingLength = range_ - current_;
        if (UNLIKELY(remainingLength < 4)) { // 4: literalFalse length - 1
            THROW_SYNTAX_ERROR_AND_RETURN(thread_, "Unexpected Text in JSON", JSTaggedValue::Exception());
        }
        bool isMatch = MatchText(literalFalse, 5); // 5: literalFalse length
        if (LIKELY(isMatch)) {
            return JSTaggedValue::False();
        }
        THROW_SYNTAX_ERROR_AND_RETURN(thread_, "Unexpected Text in JSON", JSTaggedValue::Exception());
    }

    JSTaggedValue ParseLiteralNull()
    {
        static const char literalNull[] = "null";
        uint32_t remainingLength = range_ - current_;
        if (UNLIKELY(remainingLength < 3)) { // 3: literalNull length - 1
            THROW_SYNTAX_ERROR_AND_RETURN(thread_, "Unexpected Text in JSON", JSTaggedValue::Exception());
        }
        bool isMatch = MatchText(literalNull, 4); // 4: literalNull length
        if (LIKELY(isMatch)) {
            return JSTaggedValue::Null();
        }
        THROW_SYNTAX_ERROR_AND_RETURN(thread_, "Unexpected Text in JSON", JSTaggedValue::Exception());
    }

    bool MatchText(const char *str, uint32_t matchLen)
    {
        // first char is already matched
        for (uint32_t pos = 1; pos < matchLen; ++pos) {
            if (current_[pos] != str[pos]) {
                return false;
            }
        }
        current_ += matchLen;
        return true;
    }

    bool ReadNumberRange(bool &isFast, int32_t &fastInteger)
    {
        Text current = current_;
        int32_t sign = 1;
        if (*current == '-') {
            current++;
            sign = -1;
        }

        if (*current == '0') {
            isFast = false;
            current++;
        } else {
            Text advance = AdvanceLastNumberCharacter(current);
            if (UNLIKELY(current == advance)) {
                return false;
            }
            size_t numberLength = advance - current;
            int32_t i = 0;
            if (numberLength <= INTEGER_MAX_LEN && (*advance == ',' || *advance == ']' || *advance == '}')) {
                for (; current != advance; current++) {
                    i = (i * 10) + ((*current) - '0');
                }
                fastInteger = i * sign;
                current_ = advance;
                return true;
            }
            isFast = false;
        }

        while (current != range_) {
            if (IsNumberCharacter(*current)) {
                current++;
                continue;
            } else if (IsNumberSignalCharacter(*current)) {
                isFast = false;
                current++;
                continue;
            }
            Text end = current;
            while (current != range_) {
                if (*current == ' ' || *current == '\r' || *current == '\n' || *current == '\t') {
                    current++;
                } else if (*current == ',' || *current == ']' || *current == '}') {
                    end_ = end - 1;
                    return true;
                } else {
                    return false;
                }
            }
            if (*current == ']' || *current == '}') {
                end_ = end - 1;
                return true;
            }
            return false;
        }
        end_ = range_ - 1;
        return true;
    }

    Text AdvanceLastNumberCharacter(Text current)
    {
        return std::find_if(current, range_, [this](T c) { return !IsNumberCharacter(c); });
    }

    bool IsNumberCharacter(T ch)
    {
        if (ch >= '0' && ch <= '9') {
            return true;
        }
        return false;
    }

    bool IsNumberSignalCharacter(T ch)
    {
        return ch == '.' || ch == 'e' || ch == 'E' || ch == '+' || ch == '-';
    }

    bool IsExponentNumber()
    {
        if (IsNumberCharacter(*current_)) {
            return true;
        } else if (*current_ == '-' || *current_ == '+') {
            if (current_ == end_) {
                return false;
            }
            Advance();
            if (IsNumberCharacter(*current_)) {
                return true;
            }
        }
        return false;
    }

    bool IsDecimalsLegal(bool &hasExponent)
    {
        if (current_ == end_ && !IsNumberCharacter(*++current_)) {
            return false;
        }

        while (current_ != end_) {
            Advance();
            if (IsNumberCharacter(*current_)) {
                continue;
            } else if (*current_ == 'e' || *current_ == 'E') {
                if (hasExponent || current_ == end_) {
                    return false;
                }
                Advance();
                if (!IsExponentNumber()) {
                    return false;
                }
                hasExponent = true;
            } else {
                return false;
            }
        }
        return true;
    }

    bool IsExponentLegal(bool &hasExponent)
    {
        if (hasExponent || current_ == end_) {
            return false;
        }
        Advance();
        if (!IsExponentNumber()) {
            return false;
        }
        while (current_ != end_) {
            if (!IsNumberCharacter(*current_)) {
                return false;
            }
            Advance();
        }
        return true;
    }

    bool ConvertStringUnicode(std::u16string &u16Str)
    {
        do {
            uint32_t remainingLength = end_ - current_;
            if (remainingLength < UNICODE_DIGIT_LENGTH) {
                return false;
            }
            uint16_t res = 0;
            for (uint32_t pos = 0; pos < UNICODE_DIGIT_LENGTH; pos++) {
                Advance();
                if (*current_ >= '0' && *current_ <= '9') {
                    res *= NUMBER_SIXTEEN;
                    res += (*current_ - '0');
                } else if (*current_ >= 'a' && *current_ <= 'f') {
                    res *= NUMBER_SIXTEEN;
                    res += (*current_ - 'a' + NUMBER_TEN);
                } else if (*current_ >= 'A' && *current_ <= 'F') {
                    res *= NUMBER_SIXTEEN;
                    res += (*current_ - 'A' + NUMBER_TEN);
                } else {
                    return false;
                }
            }
            u16Str.push_back(res);
        } while ([&]() -> bool {
            static const int unicodePrefixLen = 2;
            if (end_ - current_ < unicodePrefixLen) {
                return false;
            }
            if (*(current_ + 1) == '\\' && *(current_ + unicodePrefixLen) == 'u') {
                AdvanceMultiStep(unicodePrefixLen);
                return true;
            }
            return false;
        }());
        return true;
    }

    bool CheckZeroBeginNumber(bool &hasExponent)
    {
        if (current_++ != end_) {
            if (*current_ == '.') {
                if (!IsDecimalsLegal(hasExponent)) {
                    return false;
                }
            } else if (*current_ == 'e' || *current_ == 'E') {
                if (!IsExponentLegal(hasExponent)) {
                    return false;
                }
            } else {
                return false;
            }
        }
        return true;
    }

    bool CheckNonZeroBeginNumber(bool &hasExponent)
    {
        while (current_ != end_) {
            Advance();
            if (IsNumberCharacter(*current_)) {
                continue;
            } else if (*current_ == '.') {
                if (!IsDecimalsLegal(hasExponent)) {
                    return false;
                }
            } else if (*current_ == 'e' || *current_ == 'E') {
                if (!IsExponentLegal(hasExponent)) {
                    return false;
                }
            } else {
                return false;
            }
        }
        return true;
    }

    inline void Advance()
    {
        ++current_;
    }

    inline void AdvanceMultiStep(int step)
    {
        current_ += step;
    }

    Text end_ {nullptr};
    Text current_ {nullptr};
    Text range_ {nullptr};
    JSThread *thread_ {nullptr};
    ObjectFactory *factory_ {nullptr};
    GlobalEnv *env_ {nullptr};
    TransformType transformType_ {TransformType::NORMAL};
    JSHandle<JSHClass> initialJSArrayClass_;
    JSHandle<JSHClass> initialJSObjectClass_;
};

class Utf8JsonParser : public JsonParser<uint8_t> {
public:
    Utf8JsonParser() = default;
    Utf8JsonParser(JSThread *thread, TransformType transformType) : JsonParser(thread, transformType) {}
    ~Utf8JsonParser() = default;
    NO_COPY_SEMANTIC(Utf8JsonParser);
    NO_MOVE_SEMANTIC(Utf8JsonParser);

    JSHandle<JSTaggedValue> Parse(const JSHandle<EcmaString> &strHandle)
    {
        ASSERT(*strHandle != nullptr);
        auto stringAccessor = EcmaStringAccessor(strHandle);
        uint32_t len = stringAccessor.GetLength();
        ASSERT(len != UINT32_MAX);

        uint32_t slicedOffset = 0;
        if (LIKELY(stringAccessor.IsLineOrConstantString())) {
            sourceString_ = strHandle;
        } else if (stringAccessor.IsSlicedString()) {
            auto *sliced = static_cast<SlicedString *>(*strHandle);
            slicedOffset = sliced->GetStartIndex();
            sourceString_ = JSHandle<EcmaString>(thread_, EcmaString::Cast(sliced->GetParent()));
        } else {
            auto *flatten = EcmaStringAccessor::Flatten(thread_->GetEcmaVM(), strHandle);
            sourceString_ = JSHandle<EcmaString>(thread_, flatten);
        }
        begin_ = EcmaStringAccessor(sourceString_).GetDataUtf8();
        auto *heap = const_cast<Heap *>(thread_->GetEcmaVM()->GetHeap());
        auto listenerId = heap->AddGCListener(UpdatePointersListener, this);
        auto res = Launch(begin_ + slicedOffset, begin_ + slicedOffset + len);
        heap->RemoveGCListener(listenerId);
        return res;
    }

private:
    void ParticalParseString(std::string& str, Text current, Text nextCurrent) override
    {
        str += std::string_view(reinterpret_cast<const char *>(current), nextCurrent - current);
    }

    static void UpdatePointersListener(void *utf8Parser)
    {
        auto *parser = reinterpret_cast<Utf8JsonParser *>(utf8Parser);
        auto *begin = EcmaStringAccessor(parser->sourceString_).GetDataUtf8();
        if (parser->begin_ != begin) {
            uint32_t currentOffset = parser->current_ - parser->begin_;
            uint32_t endOffset = parser->end_ - parser->begin_;
            uint32_t rangeOffset = parser->range_ - parser->begin_;
            parser->current_ = reinterpret_cast<uint8_t *>(ToUintPtr(begin) + currentOffset);
            parser->end_ = reinterpret_cast<uint8_t *>(ToUintPtr(begin) + endOffset);
            parser->range_ = reinterpret_cast<uint8_t *>(ToUintPtr(begin) + rangeOffset);
            parser->begin_ = begin;
        }
    }

    JSHandle<JSTaggedValue> ParseString(bool inObjorArr = false) override
    {
        bool isFastString = true;
        if (inObjorArr) {
            if (UNLIKELY(!ReadJsonStringRange(isFastString))) {
                THROW_SYNTAX_ERROR_AND_RETURN(thread_, "Unexpected end Text in JSON",
                    JSHandle<JSTaggedValue>(thread_, JSTaggedValue::Exception()));
            }
            if (isFastString) {
                uint32_t offset = current_ - begin_;
                uint32_t strLength = end_ - current_;
                ASSERT(strLength <= static_cast<size_t>(UINT32_MAX));
                current_ = end_ + 1;
                return JSHandle<JSTaggedValue>::Cast(factory_->NewCompressedUtf8SubString(
                    sourceString_, offset, strLength));
            }
        } else {
            if (UNLIKELY(*end_ != '"' || current_ == end_ || !IsFastParseJsonString(isFastString))) {
                THROW_SYNTAX_ERROR_AND_RETURN(thread_, "Unexpected end Text in JSON",
                    JSHandle<JSTaggedValue>(thread_, JSTaggedValue::Exception()));
            }
            if (LIKELY(isFastString)) {
                uint32_t offset = current_ - begin_;
                uint32_t strLength = end_ - current_;
                ASSERT(strLength <= static_cast<size_t>(UINT32_MAX));
                current_ = end_ + 1;
                return JSHandle<JSTaggedValue>::Cast(factory_->NewFromUtf8LiteralCompressSubString(
                    sourceString_, offset, strLength));
            }
        }
        return SlowParseString();
    }

    bool ReadJsonStringRange(bool &isFastString)
    {
        Advance();
        // chars are within Ascii
        for (Text current = current_; current != range_; ++current) {
            uint8_t c = *current;
            if (c == '"') {
                end_ = current;
                return true;
            } else if (UNLIKELY(c == '\\')) {
                current++;
                isFastString = false;
            } else if (UNLIKELY(c < CODE_SPACE)) {
                return false;
            }
        }
        return false;
    }

    bool IsFastParseJsonString(bool &isFastString)
    {
        Advance();
        // chars are within Ascii
        for (Text current = current_; current != end_; ++current) {
            if (*current < CODE_SPACE) {
                return false;
            } else if (*current == '\\') {
                isFastString = false;
            }
        }
        return true;
    }

    const uint8_t *begin_;
    JSHandle<EcmaString> sourceString_;
};

class Utf16JsonParser : public JsonParser<uint16_t> {
public:
    Utf16JsonParser() = default;
    Utf16JsonParser(JSThread *thread, TransformType transformType) : JsonParser(thread, transformType) {}
    ~Utf16JsonParser() = default;
    NO_COPY_SEMANTIC(Utf16JsonParser);
    NO_MOVE_SEMANTIC(Utf16JsonParser);

    JSHandle<JSTaggedValue> Parse(EcmaString *str)
    {
        ASSERT(str != nullptr);
        uint32_t len = EcmaStringAccessor(str).GetLength();
        CVector<uint16_t> buf(len);
        EcmaStringAccessor(str).WriteToFlatUtf16(buf.data(), len);
        Text begin = buf.data();
        return Launch(begin, begin + len);
    }

private:
    void ParticalParseString(std::string& str, Text current, Text nextCurrent) override
    {
        str += StringHelper::U16stringToString(std::u16string(current, nextCurrent));
    }

    JSHandle<JSTaggedValue> ParseString(bool inObjorArr = false) override
    {
        bool isFastString = true;
        bool isAscii = true;
        if (inObjorArr) {
            if (UNLIKELY(!ReadJsonStringRange(isFastString, isAscii))) {
                THROW_SYNTAX_ERROR_AND_RETURN(thread_, "Unexpected end Text in JSON",
                    JSHandle<JSTaggedValue>(thread_, JSTaggedValue::Exception()));
            }
            if (isFastString) {
                if (isAscii) {
                    std::string value(current_, end_); // from uint16_t* to std::string, can't use std::string_view
                    current_ = end_ + 1;
                    ASSERT(value.size() <= static_cast<size_t>(UINT32_MAX));
                    return JSHandle<JSTaggedValue>::Cast(factory_->NewFromUtf8LiteralCompress(
                        reinterpret_cast<const uint8_t *>(value.c_str()), value.size()));
                }
                std::u16string_view value(reinterpret_cast<const char16_t *>(current_), end_ - current_);
                current_ = end_ + 1;
                ASSERT(value.size() <= static_cast<size_t>(UINT32_MAX));
                return JSHandle<JSTaggedValue>::Cast(factory_->NewFromUtf16LiteralNotCompress(
                    reinterpret_cast<const uint16_t *>(value.data()), value.size()));
            }
        } else {
            if (UNLIKELY(*end_ != '"' || current_ == end_ || !IsFastParseJsonString(isFastString, isAscii))) {
                THROW_SYNTAX_ERROR_AND_RETURN(thread_, "Unexpected end Text in JSON",
                    JSHandle<JSTaggedValue>(thread_, JSTaggedValue::Exception()));
            }
            if (LIKELY(isFastString)) {
                if (isAscii) {
                    std::string value(current_, end_);  // from uint16_t* to std::string, can't use std::string_view
                    ASSERT(value.size() <= static_cast<size_t>(UINT32_MAX));
                    current_ = end_ + 1;
                    return JSHandle<JSTaggedValue>::Cast(factory_->NewFromUtf8LiteralCompress(
                        reinterpret_cast<const uint8_t *>(value.c_str()), value.size()));
                }
                std::u16string_view value(reinterpret_cast<const char16_t *>(current_), end_ - current_);
                ASSERT(value.size() <= static_cast<size_t>(UINT32_MAX));
                current_ = end_ + 1;
                return JSHandle<JSTaggedValue>::Cast(factory_->NewFromUtf16LiteralNotCompress(
                    reinterpret_cast<const uint16_t *>(value.data()), value.size()));
            }
        }
        return SlowParseString();
    }

    bool ReadJsonStringRange(bool &isFastString, bool &isAscii)
    {
        Advance();
        for (Text current = current_; current != range_; ++current) {
            uint16_t c = *current;
            if (c == '"') {
                end_ = current;
                return true;
            } else if (UNLIKELY(c == '\\')) {
                ++current;
                isFastString = false;
            }
            if (!IsLegalAsciiCharacter(c, isAscii)) {
                return false;
            }
        }
        return false;
    }

    bool IsFastParseJsonString(bool &isFastString, bool &isAscii)
    {
        Advance();
        for (Text current = current_; current != end_; ++current) {
            if (!IsLegalAsciiCharacter(*current, isAscii)) {
                return false;
            }
            if (*current == '\\') {
                isFastString = false;
            }
        }
        return true;
    }

    bool IsLegalAsciiCharacter(uint16_t c, bool &isAscii)
    {
        if (c <= ASCII_END) {
            return c >= CODE_SPACE ? true : false;
        }
        isAscii = false;
        return true;
    }
};

class Internalize {
public:
    using TransformType = base::JsonHelper::TransformType;
    static JSHandle<JSTaggedValue> InternalizeJsonProperty(JSThread *thread, const JSHandle<JSObject> &holder,
                                                           const JSHandle<JSTaggedValue> &name,
                                                           const JSHandle<JSTaggedValue> &receiver,
                                                           TransformType transformType);
private:
    static bool RecurseAndApply(JSThread *thread, const JSHandle<JSObject> &holder, const JSHandle<JSTaggedValue> &name,
                                const JSHandle<JSTaggedValue> &receiver, TransformType transformType);
};
}  // namespace panda::ecmascript::base

#endif  // ECMASCRIPT_BASE_JSON_PARSE_INL_H
