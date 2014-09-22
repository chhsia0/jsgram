// Copyright (C) 2013 The University of Michigan
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Authors - Chun-Hung Hsiao (chhsiao@umich.edu)
//

#ifndef SEQUENCEEXTRACTOR_H
#define SEQUENCEEXTRACTOR_H

#include <algorithm>
#include <map>
#include <string>
#include <vector>
#include "NgramExtractor.h"

using std::map;
using std::sort;
using std::string;
using std::vector;

class SequenceExtractor : public NgramExtractor {
    public:
        template <class Iterator, class Compare> SequenceExtractor(Iterator begin, Iterator end, Compare cmp) {
            while (begin != end)
                sequence_.push_back(*begin++);
            sort(sequence_.begin(), sequence_.end(), cmp);
            for (size_t i = 0; i < sequence_.size(); ++i)
                index_[sequence_[i]] = i;
        }

        string Extract(Statement* node, int n, bool long_desc = false);

    private:
        vector<Statement*> sequence_;
        map<Statement*,int> index_;
};

#endif // SEQUENCEEXTRACTOR_H
