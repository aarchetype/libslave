#ifndef __SLAVE_TYPES_H
#define __SLAVE_TYPES_H

#include <inttypes.h>
#include <string>
#include <time.h>

#include "decimal.h"

// conflict with macro defined in mysql
#ifdef test
#undef test
#endif /* test */

#ifdef SLAVE_USE_VARIANT_FOR_FIELD_VALUE
#include <cstddef>              // for std::nullptr_t
#include <boost/variant.hpp>
#else
#include <boost/any.hpp>
#endif

namespace slave {
namespace types
{
    typedef uint32_t            MY_INT;
    typedef unsigned long long  MY_BIGINT;
    typedef uint32_t            MY_MEDIUMINT;
    typedef uint16_t            MY_SMALLINT;
    typedef char                MY_TINYINT;
    typedef uint64_t            MY_BIT;
    typedef int                 MY_ENUM;
    typedef unsigned long long  MY_SET;
    typedef float               MY_FLOAT;
    typedef double              MY_DOUBLE;
    typedef decimal::Decimal    MY_DECIMAL;
    typedef uint32_t            MY_DATE;
    typedef int32_t             MY_TIME;
    typedef unsigned long long  MY_DATETIME;
    typedef uint32_t            MY_TIMESTAMP;
    typedef std::string         MY_CHAR;
    typedef std::string         MY_VARCHAR;
    typedef std::string         MY_TINYTEXT;
    typedef std::string         MY_TEXT;
    typedef std::string         MY_MEDIUMTEXT;
    typedef std::string         MY_LONGTEXT;
    typedef std::string         MY_BLOB;

    // NOTE you should call tzset directly or indirectly before using any of these functions
    // for proper initialization of daylight variable

    // MY_TIME value is represented as a 32-bit number similar to this: [+/-]HHHMMSS (HHH - hours, MM - minutes, SS - seconds)
    // with range from -8385959 to 8385959

    // Converts date from slave to timestamp assuming date is specified in local timezone.
    inline time_t date2time(MY_DATE date)
    {
        // date value consists of a day in first 5 bits (values from 1 to 31), a month in the next 4 bits (values from 1 to 12), and a year in the rest of value

        // For '0000-00-00' return 0 immediately
        if (0 == date)
            return 0;

        struct tm t;
        memset(&t, 0, sizeof(tm));
        t.tm_year = (date >> 9) - 1900;
        t.tm_mon = (date >> 5) % (1 << 4) - 1;
        t.tm_mday = date % (1 << 5);
        t.tm_isdst = daylight;

        return mktime(&t);
    }

    // Converts date and time from slave to timestamp assuming date is specified in local timezone.
    inline time_t datetime2time(MY_DATETIME datetime)
    {
        // datetime value is represented as a 64-bit number similar to this: 20110313094909

        if (0 == datetime)
            return 0;

        struct tm t;
        memset(&t, 0, sizeof(tm));
        t.tm_year = (datetime / 10000000000) - 1900;
        t.tm_mon =  (datetime / 100000000) % 100 - 1;
        t.tm_mday = (datetime / 1000000) % 100;
        t.tm_hour = (datetime / 10000) % 100;
        t.tm_min =  (datetime / 100) % 100;
        t.tm_sec =   datetime % 100;
        t.tm_isdst = daylight;

        return mktime(&t);
    }
}// types

enum class RowType {
    Map,
    Vector
};

#ifdef SLAVE_USE_VARIANT_FOR_FIELD_VALUE
    using FieldValue = boost::variant<std::nullptr_t
                                    , int
                                    , char
                                    , uint16_t
                                    , int32_t
                                    , uint32_t
                                    , unsigned long long
                                    , float
                                    , double
                                    , std::string
                                    , decimal::Decimal
                                    >;
    inline std::nullptr_t nullFieldValue() { return nullptr; }
    inline bool isNullFieldValue(const FieldValue& v) { return v.type() == typeid(std::nullptr_t); }
    template <typename T>
    const T& get(const FieldValue& v) { return boost::get<T>(v); }
#else
    using FieldValue = boost::any;
    inline boost::any nullFieldValue() { return boost::any(); }
    inline bool isNullFieldValue(const FieldValue& v) { return v.empty(); }
    template <typename T>
    T get(const FieldValue& v) { return boost::any_cast<T>(v); }
#endif

}// slave

#endif
