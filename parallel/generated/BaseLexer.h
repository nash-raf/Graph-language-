
// Generated from Base.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"




class  BaseLexer : public antlr4::Lexer {
public:
  enum {
    T__0 = 1, T__1 = 2, T__2 = 3, T__3 = 4, T__4 = 5, T__5 = 6, T__6 = 7, 
    T__7 = 8, T__8 = 9, T__9 = 10, T__10 = 11, T__11 = 12, T__12 = 13, T__13 = 14, 
    T__14 = 15, T__15 = 16, T__16 = 17, T__17 = 18, T__18 = 19, T__19 = 20, 
    T__20 = 21, T__21 = 22, T__22 = 23, T__23 = 24, T__24 = 25, T__25 = 26, 
    T__26 = 27, T__27 = 28, T__28 = 29, T__29 = 30, T__30 = 31, T__31 = 32, 
    T__32 = 33, T__33 = 34, T__34 = 35, T__35 = 36, T__36 = 37, T__37 = 38, 
    T__38 = 39, T__39 = 40, T__40 = 41, T__41 = 42, T__42 = 43, GRAPH = 44, 
    SET = 45, UNION = 46, INTERSECT = 47, EDGE = 48, NODE = 49, TRUE = 50, 
    FALSE = 51, OF = 52, PLUS = 53, MINUS = 54, TIMES = 55, DIVIDE = 56, 
    AND = 57, OR = 58, EQUAL = 59, NOTEQUAL = 60, LESSTHAN = 61, GREATERTHAN = 62, 
    LESSEQUAL = 63, GREATEREQUAL = 64, ID = 65, INT = 66, REAL = 67, STRING = 68, 
    Comment = 69, WS = 70
  };

  explicit BaseLexer(antlr4::CharStream *input);

  ~BaseLexer() override;


  std::string getGrammarFileName() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const std::vector<std::string>& getChannelNames() const override;

  const std::vector<std::string>& getModeNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;

  const antlr4::atn::ATN& getATN() const override;

  // By default the static state used to implement the lexer is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:

  // Individual action functions triggered by action() above.

  // Individual semantic predicate functions triggered by sempred() above.

};

