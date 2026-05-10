#pragma once
#include <string>
#include <sstream>
#include <iomanip>
#include <boost/multiprecision/cpp_dec_float.hpp>

using 数学高精度 = boost::multiprecision::number<
    boost::multiprecision::cpp_dec_float<100>
>;

inline 数学高精度 toHigh(const std::string& s) {
    return 数学高精度(s);
}

inline std::string 格式化数学高精度(const 数学高精度& v, int 显示位数 = 100) {
    std::ostringstream oss;
    oss << std::setprecision(显示位数) << v;
    std::string s = oss.str();

    if (s.find('.') != std::string::npos) {
        while (!s.empty() && s.back() == '0') s.pop_back();
        if (!s.empty() && s.back() == '.') s.pop_back();
    }
    return s;
}
