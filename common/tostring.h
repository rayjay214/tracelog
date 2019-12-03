#ifndef _GM_TO_STRING_H_
#define _GM_TO_STRING_H_

#include <sstream>
#include <set>
#include <vector>
#include <iomanip>

template<typename T>
std::string ToString(T value)
{
    std::ostringstream oss;

    oss.precision(11);
    oss << value;
    return oss.str();
}

template<typename T>
std::string LngLatToString(T value, int accuracy = 6)
{
    std::ostringstream oss;
    size_t len = ToString((int)value).size();
    oss.precision(len + accuracy);
    oss << value;
    return oss.str();
}

template<typename T>
std::string DoubleFixToString(T value, int fixNum)
{   
    std::ostringstream oss;
    oss << setiosflags(std::ios::fixed);
    oss.precision(fixNum);
    oss << value;
    return oss.str();
}

template<typename T>
std::string ToString(const std::set<T> & values, const std::string & strDelimiter)
{
    std::string strRet;
    for (typename std::set<T>::const_iterator it = values.begin(); it != values.end(); ++it)
    {
        strRet += strRet.empty() ? ToString(*it) : strDelimiter + ToString(*it);
    }
    return strRet;
}

template<typename T>
std::string ToString(const std::vector<T> & values, const std::string & strDelimiter)
{
    std::string strRet;
    for (typename std::vector<T>::const_iterator it = values.begin(); it != values.end(); ++it)
    {
        strRet += strRet.empty() ? ToString(*it) : strDelimiter + ToString(*it);
    }
    return strRet;
}


#endif
