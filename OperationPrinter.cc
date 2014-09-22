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

#include "OperationPrinter.h"

#include <algorithm>
#include <checks.h>
#include <map>
#include "BuiltIns.h"

using std::sort;
using std::map;

#define DEFINE_UNREACHABLE_VISIT(type) \
void OperationPrinter::Visit##type(type* node) { \
    UNREACHABLE(); \
}
MODULE_NODE_LIST(DEFINE_UNREACHABLE_VISIT)
DECLARATION_NODE_LIST(DEFINE_UNREACHABLE_VISIT)
DEFINE_UNREACHABLE_VISIT(Block)
DEFINE_UNREACHABLE_VISIT(ModuleStatement)
DEFINE_UNREACHABLE_VISIT(ExpressionStatement)
DEFINE_UNREACHABLE_VISIT(EmptyStatement)
DEFINE_UNREACHABLE_VISIT(WithStatement)
DEFINE_UNREACHABLE_VISIT(DoWhileStatement)
DEFINE_UNREACHABLE_VISIT(ForStatement)
DEFINE_UNREACHABLE_VISIT(TryCatchStatement)
DEFINE_UNREACHABLE_VISIT(TryFinallyStatement)
DEFINE_UNREACHABLE_VISIT(ObjectLiteral)
DEFINE_UNREACHABLE_VISIT(ArrayLiteral)
DEFINE_UNREACHABLE_VISIT(Assignment)
DEFINE_UNREACHABLE_VISIT(CountOperation)
#undef DEFINE_UNREACHABLE_VISIT

#define DEFINE_EMPTY_VISIT(type) \
void OperationPrinter::Visit##type(type* node) { \
    value_ = ""; \
}
DEFINE_EMPTY_VISIT(FunctionLiteral)
DEFINE_EMPTY_VISIT(SharedFunctionInfoLiteral)
DEFINE_EMPTY_VISIT(VariableProxy)
DEFINE_EMPTY_VISIT(Literal)
DEFINE_EMPTY_VISIT(RegExpLiteral)
DEFINE_EMPTY_VISIT(ThisFunction)
#undef DEFINE_EMPTY_VISIT

void OperationPrinter::VisitContinueStatement(ContinueStatement* node) {
    value_ = "continue";
}

void OperationPrinter::VisitBreakStatement(BreakStatement* node) {
    value_ = "break";
}

void OperationPrinter::VisitReturnStatement(ReturnStatement* node) {
    value_ = "return";
}

void OperationPrinter::VisitIfStatement(IfStatement* node) {
    value_ = "if";
}

void OperationPrinter::VisitSwitchStatement(SwitchStatement* node) {
    value_ = "switch";
}

void OperationPrinter::VisitWhileStatement(WhileStatement* node) {
    value_ = "while";
}

void OperationPrinter::VisitForInStatement(ForInStatement* node) {
    value_ = "for";
}

void OperationPrinter::VisitDebuggerStatement(DebuggerStatement* node) {
    value_ = "debugger";
}

void OperationPrinter::VisitConditional(Conditional* node) {
    value_ = "?:";
}

void OperationPrinter::VisitThrow(Throw* node) {
    value_ = "throw";
}

void OperationPrinter::VisitProperty(Property* node) {
    Literal* key = node->key()->AsLiteral();
    value_ = (key != NULL && key->handle()->IsSymbol()) ? "." : "[]";
}

void OperationPrinter::VisitCall(Call* node) {
    if (node->expression()->node_type() == AstNode::kProperty) {
	Literal* method = reinterpret_cast<Property*>(node->expression())->key()->AsLiteral();
	if (method != NULL && method->handle()->IsSymbol()) {
	    string name = *Handle<String>::cast(method->handle())->ToCString();
	    value_ = BuiltIns::FindMethod(name) ? "." + name + "()" : ".()";
	} else
	    value_ = "[]()";
    } else if (node->expression()->node_type() == AstNode::kVariableProxy) {
        string name = *reinterpret_cast<VariableProxy*>(node->expression())->name()->ToCString();
        value_ = BuiltIns::FindFunction(name) ? name + "()" : "()";
    } else
        value_ = "()";
}

void OperationPrinter::VisitCallNew(CallNew* node) {
    if (node->expression()->node_type() == AstNode::kVariableProxy) {
	string name = *reinterpret_cast<VariableProxy*>(node->expression())->name()->ToCString();
	value_ = BuiltIns::FindConstructor(name) ? name : "new";
    } else
        value_ = "new";
}

void OperationPrinter::VisitCallRuntime(CallRuntime* node) {
    value_ = "()";
}

void OperationPrinter::VisitUnaryOperation(UnaryOperation* node) {
    if (node->op() == Token::ADD)
        value_ = "pos";
    else if (node->op() == Token::SUB)
        value_ = "neg";
    else
        value_ = Token::String(node->op());
}

void OperationPrinter::VisitBinaryOperation(BinaryOperation* node) {
    value_ = Token::String(node->op());
}

void OperationPrinter::VisitCompareOperation(CompareOperation* node) {
    value_ = Token::String(node->op());
}

void OperationPrinter::VisitCanonicalFunctionEntry(CanonicalFunctionEntry* node) {
    value_ = "begin";
}

void OperationPrinter::VisitCanonicalAssignment(CanonicalAssignment* node) {
    Visit(node->value());
    if (value_.empty())
	value_ = "=";
}

void OperationPrinter::VisitCanonicalPropertyAssignment(CanonicalPropertyAssignment* node) {
    Literal* key = node->target()->key()->AsLiteral();
    value_ = key != NULL && key->handle()->IsSymbol() ? ".=" : "[]=";
}

void OperationPrinter::VisitCanonicalFunctionExit(CanonicalFunctionExit* node) {
    value_ = "end";
}

#if 0
void OperationPrinter::VisitEndIfStatement(EndIfStatement* node) {
    value_ = "end_if";
}

void OperationPrinter::VisitEndSwitchStatement(EndSwitchStatement* node) {
    value_ = "end_switch";
}

void OperationPrinter::VisitEndWhileStatement(EndWhileStatement* node) {
    value_ = "end_while";
}

void OperationPrinter::VisitEndForInStatement(EndForInStatement* node) {
    value_ = "end_for";
}
#endif

string OperationPrinter::Print(Statement* node) {
    Visit(node);
    return value_;
}
