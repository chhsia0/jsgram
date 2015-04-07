CC=g++
CXXFLAGS=-m64 -fno-rtti -fno-rtti -fno-exceptions -fvisibility=hidden -Wall -W -Werror -Woverloaded-virtual -Wnon-virtual-dtor \
         -Wno-unused-const-variable -Wno-unneeded-internal-declaration -Wno-unused-function -Wno-unused-parameter \
         -Wno-unneeded-internal-declaration -Wno-unused-function -Wno-unused-private-field
CXXFLAGS+=-DOBJECT_PRINT -DENABLE_DISASSEMBLER -DENABLE_DEBUGGER_SUPPORT -DV8_ENABLE_CHECKS -DDEBUG -O3
CXXFLAGS+=-isystem v8/include -isystem v8/src
LDFLAGS=-Lv8/out/x64.debug/ -pthread
LDLIBS=-lv8 -lsqlite3 -ldl
SRCS=$(wildcard *.cc *.cpp)
V8STATICLIBS=v8/out/x64.debug/libv8_base.a v8/out/x64.debug/libv8_snapshot.a

jsgram: BuiltIns.o CanonicalAst.o DependenceGraph.o PDGExtractor.o CodePrinter.o StatementCopier.o OperationPrinter.o SequenceExtractor.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(LDLIBS) jsgram.cc $^ $(V8STATICLIBS) -o jsgram

v8: v8/out/x64.debug/libv8_base.a

v8/out/x64.debug/libv8_base.a:
	if [ ! -d depot_tools ]; then git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git; fi
	if [ ! -d v8 ]; then ./depot_tools/fetch v8; fi
	cd v8 && git checkout branch-heads/3.15
	cd v8 && CXXFLAGS="-Wno-unused-const-variable -Wno-unneeded-internal-declaration -Wno-unneeded-internal-declaration -Wno-unneeded-internal-declaration -Wno-unused-function" make x64.debug library=shared

depend:
	sed -i '/^# DO NOT DELETE$$/{q}' Makefile
	$(CXX) $(CXXFLAGS) -MM -MG $(SRCS) >> Makefile

clean:
	rm -rf *.o

# DO NOT DELETE
BuiltIns.o: BuiltIns.cc BuiltIns.h
CanonicalAst.o: CanonicalAst.cc CanonicalAst.h StatementCopier.h
CodePrinter.o: CodePrinter.cc CodePrinter.h CanonicalAst.h \
 DependenceGraph.h
DependenceGraph.o: DependenceGraph.cc DependenceGraph.h CanonicalAst.h \
 Utility.h
jsgram.o: jsgram.cc CanonicalAst.h DependenceGraph.h CodePrinter.h \
 NgramExtractor.h OperationPrinter.h PDGExtractor.h Utility.h \
 SequenceExtractor.h
OperationPrinter.o: OperationPrinter.cc OperationPrinter.h CanonicalAst.h \
 BuiltIns.h
PDGExtractor.o: PDGExtractor.cc PDGExtractor.h CanonicalAst.h \
 DependenceGraph.h NgramExtractor.h OperationPrinter.h Utility.h
SequenceExtractor.o: SequenceExtractor.cc SequenceExtractor.h \
 NgramExtractor.h OperationPrinter.h CanonicalAst.h
StatementCopier.o: StatementCopier.cc StatementCopier.h
