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

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <ast.h>
#include <token.h>
#include <scanner-character-streams.h>
#include <parser.h>
#include <api.h>
#include <compiler.h>
#include <string>
#include <unistd.h>
#include <utility>
#include <v8.h>
#include "CanonicalAst.h"
#include "DependenceGraph.h"
#include "CodePrinter.h"
#include "NgramExtractor.h"
#include "PDGExtractor.h"
#include "SequenceExtractor.h"
#include "Utility.h"

using namespace std;
using namespace v8::internal;

int main(int argc, char **argv) {
    int opt;
    enum {EXTRACT, PRINT, LIST} mode = EXTRACT;
    enum {PDG, SEQUENCE} type = PDG;
    int n = 3;
    while ((opt = getopt(argc, argv, "pn:ls")) != -1) {
	switch (opt) {
	    case 'p':
		mode = PRINT;
		break;
	    case 'n':
		n = atoi(optarg);
		break;
	    case 'l':
		mode = LIST;
		break;
            case 's':
                type = SEQUENCE;
                break;
	    default:
		cerr << "Invalid option -" << static_cast<char>(opt) << endl;
	}
    }

    v8::Persistent<v8::Context> context = v8::Context::New();
    context->Enter();
    ifstream input(argv[optind]);
    string code((istreambuf_iterator<char>(input)), istreambuf_iterator<char>());
    Utf8ToUtf16CharacterStream stream(reinterpret_cast<const unsigned char*>(code.c_str()), code.length());
    HandleScope handle_scope;
    Handle<Script> script = FACTORY->NewScript(v8::Utils::OpenHandle(*v8::String::New(code.c_str())));
    CompilationInfo info(script, handle_scope.isolate()->runtime_zone());
    info.MarkAsGlobal();
    if (!ParserApi::Parse(&info, kNoParsingFlags)) {
    	cerr << "Cannot parse " << argv[optind] << endl;
	return 1;
    }
    if (info.function()->ast_node_count() == 0) {
    	cerr << "Cannot parse " << argv[optind] << endl;
	return 1;
    }
    ZoneScope zone_scope(handle_scope.isolate()->runtime_zone(), DELETE_ON_EXIT);
    if (!Scope::Analyze(&info)) {
    	cerr << "Cannot parse " << argv[optind] << endl;
	return 1;
    }

    CanonicalAstConverter().Convert(&info);
    DependenceGraphBuilder builder;
    builder.Build(info.function());
    CodePrinter printer(info.function());

    NgramExtractor *extractor = NULL;
    switch (type) {
        case PDG:
            extractor = new PDGExtractor(builder.GetGraph(), mem_fun_less(&printer, &CodePrinter::CompareNode), 40);
            break;

        case SEQUENCE:
            extractor = new SequenceExtractor(key_iterator<DependenceGraph>(builder.GetGraph().begin()),
                                              key_iterator<DependenceGraph>(builder.GetGraph().end()),
                                              mem_fun_less(&printer, &CodePrinter::CompareNode));
            break;
    }

    Statement* node = argv[optind + 1] ? printer.GetLine(atoi(argv[optind + 1])) : NULL;

    switch (mode) {
    	case EXTRACT:
	    if (node) {
	    	string pattern = extractor->Extract(node, n, true);
	    	if (pattern != "") {
		    cout << pattern << endl;
		} else {
		    cerr << "Cannot extract " << n << "-gram for " << argv[optind] << ":" << argv[optind + 1] << endl;
		}
	    } else {
	    	for (size_t i = 1; i <= printer.NumLines(); ++i) {
	    	    node = printer.GetLine(i);
	    	    if (!builder.GetGraph().count(node))
	    	    	continue;
		    string pattern = extractor->Extract(node, n, true);
	    	    if (pattern != "") {
			cout << pattern << '\t' << i << '\t' << printer.GetFuncNo(node) << endl;
		    } else
			cerr << "Cannot extract " << n << "-gram for " << argv[optind] << ":" << i << endl;
		}
	    }
	    break;

	case PRINT:
	    if (node) {
	    	CanonicalFunctionEntry* function = (CanonicalFunctionEntry*)printer.GetLine(printer.GetFuncNo(node));
	    	printer.Print(function->literal(), builder.GetGraph().GetNeighborhood(node, n), builder.GetSuccessors(node));
	    }
	    cout << printer.GetOutput();
	    break;

/*
	case DEPEND:
	    if (node) {
		DependenceGraph neighborhood1 = extractor.FindNeighborhood(node, n);
		for (list<Statement*>::const_iterator j = builder.GetGraph().at(node).begin(); j != builder.GetGraph().at(node).end(); ++j) {
		    const DependenceGraph& neighborhood2 = extractor.FindNeighborhood(*j, n);
		    int weight = 0;
		    for (key_iterator<DependenceGraph> l = neighborhood1.begin(); l != neighborhood1.end(); ++l)
			weight += 1 - neighborhood2.count(*l);
		    for (key_iterator<DependenceGraph> l = neighborhood2.begin(); l != neighborhood2.end(); ++l)
			weight += 1 - neighborhood1.count(*l);
		    cout << printer.GetLineNo(node) << " " << printer.GetLineNo(*j) << " " << weight << endl;
		}
	    } else {
	    	map<Statement*,set<Statement*> > neighbors;
	    	for (key_iterator<DependenceGraph> i = builder.GetGraph().begin(); i != builder.GetGraph().end(); ++i) {
	    	    const DependenceGraph& neighborhood = extractor.FindNeighborhood(*i, n);
	    	    for (key_iterator<DependenceGraph> j = neighborhood.begin(); j != neighborhood.end(); ++j)
	    	    	neighbors[*i].insert(*j);
		}
	    	for (DependenceGraph::const_iterator i = builder.GetGraph().begin(); i != builder.GetGraph().end(); ++i) {
		    for (list<Statement*>::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
		    	int weight = 0;
		    	for (set<Statement*>::iterator l = neighbors[i->first].begin(); l != neighbors[i->first].end(); ++l)
		    	    weight += 1 - neighbors[*j].count(*l);
		    	for (set<Statement*>::iterator l = neighbors[*j].begin(); l != neighbors[*j].end(); ++l)
		    	    weight += 1 - neighbors[i->first].count(*l);
			cout << printer.GetLineNo(i->first) << " " << printer.GetLineNo(*j) << " " << weight << endl;
		    }
		}
	    }
	    break;
*/

	case LIST:
	    for (key_iterator<const map<int,Statement*> > i =  printer.GetFuncList().begin(); i != printer.GetFuncList().end(); ++i) {
	    	cout << *i << " ";
	    	CanonicalFunctionEntry* function = (CanonicalFunctionEntry*)printer.GetLine(*i);
	    	printer.PrintFunc(function->literal());
		cout << printer.GetOutput() << endl;
	    }
    }

    delete extractor;
    context->Exit();
    context.Dispose();

    return 0;
}
