// Copyright (C) 2016 Joywinds Inc.

#pragma once

#include <string>
#include <vector>
#include <unordered_map>

//#include "RWLock.h"

namespace joywinds {

    class SkipList {
    public:
        SkipList();

        ~SkipList();

        struct Element {
            int score;
            std::string key;
            std::string value;
            Element() : score(0) {
            }
        };

        void insert(const std::string& key, const std::string& value, int score);

        void remove(const std::string& key);

        bool addScore(const std::string& key, int score);

        bool addScoreWithValue(const std::string& key, const std::string& value, int score);

        bool getRank(const std::string& key, bool reverse, int& rank);

        bool getScore(const std::string& key, int& score);

        bool getElementByKey(const std::string& key, Element& result);

        bool getElementByRank(int rank, Element& result, bool reverse);

        void getElementsByRank(const std::vector<int>& ranks, std::vector<Element>& results, bool reverse);

        void getElementsByRange(int start, int end, std::vector<Element>& results, bool reverse);

        int getTotal();

    private:
        //RWLock lock_;
        struct zskiplist* sl_;
        std::unordered_map<std::string, Element> dict_;
    };

} // namespace joywinds