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

#include "CodePrinter.h"

#include <v8.h>
#include <scopes.h>
#include <checks.h>
#include <cwctype>
#include <utility>

#define LINENO_WIDTH 5

using std::make_pair;

CodePrinter::CodePrinter(FunctionLiteral* program) {
    const int initial_size = 256;
    output_ = NewArray<char>(initial_size);
    size_ = initial_size;
    program_ = program;
    PrintProgram();
}

CodePrinter::~CodePrinter() {
    DeleteArray(output_);
}

void CodePrinter::VisitBlock(Block* node) {
    PrintStatements(node->statements());
}

void CodePrinter::VisitVariableDeclaration(VariableDeclaration* node) {
    /*Print("%*s", indent_, "");
      Print("var ");
      PrintLiteral(node->proxy()->name(), false);
      Print(";\n");*/
}

void CodePrinter::VisitFunctionDeclaration(FunctionDeclaration* node) {
    Print("%*s  %*sfunction ", LINENO_WIDTH, "", indent_, "");
    PrintLiteral(node->proxy()->name(), false);
    Print(" = ");
    PrintFunctionLiteral(node->fun());
    Print(";\n");
}

void CodePrinter::VisitModuleDeclaration(ModuleDeclaration* node) {
    Print("%*s  %*smodule ", LINENO_WIDTH, "", indent_, "");
    PrintLiteral(node->proxy()->name(), false);
    Print(" = ");
    Visit(node->module());
    Print(";\n");
}

void CodePrinter::VisitImportDeclaration(ImportDeclaration* node) {
    Print("%*s  %*simport ", LINENO_WIDTH, "", indent_, "");
    PrintLiteral(node->proxy()->name(), false);
    Print(" from ");
    Visit(node->module());
    Print(";\n");
}

void CodePrinter::VisitExportDeclaration(ExportDeclaration* node) {
    Print("%*s  %*sexport ", LINENO_WIDTH, "", indent_, "");
    PrintLiteral(node->proxy()->name(), false);
    Print(";\n");
}

void CodePrinter::VisitModuleLiteral(ModuleLiteral* node) {
    VisitBlock(node->body());
}

void CodePrinter::VisitModuleVariable(ModuleVariable* node) {
    Visit(node->proxy());
}

void CodePrinter::VisitModulePath(ModulePath* node) {
    Visit(node->module());
    Print(".");
    PrintLiteral(node->name(), false);
}

void CodePrinter::VisitModuleUrl(ModuleUrl* node) {
    Print("at ");
    PrintLiteral(node->url(), true);
}

void CodePrinter::VisitModuleStatement(ModuleStatement* node) {
    Print("%*s  %*smodule ", LINENO_WIDTH, "", indent_, "");
    PrintLiteral(node->proxy()->name(), false);
    Print(" ");
    Visit(node->body());
    Print(";\n");
}

void CodePrinter::VisitExpressionStatement(ExpressionStatement* node) {
    UNREACHABLE();
}

void CodePrinter::VisitEmptyStatement(EmptyStatement* node) {
    UNREACHABLE();
}

void CodePrinter::VisitIfStatement(IfStatement* node) {
    PrintLineNo(node);
    Print("%*sif (", indent_, "");
    Visit(node->condition());
    Print(") {");
    PrintDependence(node);
    Print("\n");
    indent_ += 4;
    Visit(node->then_statement());
    if (node->HasElseStatement() && ((Block*)node->else_statement())->statements()->length()) {
	Print("%*s  %*s} else {\n", LINENO_WIDTH, "", indent_ - 4, "");
	Visit(node->else_statement());
    }
    indent_ -= 4;
    Print("%*s  %*s}\n", LINENO_WIDTH, "", indent_, "");
    //ExitStatement(node);
}

void CodePrinter::VisitContinueStatement(ContinueStatement* node) {
    PrintLineNo(node);
    Print("%*scontinue;", indent_, "");
    ZoneStringList* labels = node->target()->labels();
    if (labels != NULL) {
	Print(" ");
	ASSERT(labels->length() > 0);  // guaranteed to have at least one entry
	PrintLiteral(labels->at(0), false);  // any label from the list is fine
    }
    PrintDependence(node);
    Print("\n");
    //ExitStatement(node);
}

void CodePrinter::VisitBreakStatement(BreakStatement* node) {
    PrintLineNo(node);
    Print("%*sbreak;", indent_, "");
    ZoneStringList* labels = node->target()->labels();
    if (labels != NULL) {
	Print(" ");
	ASSERT(labels->length() > 0);  // guaranteed to have at least one entry
	PrintLiteral(labels->at(0), false);  // any label from the list is fine
    }
    PrintDependence(node);
    Print("\n");
    //ExitStatement(node);
}

void CodePrinter::VisitReturnStatement(ReturnStatement* node) {
    PrintLineNo(node);
    Print("%*sreturn ", indent_, "");
    Visit(node->expression());
    Print(";");
    PrintDependence(node);
    Print("\n");
    //ExitStatement(node);
}

void CodePrinter::VisitWithStatement(WithStatement* node) {
    PrintLineNo(node);
    Print("%*swith (", indent_, "");
    Visit(node->expression());
    Print(") ");
    Visit(node->statement());
    //ExitStatement(node);
}

void CodePrinter::VisitSwitchStatement(SwitchStatement* node) {
    PrintLineNo(node);
    Print("%*s", indent_, "");
    PrintLabels(node->labels());
    Print("switch (");
    Visit(node->tag());
    Print(") {");
    PrintDependence(node);
    Print("\n");
    indent_ += 4;
    ZoneList<CaseClause*>* cases = node->cases();
    for (int i = 0; i < cases->length(); i++)
	PrintCaseClause(cases->at(i));
    indent_ -= 4;
    Print("%*s  %*s}\n", LINENO_WIDTH, "", indent_, "");
    //ExitStatement(node);
}

void CodePrinter::VisitDoWhileStatement(DoWhileStatement* node) {
    UNREACHABLE();
}

void CodePrinter::VisitWhileStatement(WhileStatement* node) {
    PrintLineNo(node);
    Print("%*s", indent_, "");
    PrintLabels(node->labels());
    Print("while (");
    Visit(node->cond());
    Print(") {");
    PrintDependence(node);
    Print("\n");
    indent_ += 4;
    Visit(node->body());
    indent_ -= 4;
    Print("%*s  %*s}\n", LINENO_WIDTH, "", indent_, "");
    //ExitStatement(node);
}

void CodePrinter::VisitForStatement(ForStatement* node) {
    UNREACHABLE();
}

void CodePrinter::VisitForInStatement(ForInStatement* node) {
    PrintLineNo(node);
    Print("%*s", indent_, "");
    PrintLabels(node->labels());
    Print("for (");
    Visit(node->each());
    Print(" in ");
    Visit(node->enumerable());
    Print(") {");
    PrintDependence(node);
    Print("\n");
    indent_ += 4;
    Visit(node->body());
    indent_ -= 4;
    Print("%*s  %*s}\n", LINENO_WIDTH, "", indent_, "");
    //ExitStatement(node);
}

void CodePrinter::VisitTryCatchStatement(TryCatchStatement* node) {
    //Print("%*s", indent_, "");
    //Print("try ");
    Visit(node->try_block());
    //Print(" catch (");
    //const bool quote = false;
    //PrintLiteral(node->variable()->name(), quote);
    //Print(") ");
    //Visit(node->catch_block());
}

void CodePrinter::VisitTryFinallyStatement(TryFinallyStatement* node) {
    //Print("%*s", indent_, "");
    //Print("try ");
    Visit(node->try_block());
    //Print(" finally ");
    Visit(node->finally_block());
}

void CodePrinter::VisitDebuggerStatement(DebuggerStatement* node) {
    PrintLineNo(node);
    Print("%*sdebugger;", indent_, "");
    PrintDependence(node);
    Print("\n");
    //ExitStatement(node);
}

void CodePrinter::VisitFunctionLiteral(FunctionLiteral* node) {
    PrintFunctionLiteral(node);
}

void CodePrinter::VisitSharedFunctionInfoLiteral(
	SharedFunctionInfoLiteral* node) {
    PrintLiteral(node->shared_function_info(), true);
}

void CodePrinter::VisitConditional(Conditional* node) {
    Visit(node->condition());
    Print(" ? ");
    Visit(node->then_expression());
    Print(" : ");
    Visit(node->else_expression());
}

void CodePrinter::VisitLiteral(Literal* node) {
    PrintLiteral(node->handle(), true);
}

void CodePrinter::VisitRegExpLiteral(RegExpLiteral* node) {
    Print("RegExp(");
    PrintLiteral(node->pattern(), true);
    Print(",");
    PrintLiteral(node->flags(), true);
    Print(")");
}

void CodePrinter::VisitObjectLiteral(ObjectLiteral* node) {
    Print("{ ");
    for (int i = 0; i < node->properties()->length(); i++) {
	if (i != 0) Print(",");
	ObjectLiteral::Property* property = node->properties()->at(i);
	Print(" ");
	Visit(property->key());
	Print(": ");
	Visit(property->value());
    }
    Print(" }");
}

void CodePrinter::VisitArrayLiteral(ArrayLiteral* node) {
    Print("[ ");
    for (int i = 0; i < node->values()->length(); i++) {
	if (i != 0) Print(",");
	Visit(node->values()->at(i));
    }
    Print(" ]");
}

void CodePrinter::VisitVariableProxy(VariableProxy* node) {
    PrintLiteral(node->name(), false);
}

void CodePrinter::VisitAssignment(Assignment* node) {
    Visit(node->target());
    Print(" %s ", Token::String(node->op()));
    Visit(node->value());
}

void CodePrinter::VisitThrow(Throw* node) {
    Print("throw ");
    Visit(node->exception());
}

void CodePrinter::VisitProperty(Property* node) {
    Expression* key = node->key();
    Literal* literal = key->AsLiteral();
    if (literal != NULL && literal->handle()->IsSymbol()) {
	Visit(node->obj());
	Print(".");
	PrintLiteral(literal->handle(), false);
    } else {
	Visit(node->obj());
	Print("[");
	Visit(key);
	Print("]");
    }
}

void CodePrinter::VisitCall(Call* node) {
    Visit(node->expression());
    PrintArguments(node->arguments());
}

void CodePrinter::VisitCallNew(CallNew* node) {
    Print("new ");
    Visit(node->expression());
    PrintArguments(node->arguments());
}

void CodePrinter::VisitCallRuntime(CallRuntime* node) {
    Print("%%");
    PrintLiteral(node->name(), false);
    PrintArguments(node->arguments());
}

void CodePrinter::VisitUnaryOperation(UnaryOperation* node) {
    Token::Value op = node->op();
    bool needsSpace =
	op == Token::DELETE || op == Token::TYPEOF || op == Token::VOID;
    Print("%s%s", Token::String(op), needsSpace ? " " : "");
    Visit(node->expression());
}

void CodePrinter::VisitCountOperation(CountOperation* node) {
    if (node->is_prefix()) Print("%s", Token::String(node->op()));
    Visit(node->expression());
    if (node->is_postfix()) Print("%s", Token::String(node->op()));
}

void CodePrinter::VisitBinaryOperation(BinaryOperation* node) {
    Visit(node->left());
    Print(" %s ", Token::String(node->op()));
    Visit(node->right());
}

void CodePrinter::VisitCompareOperation(CompareOperation* node) {
    Visit(node->left());
    Print(" %s ", Token::String(node->op()));
    Visit(node->right());
}

void CodePrinter::VisitThisFunction(ThisFunction* node) {
    Print("<this-function>");
}

void CodePrinter::VisitCanonicalFunctionEntry(CanonicalFunctionEntry* node) {
    func_stack_.push(lineno_.size() + 1);
    func_[lineno_.size() + 1] = node;
    PrintLineNo(node);
    Print("%*sbegin;", indent_, "");
    PrintDependence(node);
    Print("\n");
    //indent_ += 4;
    PrintDeclarations(node->declarations());
    PrintStatements(node->body());
    //ExitStatement(node);
}

void CodePrinter::VisitCanonicalAssignment(CanonicalAssignment* node) {
    PrintLineNo(node);
    Print("%*s", indent_, "");
    Visit(node->target());
    Print(" = ");
    Visit(node->value());
    Print(";");
    PrintDependence(node);
    Print("\n");
    //ExitStatement(node);
}

void CodePrinter::VisitCanonicalPropertyAssignment(CanonicalPropertyAssignment* node) {
    PrintLineNo(node);
    Print("%*s", indent_, "");
    Visit(node->target());
    Print(" = ");
    Visit(node->value());
    Print(";");
    PrintDependence(node);
    Print("\n");
    //ExitStatement(node);
}

void CodePrinter::VisitCanonicalFunctionExit(CanonicalFunctionExit* node) {
    PrintLineNo(node);
    //indent_ -= 4;
    Print("%*send;", indent_, "");
    PrintDependence(node);
    Print("\n");
    //ExitStatement(node);
    func_stack_.pop();
}

#if 0
void CodePrinter::VisitEndIfStatement(EndIfStatement* node) {
    UNREACHABLE();
    /*PrintLineNo(node);
      indent_ -= 4;
      Print("%*send_if;", indent_, "");
      PrintDependence(node);
      Print("\n");*/
}

void CodePrinter::VisitEndSwitchStatement(EndSwitchStatement* node) {
    UNREACHABLE();
    /*PrintLineNo(node);
      indent_ -= 8;
      Print("%*send_switch;", indent_, "");
      PrintDependence(node);
      Print("\n");*/
}

void CodePrinter::VisitEndWhileStatement(EndWhileStatement* node) {
    UNREACHABLE();
    /*PrintLineNo(node);
      indent_ -= 4;
      Print("%*send_while;", indent_, "");
      PrintDependence(node);
      Print("\n");*/
}

void CodePrinter::VisitEndForInStatement(EndForInStatement* node) {
    UNREACHABLE();
    /*PrintLineNo(node);
      indent_ -= 4;
      Print("%*send_for;", indent_, "");
      PrintDependence(node);
      Print("\n");*/
}
#endif

const char* CodePrinter::PrintProgram() {
    Init();
    PrintStatements(program_->body());
    return output_;
}

const char* CodePrinter::Print(AstNode* node, const DependenceGraph& graph, const list<Statement*>& succ) {
    Init();
    graph_ = &graph;
    successors_.clear();
    successors_.insert(succ.begin(), succ.end());
    Visit(node);
    return output_;
}

const char* CodePrinter::PrintFunc(FunctionLiteral* node) {
    Init();
    graph_ = new DependenceGraph;
    successors_.clear();
    func_stack_.push(0);
    Visit(node);
    delete graph_;
    return output_;
}

/*void CodePrinter::PrintOut(AstNode* node) {
    CodePrinter printer;
    PrintF("%s", printer.Print(node));
}*/

void CodePrinter::Init() {
    output_[0] = '\0';
    pos_ = 0;
    indent_ = 0;
    graph_ = NULL;
}

void CodePrinter::Print(const char* format, ...) {
    //if (!flag_stack_.top())
    //	return;
    for (;;) {
	va_list arguments;
	va_start(arguments, format);
	int n = OS::VSNPrintF(Vector<char>(output_, size_) + pos_,
		format,
		arguments);
	va_end(arguments);

	if (n >= 0) {
	    // there was enough space - we are done
	    pos_ += n;
	    return;
	} else {
	    // there was not enough space - allocate more and try again
	    const int slack = 32;
	    int new_size = size_ + (size_ >> 1) + slack;
	    char* new_output = NewArray<char>(new_size);
	    memcpy(new_output, output_, pos_);
	    DeleteArray(output_);
	    output_ = new_output;
	    size_ = new_size;
	}
    }
}

void CodePrinter::PrintStatements(ZoneList<Statement*>* statements) {
    for (int i = 0; i < statements->length(); i++) {
	Visit(statements->at(i));
    }
}

void CodePrinter::PrintLabels(ZoneStringList* labels) {
    if (labels != NULL) {
	for (int i = 0; i < labels->length(); i++) {
	    PrintLiteral(labels->at(i), false);
	    Print(": ");
	}
    }
}

void CodePrinter::PrintArguments(ZoneList<Expression*>* arguments) {
    Print("(");
    for (int i = 0; i < arguments->length(); i++) {
	if (i != 0) Print(", ");
	Visit(arguments->at(i));
    }
    Print(")");
}

#include <iostream>

using std::cerr;
using std::endl;

void CodePrinter::PrintLiteral(Handle<Object> value, bool quote) {
    Object* object = *value;
    if (object->IsString()) {
	String* string = String::cast(object);
	for (int i = 0; i < string->length(); i++) {
	    if (!iswprint(string->Get(i))) {
	    	quote = true;
	    	break;
	    }
	}
	if (quote) Print("\"");
	for (int i = 0; i < string->length(); i++)
	    Print(!iswprint(string->Get(i)) ? "\\u%04x" : "%c", string->Get(i));
	if (quote) Print("\"");
    } else if (object->IsNull()) {
	Print("null");
    } else if (object->IsTrue()) {
	Print("true");
    } else if (object->IsFalse()) {
	Print("false");
    } else if (object->IsUndefined()) {
	Print("undefined");
    } else if (object->IsNumber()) {
	Print("%g", object->Number());
    } else if (object->IsJSObject()) {
	// regular expression
	if (object->IsJSFunction()) {
	    Print("JS-Function");
	} else if (object->IsJSArray()) {
	    Print("JS-array[%u]", JSArray::cast(object)->length());
	} else if (object->IsJSObject()) {
	    Print("JS-Object");
	} else {
	    Print("?UNKNOWN?");
	}
    } else if (object->IsFixedArray()) {
	Print("FixedArray");
    } else {
	Print("<unknown literal %p>", object);
    }
}

void CodePrinter::PrintParameters(Scope* scope) {
    Print("(");
    for (int i = 0; i < scope->num_parameters(); i++) {
	if (i  > 0) Print(", ");
	PrintLiteral(scope->parameter(i)->name(), false);
    }
    Print(")");
}

void CodePrinter::PrintDeclarations(ZoneList<Declaration*>* declarations) {
    for (int i = 0; i < declarations->length(); i++) {
	Visit(declarations->at(i));
    }
}

void CodePrinter::PrintFunctionLiteral(FunctionLiteral* function) {
    Print("function ");
    PrintLiteral(function->name(), false);
    PrintParameters(function->scope());
    // print the contents if printing the whole program or this function
    if (!graph_ || func_stack_.empty()) {
    	if (!graph_) {
	    Print(" {\n");
	    indent_ += 4;
	} else {
	    Print(":\n");
	}
	PrintStatements(function->body());
	if (!graph_) {
	    indent_ -= 4;
	    Print("%*s  %*s}", LINENO_WIDTH, "", indent_, "");
	}
    } else {
    	Print(" {...}");
    }
}

void CodePrinter::PrintCaseClause(CaseClause* clause) {
    if (clause->is_default()) {
	Print("%*s  %*sdefault", LINENO_WIDTH, "", indent_, "");
    } else {
	Print("%*s  %*scase ", LINENO_WIDTH, "", indent_, "");
	Visit(clause->label());
    }
    Print(":\n");
    indent_ += 4;
    PrintStatements(clause->statements());
    indent_ -= 4;
}

void CodePrinter::PrintDependence(Statement* node) {
    if (graph_ && graph_->count(node)) {
	Print(" [");
	for (list<Statement*>::const_iterator iter = graph_->at(node).begin(); iter != graph_->at(node).end(); ++iter) {
	    if (lineno_.count(*iter))
		Print(iter == graph_->at(node).begin() ? "%d" : ", %d", lineno_[*iter]);
	}
	Print("]");
    }
}

void CodePrinter::PrintLineNo(Statement* node) {
    if (!lineno_.count(node)) {
    	line_.push_back(node);
	lineno_.insert(make_pair(node, line_.size()));
	funcno_[node] = func_stack_.top();
    }
    //flag_stack_.push(!graph_ || graph_->count(node));
    Print("%*d%c ", LINENO_WIDTH, lineno_[node], graph_ && graph_->count(node) ? '*' : successors_.count(node) ? '>' : ' ');
}

/*void CodePrinter::ExitStatement(Statement* node) {
    //flag_stack_.pop();
}*/
