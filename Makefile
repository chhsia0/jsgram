CC=g++
CXXFLAGS=-fno-rtti -fno-rtti -fno-exceptions -fvisibility=hidden -Wall -Werror -W -Wno-unused-parameter -Woverloaded-virtual -Wnon-virtual-dtor -m64
CXXFLAGS+=-DV8_TARGET_ARCH_X64 -DOBJECT_PRINT -DENABLE_DISASSEMBLER -DENABLE_DEBUGGER_SUPPORT -DV8_ENABLE_CHECKS -DDEBUG -O3
CXXFLAGS+=-isystem v8/include -isystem v8/src
LDFLAGS=-static -Lv8/out/x64.debug/obj.target/tools/gyp -pthread
LDLIBS=-lv8_base -lv8_snapshot -lsqlite3 -ldl
SRCS=$(wildcard *.cc *.cpp)

jsgram: BuiltIns.o CanonicalAst.o DependenceGraph.o PDGExtractor.o CodePrinter.o StatementCopier.o OperationPrinter.o SequenceExtractor.o

v8:
	git clone http://github.com/v8/v8
	cd v8
	git checkout origin/3.15
	svn co http://gyp.googlecode.com/svn/trunk build/gyp --revision 1831
	make x64.debug

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
