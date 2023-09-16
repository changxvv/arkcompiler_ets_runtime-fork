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
#include "ecmascript/object_factory.h"
#include "ecmascript/object_fast_operator-inl.h"

namespace panda::ecmascript::base {
constexpr unsigned int UNICODE_DIGIT_LENGTH = 4;
constexpr unsigned int NUMBER_TEN = 10;
constexpr unsigned int NUMBER_SIXTEEN = 16;

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

template<typename T>
class JsonParser {
protected:
    using Text = const T *;
    // Instantiation of the class is prohibited
    JsonParser() = default;
    explicit JsonParser(JSThread *thread) : thread_(thread) {}
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

        auto vm = thread_->GetEcmaVM();
        factory_ = vm->GetFactory();
        env_ = *vm->GetGlobalEnv();

        SkipEndWhiteSpace();
        range_ = end_;
        JSTaggedValue result = ParseJSONText();
        return JSHandle<JSTaggedValue>(thread_, result);
    }

    JSTaggedValue ParseJSONText(bool inObjorArr = false)
    {
        SkipStartWhiteSpace();
        Tokens token = ParseToken();
        switch (token) {
            case Tokens::OBJECT:
                return ParseObject(inObjorArr);
            case Tokens::ARRAY:
                return ParseArray(inObjorArr);
            case Tokens::LITERAL_TRUE:
                return ParseLiteralTrue();
            case Tokens::LITERAL_FALSE:
                return ParseLiteralFalse();
            case Tokens::LITERAL_NULL:
                return ParseLiteralNull();
            case Tokens::NUMBER:
                return ParseNumber(inObjorArr);
            case Tokens::STRING:
                return ParseString(inObjorArr);
            default:
                THROW_SYNTAX_ERROR_AND_RETURN(thread_, "Unexpected Text in JSON", JSTaggedValue::Exception());
        }
    }

    JSTaggedValue ParseNumber(bool inObjorArr = false)
    {
        if (inObjorArr) {
            bool isFast = true;
            bool isNumber = ReadNumberRange(isFast);
            if (!isNumber) {
                THROW_SYNTAX_ERROR_AND_RETURN(thread_, "Unexpected Number in JSON", JSTaggedValue::Exception());
            }
            if (isFast) {
                std::string strNum(current_, end_ + 1);
                current_ = end_;
                double v = std::strtod(strNum.c_str(), nullptr);
                if (errno == ERANGE) {
                    errno = 0;
                    return v > 0 ? JSTaggedValue(base::POSITIVE_INFINITY): JSTaggedValue(-base::POSITIVE_INFINITY);
                }
                return JSTaggedValue::TryCastDoubleToInt32(v);
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
        current_ = end_;
        double v = std::strtod(strNum.c_str(), nullptr);
        if (errno == ERANGE) {
            errno = 0;
            return v > 0 ? JSTaggedValue(base::POSITIVE_INFINITY): JSTaggedValue(-base::POSITIVE_INFINITY);
        }
        return JSTaggedValue::TryCastDoubleToInt32(v);
    }

    bool ParseBackslash(std::string &res)
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
                res += base::StringHelper::U16stringToString(u16Str);
                break;
            }
            default:
                return false;
        }
        return true;
    }

    JSTaggedValue SlowParseString()
    {
        end_--;
        std::string res;
        while (current_ <= end_) {
            if (*current_ == '\\') {
                bool isLegalChar = ParseBackslash(res);
                if (!isLegalChar) {
                    THROW_SYNTAX_ERROR_AND_RETURN(thread_, "Unexpected string in JSON", JSTaggedValue::Exception());
                }
                Advance();
            } else if (UNLIKELY(*current_ > ASCII_END)) {
                if (UNLIKELY(*current_ >= utf_helper::DECODE_LEAD_LOW && *current_ <= utf_helper::DECODE_LEAD_HIGH &&
                             *(current_ + 1) >= utf_helper::DECODE_TRAIL_LOW &&
                             *(current_ + 1) <= utf_helper::DECODE_TRAIL_HIGH)) {
                    std::u16string str(current_, current_ + 2);  // 2 means twice as many bytes as normal u16string
                    res += StringHelper::U16stringToString(str);
                    AdvanceMultiStep(2);  // 2 means twice as many bytes as normal u16string
                } else {
                    std::u16string str(current_, current_ + 1);
                    res += StringHelper::U16stringToString(str);
                    Advance();
                }
            } else {
                res += *current_;
                Advance();
            }
        }
        ASSERT(res.size() <= static_cast<size_t>(UINT32_MAX));
        return factory_->NewFromUtf8Literal(reinterpret_cast<const uint8_t *>(res.c_str()), res.size())
            .GetTaggedValue();
    }

    virtual JSTaggedValue ParseString(bool inObjorArr = false) = 0;

    JSTaggedValue ParseArray(bool inObjorArr = false)
    {
        if (UNLIKELY(*range_ != ']' && !inObjorArr)) {
            THROW_SYNTAX_ERROR_AND_RETURN(thread_, "Unexpected Array in JSON", JSTaggedValue::Exception());
        }

        Advance();
        SkipStartWhiteSpace();
        JSHandle<JSArray> arr = factory_->NewJSArray();
        if (*current_ == ']') {
            return arr.GetTaggedValue();
        }

        JSTaggedValue value;
        uint32_t index = 0;
        while (current_ <= range_) {
            value = ParseJSONText(true);
            RETURN_EXCEPTION_IF_ABRUPT_COMPLETION(thread_);
            ObjectFastOperator::SetPropertyByIndex<true>(thread_, arr.GetTaggedValue(), index++, value);
            GetNextNonSpaceChar();
            if (*current_ == ',') {
                Advance();
            } else if (*current_ == ']') {
                if (inObjorArr || current_ == range_) {
                    return arr.GetTaggedValue();
                } else {
                    THROW_SYNTAX_ERROR_AND_RETURN(thread_, "Unexpected Array in JSON", JSTaggedValue::Exception());
                }
            }
        }
        THROW_SYNTAX_ERROR_AND_RETURN(thread_, "Unexpected Array in JSON", JSTaggedValue::Exception());
    }

    JSTaggedValue ParseObject(bool inObjorArr = false)
    {
        if (UNLIKELY(*range_ != '}' && !inObjorArr)) {
            THROW_SYNTAX_ERROR_AND_RETURN(thread_, "Unexpected Object in JSON", JSTaggedValue::Exception());
        }

        JSHandle<JSFunction> proto(env_->GetObjectFunction());
        JSHandle<JSObject> result = factory_->NewJSObjectByConstructor(proto);
        Advance();
        if (*current_ == '}') {
            return result.GetTaggedValue();
        }

        JSMutableHandle<JSTaggedValue> keyHandle(thread_, JSTaggedValue::Undefined());
        JSTaggedValue value;
        while (current_ <= range_) {
            SkipStartWhiteSpace();
            if (*current_ == '"') {
                keyHandle.Update(ParseString(true));
                RETURN_EXCEPTION_IF_ABRUPT_COMPLETION(thread_);
            } else {
                if (*current_ == '}' && (inObjorArr || current_ == range_)) {
                    return result.GetTaggedValue();
                }
                THROW_SYNTAX_ERROR_AND_RETURN(thread_, "Unexpected Object in JSON", JSTaggedValue::Exception());
            }
            GetNextNonSpaceChar();
            if (*current_ == ':') {
                Advance();
            } else {
                THROW_SYNTAX_ERROR_AND_RETURN(thread_, "Unexpected Object in JSON", JSTaggedValue::Exception());
            }
            value = ParseJSONText(true);
            RETURN_EXCEPTION_IF_ABRUPT_COMPLETION(thread_);
            // fast path
            JSTaggedValue res = ObjectFastOperator::SetPropertyByValue<true>(thread_, result.GetTaggedValue(),
                                                                             keyHandle.GetTaggedValue(), value);
            if (res.IsHole()) {
                // slow path
                JSTaggedValue::SetProperty(thread_, JSHandle<JSTaggedValue>(result), keyHandle,
                                           JSHandle<JSTaggedValue>(thread_, value), true);
                RETURN_EXCEPTION_IF_ABRUPT_COMPLETION(thread_);
            }
            GetNextNonSpaceChar();
            if (*current_ == ',') {
                Advance();
            } else if (*current_ == '}') {
                if (inObjorArr || current_ == range_) {
                    return result.GetTaggedValue();
                } else {
                    THROW_SYNTAX_ERROR_AND_RETURN(thread_, "Unexpected Object in JSON", JSTaggedValue::Exception());
                }
            }
        }
        THROW_SYNTAX_ERROR_AND_RETURN(thread_, "Unexpected Object in JSON", JSTaggedValue::Exception());
    }

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
        bool isMatch = MatchText(literalTrue, 3); // 3: literalTrue length - 1
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
        bool isMatch = MatchText(literalFalse, 4); // 4: literalFalse length - 1
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
        bool isMatch = MatchText(literalNull, 3); // 3: literalNull length - 1
        if (LIKELY(isMatch)) {
            return JSTaggedValue::Null();
        }
        THROW_SYNTAX_ERROR_AND_RETURN(thread_, "Unexpected Text in JSON", JSTaggedValue::Exception());
    }

    bool MatchText(const char *str, uint32_t matchLen)
    {
        // first char is already matched
        for (uint32_t pos = 1; pos <= matchLen; ++pos) {
            if (current_[pos] != str[pos]) {
                return false;
            }
        }
        current_ += matchLen;
        return true;
    }

    bool ReadNumberRange(bool &isFast)
    {
        Text current = current_;
        if (*current == '0') {
            isFast = false;
            current++;
        } else if (*current == '-') {
            current++;
            if (*current == '0') {
                isFast = false;
                current++;
            }
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
};

class Utf8JsonParser : public JsonParser<uint8_t> {
public:
    Utf8JsonParser() = default;
    explicit Utf8JsonParser(JSThread *thread) : JsonParser(thread) {}
    ~Utf8JsonParser() = default;
    NO_COPY_SEMANTIC(Utf8JsonParser);
    NO_MOVE_SEMANTIC(Utf8JsonParser);

    JSHandle<JSTaggedValue> Parse(EcmaString *str)
    {
        ASSERT(str != nullptr);
        uint32_t len = EcmaStringAccessor(str).GetLength();
        ASSERT(len != UINT32_MAX);
        CVector<uint8_t> buf(len);
        EcmaStringAccessor(str).WriteToFlatUtf8(buf.data(), len);
        Text begin = buf.data();
        return Launch(begin, begin + len);
    }

private:
    JSTaggedValue ParseString(bool inObjorArr = false) override
    {
        bool isFastString = true;
        bool isLegal = true;
        if (inObjorArr) {
            isLegal = ReadJsonStringRange(isFastString);
            if (!isLegal) {
                THROW_SYNTAX_ERROR_AND_RETURN(thread_, "Unexpected end Text in JSON", JSTaggedValue::Exception());
            }
            if (isFastString) {
                std::string_view value(reinterpret_cast<const char *>(current_), end_ - current_);
                current_ = end_;
                ASSERT(value.size() <= static_cast<size_t>(UINT32_MAX));
                return factory_->NewFromUtf8LiteralCompress(
                    reinterpret_cast<const uint8_t *>(value.data()), value.size()).GetTaggedValue();
            }
        } else {
            if (*end_ != '"' || current_ == end_) {
                THROW_SYNTAX_ERROR_AND_RETURN(thread_, "Unexpected end Text in JSON", JSTaggedValue::Exception());
            }
            isLegal = IsFastParseJsonString(isFastString);
            if (!isLegal) {
                THROW_SYNTAX_ERROR_AND_RETURN(thread_, "Unexpected end Text in JSON", JSTaggedValue::Exception());
            }
            if (LIKELY(isFastString)) {
                std::string_view value(reinterpret_cast<const char *>(current_), end_ - current_);
                ASSERT(value.size() <= static_cast<size_t>(UINT32_MAX));
                return factory_->NewFromUtf8LiteralCompress(
                    reinterpret_cast<const uint8_t *>(value.data()), value.size()).GetTaggedValue();
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
};

class Utf16JsonParser : public JsonParser<uint16_t> {
public:
    Utf16JsonParser() = default;
    explicit Utf16JsonParser(JSThread *thread) : JsonParser(thread) {}
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
    JSTaggedValue ParseString(bool inObjorArr = false) override
    {
        bool isFastString = true;
        bool isAscii = true;
        bool isLegal = true;
        if (inObjorArr) {
            isLegal = ReadJsonStringRange(isFastString, isAscii);
            if (!isLegal) {
                THROW_SYNTAX_ERROR_AND_RETURN(thread_, "Unexpected end Text in JSON", JSTaggedValue::Exception());
            }
            if (isFastString) {
                if (isAscii) {
                    std::string value(current_, end_); // from uint16_t* to std::string, can't use std::string_view
                    current_ = end_;
                    ASSERT(value.size() <= static_cast<size_t>(UINT32_MAX));
                    return factory_->NewFromUtf8LiteralCompress(
                        reinterpret_cast<const uint8_t *>(value.c_str()), value.size()).GetTaggedValue();
                }
                std::u16string_view value(reinterpret_cast<const char16_t *>(current_), end_ - current_);
                current_ = end_;
                ASSERT(value.size() <= static_cast<size_t>(UINT32_MAX));
                return factory_->NewFromUtf16LiteralNotCompress(
                    reinterpret_cast<const uint16_t *>(value.data()), value.size()).GetTaggedValue();
            }
        } else {
            if (*end_ != '"' || current_ == end_) {
                THROW_SYNTAX_ERROR_AND_RETURN(thread_, "Unexpected end Text in JSON", JSTaggedValue::Exception());
            }
            isLegal = IsFastParseJsonString(isFastString, isAscii);
            if (!isLegal) {
                THROW_SYNTAX_ERROR_AND_RETURN(thread_, "Unexpected end Text in JSON", JSTaggedValue::Exception());
            }
            if (LIKELY(isFastString)) {
                if (isAscii) {
                    std::string value(current_, end_);  // from uint16_t* to std::string, can't use std::string_view
                    ASSERT(value.size() <= static_cast<size_t>(UINT32_MAX));
                    return factory_->NewFromUtf8LiteralCompress(
                        reinterpret_cast<const uint8_t *>(value.c_str()), value.size()).GetTaggedValue();
                }
                std::u16string_view value(reinterpret_cast<const char16_t *>(current_), end_ - current_);
                ASSERT(value.size() <= static_cast<size_t>(UINT32_MAX));
                return factory_->NewFromUtf16LiteralNotCompress(
                    reinterpret_cast<const uint16_t *>(value.data()), value.size()).GetTaggedValue();
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
    static JSHandle<JSTaggedValue> InternalizeJsonProperty(JSThread *thread, const JSHandle<JSObject> &holder,
                                                           const JSHandle<JSTaggedValue> &name,
                                                           const JSHandle<JSTaggedValue> &receiver);
private:
    static bool RecurseAndApply(JSThread *thread, const JSHandle<JSObject> &holder, const JSHandle<JSTaggedValue> &name,
                                const JSHandle<JSTaggedValue> &receiver);
};
}  // namespace panda::ecmascript::base

#endif  // ECMASCRIPT_BASE_JSON_PARSE_INL_H
