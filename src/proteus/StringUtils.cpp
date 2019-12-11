//
// Created by Paul on 04.10.2017.
//

#include <proteus/StringUtils.h>
#include <sstream>
#include <iostream>

namespace PAX {
    namespace String {
        void toLower(std::string & str) {
            std::transform(str.begin(), str.end(), str.begin(), ::tolower);
        }

        void toUpper(std::string & str) {
            std::transform(str.begin(), str.end(), str.begin(), ::toupper);
        }

        void ltrim(std::string &s) {
            s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
                return !std::isspace(ch);
            }));
        }

        void rtrim(std::string &s) {
            s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
                return !std::isspace(ch);
            }).base(), s.end());
        }

        void trim(std::string &s) {
            ltrim(s);
            rtrim(s);
        }

        std::string trimmed(std::string s) {
            trim(s);
            return s;
        }

        bool startsWith(const std::string &s, const std::string &prefix) {
            return s.find(prefix) == 0;
        }

        bool endsWith(const std::string &s, const std::string &suffix) {
            return s.size() >= suffix.size() && s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
        }

        std::vector<std::string> split(const char separator, const std::string & subject, bool keepEmptySubstrings) {
            std::vector<std::string> result;
            std::istringstream ss(subject);
            while (!ss.eof())
            {
                std::string field;
                std::getline(ss, field, separator);
                if (!keepEmptySubstrings && field.empty()) continue;
                result.push_back(field);
            }
            return result;
        }

        std::string getExtension(const std::string &path) {
            auto dotIndex = path.find_last_of('.');

            if (dotIndex == std::string::npos)
                return "";

            return path.substr(dotIndex + 1);
        }

        void replace(std::string &string, const char &from, const char &to) {
            std::replace(string.begin(), string.end(), from, to);
        }
    }

    bool TryParser<std::string, bool>::tryParse(const std::string & str) {
        std::string strcopy = str;
        String::toLower(strcopy);

        return
                strcopy == "true"
                || strcopy == "1"
                || strcopy == "on"
                || strcopy == "yes"
                || strcopy == "y";
    }

    int TryParser<std::string, int>::tryParse(const std::string &str) {
        return std::stoi(str);
    }

    unsigned int TryParser<std::string, unsigned int>::tryParse(const std::string &str) {
        return static_cast<unsigned int>(std::stoul(str));
    }

    unsigned long TryParser<std::string, unsigned long>::tryParse(const std::string &str) {
        return std::stoul(str);
    }

    float TryParser<std::string, float>::tryParse(const std::string &str) {
        return std::stof(str);
    }

    double TryParser<std::string, double>::tryParse(const std::string &str) {
        return std::stod(str);
    }

    /*
    template<>
    std::string tryParse<std::string>(const std::string &str) {
        return str;
    }*/
}