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

#include "SequenceExtractor.h"

using std::max;

string SequenceExtractor::Extract(Statement* node, int n, bool long_desc) {
    string pattern;

    int index = index_[node];
    int i = max(0, index - n + 1);
    pattern = Serialize(sequence_[i++]);
    while (i <= index)
        pattern += " " + Serialize(sequence_[i++]);

    return pattern;
}
