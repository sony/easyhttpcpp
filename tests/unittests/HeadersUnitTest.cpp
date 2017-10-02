/*
 * Copyright 2017 Sony Corporation
 */

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "Poco/ListMap.h"

#include "easyhttpcpp/Headers.h"
#include "easyhttpcpp/HttpException.h"
#include "EasyHttpCppAssertions.h"

namespace easyhttpcpp {
namespace test {

static const std::string HeaderName = "X-My-Header-Name";
static const std::string HeaderNameInLowerCase = "x-my-header-name";
static const std::string HeaderNameInUpperCase = "X-MY-HEADER-NAME";
static const std::string HeaderName1 = "X-My-Header-Name1";
static const std::string HeaderName1InLowerCase = "x-my-header-name1";
static const std::string HeaderName1InUpperCase = "X-MY-HEADER-NAME1";
static const std::string HeaderName2 = "X-My-Header-Name2";
static const std::string HeaderName2InLowerCase = "x-my-header-name2";
static const std::string HeaderName2InUpperCase = "X-MY-HEADER-NAME2";
static const std::string HeaderValueFoo = "foo";
static const std::string HeaderValueBar = "bar";
static const std::string HeaderValueBaz = "baz";
static const std::string HeaderDefaultValue = "default";

TEST(HeadersUnitTest, constructor_ReturnsInstance)
{
    // Given: none
    // When: call Headers()
    Headers headers;

    // Then: size is 0
    EXPECT_EQ(0, headers.getSize());
    EXPECT_EQ("", headers.toString());
}

TEST(HeadersUnitTest, copyConstructor_CopiesAllProperties)
{
    // Given: create Headers instance
    Headers original;
    original.add(HeaderName, HeaderValueFoo);
    original.add(HeaderName1, HeaderValueBar);
    original.add(HeaderName2, HeaderValueBaz);

    // When: call Headers() with Headers instance
    Headers headers(original);

    // Then: all elements are copied
    EXPECT_EQ(original.getSize(), headers.getSize());
    EXPECT_EQ(original.getValue(HeaderName, HeaderDefaultValue), headers.getValue(HeaderName, HeaderDefaultValue));
    EXPECT_EQ(original.getValue(HeaderName1, HeaderDefaultValue), headers.getValue(HeaderName1, HeaderDefaultValue));
    EXPECT_EQ(original.getValue(HeaderName2, HeaderDefaultValue), headers.getValue(HeaderName2, HeaderDefaultValue));
    EXPECT_EQ(original.toString(), headers.toString());
}

TEST(HeadersUnitTest, assignmentOperator_CopiesElements_WhenOtherInstance)
{
    // Given: create Headers instance
    Headers headers1;
    headers1.add(HeaderName, HeaderValueFoo);
    headers1.add(HeaderName1, HeaderValueBar);
    headers1.add(HeaderName2, HeaderValueBaz);

    // When: operator =
    Headers headers2;
    headers2 = headers1;

    // Then: copies elements
    EXPECT_EQ(headers1.getSize(), headers2.getSize());
    EXPECT_TRUE(headers2.has(HeaderName));
    EXPECT_TRUE(headers2.has(HeaderName1));
    EXPECT_TRUE(headers2.has(HeaderName2));
    EXPECT_EQ(headers1.getValue(HeaderName, HeaderDefaultValue), headers2.getValue(HeaderName, HeaderDefaultValue));
    EXPECT_EQ(headers1.getValue(HeaderName1, HeaderDefaultValue), headers2.getValue(HeaderName1, HeaderDefaultValue));
    EXPECT_EQ(headers1.getValue(HeaderName2, HeaderDefaultValue), headers2.getValue(HeaderName2, HeaderDefaultValue));
    EXPECT_EQ(headers1.toString(), headers2.toString());
}

TEST(HeadersUnitTest, assignmentOperator_NoCopiesElements_WhenSameInstance)
{
    // Given: create Headers instance
    Headers headers1;
    headers1.add(HeaderName, HeaderValueFoo);
    headers1.add(HeaderName1, HeaderValueBar);
    headers1.add(HeaderName2, HeaderValueBaz);

    // When: operator =
    headers1 = headers1;

    // Then: no copies elements
    EXPECT_EQ(3, headers1.getSize());
    EXPECT_TRUE(headers1.has(HeaderName));
    EXPECT_TRUE(headers1.has(HeaderName1));
    EXPECT_TRUE(headers1.has(HeaderName2));
    EXPECT_EQ(HeaderValueFoo, headers1.getValue(HeaderName, ""));
    EXPECT_EQ(HeaderValueBar, headers1.getValue(HeaderName1, ""));
    EXPECT_EQ(HeaderValueBaz, headers1.getValue(HeaderName2, ""));
}

TEST(HeadersUnitTest, add_StoresElement)
{
    // Given: none
    Headers headers;

    // When: call add()
    headers.add(HeaderName, HeaderValueFoo);
    headers.add(HeaderName1, HeaderValueBar);
    headers.add(HeaderName2, HeaderValueBaz);

    // Then: elements are stored
    EXPECT_EQ(3, headers.getSize());
    EXPECT_TRUE(headers.has(HeaderName));
    EXPECT_TRUE(headers.has(HeaderNameInLowerCase));
    EXPECT_TRUE(headers.has(HeaderNameInUpperCase));
    EXPECT_EQ(HeaderValueFoo, headers.getValue(HeaderName, ""));
    EXPECT_EQ(HeaderValueFoo, headers.getValue(HeaderNameInLowerCase, ""));
    EXPECT_EQ(HeaderValueFoo, headers.getValue(HeaderNameInUpperCase, ""));
    EXPECT_TRUE(headers.has(HeaderName1));
    EXPECT_TRUE(headers.has(HeaderName1InLowerCase));
    EXPECT_TRUE(headers.has(HeaderName1InUpperCase));
    EXPECT_EQ(HeaderValueBar, headers.getValue(HeaderName1, ""));
    EXPECT_EQ(HeaderValueBar, headers.getValue(HeaderName1InLowerCase, ""));
    EXPECT_EQ(HeaderValueBar, headers.getValue(HeaderName1InUpperCase, ""));
    EXPECT_TRUE(headers.has(HeaderName2));
    EXPECT_TRUE(headers.has(HeaderName2InLowerCase));
    EXPECT_TRUE(headers.has(HeaderName2InUpperCase));
    EXPECT_EQ(HeaderValueBaz, headers.getValue(HeaderName2, ""));
    EXPECT_EQ(HeaderValueBaz, headers.getValue(HeaderName2InLowerCase, ""));
    EXPECT_EQ(HeaderValueBaz, headers.getValue(HeaderName2InUpperCase, ""));
    EXPECT_THAT(headers.toString(), testing::ContainsRegex(std::string(HeaderName + ":" + HeaderValueFoo + "\n")));
    EXPECT_THAT(headers.toString(), testing::ContainsRegex(std::string(HeaderName1 + ":" + HeaderValueBar + "\n")));
    EXPECT_THAT(headers.toString(), testing::ContainsRegex(std::string(HeaderName2 + ":" + HeaderValueBaz + "\n")));
}

TEST(HeadersUnitTest, add_StoresElement_WhenHeaderNameIsLowerCase)
{
    // Given: none
    Headers headers;

    // When: call add()
    headers.add(HeaderNameInLowerCase, HeaderValueFoo);
    headers.add(HeaderName1InLowerCase, HeaderValueBar);
    headers.add(HeaderName2InLowerCase, HeaderValueBaz);

    // Then: elements are stored
    EXPECT_EQ(3, headers.getSize());
    EXPECT_TRUE(headers.has(HeaderName));
    EXPECT_TRUE(headers.has(HeaderNameInLowerCase));
    EXPECT_TRUE(headers.has(HeaderNameInUpperCase));
    EXPECT_EQ(HeaderValueFoo, headers.getValue(HeaderName, ""));
    EXPECT_EQ(HeaderValueFoo, headers.getValue(HeaderNameInLowerCase, ""));
    EXPECT_EQ(HeaderValueFoo, headers.getValue(HeaderNameInUpperCase, ""));
    EXPECT_TRUE(headers.has(HeaderName1));
    EXPECT_TRUE(headers.has(HeaderName1InLowerCase));
    EXPECT_TRUE(headers.has(HeaderName1InUpperCase));
    EXPECT_EQ(HeaderValueBar, headers.getValue(HeaderName1, ""));
    EXPECT_EQ(HeaderValueBar, headers.getValue(HeaderName1InLowerCase, ""));
    EXPECT_EQ(HeaderValueBar, headers.getValue(HeaderName1InUpperCase, ""));
    EXPECT_TRUE(headers.has(HeaderName2));
    EXPECT_TRUE(headers.has(HeaderName2InLowerCase));
    EXPECT_TRUE(headers.has(HeaderName2InUpperCase));
    EXPECT_EQ(HeaderValueBaz, headers.getValue(HeaderName2, ""));
    EXPECT_EQ(HeaderValueBaz, headers.getValue(HeaderName2InLowerCase, ""));
    EXPECT_EQ(HeaderValueBaz, headers.getValue(HeaderName2InUpperCase, ""));
    EXPECT_THAT(headers.toString(),
            testing::ContainsRegex(std::string(HeaderNameInLowerCase + ":" + HeaderValueFoo + "\n")));
    EXPECT_THAT(headers.toString(),
            testing::ContainsRegex(std::string(HeaderName1InLowerCase + ":" + HeaderValueBar + "\n")));
    EXPECT_THAT(headers.toString(),
            testing::ContainsRegex(std::string(HeaderName2InLowerCase + ":" + HeaderValueBaz + "\n")));
}

TEST(HeadersUnitTest, add_StoresElement_WhenHeaderNameIsUpperCase)
{
    // Given: none
    Headers headers;

    // When: call add()
    headers.add(HeaderNameInUpperCase, HeaderValueFoo);
    headers.add(HeaderName1InUpperCase, HeaderValueBar);
    headers.add(HeaderName2InUpperCase, HeaderValueBaz);

    // Then: elements are stored
    EXPECT_EQ(3, headers.getSize());
    EXPECT_TRUE(headers.has(HeaderName));
    EXPECT_TRUE(headers.has(HeaderNameInLowerCase));
    EXPECT_TRUE(headers.has(HeaderNameInUpperCase));
    EXPECT_EQ(HeaderValueFoo, headers.getValue(HeaderName, ""));
    EXPECT_EQ(HeaderValueFoo, headers.getValue(HeaderNameInLowerCase, ""));
    EXPECT_EQ(HeaderValueFoo, headers.getValue(HeaderNameInUpperCase, ""));
    EXPECT_TRUE(headers.has(HeaderName1));
    EXPECT_TRUE(headers.has(HeaderName1InLowerCase));
    EXPECT_TRUE(headers.has(HeaderName1InUpperCase));
    EXPECT_EQ(HeaderValueBar, headers.getValue(HeaderName1, ""));
    EXPECT_EQ(HeaderValueBar, headers.getValue(HeaderName1InLowerCase, ""));
    EXPECT_EQ(HeaderValueBar, headers.getValue(HeaderName1InUpperCase, ""));
    EXPECT_TRUE(headers.has(HeaderName2));
    EXPECT_TRUE(headers.has(HeaderName2InLowerCase));
    EXPECT_TRUE(headers.has(HeaderName2InUpperCase));
    EXPECT_EQ(HeaderValueBaz, headers.getValue(HeaderName2, ""));
    EXPECT_EQ(HeaderValueBaz, headers.getValue(HeaderName2InLowerCase, ""));
    EXPECT_EQ(HeaderValueBaz, headers.getValue(HeaderName2InUpperCase, ""));
    EXPECT_THAT(headers.toString(),
            testing::ContainsRegex(std::string(HeaderNameInUpperCase + ":" + HeaderValueFoo + "\n")));
    EXPECT_THAT(headers.toString(),
            testing::ContainsRegex(std::string(HeaderName1InUpperCase + ":" + HeaderValueBar + "\n")));
    EXPECT_THAT(headers.toString(),
            testing::ContainsRegex(std::string(HeaderName2InUpperCase + ":" + HeaderValueBaz + "\n")));
}

TEST(HeadersUnitTest, add_StoresElementAndGetValueReturnsStoredFirstElement_WhenCalledWithSameName)
{
    // Given: none
    Headers headers;

    // When: call add()
    headers.add(HeaderName, HeaderValueFoo);
    headers.add(HeaderName, HeaderValueBar);
    headers.add(HeaderName, HeaderValueBaz);

    // Then: elements are stored
    EXPECT_EQ(3, headers.getSize());
    EXPECT_TRUE(headers.has(HeaderName));
    EXPECT_TRUE(headers.has(HeaderNameInLowerCase));
    EXPECT_TRUE(headers.has(HeaderNameInUpperCase));
    EXPECT_EQ(HeaderValueFoo, headers.getValue(HeaderName, ""));
    EXPECT_EQ(HeaderValueFoo, headers.getValue(HeaderNameInLowerCase, ""));
    EXPECT_EQ(HeaderValueFoo, headers.getValue(HeaderNameInUpperCase, ""));
    EXPECT_THAT(headers.toString(), testing::ContainsRegex(std::string(HeaderName + ":" + HeaderValueFoo + "\n")));
    EXPECT_THAT(headers.toString(), testing::ContainsRegex(std::string(HeaderName + ":" + HeaderValueBar + "\n")));
    EXPECT_THAT(headers.toString(), testing::ContainsRegex(std::string(HeaderName + ":" + HeaderValueBaz + "\n")));
}

TEST(HeadersUnitTest, add_StoresElementAndGetValueReturnsStoredFirstElementWithCaseInsensitive_WhenCalledWithSameNameButDifferentLetterCase)
{
    // Given: none
    Headers headers;

    // When: call add()
    headers.add(HeaderName, HeaderValueFoo);
    headers.add(HeaderNameInLowerCase, HeaderValueBar);
    headers.add(HeaderNameInUpperCase, HeaderValueBaz);

    // Then: elements are stored
    EXPECT_EQ(3, headers.getSize());
    EXPECT_TRUE(headers.has(HeaderName));
    EXPECT_TRUE(headers.has(HeaderNameInLowerCase));
    EXPECT_TRUE(headers.has(HeaderNameInUpperCase));
    EXPECT_EQ(HeaderValueFoo, headers.getValue(HeaderName, ""));
    EXPECT_EQ(HeaderValueFoo, headers.getValue(HeaderNameInLowerCase, ""));
    EXPECT_EQ(HeaderValueFoo, headers.getValue(HeaderNameInUpperCase, ""));
    EXPECT_THAT(headers.toString(),
            testing::ContainsRegex(std::string(HeaderName + ":" + HeaderValueFoo + "\n")));
    EXPECT_THAT(headers.toString(),
            testing::ContainsRegex(std::string(HeaderNameInLowerCase + ":" + HeaderValueBar + "\n")));
    EXPECT_THAT(headers.toString(),
            testing::ContainsRegex(std::string(HeaderNameInUpperCase + ":" + HeaderValueBaz + "\n")));
}

TEST(HeadersUnitTest, add_ThrowsHttpIllegalArgumentException_WhenNameIsEmpty)
{
    // Given: none
    Headers headers;

    // When: call add() with empty string
    // Then: throw exception
    EASYHTTPCPP_EXPECT_THROW(headers.add("", HeaderValueFoo), HttpIllegalArgumentException, 100700);
}

TEST(HeadersUnitTest, set_StoresElement)
{
    // Given: none
    Headers headers;

    // When: call set()
    headers.set(HeaderName, HeaderValueFoo);
    headers.set(HeaderName1, HeaderValueBar);
    headers.set(HeaderName2, HeaderValueBaz);

    // Then: elements are stored
    EXPECT_EQ(3, headers.getSize());
    EXPECT_TRUE(headers.has(HeaderName));
    EXPECT_TRUE(headers.has(HeaderNameInLowerCase));
    EXPECT_TRUE(headers.has(HeaderNameInUpperCase));
    EXPECT_EQ(HeaderValueFoo, headers.getValue(HeaderName, ""));
    EXPECT_EQ(HeaderValueFoo, headers.getValue(HeaderNameInLowerCase, ""));
    EXPECT_EQ(HeaderValueFoo, headers.getValue(HeaderNameInUpperCase, ""));
    EXPECT_TRUE(headers.has(HeaderName1));
    EXPECT_TRUE(headers.has(HeaderName1InLowerCase));
    EXPECT_TRUE(headers.has(HeaderName1InUpperCase));
    EXPECT_EQ(HeaderValueBar, headers.getValue(HeaderName1, ""));
    EXPECT_EQ(HeaderValueBar, headers.getValue(HeaderName1InLowerCase, ""));
    EXPECT_EQ(HeaderValueBar, headers.getValue(HeaderName1InUpperCase, ""));
    EXPECT_TRUE(headers.has(HeaderName2));
    EXPECT_TRUE(headers.has(HeaderName2InLowerCase));
    EXPECT_TRUE(headers.has(HeaderName2InUpperCase));
    EXPECT_EQ(HeaderValueBaz, headers.getValue(HeaderName2, ""));
    EXPECT_EQ(HeaderValueBaz, headers.getValue(HeaderName2InLowerCase, ""));
    EXPECT_EQ(HeaderValueBaz, headers.getValue(HeaderName2InUpperCase, ""));
    EXPECT_THAT(headers.toString(), testing::ContainsRegex(std::string(HeaderName + ":" + HeaderValueFoo + "\n")));
    EXPECT_THAT(headers.toString(), testing::ContainsRegex(std::string(HeaderName1 + ":" + HeaderValueBar + "\n")));
    EXPECT_THAT(headers.toString(), testing::ContainsRegex(std::string(HeaderName2 + ":" + HeaderValueBaz + "\n")));
}

TEST(HeadersUnitTest, set_StoresElement_WhenHeaderNameIsLowerCase)
{
    // Given: none
    Headers headers;

    // When: call set()
    headers.set(HeaderNameInLowerCase, HeaderValueFoo);
    headers.set(HeaderName1InLowerCase, HeaderValueBar);
    headers.set(HeaderName2InLowerCase, HeaderValueBaz);

    // Then: elements are stored
    EXPECT_EQ(3, headers.getSize());
    EXPECT_TRUE(headers.has(HeaderName));
    EXPECT_TRUE(headers.has(HeaderNameInLowerCase));
    EXPECT_TRUE(headers.has(HeaderNameInUpperCase));
    EXPECT_EQ(HeaderValueFoo, headers.getValue(HeaderName, ""));
    EXPECT_EQ(HeaderValueFoo, headers.getValue(HeaderNameInLowerCase, ""));
    EXPECT_EQ(HeaderValueFoo, headers.getValue(HeaderNameInUpperCase, ""));
    EXPECT_TRUE(headers.has(HeaderName1));
    EXPECT_TRUE(headers.has(HeaderName1InLowerCase));
    EXPECT_TRUE(headers.has(HeaderName1InUpperCase));
    EXPECT_EQ(HeaderValueBar, headers.getValue(HeaderName1, ""));
    EXPECT_EQ(HeaderValueBar, headers.getValue(HeaderName1InLowerCase, ""));
    EXPECT_EQ(HeaderValueBar, headers.getValue(HeaderName1InUpperCase, ""));
    EXPECT_TRUE(headers.has(HeaderName2));
    EXPECT_TRUE(headers.has(HeaderName2InLowerCase));
    EXPECT_TRUE(headers.has(HeaderName2InUpperCase));
    EXPECT_EQ(HeaderValueBaz, headers.getValue(HeaderName2, ""));
    EXPECT_EQ(HeaderValueBaz, headers.getValue(HeaderName2InLowerCase, ""));
    EXPECT_EQ(HeaderValueBaz, headers.getValue(HeaderName2InUpperCase, ""));
    EXPECT_THAT(headers.toString(),
            testing::ContainsRegex(std::string(HeaderNameInLowerCase + ":" + HeaderValueFoo + "\n")));
    EXPECT_THAT(headers.toString(),
            testing::ContainsRegex(std::string(HeaderName1InLowerCase + ":" + HeaderValueBar + "\n")));
    EXPECT_THAT(headers.toString(),
            testing::ContainsRegex(std::string(HeaderName2InLowerCase + ":" + HeaderValueBaz + "\n")));
}

TEST(HeadersUnitTest, set_StoresElement_WhenHeaderNameIsUpperCase)
{
    // Given: none
    Headers headers;

    // When: call set()
    headers.set(HeaderNameInUpperCase, HeaderValueFoo);
    headers.set(HeaderName1InUpperCase, HeaderValueBar);
    headers.set(HeaderName2InUpperCase, HeaderValueBaz);

    // Then: elements are stored
    EXPECT_EQ(3, headers.getSize());
    EXPECT_TRUE(headers.has(HeaderName));
    EXPECT_TRUE(headers.has(HeaderNameInLowerCase));
    EXPECT_TRUE(headers.has(HeaderNameInUpperCase));
    EXPECT_EQ(HeaderValueFoo, headers.getValue(HeaderName, ""));
    EXPECT_EQ(HeaderValueFoo, headers.getValue(HeaderNameInLowerCase, ""));
    EXPECT_EQ(HeaderValueFoo, headers.getValue(HeaderNameInUpperCase, ""));
    EXPECT_TRUE(headers.has(HeaderName1));
    EXPECT_TRUE(headers.has(HeaderName1InLowerCase));
    EXPECT_TRUE(headers.has(HeaderName1InUpperCase));
    EXPECT_EQ(HeaderValueBar, headers.getValue(HeaderName1, ""));
    EXPECT_EQ(HeaderValueBar, headers.getValue(HeaderName1InLowerCase, ""));
    EXPECT_EQ(HeaderValueBar, headers.getValue(HeaderName1InUpperCase, ""));
    EXPECT_TRUE(headers.has(HeaderName2));
    EXPECT_TRUE(headers.has(HeaderName2InLowerCase));
    EXPECT_TRUE(headers.has(HeaderName2InUpperCase));
    EXPECT_EQ(HeaderValueBaz, headers.getValue(HeaderName2, ""));
    EXPECT_EQ(HeaderValueBaz, headers.getValue(HeaderName2InLowerCase, ""));
    EXPECT_EQ(HeaderValueBaz, headers.getValue(HeaderName2InUpperCase, ""));
    EXPECT_THAT(headers.toString(),
            testing::ContainsRegex(std::string(HeaderNameInUpperCase + ":" + HeaderValueFoo + "\n")));
    EXPECT_THAT(headers.toString(),
            testing::ContainsRegex(std::string(HeaderName1InUpperCase + ":" + HeaderValueBar + "\n")));
    EXPECT_THAT(headers.toString(),
            testing::ContainsRegex(std::string(HeaderName2InUpperCase + ":" + HeaderValueBaz + "\n")));
}

TEST(HeadersUnitTest, set_StoresElementAndGetValueReturnsStoredLastElement_WhenCalledWithSameName)
{
    // Given: none
    Headers headers;

    // When: call set()
    headers.set(HeaderName, HeaderValueFoo);
    headers.set(HeaderName, HeaderValueBar);
    headers.set(HeaderName, HeaderValueBaz);

    // Then: elements are stored
    EXPECT_EQ(1, headers.getSize());
    EXPECT_TRUE(headers.has(HeaderName));
    EXPECT_TRUE(headers.has(HeaderNameInLowerCase));
    EXPECT_TRUE(headers.has(HeaderNameInUpperCase));
    EXPECT_EQ(HeaderValueBaz, headers.getValue(HeaderName, ""));
    EXPECT_EQ(HeaderValueBaz, headers.getValue(HeaderNameInLowerCase, ""));
    EXPECT_EQ(HeaderValueBaz, headers.getValue(HeaderNameInUpperCase, ""));
    EXPECT_EQ(std::string(HeaderName + ":" + HeaderValueBaz + "\n"), headers.toString());
}

TEST(HeadersUnitTest, set_StoresElementAndGetValueReturnsStoredLastElementWithCaseInsensitive_WhenCalledWithSameNameButDifferentLetterCase)
{
    // Given: none
    Headers headers;

    // When: call set()
    headers.set(HeaderName, HeaderValueFoo);
    headers.set(HeaderNameInLowerCase, HeaderValueBar);
    headers.set(HeaderNameInUpperCase, HeaderValueBaz);

    // Then: elements are stored
    EXPECT_EQ(1, headers.getSize());
    EXPECT_TRUE(headers.has(HeaderName));
    EXPECT_TRUE(headers.has(HeaderNameInLowerCase));
    EXPECT_TRUE(headers.has(HeaderNameInUpperCase));
    EXPECT_EQ(HeaderValueBaz, headers.getValue(HeaderName, ""));
    EXPECT_EQ(HeaderValueBaz, headers.getValue(HeaderNameInLowerCase, ""));
    EXPECT_EQ(HeaderValueBaz, headers.getValue(HeaderNameInUpperCase, ""));
    EXPECT_EQ(std::string(HeaderNameInUpperCase + ":" + HeaderValueBaz + "\n"), headers.toString());
}

TEST(HeadersUnitTest, set_ThrowsHttpIllegalArgumentException_WhenNameIsEmpty)
{
    // Given: set element
    Headers headers;

    // When: call add() with empty string
    // Then: throw exception
    EASYHTTPCPP_EXPECT_THROW(headers.set("", HeaderValueFoo), HttpIllegalArgumentException, 100700);
}

TEST(HeadersUnitTest, getValue_ReturnsValue_WhenElementIsFound)
{
    // Given: set element
    Headers headers;
    headers.set(HeaderName, HeaderValueFoo);

    // When: call getValue()
    // Then: returns value
    EXPECT_EQ(HeaderValueFoo, headers.getValue(HeaderName, ""));
}

TEST(HeadersUnitTest, getValue_ReturnsDefaultValue_WhenElementIsNotFound)
{
    // Given: set element
    Headers headers;
    headers.set(HeaderName, HeaderValueFoo);

    // When: call getValue()
    // Then: returns default value
    EXPECT_EQ(HeaderDefaultValue, headers.getValue(HeaderName1, HeaderDefaultValue));
}

TEST(HeadersUnitTest, getValue_ReturnsDefaultValue_WhenNameIsEmpty)
{
    // Given: set element
    Headers headers;
    headers.set(HeaderName, HeaderValueFoo);

    // When: call add() with empty string
    // Then: throw exception
    EXPECT_EQ(HeaderDefaultValue, headers.getValue("", HeaderDefaultValue));
}

TEST(HeadersUnitTest, has_ReturnsTrue_WhenElementIsFound)
{
    // Given: set element
    Headers headers;
    headers.set(HeaderName, HeaderValueFoo);

    // When: call has()
    // Then: returns true
    EXPECT_TRUE(headers.has(HeaderName));
}

TEST(HeadersUnitTest, has_ReturnsFalse_WhenElementIsNotFound)
{
    // Given: set element
    Headers headers;
    headers.set(HeaderName, HeaderValueFoo);

    // When: call has()
    // Then: returns false
    EXPECT_FALSE(headers.has(HeaderName1));
}

TEST(HeadersUnitTest, has_ReturnsFalse_WhenNameIsEmpty)
{
    // Given: set element
    Headers headers;
    headers.set(HeaderName, HeaderValueFoo);

    // When: call has()
    // Then: returns false
    EXPECT_FALSE(headers.has(""));
}

TEST(HeadersUnitTest, getSize_ReturnsExpectedValue)
{
    // Given: set elements
    Headers headers;
    headers.set(HeaderName, HeaderValueFoo);
    headers.set(HeaderName1, HeaderValueFoo);
    headers.set(HeaderName2, HeaderValueFoo);

    // When: call getSize()
    // Then: returns number of added elements
    EXPECT_EQ(3, headers.getSize());
}

TEST(HeadersUnitTest, empty_ReturnsTrue_WhenHeadersHasNoElement)
{
    // Given: not set elements
    Headers headers;

    // When: call empty()
    // Then: returns true
    EXPECT_TRUE(headers.empty());
}

TEST(HeadersUnitTest, empty_ReturnsFalse_WhenHeadersHasElements)
{
    // Given: set elements
    Headers headers;
    headers.set(HeaderName, HeaderValueFoo);
    headers.set(HeaderName1, HeaderValueFoo);
    headers.set(HeaderName2, HeaderValueFoo);

    // When: call empty()
    // Then: returns false
    EXPECT_FALSE(headers.empty());
}

TEST(HeadersUnitTest, beginAndEnd_ReturnsIteratorInstanceUntilEndOfElements)
{
    // Given: set elements
    Headers headers;
    headers.set(HeaderName, HeaderValueFoo);
    headers.set(HeaderName1, HeaderValueFoo);
    headers.set(HeaderName2, HeaderValueFoo);

    size_t size = 0;
    // When: call begin() and end()
    // Then: returns iterator instance until end of elements
    for (Headers::HeaderMap::ConstIterator it = headers.begin(); it != headers.end(); it++) {
        size++;
        EXPECT_TRUE(headers.has(it->first));
        EXPECT_EQ(headers.getValue(it->first, HeaderDefaultValue), it->second);
    }
    EXPECT_EQ(headers.getSize(), size);
}

TEST(HeadersUnitTest, beginAndEnd_ReturnsIteratorInstanceUntilEndOfElements_WhenNoElements)
{
    // Given: none
    Headers headers;

    size_t size = 0;
    // When: call begin() and end()
    // Then: returns iterator instance until end of elements
    for (Headers::HeaderMap::ConstIterator it = headers.begin(); it != headers.end(); it++) {
        size++;
    }
    EXPECT_EQ(0, size);
}

} /* namespace test */
} /* namespace easyhttpcpp */

