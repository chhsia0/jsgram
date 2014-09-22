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

#ifndef BUILTINS_H
#define BUILTINS_H

#include <set>
#include <string>

using std::set;
using std::string;

class BuiltIns {
    public:
	inline static bool FindFunction(string name) {
            return builtins_.functions_.count(name);
        }
	inline static bool FindMethod(string name) {
            return builtins_.methods_.count(name);
        }
	inline static bool FindConstructor(string name) {
            return builtins_.constructors_.count(name);
        }

    private:
	BuiltIns();

	set<string> functions_;
	set<string> methods_;
	set<string> constructors_;

        static BuiltIns builtins_;
};



#endif // BUILTINTABLE_H
