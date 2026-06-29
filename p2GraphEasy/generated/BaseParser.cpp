
// Generated from Base.g4 by ANTLR 4.13.1


#include "BaseListener.h"
#include "BaseVisitor.h"

#include "BaseParser.h"


using namespace antlrcpp;

using namespace antlr4;

namespace {

struct BaseParserStaticData final {
  BaseParserStaticData(std::vector<std::string> ruleNames,
                        std::vector<std::string> literalNames,
                        std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  BaseParserStaticData(const BaseParserStaticData&) = delete;
  BaseParserStaticData(BaseParserStaticData&&) = delete;
  BaseParserStaticData& operator=(const BaseParserStaticData&) = delete;
  BaseParserStaticData& operator=(BaseParserStaticData&&) = delete;

  std::vector<antlr4::dfa::DFA> decisionToDFA;
  antlr4::atn::PredictionContextCache sharedContextCache;
  const std::vector<std::string> ruleNames;
  const std::vector<std::string> literalNames;
  const std::vector<std::string> symbolicNames;
  const antlr4::dfa::Vocabulary vocabulary;
  antlr4::atn::SerializedATNView serializedATN;
  std::unique_ptr<antlr4::atn::ATN> atn;
};

::antlr4::internal::OnceFlag baseParserOnceFlag;
#if ANTLR4_USE_THREAD_LOCAL_CACHE
static thread_local
#endif
BaseParserStaticData *baseParserStaticData = nullptr;

void baseParserInitialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  if (baseParserStaticData != nullptr) {
    return;
  }
#else
  assert(baseParserStaticData == nullptr);
#endif
  auto staticData = std::make_unique<BaseParserStaticData>(
    std::vector<std::string>{
      "program", "statement", "swapStatement", "graphDef", "graphProperty", 
      "boolLiteral", "nodes", "edges", "nodeList", "edgeList", "graphID", 
      "nodeID", "fileEdgeList", "edge", "varDecl", "setDecl", "setInitializer", 
      "setOperation", "setTarget", "setExpr", "setMethodCall", "conditionalStatement", 
      "condition", "graphComprehension", "graphExpr", "graphCondition", 
      "loopStatement", "foreachStatement", "loopTarget", "whileStatement", 
      "nodeEdgeOperation", "addOperation", "removeOperation", "addTargets", 
      "removeTargets", "queryStatement", "showgraph", "function", "returnType", 
      "paramList", "param", "type", "functionCall", "functionName", "argumentList", 
      "sleepStatement", "block", "returnStatement", "breakStatement", "continueStatement", 
      "printStatement", "printExpr", "printArrayStatement", "printgraph", 
      "expr", "arrayDeclarator", "arrayInitializer", "assignmentStatement", 
      "arrayAssignStatement", "weights"
    },
    std::vector<std::string>{
      "", "';'", "'swap'", "'('", "','", "')'", "'{'", "'}'", "'directed'", 
      "':'", "'true'", "'false'", "'nodes:'", "'edges:'", "'file'", "'->'", 
      "'='", "'.'", "'add'", "'remove'", "'if'", "'else'", "'in'", "'['", 
      "'where'", "']'", "'degree'", "'connected'", "'with'", "'edge'", "'has'", 
      "'vertex'", "'cycle'", "'for'", "'each'", "'out'", "'neighbor'", "'element'", 
      "'while'", "'to'", "'from'", "'query'", "'show'", "'fn'", "'int'", 
      "'real'", "'bool'", "'void'", "'string'", "'sleep'", "'return'", "'break'", 
      "'continue'", "'print'", "'contains'", "'size'", "'pop'", "'[]'", 
      "'INF'", "'timer'", "'graph'", "'weights'", "'set'", "'union'", "'intersect'", 
      "'!'", "'%'", "'edges'", "'nodes'", "'vertices'", "'TRUE'", "'FALSE'", 
      "'of'", "'+'", "'-'", "'*'", "'/'", "'&&'", "'||'", "'=='", "'!='", 
      "'<'", "'>'", "'<='", "'>='"
    },
    std::vector<std::string>{
      "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", 
      "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", 
      "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", 
      "", "", "", "", "", "", "", "", "", "GRAPH", "WEIGHTS", "SET", "UNION", 
      "INTERSECT", "NOT", "MODULO", "EDGE", "NODE", "VERTICES", "TRUE", 
      "FALSE", "OF", "PLUS", "MINUS", "TIMES", "DIVIDE", "AND", "OR", "EQUAL", 
      "NOTEQUAL", "LESSTHAN", "GREATERTHAN", "LESSEQUAL", "GREATEREQUAL", 
      "ID", "INT", "REAL", "STRING", "Comment", "WS"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,1,90,812,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,2,
  	7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,7,
  	14,2,15,7,15,2,16,7,16,2,17,7,17,2,18,7,18,2,19,7,19,2,20,7,20,2,21,7,
  	21,2,22,7,22,2,23,7,23,2,24,7,24,2,25,7,25,2,26,7,26,2,27,7,27,2,28,7,
  	28,2,29,7,29,2,30,7,30,2,31,7,31,2,32,7,32,2,33,7,33,2,34,7,34,2,35,7,
  	35,2,36,7,36,2,37,7,37,2,38,7,38,2,39,7,39,2,40,7,40,2,41,7,41,2,42,7,
  	42,2,43,7,43,2,44,7,44,2,45,7,45,2,46,7,46,2,47,7,47,2,48,7,48,2,49,7,
  	49,2,50,7,50,2,51,7,51,2,52,7,52,2,53,7,53,2,54,7,54,2,55,7,55,2,56,7,
  	56,2,57,7,57,2,58,7,58,2,59,7,59,1,0,1,0,5,0,123,8,0,10,0,12,0,126,9,
  	0,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  	1,1,1,1,1,1,1,1,1,1,1,1,3,1,151,8,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,
  	3,1,3,1,3,1,3,5,3,165,8,3,10,3,12,3,168,9,3,1,3,3,3,171,8,3,1,3,5,3,174,
  	8,3,10,3,12,3,177,9,3,1,3,3,3,180,8,3,1,3,5,3,183,8,3,10,3,12,3,186,9,
  	3,1,3,1,3,5,3,190,8,3,10,3,12,3,193,9,3,1,3,1,3,1,3,1,3,1,3,1,3,1,3,5,
  	3,202,8,3,10,3,12,3,205,9,3,1,3,3,3,208,8,3,1,3,5,3,211,8,3,10,3,12,3,
  	214,9,3,1,3,3,3,217,8,3,1,3,5,3,220,8,3,10,3,12,3,223,9,3,1,3,1,3,1,3,
  	3,3,228,8,3,1,4,1,4,1,4,1,4,1,4,1,5,1,5,1,6,1,6,1,6,1,6,1,7,1,7,1,7,3,
  	7,244,8,7,1,7,1,7,1,8,1,8,1,8,5,8,251,8,8,10,8,12,8,254,9,8,1,9,1,9,1,
  	9,5,9,259,8,9,10,9,12,9,262,9,9,1,10,1,10,1,11,1,11,1,12,1,12,1,12,1,
  	13,1,13,1,13,1,13,1,14,1,14,1,14,1,14,3,14,279,8,14,1,14,1,14,1,14,1,
  	14,1,14,1,14,3,14,287,8,14,1,14,1,14,3,14,291,8,14,1,15,1,15,1,15,1,15,
  	1,15,1,15,1,15,1,15,1,15,1,15,1,15,1,15,1,15,1,15,1,15,3,15,308,8,15,
  	1,16,1,16,1,16,1,16,5,16,314,8,16,10,16,12,16,317,9,16,3,16,319,8,16,
  	1,16,1,16,1,17,1,17,1,17,1,17,1,17,1,18,1,18,1,18,1,18,1,18,1,18,1,18,
  	1,18,1,18,3,18,337,8,18,1,19,1,19,1,19,1,19,1,19,1,19,1,19,1,19,1,19,
  	1,19,1,19,1,19,1,19,1,19,1,19,3,19,354,8,19,1,19,1,19,1,19,1,19,1,19,
  	1,19,5,19,362,8,19,10,19,12,19,365,9,19,1,20,1,20,1,20,1,20,1,20,1,20,
  	1,20,1,20,1,20,1,20,1,20,1,20,1,20,1,20,1,20,1,20,3,20,383,8,20,1,21,
  	1,21,1,21,1,21,1,21,1,21,1,21,1,21,1,21,3,21,394,8,21,1,22,1,22,1,22,
  	1,22,1,22,1,22,1,22,1,22,1,22,1,22,1,22,1,22,1,22,1,22,3,22,410,8,22,
  	1,22,1,22,1,22,1,22,1,22,1,22,5,22,418,8,22,10,22,12,22,421,9,22,1,23,
  	3,23,424,8,23,1,23,1,23,1,23,1,23,1,23,1,23,3,23,432,8,23,1,23,1,23,1,
  	23,1,24,1,24,1,24,5,24,440,8,24,10,24,12,24,443,9,24,1,25,1,25,1,25,1,
  	25,1,25,1,25,1,25,1,25,1,25,1,25,1,25,1,25,1,25,1,25,1,25,1,25,1,25,1,
  	25,3,25,463,8,25,1,25,1,25,1,25,1,25,1,25,1,25,5,25,471,8,25,10,25,12,
  	25,474,9,25,1,26,1,26,3,26,478,8,26,1,27,1,27,1,27,1,27,1,27,1,27,1,27,
  	1,28,1,28,1,28,1,28,1,28,1,28,1,28,1,28,1,28,1,28,1,28,1,28,1,28,1,28,
  	1,28,1,28,1,28,1,28,1,28,1,28,1,28,1,28,1,28,3,28,510,8,28,1,29,1,29,
  	1,29,1,29,1,29,1,29,1,30,1,30,3,30,520,8,30,1,31,1,31,1,31,1,31,1,31,
  	1,31,1,32,1,32,1,32,1,32,1,32,1,32,1,33,1,33,1,33,1,33,3,33,538,8,33,
  	1,34,1,34,1,34,1,34,3,34,544,8,34,1,35,1,35,1,35,1,35,1,35,3,35,551,8,
  	35,1,35,1,35,1,35,1,35,1,36,1,36,1,36,1,36,1,37,1,37,1,37,1,37,1,37,1,
  	37,1,38,1,38,1,39,1,39,1,39,1,39,5,39,573,8,39,10,39,12,39,576,9,39,3,
  	39,578,8,39,1,39,1,39,1,40,1,40,1,40,1,41,1,41,1,42,1,42,1,42,3,42,590,
  	8,42,1,42,1,42,1,43,1,43,1,44,1,44,1,44,5,44,599,8,44,10,44,12,44,602,
  	9,44,1,45,1,45,1,45,1,45,1,45,1,45,1,46,1,46,1,46,1,46,1,46,5,46,615,
  	8,46,10,46,12,46,618,9,46,1,46,1,46,1,46,3,46,623,8,46,1,47,1,47,1,47,
  	1,47,1,48,1,48,1,48,1,49,1,49,1,49,1,50,1,50,1,50,1,50,1,50,1,50,3,50,
  	641,8,50,1,51,1,51,1,51,3,51,646,8,51,1,51,1,51,1,51,5,51,651,8,51,10,
  	51,12,51,654,9,51,1,52,1,52,1,52,1,52,1,52,1,52,1,52,1,53,1,53,1,53,1,
  	53,1,53,1,53,1,53,1,53,1,53,1,53,1,53,1,53,1,53,1,53,1,53,1,53,1,53,1,
  	53,3,53,681,8,53,1,54,1,54,1,54,1,54,1,54,1,54,1,54,1,54,1,54,1,54,1,
  	54,1,54,1,54,1,54,1,54,1,54,1,54,1,54,1,54,1,54,1,54,1,54,1,54,1,54,1,
  	54,1,54,1,54,1,54,1,54,1,54,1,54,1,54,1,54,1,54,1,54,1,54,1,54,1,54,1,
  	54,1,54,1,54,1,54,1,54,1,54,1,54,1,54,1,54,1,54,1,54,1,54,1,54,1,54,3,
  	54,735,8,54,1,54,1,54,1,54,1,54,1,54,1,54,1,54,1,54,1,54,5,54,746,8,54,
  	10,54,12,54,749,9,54,1,55,1,55,1,55,1,55,1,55,1,55,1,55,1,55,1,55,1,55,
  	1,55,1,55,1,55,1,55,1,55,1,55,3,55,767,8,55,1,56,1,56,1,56,1,56,5,56,
  	773,8,56,10,56,12,56,776,9,56,1,56,1,56,1,57,1,57,1,57,1,57,1,57,1,57,
  	1,57,3,57,787,8,57,1,58,1,58,1,58,1,58,1,58,1,58,1,58,1,58,1,58,1,58,
  	1,58,1,58,1,58,1,58,1,58,1,58,1,58,1,58,1,58,3,58,808,8,58,1,59,1,59,
  	1,59,0,5,38,44,50,102,108,60,0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,
  	30,32,34,36,38,40,42,44,46,48,50,52,54,56,58,60,62,64,66,68,70,72,74,
  	76,78,80,82,84,86,88,90,92,94,96,98,100,102,104,106,108,110,112,114,116,
  	118,0,10,2,0,10,11,70,71,1,0,68,69,1,0,79,84,1,0,77,78,5,0,29,29,31,31,
  	44,48,60,60,62,62,6,0,29,29,31,31,44,46,48,48,60,60,62,62,2,0,26,26,85,
  	85,2,0,66,66,75,76,1,0,73,74,1,0,70,71,874,0,124,1,0,0,0,2,150,1,0,0,
  	0,4,152,1,0,0,0,6,227,1,0,0,0,8,229,1,0,0,0,10,234,1,0,0,0,12,236,1,0,
  	0,0,14,240,1,0,0,0,16,247,1,0,0,0,18,255,1,0,0,0,20,263,1,0,0,0,22,265,
  	1,0,0,0,24,267,1,0,0,0,26,270,1,0,0,0,28,290,1,0,0,0,30,307,1,0,0,0,32,
  	309,1,0,0,0,34,322,1,0,0,0,36,336,1,0,0,0,38,353,1,0,0,0,40,382,1,0,0,
  	0,42,384,1,0,0,0,44,409,1,0,0,0,46,423,1,0,0,0,48,436,1,0,0,0,50,462,
  	1,0,0,0,52,477,1,0,0,0,54,479,1,0,0,0,56,509,1,0,0,0,58,511,1,0,0,0,60,
  	519,1,0,0,0,62,521,1,0,0,0,64,527,1,0,0,0,66,537,1,0,0,0,68,543,1,0,0,
  	0,70,545,1,0,0,0,72,556,1,0,0,0,74,560,1,0,0,0,76,566,1,0,0,0,78,568,
  	1,0,0,0,80,581,1,0,0,0,82,584,1,0,0,0,84,586,1,0,0,0,86,593,1,0,0,0,88,
  	595,1,0,0,0,90,603,1,0,0,0,92,622,1,0,0,0,94,624,1,0,0,0,96,628,1,0,0,
  	0,98,631,1,0,0,0,100,640,1,0,0,0,102,645,1,0,0,0,104,655,1,0,0,0,106,
  	680,1,0,0,0,108,734,1,0,0,0,110,766,1,0,0,0,112,768,1,0,0,0,114,786,1,
  	0,0,0,116,807,1,0,0,0,118,809,1,0,0,0,120,123,3,2,1,0,121,123,3,74,37,
  	0,122,120,1,0,0,0,122,121,1,0,0,0,123,126,1,0,0,0,124,122,1,0,0,0,124,
  	125,1,0,0,0,125,127,1,0,0,0,126,124,1,0,0,0,127,128,5,0,0,1,128,1,1,0,
  	0,0,129,151,3,6,3,0,130,151,3,42,21,0,131,151,3,100,50,0,132,151,3,58,
  	29,0,133,151,3,54,27,0,134,151,3,28,14,0,135,151,3,30,15,0,136,137,3,
  	84,42,0,137,138,5,1,0,0,138,151,1,0,0,0,139,151,3,90,45,0,140,151,3,46,
  	23,0,141,151,3,116,58,0,142,151,3,114,57,0,143,151,3,70,35,0,144,151,
  	3,72,36,0,145,151,3,60,30,0,146,151,3,34,17,0,147,151,3,40,20,0,148,151,
  	3,4,2,0,149,151,5,1,0,0,150,129,1,0,0,0,150,130,1,0,0,0,150,131,1,0,0,
  	0,150,132,1,0,0,0,150,133,1,0,0,0,150,134,1,0,0,0,150,135,1,0,0,0,150,
  	136,1,0,0,0,150,139,1,0,0,0,150,140,1,0,0,0,150,141,1,0,0,0,150,142,1,
  	0,0,0,150,143,1,0,0,0,150,144,1,0,0,0,150,145,1,0,0,0,150,146,1,0,0,0,
  	150,147,1,0,0,0,150,148,1,0,0,0,150,149,1,0,0,0,151,3,1,0,0,0,152,153,
  	5,2,0,0,153,154,5,3,0,0,154,155,5,85,0,0,155,156,5,4,0,0,156,157,5,85,
  	0,0,157,158,5,5,0,0,158,159,5,1,0,0,159,5,1,0,0,0,160,161,5,60,0,0,161,
  	162,3,20,10,0,162,166,5,6,0,0,163,165,3,8,4,0,164,163,1,0,0,0,165,168,
  	1,0,0,0,166,164,1,0,0,0,166,167,1,0,0,0,167,170,1,0,0,0,168,166,1,0,0,
  	0,169,171,3,12,6,0,170,169,1,0,0,0,170,171,1,0,0,0,171,175,1,0,0,0,172,
  	174,3,8,4,0,173,172,1,0,0,0,174,177,1,0,0,0,175,173,1,0,0,0,175,176,1,
  	0,0,0,176,179,1,0,0,0,177,175,1,0,0,0,178,180,3,14,7,0,179,178,1,0,0,
  	0,179,180,1,0,0,0,180,184,1,0,0,0,181,183,3,8,4,0,182,181,1,0,0,0,183,
  	186,1,0,0,0,184,182,1,0,0,0,184,185,1,0,0,0,185,187,1,0,0,0,186,184,1,
  	0,0,0,187,191,5,70,0,0,188,190,3,8,4,0,189,188,1,0,0,0,190,193,1,0,0,
  	0,191,189,1,0,0,0,191,192,1,0,0,0,192,194,1,0,0,0,193,191,1,0,0,0,194,
  	195,5,7,0,0,195,196,5,1,0,0,196,228,1,0,0,0,197,198,5,60,0,0,198,199,
  	3,20,10,0,199,203,5,6,0,0,200,202,3,8,4,0,201,200,1,0,0,0,202,205,1,0,
  	0,0,203,201,1,0,0,0,203,204,1,0,0,0,204,207,1,0,0,0,205,203,1,0,0,0,206,
  	208,3,12,6,0,207,206,1,0,0,0,207,208,1,0,0,0,208,212,1,0,0,0,209,211,
  	3,8,4,0,210,209,1,0,0,0,211,214,1,0,0,0,212,210,1,0,0,0,212,213,1,0,0,
  	0,213,216,1,0,0,0,214,212,1,0,0,0,215,217,3,14,7,0,216,215,1,0,0,0,216,
  	217,1,0,0,0,217,221,1,0,0,0,218,220,3,8,4,0,219,218,1,0,0,0,220,223,1,
  	0,0,0,221,219,1,0,0,0,221,222,1,0,0,0,222,224,1,0,0,0,223,221,1,0,0,0,
  	224,225,5,7,0,0,225,226,5,1,0,0,226,228,1,0,0,0,227,160,1,0,0,0,227,197,
  	1,0,0,0,228,7,1,0,0,0,229,230,5,8,0,0,230,231,5,9,0,0,231,232,3,10,5,
  	0,232,233,5,1,0,0,233,9,1,0,0,0,234,235,7,0,0,0,235,11,1,0,0,0,236,237,
  	5,12,0,0,237,238,3,16,8,0,238,239,5,1,0,0,239,13,1,0,0,0,240,243,5,13,
  	0,0,241,244,3,18,9,0,242,244,3,24,12,0,243,241,1,0,0,0,243,242,1,0,0,
  	0,244,245,1,0,0,0,245,246,5,1,0,0,246,15,1,0,0,0,247,252,3,22,11,0,248,
  	249,5,4,0,0,249,251,3,22,11,0,250,248,1,0,0,0,251,254,1,0,0,0,252,250,
  	1,0,0,0,252,253,1,0,0,0,253,17,1,0,0,0,254,252,1,0,0,0,255,260,3,26,13,
  	0,256,257,5,4,0,0,257,259,3,26,13,0,258,256,1,0,0,0,259,262,1,0,0,0,260,
  	258,1,0,0,0,260,261,1,0,0,0,261,19,1,0,0,0,262,260,1,0,0,0,263,264,5,
  	85,0,0,264,21,1,0,0,0,265,266,5,86,0,0,266,23,1,0,0,0,267,268,5,14,0,
  	0,268,269,5,88,0,0,269,25,1,0,0,0,270,271,3,22,11,0,271,272,5,15,0,0,
  	272,273,3,22,11,0,273,27,1,0,0,0,274,275,3,82,41,0,275,278,5,85,0,0,276,
  	277,5,16,0,0,277,279,3,108,54,0,278,276,1,0,0,0,278,279,1,0,0,0,279,280,
  	1,0,0,0,280,281,5,1,0,0,281,291,1,0,0,0,282,283,3,82,41,0,283,286,3,110,
  	55,0,284,285,5,16,0,0,285,287,3,112,56,0,286,284,1,0,0,0,286,287,1,0,
  	0,0,287,288,1,0,0,0,288,289,5,1,0,0,289,291,1,0,0,0,290,274,1,0,0,0,290,
  	282,1,0,0,0,291,29,1,0,0,0,292,293,5,62,0,0,293,294,5,85,0,0,294,308,
  	5,1,0,0,295,296,5,62,0,0,296,297,5,85,0,0,297,298,5,16,0,0,298,299,3,
  	32,16,0,299,300,5,1,0,0,300,308,1,0,0,0,301,302,5,62,0,0,302,303,5,85,
  	0,0,303,304,5,16,0,0,304,305,3,38,19,0,305,306,5,1,0,0,306,308,1,0,0,
  	0,307,292,1,0,0,0,307,295,1,0,0,0,307,301,1,0,0,0,308,31,1,0,0,0,309,
  	318,5,6,0,0,310,315,3,108,54,0,311,312,5,4,0,0,312,314,3,108,54,0,313,
  	311,1,0,0,0,314,317,1,0,0,0,315,313,1,0,0,0,315,316,1,0,0,0,316,319,1,
  	0,0,0,317,315,1,0,0,0,318,310,1,0,0,0,318,319,1,0,0,0,319,320,1,0,0,0,
  	320,321,5,7,0,0,321,33,1,0,0,0,322,323,5,85,0,0,323,324,5,16,0,0,324,
  	325,3,38,19,0,325,326,5,1,0,0,326,35,1,0,0,0,327,337,5,85,0,0,328,329,
  	3,20,10,0,329,330,5,17,0,0,330,331,7,1,0,0,331,337,1,0,0,0,332,333,3,
  	20,10,0,333,334,5,17,0,0,334,335,5,67,0,0,335,337,1,0,0,0,336,327,1,0,
  	0,0,336,328,1,0,0,0,336,332,1,0,0,0,337,37,1,0,0,0,338,339,6,19,-1,0,
  	339,340,3,20,10,0,340,341,5,17,0,0,341,342,7,1,0,0,342,354,1,0,0,0,343,
  	344,3,20,10,0,344,345,5,17,0,0,345,346,5,67,0,0,346,354,1,0,0,0,347,354,
  	5,85,0,0,348,354,3,32,16,0,349,350,5,3,0,0,350,351,3,38,19,0,351,352,
  	5,5,0,0,352,354,1,0,0,0,353,338,1,0,0,0,353,343,1,0,0,0,353,347,1,0,0,
  	0,353,348,1,0,0,0,353,349,1,0,0,0,354,363,1,0,0,0,355,356,10,7,0,0,356,
  	357,5,63,0,0,357,362,3,38,19,8,358,359,10,6,0,0,359,360,5,64,0,0,360,
  	362,3,38,19,7,361,355,1,0,0,0,361,358,1,0,0,0,362,365,1,0,0,0,363,361,
  	1,0,0,0,363,364,1,0,0,0,364,39,1,0,0,0,365,363,1,0,0,0,366,367,3,36,18,
  	0,367,368,5,17,0,0,368,369,5,18,0,0,369,370,5,3,0,0,370,371,3,108,54,
  	0,371,372,5,5,0,0,372,373,5,1,0,0,373,383,1,0,0,0,374,375,3,36,18,0,375,
  	376,5,17,0,0,376,377,5,19,0,0,377,378,5,3,0,0,378,379,3,108,54,0,379,
  	380,5,5,0,0,380,381,5,1,0,0,381,383,1,0,0,0,382,366,1,0,0,0,382,374,1,
  	0,0,0,383,41,1,0,0,0,384,385,5,20,0,0,385,386,5,3,0,0,386,387,3,44,22,
  	0,387,388,5,5,0,0,388,393,3,92,46,0,389,390,5,21,0,0,390,394,3,42,21,
  	0,391,392,5,21,0,0,392,394,3,92,46,0,393,389,1,0,0,0,393,391,1,0,0,0,
  	393,394,1,0,0,0,394,43,1,0,0,0,395,396,6,22,-1,0,396,397,3,108,54,0,397,
  	398,7,2,0,0,398,399,3,108,54,0,399,410,1,0,0,0,400,401,3,22,11,0,401,
  	402,5,22,0,0,402,403,3,20,10,0,403,410,1,0,0,0,404,405,3,26,13,0,405,
  	406,5,22,0,0,406,407,3,20,10,0,407,410,1,0,0,0,408,410,3,108,54,0,409,
  	395,1,0,0,0,409,400,1,0,0,0,409,404,1,0,0,0,409,408,1,0,0,0,410,419,1,
  	0,0,0,411,412,10,6,0,0,412,413,5,77,0,0,413,418,3,44,22,7,414,415,10,
  	5,0,0,415,416,5,78,0,0,416,418,3,44,22,6,417,411,1,0,0,0,417,414,1,0,
  	0,0,418,421,1,0,0,0,419,417,1,0,0,0,419,420,1,0,0,0,420,45,1,0,0,0,421,
  	419,1,0,0,0,422,424,5,60,0,0,423,422,1,0,0,0,423,424,1,0,0,0,424,425,
  	1,0,0,0,425,426,5,85,0,0,426,427,5,16,0,0,427,428,5,23,0,0,428,431,3,
  	48,24,0,429,430,5,24,0,0,430,432,3,50,25,0,431,429,1,0,0,0,431,432,1,
  	0,0,0,432,433,1,0,0,0,433,434,5,25,0,0,434,435,5,1,0,0,435,47,1,0,0,0,
  	436,441,3,20,10,0,437,438,7,3,0,0,438,440,3,20,10,0,439,437,1,0,0,0,440,
  	443,1,0,0,0,441,439,1,0,0,0,441,442,1,0,0,0,442,49,1,0,0,0,443,441,1,
  	0,0,0,444,445,6,25,-1,0,445,446,5,26,0,0,446,447,7,2,0,0,447,463,5,86,
  	0,0,448,449,5,27,0,0,449,450,5,28,0,0,450,463,3,22,11,0,451,452,5,29,
  	0,0,452,453,5,30,0,0,453,463,3,108,54,0,454,455,5,31,0,0,455,456,5,22,
  	0,0,456,463,5,85,0,0,457,463,5,32,0,0,458,459,5,3,0,0,459,460,3,50,25,
  	0,460,461,5,5,0,0,461,463,1,0,0,0,462,444,1,0,0,0,462,448,1,0,0,0,462,
  	451,1,0,0,0,462,454,1,0,0,0,462,457,1,0,0,0,462,458,1,0,0,0,463,472,1,
  	0,0,0,464,465,10,8,0,0,465,466,5,77,0,0,466,471,3,50,25,9,467,468,10,
  	7,0,0,468,469,5,78,0,0,469,471,3,50,25,8,470,464,1,0,0,0,470,467,1,0,
  	0,0,471,474,1,0,0,0,472,470,1,0,0,0,472,473,1,0,0,0,473,51,1,0,0,0,474,
  	472,1,0,0,0,475,478,3,54,27,0,476,478,3,58,29,0,477,475,1,0,0,0,477,476,
  	1,0,0,0,478,53,1,0,0,0,479,480,5,33,0,0,480,481,5,34,0,0,481,482,3,56,
  	28,0,482,483,5,22,0,0,483,484,3,20,10,0,484,485,3,92,46,0,485,55,1,0,
  	0,0,486,487,5,31,0,0,487,510,5,85,0,0,488,489,5,29,0,0,489,490,5,85,0,
  	0,490,491,5,4,0,0,491,510,5,85,0,0,492,493,5,35,0,0,493,494,5,36,0,0,
  	494,495,5,85,0,0,495,496,5,72,0,0,496,510,3,108,54,0,497,498,5,22,0,0,
  	498,499,5,36,0,0,499,500,5,85,0,0,500,501,5,72,0,0,501,510,3,108,54,0,
  	502,503,5,36,0,0,503,504,5,85,0,0,504,505,5,72,0,0,505,510,3,108,54,0,
  	506,507,5,37,0,0,507,510,5,85,0,0,508,510,5,85,0,0,509,486,1,0,0,0,509,
  	488,1,0,0,0,509,492,1,0,0,0,509,497,1,0,0,0,509,502,1,0,0,0,509,506,1,
  	0,0,0,509,508,1,0,0,0,510,57,1,0,0,0,511,512,5,38,0,0,512,513,5,3,0,0,
  	513,514,3,44,22,0,514,515,5,5,0,0,515,516,3,92,46,0,516,59,1,0,0,0,517,
  	520,3,62,31,0,518,520,3,64,32,0,519,517,1,0,0,0,519,518,1,0,0,0,520,61,
  	1,0,0,0,521,522,5,18,0,0,522,523,3,66,33,0,523,524,5,39,0,0,524,525,3,
  	20,10,0,525,526,5,1,0,0,526,63,1,0,0,0,527,528,5,19,0,0,528,529,3,68,
  	34,0,529,530,5,40,0,0,530,531,3,20,10,0,531,532,5,1,0,0,532,65,1,0,0,
  	0,533,538,3,22,11,0,534,538,3,26,13,0,535,538,3,16,8,0,536,538,3,18,9,
  	0,537,533,1,0,0,0,537,534,1,0,0,0,537,535,1,0,0,0,537,536,1,0,0,0,538,
  	67,1,0,0,0,539,544,3,22,11,0,540,544,3,26,13,0,541,544,3,16,8,0,542,544,
  	3,18,9,0,543,539,1,0,0,0,543,540,1,0,0,0,543,541,1,0,0,0,543,542,1,0,
  	0,0,544,69,1,0,0,0,545,546,5,41,0,0,546,547,5,85,0,0,547,548,5,9,0,0,
  	548,550,5,88,0,0,549,551,5,86,0,0,550,549,1,0,0,0,550,551,1,0,0,0,551,
  	552,1,0,0,0,552,553,5,72,0,0,553,554,3,20,10,0,554,555,5,1,0,0,555,71,
  	1,0,0,0,556,557,5,42,0,0,557,558,3,20,10,0,558,559,5,1,0,0,559,73,1,0,
  	0,0,560,561,5,43,0,0,561,562,3,76,38,0,562,563,5,85,0,0,563,564,3,78,
  	39,0,564,565,3,92,46,0,565,75,1,0,0,0,566,567,7,4,0,0,567,77,1,0,0,0,
  	568,577,5,3,0,0,569,574,3,80,40,0,570,571,5,4,0,0,571,573,3,80,40,0,572,
  	570,1,0,0,0,573,576,1,0,0,0,574,572,1,0,0,0,574,575,1,0,0,0,575,578,1,
  	0,0,0,576,574,1,0,0,0,577,569,1,0,0,0,577,578,1,0,0,0,578,579,1,0,0,0,
  	579,580,5,5,0,0,580,79,1,0,0,0,581,582,3,82,41,0,582,583,5,85,0,0,583,
  	81,1,0,0,0,584,585,7,5,0,0,585,83,1,0,0,0,586,587,3,86,43,0,587,589,5,
  	3,0,0,588,590,3,88,44,0,589,588,1,0,0,0,589,590,1,0,0,0,590,591,1,0,0,
  	0,591,592,5,5,0,0,592,85,1,0,0,0,593,594,7,6,0,0,594,87,1,0,0,0,595,600,
  	3,108,54,0,596,597,5,4,0,0,597,599,3,108,54,0,598,596,1,0,0,0,599,602,
  	1,0,0,0,600,598,1,0,0,0,600,601,1,0,0,0,601,89,1,0,0,0,602,600,1,0,0,
  	0,603,604,5,49,0,0,604,605,5,3,0,0,605,606,3,108,54,0,606,607,5,5,0,0,
  	607,608,5,1,0,0,608,91,1,0,0,0,609,616,5,6,0,0,610,615,3,2,1,0,611,615,
  	3,94,47,0,612,615,3,96,48,0,613,615,3,98,49,0,614,610,1,0,0,0,614,611,
  	1,0,0,0,614,612,1,0,0,0,614,613,1,0,0,0,615,618,1,0,0,0,616,614,1,0,0,
  	0,616,617,1,0,0,0,617,619,1,0,0,0,618,616,1,0,0,0,619,623,5,7,0,0,620,
  	621,5,6,0,0,621,623,5,7,0,0,622,609,1,0,0,0,622,620,1,0,0,0,623,93,1,
  	0,0,0,624,625,5,50,0,0,625,626,3,108,54,0,626,627,5,1,0,0,627,95,1,0,
  	0,0,628,629,5,51,0,0,629,630,5,1,0,0,630,97,1,0,0,0,631,632,5,52,0,0,
  	632,633,5,1,0,0,633,99,1,0,0,0,634,635,5,53,0,0,635,636,3,102,51,0,636,
  	637,5,1,0,0,637,641,1,0,0,0,638,641,3,104,52,0,639,641,3,106,53,0,640,
  	634,1,0,0,0,640,638,1,0,0,0,640,639,1,0,0,0,641,101,1,0,0,0,642,643,6,
  	51,-1,0,643,646,5,88,0,0,644,646,3,108,54,0,645,642,1,0,0,0,645,644,1,
  	0,0,0,646,652,1,0,0,0,647,648,10,1,0,0,648,649,5,73,0,0,649,651,3,102,
  	51,2,650,647,1,0,0,0,651,654,1,0,0,0,652,650,1,0,0,0,652,653,1,0,0,0,
  	653,103,1,0,0,0,654,652,1,0,0,0,655,656,5,53,0,0,656,657,5,85,0,0,657,
  	658,5,23,0,0,658,659,3,108,54,0,659,660,5,25,0,0,660,661,5,1,0,0,661,
  	105,1,0,0,0,662,663,5,53,0,0,663,664,5,67,0,0,664,665,5,72,0,0,665,666,
  	3,20,10,0,666,667,5,1,0,0,667,681,1,0,0,0,668,669,5,53,0,0,669,670,5,
  	68,0,0,670,671,5,72,0,0,671,672,3,20,10,0,672,673,5,1,0,0,673,681,1,0,
  	0,0,674,675,5,53,0,0,675,676,5,60,0,0,676,677,5,72,0,0,677,678,3,20,10,
  	0,678,679,5,1,0,0,679,681,1,0,0,0,680,662,1,0,0,0,680,668,1,0,0,0,680,
  	674,1,0,0,0,681,107,1,0,0,0,682,683,6,54,-1,0,683,684,5,74,0,0,684,735,
  	3,108,54,18,685,686,5,65,0,0,686,735,3,108,54,17,687,735,3,84,42,0,688,
  	735,5,86,0,0,689,735,5,85,0,0,690,691,5,3,0,0,691,692,3,108,54,0,692,
  	693,5,5,0,0,693,735,1,0,0,0,694,695,5,85,0,0,695,696,5,23,0,0,696,697,
  	3,108,54,0,697,698,5,25,0,0,698,699,5,23,0,0,699,700,3,108,54,0,700,701,
  	5,25,0,0,701,735,1,0,0,0,702,703,5,85,0,0,703,704,5,23,0,0,704,705,3,
  	108,54,0,705,706,5,25,0,0,706,735,1,0,0,0,707,708,3,36,18,0,708,709,5,
  	17,0,0,709,710,5,54,0,0,710,711,5,3,0,0,711,712,3,108,54,0,712,713,5,
  	5,0,0,713,735,1,0,0,0,714,715,5,85,0,0,715,716,5,17,0,0,716,717,5,55,
  	0,0,717,718,5,3,0,0,718,735,5,5,0,0,719,720,5,85,0,0,720,721,5,17,0,0,
  	721,722,5,56,0,0,722,723,5,3,0,0,723,735,5,5,0,0,724,735,5,70,0,0,725,
  	735,5,71,0,0,726,727,5,85,0,0,727,735,5,57,0,0,728,735,5,87,0,0,729,735,
  	3,32,16,0,730,735,5,58,0,0,731,732,5,59,0,0,732,733,5,3,0,0,733,735,5,
  	5,0,0,734,682,1,0,0,0,734,685,1,0,0,0,734,687,1,0,0,0,734,688,1,0,0,0,
  	734,689,1,0,0,0,734,690,1,0,0,0,734,694,1,0,0,0,734,702,1,0,0,0,734,707,
  	1,0,0,0,734,714,1,0,0,0,734,719,1,0,0,0,734,724,1,0,0,0,734,725,1,0,0,
  	0,734,726,1,0,0,0,734,728,1,0,0,0,734,729,1,0,0,0,734,730,1,0,0,0,734,
  	731,1,0,0,0,735,747,1,0,0,0,736,737,10,21,0,0,737,738,7,3,0,0,738,746,
  	3,108,54,22,739,740,10,20,0,0,740,741,7,7,0,0,741,746,3,108,54,21,742,
  	743,10,19,0,0,743,744,7,8,0,0,744,746,3,108,54,20,745,736,1,0,0,0,745,
  	739,1,0,0,0,745,742,1,0,0,0,746,749,1,0,0,0,747,745,1,0,0,0,747,748,1,
  	0,0,0,748,109,1,0,0,0,749,747,1,0,0,0,750,751,5,85,0,0,751,752,5,23,0,
  	0,752,753,3,108,54,0,753,754,5,25,0,0,754,755,5,23,0,0,755,756,3,108,
  	54,0,756,757,5,25,0,0,757,767,1,0,0,0,758,759,5,85,0,0,759,760,5,23,0,
  	0,760,761,3,108,54,0,761,762,5,25,0,0,762,767,1,0,0,0,763,764,5,85,0,
  	0,764,765,5,23,0,0,765,767,5,25,0,0,766,750,1,0,0,0,766,758,1,0,0,0,766,
  	763,1,0,0,0,767,111,1,0,0,0,768,769,5,23,0,0,769,774,3,108,54,0,770,771,
  	5,4,0,0,771,773,3,108,54,0,772,770,1,0,0,0,773,776,1,0,0,0,774,772,1,
  	0,0,0,774,775,1,0,0,0,775,777,1,0,0,0,776,774,1,0,0,0,777,778,5,25,0,
  	0,778,113,1,0,0,0,779,780,5,85,0,0,780,781,5,16,0,0,781,782,3,108,54,
  	0,782,783,5,1,0,0,783,787,1,0,0,0,784,785,5,85,0,0,785,787,5,1,0,0,786,
  	779,1,0,0,0,786,784,1,0,0,0,787,115,1,0,0,0,788,789,5,85,0,0,789,790,
  	5,23,0,0,790,791,3,108,54,0,791,792,5,25,0,0,792,793,5,23,0,0,793,794,
  	3,108,54,0,794,795,5,25,0,0,795,796,5,16,0,0,796,797,3,108,54,0,797,798,
  	5,1,0,0,798,808,1,0,0,0,799,800,5,85,0,0,800,801,5,23,0,0,801,802,3,108,
  	54,0,802,803,5,25,0,0,803,804,5,16,0,0,804,805,3,108,54,0,805,806,5,1,
  	0,0,806,808,1,0,0,0,807,788,1,0,0,0,807,799,1,0,0,0,808,117,1,0,0,0,809,
  	810,7,9,0,0,810,119,1,0,0,0,63,122,124,150,166,170,175,179,184,191,203,
  	207,212,216,221,227,243,252,260,278,286,290,307,315,318,336,353,361,363,
  	382,393,409,417,419,423,431,441,462,470,472,477,509,519,537,543,550,574,
  	577,589,600,614,616,622,640,645,652,680,734,745,747,766,774,786,807
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  baseParserStaticData = staticData.release();
}

}

BaseParser::BaseParser(TokenStream *input) : BaseParser(input, antlr4::atn::ParserATNSimulatorOptions()) {}

BaseParser::BaseParser(TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options) : Parser(input) {
  BaseParser::initialize();
  _interpreter = new atn::ParserATNSimulator(this, *baseParserStaticData->atn, baseParserStaticData->decisionToDFA, baseParserStaticData->sharedContextCache, options);
}

BaseParser::~BaseParser() {
  delete _interpreter;
}

const atn::ATN& BaseParser::getATN() const {
  return *baseParserStaticData->atn;
}

std::string BaseParser::getGrammarFileName() const {
  return "Base.g4";
}

const std::vector<std::string>& BaseParser::getRuleNames() const {
  return baseParserStaticData->ruleNames;
}

const dfa::Vocabulary& BaseParser::getVocabulary() const {
  return baseParserStaticData->vocabulary;
}

antlr4::atn::SerializedATNView BaseParser::getSerializedATN() const {
  return baseParserStaticData->serializedATN;
}


//----------------- ProgramContext ------------------------------------------------------------------

BaseParser::ProgramContext::ProgramContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* BaseParser::ProgramContext::EOF() {
  return getToken(BaseParser::EOF, 0);
}

std::vector<BaseParser::StatementContext *> BaseParser::ProgramContext::statement() {
  return getRuleContexts<BaseParser::StatementContext>();
}

BaseParser::StatementContext* BaseParser::ProgramContext::statement(size_t i) {
  return getRuleContext<BaseParser::StatementContext>(i);
}

std::vector<BaseParser::FunctionContext *> BaseParser::ProgramContext::function() {
  return getRuleContexts<BaseParser::FunctionContext>();
}

BaseParser::FunctionContext* BaseParser::ProgramContext::function(size_t i) {
  return getRuleContext<BaseParser::FunctionContext>(i);
}


size_t BaseParser::ProgramContext::getRuleIndex() const {
  return BaseParser::RuleProgram;
}

void BaseParser::ProgramContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterProgram(this);
}

void BaseParser::ProgramContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitProgram(this);
}


std::any BaseParser::ProgramContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitProgram(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::ProgramContext* BaseParser::program() {
  ProgramContext *_localctx = _tracker.createInstance<ProgramContext>(_ctx, getState());
  enterRule(_localctx, 0, BaseParser::RuleProgram);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(124);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 5774597971905347590) != 0) || _la == BaseParser::ID) {
      setState(122);
      _errHandler->sync(this);
      switch (_input->LA(1)) {
        case BaseParser::T__0:
        case BaseParser::T__1:
        case BaseParser::T__17:
        case BaseParser::T__18:
        case BaseParser::T__19:
        case BaseParser::T__25:
        case BaseParser::T__28:
        case BaseParser::T__30:
        case BaseParser::T__32:
        case BaseParser::T__37:
        case BaseParser::T__40:
        case BaseParser::T__41:
        case BaseParser::T__43:
        case BaseParser::T__44:
        case BaseParser::T__45:
        case BaseParser::T__47:
        case BaseParser::T__48:
        case BaseParser::T__52:
        case BaseParser::GRAPH:
        case BaseParser::SET:
        case BaseParser::ID: {
          setState(120);
          statement();
          break;
        }

        case BaseParser::T__42: {
          setState(121);
          function();
          break;
        }

      default:
        throw NoViableAltException(this);
      }
      setState(126);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(127);
    match(BaseParser::EOF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- StatementContext ------------------------------------------------------------------

BaseParser::StatementContext::StatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

BaseParser::GraphDefContext* BaseParser::StatementContext::graphDef() {
  return getRuleContext<BaseParser::GraphDefContext>(0);
}

BaseParser::ConditionalStatementContext* BaseParser::StatementContext::conditionalStatement() {
  return getRuleContext<BaseParser::ConditionalStatementContext>(0);
}

BaseParser::PrintStatementContext* BaseParser::StatementContext::printStatement() {
  return getRuleContext<BaseParser::PrintStatementContext>(0);
}

BaseParser::WhileStatementContext* BaseParser::StatementContext::whileStatement() {
  return getRuleContext<BaseParser::WhileStatementContext>(0);
}

BaseParser::ForeachStatementContext* BaseParser::StatementContext::foreachStatement() {
  return getRuleContext<BaseParser::ForeachStatementContext>(0);
}

BaseParser::VarDeclContext* BaseParser::StatementContext::varDecl() {
  return getRuleContext<BaseParser::VarDeclContext>(0);
}

BaseParser::SetDeclContext* BaseParser::StatementContext::setDecl() {
  return getRuleContext<BaseParser::SetDeclContext>(0);
}

BaseParser::FunctionCallContext* BaseParser::StatementContext::functionCall() {
  return getRuleContext<BaseParser::FunctionCallContext>(0);
}

BaseParser::SleepStatementContext* BaseParser::StatementContext::sleepStatement() {
  return getRuleContext<BaseParser::SleepStatementContext>(0);
}

BaseParser::GraphComprehensionContext* BaseParser::StatementContext::graphComprehension() {
  return getRuleContext<BaseParser::GraphComprehensionContext>(0);
}

BaseParser::ArrayAssignStatementContext* BaseParser::StatementContext::arrayAssignStatement() {
  return getRuleContext<BaseParser::ArrayAssignStatementContext>(0);
}

BaseParser::AssignmentStatementContext* BaseParser::StatementContext::assignmentStatement() {
  return getRuleContext<BaseParser::AssignmentStatementContext>(0);
}

BaseParser::QueryStatementContext* BaseParser::StatementContext::queryStatement() {
  return getRuleContext<BaseParser::QueryStatementContext>(0);
}

BaseParser::ShowgraphContext* BaseParser::StatementContext::showgraph() {
  return getRuleContext<BaseParser::ShowgraphContext>(0);
}

BaseParser::NodeEdgeOperationContext* BaseParser::StatementContext::nodeEdgeOperation() {
  return getRuleContext<BaseParser::NodeEdgeOperationContext>(0);
}

BaseParser::SetOperationContext* BaseParser::StatementContext::setOperation() {
  return getRuleContext<BaseParser::SetOperationContext>(0);
}

BaseParser::SetMethodCallContext* BaseParser::StatementContext::setMethodCall() {
  return getRuleContext<BaseParser::SetMethodCallContext>(0);
}

BaseParser::SwapStatementContext* BaseParser::StatementContext::swapStatement() {
  return getRuleContext<BaseParser::SwapStatementContext>(0);
}


size_t BaseParser::StatementContext::getRuleIndex() const {
  return BaseParser::RuleStatement;
}

void BaseParser::StatementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterStatement(this);
}

void BaseParser::StatementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitStatement(this);
}


std::any BaseParser::StatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitStatement(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::StatementContext* BaseParser::statement() {
  StatementContext *_localctx = _tracker.createInstance<StatementContext>(_ctx, getState());
  enterRule(_localctx, 2, BaseParser::RuleStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(150);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 2, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(129);
      graphDef();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(130);
      conditionalStatement();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(131);
      printStatement();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(132);
      whileStatement();
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(133);
      foreachStatement();
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(134);
      varDecl();
      break;
    }

    case 7: {
      enterOuterAlt(_localctx, 7);
      setState(135);
      setDecl();
      break;
    }

    case 8: {
      enterOuterAlt(_localctx, 8);
      setState(136);
      functionCall();
      setState(137);
      match(BaseParser::T__0);
      break;
    }

    case 9: {
      enterOuterAlt(_localctx, 9);
      setState(139);
      sleepStatement();
      break;
    }

    case 10: {
      enterOuterAlt(_localctx, 10);
      setState(140);
      graphComprehension();
      break;
    }

    case 11: {
      enterOuterAlt(_localctx, 11);
      setState(141);
      arrayAssignStatement();
      break;
    }

    case 12: {
      enterOuterAlt(_localctx, 12);
      setState(142);
      assignmentStatement();
      break;
    }

    case 13: {
      enterOuterAlt(_localctx, 13);
      setState(143);
      queryStatement();
      break;
    }

    case 14: {
      enterOuterAlt(_localctx, 14);
      setState(144);
      showgraph();
      break;
    }

    case 15: {
      enterOuterAlt(_localctx, 15);
      setState(145);
      nodeEdgeOperation();
      break;
    }

    case 16: {
      enterOuterAlt(_localctx, 16);
      setState(146);
      setOperation();
      break;
    }

    case 17: {
      enterOuterAlt(_localctx, 17);
      setState(147);
      setMethodCall();
      break;
    }

    case 18: {
      enterOuterAlt(_localctx, 18);
      setState(148);
      swapStatement();
      break;
    }

    case 19: {
      enterOuterAlt(_localctx, 19);
      setState(149);
      match(BaseParser::T__0);
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- SwapStatementContext ------------------------------------------------------------------

BaseParser::SwapStatementContext::SwapStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<tree::TerminalNode *> BaseParser::SwapStatementContext::ID() {
  return getTokens(BaseParser::ID);
}

tree::TerminalNode* BaseParser::SwapStatementContext::ID(size_t i) {
  return getToken(BaseParser::ID, i);
}


size_t BaseParser::SwapStatementContext::getRuleIndex() const {
  return BaseParser::RuleSwapStatement;
}

void BaseParser::SwapStatementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterSwapStatement(this);
}

void BaseParser::SwapStatementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitSwapStatement(this);
}


std::any BaseParser::SwapStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitSwapStatement(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::SwapStatementContext* BaseParser::swapStatement() {
  SwapStatementContext *_localctx = _tracker.createInstance<SwapStatementContext>(_ctx, getState());
  enterRule(_localctx, 4, BaseParser::RuleSwapStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(152);
    match(BaseParser::T__1);
    setState(153);
    match(BaseParser::T__2);
    setState(154);
    match(BaseParser::ID);
    setState(155);
    match(BaseParser::T__3);
    setState(156);
    match(BaseParser::ID);
    setState(157);
    match(BaseParser::T__4);
    setState(158);
    match(BaseParser::T__0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- GraphDefContext ------------------------------------------------------------------

BaseParser::GraphDefContext::GraphDefContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t BaseParser::GraphDefContext::getRuleIndex() const {
  return BaseParser::RuleGraphDef;
}

void BaseParser::GraphDefContext::copyFrom(GraphDefContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- WeightedGraphDefContext ------------------------------------------------------------------

tree::TerminalNode* BaseParser::WeightedGraphDefContext::GRAPH() {
  return getToken(BaseParser::GRAPH, 0);
}

BaseParser::GraphIDContext* BaseParser::WeightedGraphDefContext::graphID() {
  return getRuleContext<BaseParser::GraphIDContext>(0);
}

tree::TerminalNode* BaseParser::WeightedGraphDefContext::TRUE() {
  return getToken(BaseParser::TRUE, 0);
}

std::vector<BaseParser::GraphPropertyContext *> BaseParser::WeightedGraphDefContext::graphProperty() {
  return getRuleContexts<BaseParser::GraphPropertyContext>();
}

BaseParser::GraphPropertyContext* BaseParser::WeightedGraphDefContext::graphProperty(size_t i) {
  return getRuleContext<BaseParser::GraphPropertyContext>(i);
}

BaseParser::NodesContext* BaseParser::WeightedGraphDefContext::nodes() {
  return getRuleContext<BaseParser::NodesContext>(0);
}

BaseParser::EdgesContext* BaseParser::WeightedGraphDefContext::edges() {
  return getRuleContext<BaseParser::EdgesContext>(0);
}

BaseParser::WeightedGraphDefContext::WeightedGraphDefContext(GraphDefContext *ctx) { copyFrom(ctx); }

void BaseParser::WeightedGraphDefContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterWeightedGraphDef(this);
}
void BaseParser::WeightedGraphDefContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitWeightedGraphDef(this);
}

std::any BaseParser::WeightedGraphDefContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitWeightedGraphDef(this);
  else
    return visitor->visitChildren(this);
}
//----------------- UnweightedGraphDefContext ------------------------------------------------------------------

tree::TerminalNode* BaseParser::UnweightedGraphDefContext::GRAPH() {
  return getToken(BaseParser::GRAPH, 0);
}

BaseParser::GraphIDContext* BaseParser::UnweightedGraphDefContext::graphID() {
  return getRuleContext<BaseParser::GraphIDContext>(0);
}

std::vector<BaseParser::GraphPropertyContext *> BaseParser::UnweightedGraphDefContext::graphProperty() {
  return getRuleContexts<BaseParser::GraphPropertyContext>();
}

BaseParser::GraphPropertyContext* BaseParser::UnweightedGraphDefContext::graphProperty(size_t i) {
  return getRuleContext<BaseParser::GraphPropertyContext>(i);
}

BaseParser::NodesContext* BaseParser::UnweightedGraphDefContext::nodes() {
  return getRuleContext<BaseParser::NodesContext>(0);
}

BaseParser::EdgesContext* BaseParser::UnweightedGraphDefContext::edges() {
  return getRuleContext<BaseParser::EdgesContext>(0);
}

BaseParser::UnweightedGraphDefContext::UnweightedGraphDefContext(GraphDefContext *ctx) { copyFrom(ctx); }

void BaseParser::UnweightedGraphDefContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnweightedGraphDef(this);
}
void BaseParser::UnweightedGraphDefContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnweightedGraphDef(this);
}

std::any BaseParser::UnweightedGraphDefContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitUnweightedGraphDef(this);
  else
    return visitor->visitChildren(this);
}
BaseParser::GraphDefContext* BaseParser::graphDef() {
  GraphDefContext *_localctx = _tracker.createInstance<GraphDefContext>(_ctx, getState());
  enterRule(_localctx, 6, BaseParser::RuleGraphDef);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    setState(227);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 14, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<BaseParser::WeightedGraphDefContext>(_localctx);
      enterOuterAlt(_localctx, 1);
      setState(160);
      match(BaseParser::GRAPH);
      setState(161);
      graphID();
      setState(162);
      match(BaseParser::T__5);
      setState(166);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 3, _ctx);
      while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
        if (alt == 1) {
          setState(163);
          graphProperty(); 
        }
        setState(168);
        _errHandler->sync(this);
        alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 3, _ctx);
      }
      setState(170);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == BaseParser::T__11) {
        setState(169);
        nodes();
      }
      setState(175);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 5, _ctx);
      while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
        if (alt == 1) {
          setState(172);
          graphProperty(); 
        }
        setState(177);
        _errHandler->sync(this);
        alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 5, _ctx);
      }
      setState(179);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == BaseParser::T__12) {
        setState(178);
        edges();
      }
      setState(184);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == BaseParser::T__7) {
        setState(181);
        graphProperty();
        setState(186);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
      setState(187);
      match(BaseParser::TRUE);
      setState(191);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == BaseParser::T__7) {
        setState(188);
        graphProperty();
        setState(193);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
      setState(194);
      match(BaseParser::T__6);
      setState(195);
      match(BaseParser::T__0);
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<BaseParser::UnweightedGraphDefContext>(_localctx);
      enterOuterAlt(_localctx, 2);
      setState(197);
      match(BaseParser::GRAPH);
      setState(198);
      graphID();
      setState(199);
      match(BaseParser::T__5);
      setState(203);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 9, _ctx);
      while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
        if (alt == 1) {
          setState(200);
          graphProperty(); 
        }
        setState(205);
        _errHandler->sync(this);
        alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 9, _ctx);
      }
      setState(207);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == BaseParser::T__11) {
        setState(206);
        nodes();
      }
      setState(212);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 11, _ctx);
      while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
        if (alt == 1) {
          setState(209);
          graphProperty(); 
        }
        setState(214);
        _errHandler->sync(this);
        alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 11, _ctx);
      }
      setState(216);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == BaseParser::T__12) {
        setState(215);
        edges();
      }
      setState(221);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == BaseParser::T__7) {
        setState(218);
        graphProperty();
        setState(223);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
      setState(224);
      match(BaseParser::T__6);
      setState(225);
      match(BaseParser::T__0);
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- GraphPropertyContext ------------------------------------------------------------------

BaseParser::GraphPropertyContext::GraphPropertyContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

BaseParser::BoolLiteralContext* BaseParser::GraphPropertyContext::boolLiteral() {
  return getRuleContext<BaseParser::BoolLiteralContext>(0);
}


size_t BaseParser::GraphPropertyContext::getRuleIndex() const {
  return BaseParser::RuleGraphProperty;
}

void BaseParser::GraphPropertyContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterGraphProperty(this);
}

void BaseParser::GraphPropertyContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitGraphProperty(this);
}


std::any BaseParser::GraphPropertyContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitGraphProperty(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::GraphPropertyContext* BaseParser::graphProperty() {
  GraphPropertyContext *_localctx = _tracker.createInstance<GraphPropertyContext>(_ctx, getState());
  enterRule(_localctx, 8, BaseParser::RuleGraphProperty);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(229);
    match(BaseParser::T__7);
    setState(230);
    match(BaseParser::T__8);
    setState(231);
    boolLiteral();
    setState(232);
    match(BaseParser::T__0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- BoolLiteralContext ------------------------------------------------------------------

BaseParser::BoolLiteralContext::BoolLiteralContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* BaseParser::BoolLiteralContext::TRUE() {
  return getToken(BaseParser::TRUE, 0);
}

tree::TerminalNode* BaseParser::BoolLiteralContext::FALSE() {
  return getToken(BaseParser::FALSE, 0);
}


size_t BaseParser::BoolLiteralContext::getRuleIndex() const {
  return BaseParser::RuleBoolLiteral;
}

void BaseParser::BoolLiteralContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterBoolLiteral(this);
}

void BaseParser::BoolLiteralContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitBoolLiteral(this);
}


std::any BaseParser::BoolLiteralContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitBoolLiteral(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::BoolLiteralContext* BaseParser::boolLiteral() {
  BoolLiteralContext *_localctx = _tracker.createInstance<BoolLiteralContext>(_ctx, getState());
  enterRule(_localctx, 10, BaseParser::RuleBoolLiteral);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(234);
    _la = _input->LA(1);
    if (!(((((_la - 10) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 10)) & 3458764513820540931) != 0))) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- NodesContext ------------------------------------------------------------------

BaseParser::NodesContext::NodesContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

BaseParser::NodeListContext* BaseParser::NodesContext::nodeList() {
  return getRuleContext<BaseParser::NodeListContext>(0);
}


size_t BaseParser::NodesContext::getRuleIndex() const {
  return BaseParser::RuleNodes;
}

void BaseParser::NodesContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterNodes(this);
}

void BaseParser::NodesContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitNodes(this);
}


std::any BaseParser::NodesContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitNodes(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::NodesContext* BaseParser::nodes() {
  NodesContext *_localctx = _tracker.createInstance<NodesContext>(_ctx, getState());
  enterRule(_localctx, 12, BaseParser::RuleNodes);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(236);
    match(BaseParser::T__11);
    setState(237);
    nodeList();
    setState(238);
    match(BaseParser::T__0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- EdgesContext ------------------------------------------------------------------

BaseParser::EdgesContext::EdgesContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

BaseParser::EdgeListContext* BaseParser::EdgesContext::edgeList() {
  return getRuleContext<BaseParser::EdgeListContext>(0);
}

BaseParser::FileEdgeListContext* BaseParser::EdgesContext::fileEdgeList() {
  return getRuleContext<BaseParser::FileEdgeListContext>(0);
}


size_t BaseParser::EdgesContext::getRuleIndex() const {
  return BaseParser::RuleEdges;
}

void BaseParser::EdgesContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterEdges(this);
}

void BaseParser::EdgesContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitEdges(this);
}


std::any BaseParser::EdgesContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitEdges(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::EdgesContext* BaseParser::edges() {
  EdgesContext *_localctx = _tracker.createInstance<EdgesContext>(_ctx, getState());
  enterRule(_localctx, 14, BaseParser::RuleEdges);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(240);
    match(BaseParser::T__12);
    setState(243);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case BaseParser::INT: {
        setState(241);
        edgeList();
        break;
      }

      case BaseParser::T__13: {
        setState(242);
        fileEdgeList();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    setState(245);
    match(BaseParser::T__0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- NodeListContext ------------------------------------------------------------------

BaseParser::NodeListContext::NodeListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<BaseParser::NodeIDContext *> BaseParser::NodeListContext::nodeID() {
  return getRuleContexts<BaseParser::NodeIDContext>();
}

BaseParser::NodeIDContext* BaseParser::NodeListContext::nodeID(size_t i) {
  return getRuleContext<BaseParser::NodeIDContext>(i);
}


size_t BaseParser::NodeListContext::getRuleIndex() const {
  return BaseParser::RuleNodeList;
}

void BaseParser::NodeListContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterNodeList(this);
}

void BaseParser::NodeListContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitNodeList(this);
}


std::any BaseParser::NodeListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitNodeList(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::NodeListContext* BaseParser::nodeList() {
  NodeListContext *_localctx = _tracker.createInstance<NodeListContext>(_ctx, getState());
  enterRule(_localctx, 16, BaseParser::RuleNodeList);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(247);
    nodeID();
    setState(252);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == BaseParser::T__3) {
      setState(248);
      match(BaseParser::T__3);
      setState(249);
      nodeID();
      setState(254);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- EdgeListContext ------------------------------------------------------------------

BaseParser::EdgeListContext::EdgeListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<BaseParser::EdgeContext *> BaseParser::EdgeListContext::edge() {
  return getRuleContexts<BaseParser::EdgeContext>();
}

BaseParser::EdgeContext* BaseParser::EdgeListContext::edge(size_t i) {
  return getRuleContext<BaseParser::EdgeContext>(i);
}


size_t BaseParser::EdgeListContext::getRuleIndex() const {
  return BaseParser::RuleEdgeList;
}

void BaseParser::EdgeListContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterEdgeList(this);
}

void BaseParser::EdgeListContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitEdgeList(this);
}


std::any BaseParser::EdgeListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitEdgeList(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::EdgeListContext* BaseParser::edgeList() {
  EdgeListContext *_localctx = _tracker.createInstance<EdgeListContext>(_ctx, getState());
  enterRule(_localctx, 18, BaseParser::RuleEdgeList);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(255);
    edge();
    setState(260);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == BaseParser::T__3) {
      setState(256);
      match(BaseParser::T__3);
      setState(257);
      edge();
      setState(262);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- GraphIDContext ------------------------------------------------------------------

BaseParser::GraphIDContext::GraphIDContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* BaseParser::GraphIDContext::ID() {
  return getToken(BaseParser::ID, 0);
}


size_t BaseParser::GraphIDContext::getRuleIndex() const {
  return BaseParser::RuleGraphID;
}

void BaseParser::GraphIDContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterGraphID(this);
}

void BaseParser::GraphIDContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitGraphID(this);
}


std::any BaseParser::GraphIDContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitGraphID(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::GraphIDContext* BaseParser::graphID() {
  GraphIDContext *_localctx = _tracker.createInstance<GraphIDContext>(_ctx, getState());
  enterRule(_localctx, 20, BaseParser::RuleGraphID);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(263);
    match(BaseParser::ID);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- NodeIDContext ------------------------------------------------------------------

BaseParser::NodeIDContext::NodeIDContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* BaseParser::NodeIDContext::INT() {
  return getToken(BaseParser::INT, 0);
}


size_t BaseParser::NodeIDContext::getRuleIndex() const {
  return BaseParser::RuleNodeID;
}

void BaseParser::NodeIDContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterNodeID(this);
}

void BaseParser::NodeIDContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitNodeID(this);
}


std::any BaseParser::NodeIDContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitNodeID(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::NodeIDContext* BaseParser::nodeID() {
  NodeIDContext *_localctx = _tracker.createInstance<NodeIDContext>(_ctx, getState());
  enterRule(_localctx, 22, BaseParser::RuleNodeID);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(265);
    match(BaseParser::INT);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FileEdgeListContext ------------------------------------------------------------------

BaseParser::FileEdgeListContext::FileEdgeListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* BaseParser::FileEdgeListContext::STRING() {
  return getToken(BaseParser::STRING, 0);
}


size_t BaseParser::FileEdgeListContext::getRuleIndex() const {
  return BaseParser::RuleFileEdgeList;
}

void BaseParser::FileEdgeListContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterFileEdgeList(this);
}

void BaseParser::FileEdgeListContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitFileEdgeList(this);
}


std::any BaseParser::FileEdgeListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitFileEdgeList(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::FileEdgeListContext* BaseParser::fileEdgeList() {
  FileEdgeListContext *_localctx = _tracker.createInstance<FileEdgeListContext>(_ctx, getState());
  enterRule(_localctx, 24, BaseParser::RuleFileEdgeList);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(267);
    match(BaseParser::T__13);
    setState(268);
    match(BaseParser::STRING);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- EdgeContext ------------------------------------------------------------------

BaseParser::EdgeContext::EdgeContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<BaseParser::NodeIDContext *> BaseParser::EdgeContext::nodeID() {
  return getRuleContexts<BaseParser::NodeIDContext>();
}

BaseParser::NodeIDContext* BaseParser::EdgeContext::nodeID(size_t i) {
  return getRuleContext<BaseParser::NodeIDContext>(i);
}


size_t BaseParser::EdgeContext::getRuleIndex() const {
  return BaseParser::RuleEdge;
}

void BaseParser::EdgeContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterEdge(this);
}

void BaseParser::EdgeContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitEdge(this);
}


std::any BaseParser::EdgeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitEdge(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::EdgeContext* BaseParser::edge() {
  EdgeContext *_localctx = _tracker.createInstance<EdgeContext>(_ctx, getState());
  enterRule(_localctx, 26, BaseParser::RuleEdge);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(270);
    nodeID();
    setState(271);
    match(BaseParser::T__14);
    setState(272);
    nodeID();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- VarDeclContext ------------------------------------------------------------------

BaseParser::VarDeclContext::VarDeclContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t BaseParser::VarDeclContext::getRuleIndex() const {
  return BaseParser::RuleVarDecl;
}

void BaseParser::VarDeclContext::copyFrom(VarDeclContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- SimpleDeclarationContext ------------------------------------------------------------------

BaseParser::TypeContext* BaseParser::SimpleDeclarationContext::type() {
  return getRuleContext<BaseParser::TypeContext>(0);
}

tree::TerminalNode* BaseParser::SimpleDeclarationContext::ID() {
  return getToken(BaseParser::ID, 0);
}

BaseParser::ExprContext* BaseParser::SimpleDeclarationContext::expr() {
  return getRuleContext<BaseParser::ExprContext>(0);
}

BaseParser::SimpleDeclarationContext::SimpleDeclarationContext(VarDeclContext *ctx) { copyFrom(ctx); }

void BaseParser::SimpleDeclarationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterSimpleDeclaration(this);
}
void BaseParser::SimpleDeclarationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitSimpleDeclaration(this);
}

std::any BaseParser::SimpleDeclarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitSimpleDeclaration(this);
  else
    return visitor->visitChildren(this);
}
//----------------- ArrayDeclarationContext ------------------------------------------------------------------

BaseParser::TypeContext* BaseParser::ArrayDeclarationContext::type() {
  return getRuleContext<BaseParser::TypeContext>(0);
}

BaseParser::ArrayDeclaratorContext* BaseParser::ArrayDeclarationContext::arrayDeclarator() {
  return getRuleContext<BaseParser::ArrayDeclaratorContext>(0);
}

BaseParser::ArrayInitializerContext* BaseParser::ArrayDeclarationContext::arrayInitializer() {
  return getRuleContext<BaseParser::ArrayInitializerContext>(0);
}

BaseParser::ArrayDeclarationContext::ArrayDeclarationContext(VarDeclContext *ctx) { copyFrom(ctx); }

void BaseParser::ArrayDeclarationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterArrayDeclaration(this);
}
void BaseParser::ArrayDeclarationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitArrayDeclaration(this);
}

std::any BaseParser::ArrayDeclarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitArrayDeclaration(this);
  else
    return visitor->visitChildren(this);
}
BaseParser::VarDeclContext* BaseParser::varDecl() {
  VarDeclContext *_localctx = _tracker.createInstance<VarDeclContext>(_ctx, getState());
  enterRule(_localctx, 28, BaseParser::RuleVarDecl);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(290);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 20, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<BaseParser::SimpleDeclarationContext>(_localctx);
      enterOuterAlt(_localctx, 1);
      setState(274);
      type();
      setState(275);
      match(BaseParser::ID);
      setState(278);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == BaseParser::T__15) {
        setState(276);
        match(BaseParser::T__15);
        setState(277);
        expr(0);
      }
      setState(280);
      match(BaseParser::T__0);
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<BaseParser::ArrayDeclarationContext>(_localctx);
      enterOuterAlt(_localctx, 2);
      setState(282);
      type();
      setState(283);
      arrayDeclarator();
      setState(286);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == BaseParser::T__15) {
        setState(284);
        match(BaseParser::T__15);
        setState(285);
        arrayInitializer();
      }
      setState(288);
      match(BaseParser::T__0);
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- SetDeclContext ------------------------------------------------------------------

BaseParser::SetDeclContext::SetDeclContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* BaseParser::SetDeclContext::SET() {
  return getToken(BaseParser::SET, 0);
}

tree::TerminalNode* BaseParser::SetDeclContext::ID() {
  return getToken(BaseParser::ID, 0);
}

BaseParser::SetInitializerContext* BaseParser::SetDeclContext::setInitializer() {
  return getRuleContext<BaseParser::SetInitializerContext>(0);
}

BaseParser::SetExprContext* BaseParser::SetDeclContext::setExpr() {
  return getRuleContext<BaseParser::SetExprContext>(0);
}


size_t BaseParser::SetDeclContext::getRuleIndex() const {
  return BaseParser::RuleSetDecl;
}

void BaseParser::SetDeclContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterSetDecl(this);
}

void BaseParser::SetDeclContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitSetDecl(this);
}


std::any BaseParser::SetDeclContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitSetDecl(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::SetDeclContext* BaseParser::setDecl() {
  SetDeclContext *_localctx = _tracker.createInstance<SetDeclContext>(_ctx, getState());
  enterRule(_localctx, 30, BaseParser::RuleSetDecl);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(307);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 21, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(292);
      match(BaseParser::SET);
      setState(293);
      match(BaseParser::ID);
      setState(294);
      match(BaseParser::T__0);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(295);
      match(BaseParser::SET);
      setState(296);
      match(BaseParser::ID);
      setState(297);
      match(BaseParser::T__15);
      setState(298);
      setInitializer();
      setState(299);
      match(BaseParser::T__0);
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(301);
      match(BaseParser::SET);
      setState(302);
      match(BaseParser::ID);
      setState(303);
      match(BaseParser::T__15);
      setState(304);
      setExpr(0);
      setState(305);
      match(BaseParser::T__0);
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- SetInitializerContext ------------------------------------------------------------------

BaseParser::SetInitializerContext::SetInitializerContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<BaseParser::ExprContext *> BaseParser::SetInitializerContext::expr() {
  return getRuleContexts<BaseParser::ExprContext>();
}

BaseParser::ExprContext* BaseParser::SetInitializerContext::expr(size_t i) {
  return getRuleContext<BaseParser::ExprContext>(i);
}


size_t BaseParser::SetInitializerContext::getRuleIndex() const {
  return BaseParser::RuleSetInitializer;
}

void BaseParser::SetInitializerContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterSetInitializer(this);
}

void BaseParser::SetInitializerContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitSetInitializer(this);
}


std::any BaseParser::SetInitializerContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitSetInitializer(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::SetInitializerContext* BaseParser::setInitializer() {
  SetInitializerContext *_localctx = _tracker.createInstance<SetInitializerContext>(_ctx, getState());
  enterRule(_localctx, 32, BaseParser::RuleSetInitializer);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(309);
    match(BaseParser::T__5);
    setState(318);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 864691128522244168) != 0) || ((((_la - 65) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 65)) & 7340641) != 0)) {
      setState(310);
      expr(0);
      setState(315);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == BaseParser::T__3) {
        setState(311);
        match(BaseParser::T__3);
        setState(312);
        expr(0);
        setState(317);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
    }
    setState(320);
    match(BaseParser::T__6);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- SetOperationContext ------------------------------------------------------------------

BaseParser::SetOperationContext::SetOperationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* BaseParser::SetOperationContext::ID() {
  return getToken(BaseParser::ID, 0);
}

BaseParser::SetExprContext* BaseParser::SetOperationContext::setExpr() {
  return getRuleContext<BaseParser::SetExprContext>(0);
}


size_t BaseParser::SetOperationContext::getRuleIndex() const {
  return BaseParser::RuleSetOperation;
}

void BaseParser::SetOperationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterSetOperation(this);
}

void BaseParser::SetOperationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitSetOperation(this);
}


std::any BaseParser::SetOperationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitSetOperation(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::SetOperationContext* BaseParser::setOperation() {
  SetOperationContext *_localctx = _tracker.createInstance<SetOperationContext>(_ctx, getState());
  enterRule(_localctx, 34, BaseParser::RuleSetOperation);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(322);
    match(BaseParser::ID);
    setState(323);
    match(BaseParser::T__15);
    setState(324);
    setExpr(0);
    setState(325);
    match(BaseParser::T__0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- SetTargetContext ------------------------------------------------------------------

BaseParser::SetTargetContext::SetTargetContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* BaseParser::SetTargetContext::ID() {
  return getToken(BaseParser::ID, 0);
}

BaseParser::GraphIDContext* BaseParser::SetTargetContext::graphID() {
  return getRuleContext<BaseParser::GraphIDContext>(0);
}

tree::TerminalNode* BaseParser::SetTargetContext::NODE() {
  return getToken(BaseParser::NODE, 0);
}

tree::TerminalNode* BaseParser::SetTargetContext::VERTICES() {
  return getToken(BaseParser::VERTICES, 0);
}

tree::TerminalNode* BaseParser::SetTargetContext::EDGE() {
  return getToken(BaseParser::EDGE, 0);
}


size_t BaseParser::SetTargetContext::getRuleIndex() const {
  return BaseParser::RuleSetTarget;
}

void BaseParser::SetTargetContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterSetTarget(this);
}

void BaseParser::SetTargetContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitSetTarget(this);
}


std::any BaseParser::SetTargetContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitSetTarget(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::SetTargetContext* BaseParser::setTarget() {
  SetTargetContext *_localctx = _tracker.createInstance<SetTargetContext>(_ctx, getState());
  enterRule(_localctx, 36, BaseParser::RuleSetTarget);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(336);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 24, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(327);
      match(BaseParser::ID);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(328);
      graphID();
      setState(329);
      match(BaseParser::T__16);
      setState(330);
      _la = _input->LA(1);
      if (!(_la == BaseParser::NODE

      || _la == BaseParser::VERTICES)) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(332);
      graphID();
      setState(333);
      match(BaseParser::T__16);
      setState(334);
      match(BaseParser::EDGE);
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- SetExprContext ------------------------------------------------------------------

BaseParser::SetExprContext::SetExprContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t BaseParser::SetExprContext::getRuleIndex() const {
  return BaseParser::RuleSetExpr;
}

void BaseParser::SetExprContext::copyFrom(SetExprContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- SetUnionContext ------------------------------------------------------------------

std::vector<BaseParser::SetExprContext *> BaseParser::SetUnionContext::setExpr() {
  return getRuleContexts<BaseParser::SetExprContext>();
}

BaseParser::SetExprContext* BaseParser::SetUnionContext::setExpr(size_t i) {
  return getRuleContext<BaseParser::SetExprContext>(i);
}

tree::TerminalNode* BaseParser::SetUnionContext::UNION() {
  return getToken(BaseParser::UNION, 0);
}

BaseParser::SetUnionContext::SetUnionContext(SetExprContext *ctx) { copyFrom(ctx); }

void BaseParser::SetUnionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterSetUnion(this);
}
void BaseParser::SetUnionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitSetUnion(this);
}

std::any BaseParser::SetUnionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitSetUnion(this);
  else
    return visitor->visitChildren(this);
}
//----------------- GraphEdgesSetContext ------------------------------------------------------------------

BaseParser::GraphIDContext* BaseParser::GraphEdgesSetContext::graphID() {
  return getRuleContext<BaseParser::GraphIDContext>(0);
}

tree::TerminalNode* BaseParser::GraphEdgesSetContext::EDGE() {
  return getToken(BaseParser::EDGE, 0);
}

BaseParser::GraphEdgesSetContext::GraphEdgesSetContext(SetExprContext *ctx) { copyFrom(ctx); }

void BaseParser::GraphEdgesSetContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterGraphEdgesSet(this);
}
void BaseParser::GraphEdgesSetContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitGraphEdgesSet(this);
}

std::any BaseParser::GraphEdgesSetContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitGraphEdgesSet(this);
  else
    return visitor->visitChildren(this);
}
//----------------- SetIntersectContext ------------------------------------------------------------------

std::vector<BaseParser::SetExprContext *> BaseParser::SetIntersectContext::setExpr() {
  return getRuleContexts<BaseParser::SetExprContext>();
}

BaseParser::SetExprContext* BaseParser::SetIntersectContext::setExpr(size_t i) {
  return getRuleContext<BaseParser::SetExprContext>(i);
}

tree::TerminalNode* BaseParser::SetIntersectContext::INTERSECT() {
  return getToken(BaseParser::INTERSECT, 0);
}

BaseParser::SetIntersectContext::SetIntersectContext(SetExprContext *ctx) { copyFrom(ctx); }

void BaseParser::SetIntersectContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterSetIntersect(this);
}
void BaseParser::SetIntersectContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitSetIntersect(this);
}

std::any BaseParser::SetIntersectContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitSetIntersect(this);
  else
    return visitor->visitChildren(this);
}
//----------------- SetIdContext ------------------------------------------------------------------

tree::TerminalNode* BaseParser::SetIdContext::ID() {
  return getToken(BaseParser::ID, 0);
}

BaseParser::SetIdContext::SetIdContext(SetExprContext *ctx) { copyFrom(ctx); }

void BaseParser::SetIdContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterSetId(this);
}
void BaseParser::SetIdContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitSetId(this);
}

std::any BaseParser::SetIdContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitSetId(this);
  else
    return visitor->visitChildren(this);
}
//----------------- SetLiteralContext ------------------------------------------------------------------

BaseParser::SetInitializerContext* BaseParser::SetLiteralContext::setInitializer() {
  return getRuleContext<BaseParser::SetInitializerContext>(0);
}

BaseParser::SetLiteralContext::SetLiteralContext(SetExprContext *ctx) { copyFrom(ctx); }

void BaseParser::SetLiteralContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterSetLiteral(this);
}
void BaseParser::SetLiteralContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitSetLiteral(this);
}

std::any BaseParser::SetLiteralContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitSetLiteral(this);
  else
    return visitor->visitChildren(this);
}
//----------------- ParenSetContext ------------------------------------------------------------------

BaseParser::SetExprContext* BaseParser::ParenSetContext::setExpr() {
  return getRuleContext<BaseParser::SetExprContext>(0);
}

BaseParser::ParenSetContext::ParenSetContext(SetExprContext *ctx) { copyFrom(ctx); }

void BaseParser::ParenSetContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterParenSet(this);
}
void BaseParser::ParenSetContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitParenSet(this);
}

std::any BaseParser::ParenSetContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitParenSet(this);
  else
    return visitor->visitChildren(this);
}
//----------------- GraphNodesSetContext ------------------------------------------------------------------

BaseParser::GraphIDContext* BaseParser::GraphNodesSetContext::graphID() {
  return getRuleContext<BaseParser::GraphIDContext>(0);
}

tree::TerminalNode* BaseParser::GraphNodesSetContext::NODE() {
  return getToken(BaseParser::NODE, 0);
}

tree::TerminalNode* BaseParser::GraphNodesSetContext::VERTICES() {
  return getToken(BaseParser::VERTICES, 0);
}

BaseParser::GraphNodesSetContext::GraphNodesSetContext(SetExprContext *ctx) { copyFrom(ctx); }

void BaseParser::GraphNodesSetContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterGraphNodesSet(this);
}
void BaseParser::GraphNodesSetContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitGraphNodesSet(this);
}

std::any BaseParser::GraphNodesSetContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitGraphNodesSet(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::SetExprContext* BaseParser::setExpr() {
   return setExpr(0);
}

BaseParser::SetExprContext* BaseParser::setExpr(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  BaseParser::SetExprContext *_localctx = _tracker.createInstance<SetExprContext>(_ctx, parentState);
  BaseParser::SetExprContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 38;
  enterRecursionRule(_localctx, 38, BaseParser::RuleSetExpr, precedence);

    size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(353);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 25, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<GraphNodesSetContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;

      setState(339);
      graphID();
      setState(340);
      match(BaseParser::T__16);
      setState(341);
      _la = _input->LA(1);
      if (!(_la == BaseParser::NODE

      || _la == BaseParser::VERTICES)) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<GraphEdgesSetContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(343);
      graphID();
      setState(344);
      match(BaseParser::T__16);
      setState(345);
      match(BaseParser::EDGE);
      break;
    }

    case 3: {
      _localctx = _tracker.createInstance<SetIdContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(347);
      match(BaseParser::ID);
      break;
    }

    case 4: {
      _localctx = _tracker.createInstance<SetLiteralContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(348);
      setInitializer();
      break;
    }

    case 5: {
      _localctx = _tracker.createInstance<ParenSetContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(349);
      match(BaseParser::T__2);
      setState(350);
      setExpr(0);
      setState(351);
      match(BaseParser::T__4);
      break;
    }

    default:
      break;
    }
    _ctx->stop = _input->LT(-1);
    setState(363);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 27, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(361);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 26, _ctx)) {
        case 1: {
          auto newContext = _tracker.createInstance<SetUnionContext>(_tracker.createInstance<SetExprContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleSetExpr);
          setState(355);

          if (!(precpred(_ctx, 7))) throw FailedPredicateException(this, "precpred(_ctx, 7)");
          setState(356);
          match(BaseParser::UNION);
          setState(357);
          setExpr(8);
          break;
        }

        case 2: {
          auto newContext = _tracker.createInstance<SetIntersectContext>(_tracker.createInstance<SetExprContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleSetExpr);
          setState(358);

          if (!(precpred(_ctx, 6))) throw FailedPredicateException(this, "precpred(_ctx, 6)");
          setState(359);
          match(BaseParser::INTERSECT);
          setState(360);
          setExpr(7);
          break;
        }

        default:
          break;
        } 
      }
      setState(365);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 27, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

//----------------- SetMethodCallContext ------------------------------------------------------------------

BaseParser::SetMethodCallContext::SetMethodCallContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t BaseParser::SetMethodCallContext::getRuleIndex() const {
  return BaseParser::RuleSetMethodCall;
}

void BaseParser::SetMethodCallContext::copyFrom(SetMethodCallContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- SetRemoveMethodContext ------------------------------------------------------------------

BaseParser::SetTargetContext* BaseParser::SetRemoveMethodContext::setTarget() {
  return getRuleContext<BaseParser::SetTargetContext>(0);
}

BaseParser::ExprContext* BaseParser::SetRemoveMethodContext::expr() {
  return getRuleContext<BaseParser::ExprContext>(0);
}

BaseParser::SetRemoveMethodContext::SetRemoveMethodContext(SetMethodCallContext *ctx) { copyFrom(ctx); }

void BaseParser::SetRemoveMethodContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterSetRemoveMethod(this);
}
void BaseParser::SetRemoveMethodContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitSetRemoveMethod(this);
}

std::any BaseParser::SetRemoveMethodContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitSetRemoveMethod(this);
  else
    return visitor->visitChildren(this);
}
//----------------- SetAddMethodContext ------------------------------------------------------------------

BaseParser::SetTargetContext* BaseParser::SetAddMethodContext::setTarget() {
  return getRuleContext<BaseParser::SetTargetContext>(0);
}

BaseParser::ExprContext* BaseParser::SetAddMethodContext::expr() {
  return getRuleContext<BaseParser::ExprContext>(0);
}

BaseParser::SetAddMethodContext::SetAddMethodContext(SetMethodCallContext *ctx) { copyFrom(ctx); }

void BaseParser::SetAddMethodContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterSetAddMethod(this);
}
void BaseParser::SetAddMethodContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitSetAddMethod(this);
}

std::any BaseParser::SetAddMethodContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitSetAddMethod(this);
  else
    return visitor->visitChildren(this);
}
BaseParser::SetMethodCallContext* BaseParser::setMethodCall() {
  SetMethodCallContext *_localctx = _tracker.createInstance<SetMethodCallContext>(_ctx, getState());
  enterRule(_localctx, 40, BaseParser::RuleSetMethodCall);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(382);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 28, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<BaseParser::SetAddMethodContext>(_localctx);
      enterOuterAlt(_localctx, 1);
      setState(366);
      setTarget();
      setState(367);
      match(BaseParser::T__16);
      setState(368);
      match(BaseParser::T__17);
      setState(369);
      match(BaseParser::T__2);
      setState(370);
      expr(0);
      setState(371);
      match(BaseParser::T__4);
      setState(372);
      match(BaseParser::T__0);
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<BaseParser::SetRemoveMethodContext>(_localctx);
      enterOuterAlt(_localctx, 2);
      setState(374);
      setTarget();
      setState(375);
      match(BaseParser::T__16);
      setState(376);
      match(BaseParser::T__18);
      setState(377);
      match(BaseParser::T__2);
      setState(378);
      expr(0);
      setState(379);
      match(BaseParser::T__4);
      setState(380);
      match(BaseParser::T__0);
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ConditionalStatementContext ------------------------------------------------------------------

BaseParser::ConditionalStatementContext::ConditionalStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

BaseParser::ConditionContext* BaseParser::ConditionalStatementContext::condition() {
  return getRuleContext<BaseParser::ConditionContext>(0);
}

std::vector<BaseParser::BlockContext *> BaseParser::ConditionalStatementContext::block() {
  return getRuleContexts<BaseParser::BlockContext>();
}

BaseParser::BlockContext* BaseParser::ConditionalStatementContext::block(size_t i) {
  return getRuleContext<BaseParser::BlockContext>(i);
}

BaseParser::ConditionalStatementContext* BaseParser::ConditionalStatementContext::conditionalStatement() {
  return getRuleContext<BaseParser::ConditionalStatementContext>(0);
}


size_t BaseParser::ConditionalStatementContext::getRuleIndex() const {
  return BaseParser::RuleConditionalStatement;
}

void BaseParser::ConditionalStatementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterConditionalStatement(this);
}

void BaseParser::ConditionalStatementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitConditionalStatement(this);
}


std::any BaseParser::ConditionalStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitConditionalStatement(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::ConditionalStatementContext* BaseParser::conditionalStatement() {
  ConditionalStatementContext *_localctx = _tracker.createInstance<ConditionalStatementContext>(_ctx, getState());
  enterRule(_localctx, 42, BaseParser::RuleConditionalStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(384);
    match(BaseParser::T__19);
    setState(385);
    match(BaseParser::T__2);
    setState(386);
    condition(0);
    setState(387);
    match(BaseParser::T__4);
    setState(388);
    block();
    setState(393);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 29, _ctx)) {
    case 1: {
      setState(389);
      match(BaseParser::T__20);
      setState(390);
      conditionalStatement();
      break;
    }

    case 2: {
      setState(391);
      match(BaseParser::T__20);
      setState(392);
      block();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ConditionContext ------------------------------------------------------------------

BaseParser::ConditionContext::ConditionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t BaseParser::ConditionContext::getRuleIndex() const {
  return BaseParser::RuleCondition;
}

void BaseParser::ConditionContext::copyFrom(ConditionContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- ExprConditionContext ------------------------------------------------------------------

BaseParser::ExprContext* BaseParser::ExprConditionContext::expr() {
  return getRuleContext<BaseParser::ExprContext>(0);
}

BaseParser::ExprConditionContext::ExprConditionContext(ConditionContext *ctx) { copyFrom(ctx); }

void BaseParser::ExprConditionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterExprCondition(this);
}
void BaseParser::ExprConditionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitExprCondition(this);
}

std::any BaseParser::ExprConditionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitExprCondition(this);
  else
    return visitor->visitChildren(this);
}
//----------------- NodeCheckContext ------------------------------------------------------------------

BaseParser::NodeIDContext* BaseParser::NodeCheckContext::nodeID() {
  return getRuleContext<BaseParser::NodeIDContext>(0);
}

BaseParser::GraphIDContext* BaseParser::NodeCheckContext::graphID() {
  return getRuleContext<BaseParser::GraphIDContext>(0);
}

BaseParser::NodeCheckContext::NodeCheckContext(ConditionContext *ctx) { copyFrom(ctx); }

void BaseParser::NodeCheckContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterNodeCheck(this);
}
void BaseParser::NodeCheckContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitNodeCheck(this);
}

std::any BaseParser::NodeCheckContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitNodeCheck(this);
  else
    return visitor->visitChildren(this);
}
//----------------- LogicalAndContext ------------------------------------------------------------------

std::vector<BaseParser::ConditionContext *> BaseParser::LogicalAndContext::condition() {
  return getRuleContexts<BaseParser::ConditionContext>();
}

BaseParser::ConditionContext* BaseParser::LogicalAndContext::condition(size_t i) {
  return getRuleContext<BaseParser::ConditionContext>(i);
}

tree::TerminalNode* BaseParser::LogicalAndContext::AND() {
  return getToken(BaseParser::AND, 0);
}

BaseParser::LogicalAndContext::LogicalAndContext(ConditionContext *ctx) { copyFrom(ctx); }

void BaseParser::LogicalAndContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterLogicalAnd(this);
}
void BaseParser::LogicalAndContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitLogicalAnd(this);
}

std::any BaseParser::LogicalAndContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitLogicalAnd(this);
  else
    return visitor->visitChildren(this);
}
//----------------- RelationalContext ------------------------------------------------------------------

std::vector<BaseParser::ExprContext *> BaseParser::RelationalContext::expr() {
  return getRuleContexts<BaseParser::ExprContext>();
}

BaseParser::ExprContext* BaseParser::RelationalContext::expr(size_t i) {
  return getRuleContext<BaseParser::ExprContext>(i);
}

tree::TerminalNode* BaseParser::RelationalContext::EQUAL() {
  return getToken(BaseParser::EQUAL, 0);
}

tree::TerminalNode* BaseParser::RelationalContext::NOTEQUAL() {
  return getToken(BaseParser::NOTEQUAL, 0);
}

tree::TerminalNode* BaseParser::RelationalContext::LESSEQUAL() {
  return getToken(BaseParser::LESSEQUAL, 0);
}

tree::TerminalNode* BaseParser::RelationalContext::GREATEREQUAL() {
  return getToken(BaseParser::GREATEREQUAL, 0);
}

tree::TerminalNode* BaseParser::RelationalContext::LESSTHAN() {
  return getToken(BaseParser::LESSTHAN, 0);
}

tree::TerminalNode* BaseParser::RelationalContext::GREATERTHAN() {
  return getToken(BaseParser::GREATERTHAN, 0);
}

BaseParser::RelationalContext::RelationalContext(ConditionContext *ctx) { copyFrom(ctx); }

void BaseParser::RelationalContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRelational(this);
}
void BaseParser::RelationalContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRelational(this);
}

std::any BaseParser::RelationalContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitRelational(this);
  else
    return visitor->visitChildren(this);
}
//----------------- EdgeCheckContext ------------------------------------------------------------------

BaseParser::EdgeContext* BaseParser::EdgeCheckContext::edge() {
  return getRuleContext<BaseParser::EdgeContext>(0);
}

BaseParser::GraphIDContext* BaseParser::EdgeCheckContext::graphID() {
  return getRuleContext<BaseParser::GraphIDContext>(0);
}

BaseParser::EdgeCheckContext::EdgeCheckContext(ConditionContext *ctx) { copyFrom(ctx); }

void BaseParser::EdgeCheckContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterEdgeCheck(this);
}
void BaseParser::EdgeCheckContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitEdgeCheck(this);
}

std::any BaseParser::EdgeCheckContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitEdgeCheck(this);
  else
    return visitor->visitChildren(this);
}
//----------------- LogicalOrContext ------------------------------------------------------------------

std::vector<BaseParser::ConditionContext *> BaseParser::LogicalOrContext::condition() {
  return getRuleContexts<BaseParser::ConditionContext>();
}

BaseParser::ConditionContext* BaseParser::LogicalOrContext::condition(size_t i) {
  return getRuleContext<BaseParser::ConditionContext>(i);
}

tree::TerminalNode* BaseParser::LogicalOrContext::OR() {
  return getToken(BaseParser::OR, 0);
}

BaseParser::LogicalOrContext::LogicalOrContext(ConditionContext *ctx) { copyFrom(ctx); }

void BaseParser::LogicalOrContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterLogicalOr(this);
}
void BaseParser::LogicalOrContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitLogicalOr(this);
}

std::any BaseParser::LogicalOrContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitLogicalOr(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::ConditionContext* BaseParser::condition() {
   return condition(0);
}

BaseParser::ConditionContext* BaseParser::condition(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  BaseParser::ConditionContext *_localctx = _tracker.createInstance<ConditionContext>(_ctx, parentState);
  BaseParser::ConditionContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 44;
  enterRecursionRule(_localctx, 44, BaseParser::RuleCondition, precedence);

    size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(409);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 30, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<RelationalContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;

      setState(396);
      expr(0);
      setState(397);
      _la = _input->LA(1);
      if (!(((((_la - 79) & ~ 0x3fULL) == 0) &&
        ((1ULL << (_la - 79)) & 63) != 0))) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(398);
      expr(0);
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<NodeCheckContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(400);
      nodeID();
      setState(401);
      match(BaseParser::T__21);
      setState(402);
      graphID();
      break;
    }

    case 3: {
      _localctx = _tracker.createInstance<EdgeCheckContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(404);
      edge();
      setState(405);
      match(BaseParser::T__21);
      setState(406);
      graphID();
      break;
    }

    case 4: {
      _localctx = _tracker.createInstance<ExprConditionContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(408);
      expr(0);
      break;
    }

    default:
      break;
    }
    _ctx->stop = _input->LT(-1);
    setState(419);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 32, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(417);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 31, _ctx)) {
        case 1: {
          auto newContext = _tracker.createInstance<LogicalAndContext>(_tracker.createInstance<ConditionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleCondition);
          setState(411);

          if (!(precpred(_ctx, 6))) throw FailedPredicateException(this, "precpred(_ctx, 6)");
          setState(412);
          match(BaseParser::AND);
          setState(413);
          condition(7);
          break;
        }

        case 2: {
          auto newContext = _tracker.createInstance<LogicalOrContext>(_tracker.createInstance<ConditionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleCondition);
          setState(414);

          if (!(precpred(_ctx, 5))) throw FailedPredicateException(this, "precpred(_ctx, 5)");
          setState(415);
          match(BaseParser::OR);
          setState(416);
          condition(6);
          break;
        }

        default:
          break;
        } 
      }
      setState(421);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 32, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

//----------------- GraphComprehensionContext ------------------------------------------------------------------

BaseParser::GraphComprehensionContext::GraphComprehensionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* BaseParser::GraphComprehensionContext::ID() {
  return getToken(BaseParser::ID, 0);
}

BaseParser::GraphExprContext* BaseParser::GraphComprehensionContext::graphExpr() {
  return getRuleContext<BaseParser::GraphExprContext>(0);
}

tree::TerminalNode* BaseParser::GraphComprehensionContext::GRAPH() {
  return getToken(BaseParser::GRAPH, 0);
}

BaseParser::GraphConditionContext* BaseParser::GraphComprehensionContext::graphCondition() {
  return getRuleContext<BaseParser::GraphConditionContext>(0);
}


size_t BaseParser::GraphComprehensionContext::getRuleIndex() const {
  return BaseParser::RuleGraphComprehension;
}

void BaseParser::GraphComprehensionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterGraphComprehension(this);
}

void BaseParser::GraphComprehensionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitGraphComprehension(this);
}


std::any BaseParser::GraphComprehensionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitGraphComprehension(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::GraphComprehensionContext* BaseParser::graphComprehension() {
  GraphComprehensionContext *_localctx = _tracker.createInstance<GraphComprehensionContext>(_ctx, getState());
  enterRule(_localctx, 46, BaseParser::RuleGraphComprehension);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(423);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == BaseParser::GRAPH) {
      setState(422);
      match(BaseParser::GRAPH);
    }
    setState(425);
    match(BaseParser::ID);
    setState(426);
    match(BaseParser::T__15);
    setState(427);
    match(BaseParser::T__22);
    setState(428);
    graphExpr();
    setState(431);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == BaseParser::T__23) {
      setState(429);
      match(BaseParser::T__23);
      setState(430);
      graphCondition(0);
    }
    setState(433);
    match(BaseParser::T__24);
    setState(434);
    match(BaseParser::T__0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- GraphExprContext ------------------------------------------------------------------

BaseParser::GraphExprContext::GraphExprContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<BaseParser::GraphIDContext *> BaseParser::GraphExprContext::graphID() {
  return getRuleContexts<BaseParser::GraphIDContext>();
}

BaseParser::GraphIDContext* BaseParser::GraphExprContext::graphID(size_t i) {
  return getRuleContext<BaseParser::GraphIDContext>(i);
}

std::vector<tree::TerminalNode *> BaseParser::GraphExprContext::AND() {
  return getTokens(BaseParser::AND);
}

tree::TerminalNode* BaseParser::GraphExprContext::AND(size_t i) {
  return getToken(BaseParser::AND, i);
}

std::vector<tree::TerminalNode *> BaseParser::GraphExprContext::OR() {
  return getTokens(BaseParser::OR);
}

tree::TerminalNode* BaseParser::GraphExprContext::OR(size_t i) {
  return getToken(BaseParser::OR, i);
}


size_t BaseParser::GraphExprContext::getRuleIndex() const {
  return BaseParser::RuleGraphExpr;
}

void BaseParser::GraphExprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterGraphExpr(this);
}

void BaseParser::GraphExprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitGraphExpr(this);
}


std::any BaseParser::GraphExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitGraphExpr(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::GraphExprContext* BaseParser::graphExpr() {
  GraphExprContext *_localctx = _tracker.createInstance<GraphExprContext>(_ctx, getState());
  enterRule(_localctx, 48, BaseParser::RuleGraphExpr);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(436);
    graphID();
    setState(441);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == BaseParser::AND

    || _la == BaseParser::OR) {
      setState(437);
      _la = _input->LA(1);
      if (!(_la == BaseParser::AND

      || _la == BaseParser::OR)) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(438);
      graphID();
      setState(443);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- GraphConditionContext ------------------------------------------------------------------

BaseParser::GraphConditionContext::GraphConditionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t BaseParser::GraphConditionContext::getRuleIndex() const {
  return BaseParser::RuleGraphCondition;
}

void BaseParser::GraphConditionContext::copyFrom(GraphConditionContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- GraphLogicalAndContext ------------------------------------------------------------------

std::vector<BaseParser::GraphConditionContext *> BaseParser::GraphLogicalAndContext::graphCondition() {
  return getRuleContexts<BaseParser::GraphConditionContext>();
}

BaseParser::GraphConditionContext* BaseParser::GraphLogicalAndContext::graphCondition(size_t i) {
  return getRuleContext<BaseParser::GraphConditionContext>(i);
}

tree::TerminalNode* BaseParser::GraphLogicalAndContext::AND() {
  return getToken(BaseParser::AND, 0);
}

BaseParser::GraphLogicalAndContext::GraphLogicalAndContext(GraphConditionContext *ctx) { copyFrom(ctx); }

void BaseParser::GraphLogicalAndContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterGraphLogicalAnd(this);
}
void BaseParser::GraphLogicalAndContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitGraphLogicalAnd(this);
}

std::any BaseParser::GraphLogicalAndContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitGraphLogicalAnd(this);
  else
    return visitor->visitChildren(this);
}
//----------------- DegreeConditionContext ------------------------------------------------------------------

tree::TerminalNode* BaseParser::DegreeConditionContext::INT() {
  return getToken(BaseParser::INT, 0);
}

tree::TerminalNode* BaseParser::DegreeConditionContext::EQUAL() {
  return getToken(BaseParser::EQUAL, 0);
}

tree::TerminalNode* BaseParser::DegreeConditionContext::NOTEQUAL() {
  return getToken(BaseParser::NOTEQUAL, 0);
}

tree::TerminalNode* BaseParser::DegreeConditionContext::LESSEQUAL() {
  return getToken(BaseParser::LESSEQUAL, 0);
}

tree::TerminalNode* BaseParser::DegreeConditionContext::GREATEREQUAL() {
  return getToken(BaseParser::GREATEREQUAL, 0);
}

tree::TerminalNode* BaseParser::DegreeConditionContext::LESSTHAN() {
  return getToken(BaseParser::LESSTHAN, 0);
}

tree::TerminalNode* BaseParser::DegreeConditionContext::GREATERTHAN() {
  return getToken(BaseParser::GREATERTHAN, 0);
}

BaseParser::DegreeConditionContext::DegreeConditionContext(GraphConditionContext *ctx) { copyFrom(ctx); }

void BaseParser::DegreeConditionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterDegreeCondition(this);
}
void BaseParser::DegreeConditionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitDegreeCondition(this);
}

std::any BaseParser::DegreeConditionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitDegreeCondition(this);
  else
    return visitor->visitChildren(this);
}
//----------------- EdgeHasConditionContext ------------------------------------------------------------------

BaseParser::ExprContext* BaseParser::EdgeHasConditionContext::expr() {
  return getRuleContext<BaseParser::ExprContext>(0);
}

BaseParser::EdgeHasConditionContext::EdgeHasConditionContext(GraphConditionContext *ctx) { copyFrom(ctx); }

void BaseParser::EdgeHasConditionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterEdgeHasCondition(this);
}
void BaseParser::EdgeHasConditionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitEdgeHasCondition(this);
}

std::any BaseParser::EdgeHasConditionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitEdgeHasCondition(this);
  else
    return visitor->visitChildren(this);
}
//----------------- CycleConditionContext ------------------------------------------------------------------

BaseParser::CycleConditionContext::CycleConditionContext(GraphConditionContext *ctx) { copyFrom(ctx); }

void BaseParser::CycleConditionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterCycleCondition(this);
}
void BaseParser::CycleConditionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitCycleCondition(this);
}

std::any BaseParser::CycleConditionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitCycleCondition(this);
  else
    return visitor->visitChildren(this);
}
//----------------- ConnectedConditionContext ------------------------------------------------------------------

BaseParser::NodeIDContext* BaseParser::ConnectedConditionContext::nodeID() {
  return getRuleContext<BaseParser::NodeIDContext>(0);
}

BaseParser::ConnectedConditionContext::ConnectedConditionContext(GraphConditionContext *ctx) { copyFrom(ctx); }

void BaseParser::ConnectedConditionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterConnectedCondition(this);
}
void BaseParser::ConnectedConditionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitConnectedCondition(this);
}

std::any BaseParser::ConnectedConditionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitConnectedCondition(this);
  else
    return visitor->visitChildren(this);
}
//----------------- GraphLogicalOrContext ------------------------------------------------------------------

std::vector<BaseParser::GraphConditionContext *> BaseParser::GraphLogicalOrContext::graphCondition() {
  return getRuleContexts<BaseParser::GraphConditionContext>();
}

BaseParser::GraphConditionContext* BaseParser::GraphLogicalOrContext::graphCondition(size_t i) {
  return getRuleContext<BaseParser::GraphConditionContext>(i);
}

tree::TerminalNode* BaseParser::GraphLogicalOrContext::OR() {
  return getToken(BaseParser::OR, 0);
}

BaseParser::GraphLogicalOrContext::GraphLogicalOrContext(GraphConditionContext *ctx) { copyFrom(ctx); }

void BaseParser::GraphLogicalOrContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterGraphLogicalOr(this);
}
void BaseParser::GraphLogicalOrContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitGraphLogicalOr(this);
}

std::any BaseParser::GraphLogicalOrContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitGraphLogicalOr(this);
  else
    return visitor->visitChildren(this);
}
//----------------- ParenGraphConditionContext ------------------------------------------------------------------

BaseParser::GraphConditionContext* BaseParser::ParenGraphConditionContext::graphCondition() {
  return getRuleContext<BaseParser::GraphConditionContext>(0);
}

BaseParser::ParenGraphConditionContext::ParenGraphConditionContext(GraphConditionContext *ctx) { copyFrom(ctx); }

void BaseParser::ParenGraphConditionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterParenGraphCondition(this);
}
void BaseParser::ParenGraphConditionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitParenGraphCondition(this);
}

std::any BaseParser::ParenGraphConditionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitParenGraphCondition(this);
  else
    return visitor->visitChildren(this);
}
//----------------- VertexInSetConditionContext ------------------------------------------------------------------

tree::TerminalNode* BaseParser::VertexInSetConditionContext::ID() {
  return getToken(BaseParser::ID, 0);
}

BaseParser::VertexInSetConditionContext::VertexInSetConditionContext(GraphConditionContext *ctx) { copyFrom(ctx); }

void BaseParser::VertexInSetConditionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterVertexInSetCondition(this);
}
void BaseParser::VertexInSetConditionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitVertexInSetCondition(this);
}

std::any BaseParser::VertexInSetConditionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitVertexInSetCondition(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::GraphConditionContext* BaseParser::graphCondition() {
   return graphCondition(0);
}

BaseParser::GraphConditionContext* BaseParser::graphCondition(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  BaseParser::GraphConditionContext *_localctx = _tracker.createInstance<GraphConditionContext>(_ctx, parentState);
  BaseParser::GraphConditionContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 50;
  enterRecursionRule(_localctx, 50, BaseParser::RuleGraphCondition, precedence);

    size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(462);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case BaseParser::T__25: {
        _localctx = _tracker.createInstance<DegreeConditionContext>(_localctx);
        _ctx = _localctx;
        previousContext = _localctx;

        setState(445);
        match(BaseParser::T__25);
        setState(446);
        _la = _input->LA(1);
        if (!(((((_la - 79) & ~ 0x3fULL) == 0) &&
          ((1ULL << (_la - 79)) & 63) != 0))) {
        _errHandler->recoverInline(this);
        }
        else {
          _errHandler->reportMatch(this);
          consume();
        }
        setState(447);
        match(BaseParser::INT);
        break;
      }

      case BaseParser::T__26: {
        _localctx = _tracker.createInstance<ConnectedConditionContext>(_localctx);
        _ctx = _localctx;
        previousContext = _localctx;
        setState(448);
        match(BaseParser::T__26);
        setState(449);
        match(BaseParser::T__27);
        setState(450);
        nodeID();
        break;
      }

      case BaseParser::T__28: {
        _localctx = _tracker.createInstance<EdgeHasConditionContext>(_localctx);
        _ctx = _localctx;
        previousContext = _localctx;
        setState(451);
        match(BaseParser::T__28);
        setState(452);
        match(BaseParser::T__29);
        setState(453);
        expr(0);
        break;
      }

      case BaseParser::T__30: {
        _localctx = _tracker.createInstance<VertexInSetConditionContext>(_localctx);
        _ctx = _localctx;
        previousContext = _localctx;
        setState(454);
        match(BaseParser::T__30);
        setState(455);
        match(BaseParser::T__21);
        setState(456);
        match(BaseParser::ID);
        break;
      }

      case BaseParser::T__31: {
        _localctx = _tracker.createInstance<CycleConditionContext>(_localctx);
        _ctx = _localctx;
        previousContext = _localctx;
        setState(457);
        match(BaseParser::T__31);
        break;
      }

      case BaseParser::T__2: {
        _localctx = _tracker.createInstance<ParenGraphConditionContext>(_localctx);
        _ctx = _localctx;
        previousContext = _localctx;
        setState(458);
        match(BaseParser::T__2);
        setState(459);
        graphCondition(0);
        setState(460);
        match(BaseParser::T__4);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    _ctx->stop = _input->LT(-1);
    setState(472);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 38, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(470);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 37, _ctx)) {
        case 1: {
          auto newContext = _tracker.createInstance<GraphLogicalAndContext>(_tracker.createInstance<GraphConditionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleGraphCondition);
          setState(464);

          if (!(precpred(_ctx, 8))) throw FailedPredicateException(this, "precpred(_ctx, 8)");
          setState(465);
          match(BaseParser::AND);
          setState(466);
          graphCondition(9);
          break;
        }

        case 2: {
          auto newContext = _tracker.createInstance<GraphLogicalOrContext>(_tracker.createInstance<GraphConditionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleGraphCondition);
          setState(467);

          if (!(precpred(_ctx, 7))) throw FailedPredicateException(this, "precpred(_ctx, 7)");
          setState(468);
          match(BaseParser::OR);
          setState(469);
          graphCondition(8);
          break;
        }

        default:
          break;
        } 
      }
      setState(474);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 38, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

//----------------- LoopStatementContext ------------------------------------------------------------------

BaseParser::LoopStatementContext::LoopStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

BaseParser::ForeachStatementContext* BaseParser::LoopStatementContext::foreachStatement() {
  return getRuleContext<BaseParser::ForeachStatementContext>(0);
}

BaseParser::WhileStatementContext* BaseParser::LoopStatementContext::whileStatement() {
  return getRuleContext<BaseParser::WhileStatementContext>(0);
}


size_t BaseParser::LoopStatementContext::getRuleIndex() const {
  return BaseParser::RuleLoopStatement;
}

void BaseParser::LoopStatementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterLoopStatement(this);
}

void BaseParser::LoopStatementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitLoopStatement(this);
}


std::any BaseParser::LoopStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitLoopStatement(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::LoopStatementContext* BaseParser::loopStatement() {
  LoopStatementContext *_localctx = _tracker.createInstance<LoopStatementContext>(_ctx, getState());
  enterRule(_localctx, 52, BaseParser::RuleLoopStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(477);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case BaseParser::T__32: {
        enterOuterAlt(_localctx, 1);
        setState(475);
        foreachStatement();
        break;
      }

      case BaseParser::T__37: {
        enterOuterAlt(_localctx, 2);
        setState(476);
        whileStatement();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ForeachStatementContext ------------------------------------------------------------------

BaseParser::ForeachStatementContext::ForeachStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

BaseParser::LoopTargetContext* BaseParser::ForeachStatementContext::loopTarget() {
  return getRuleContext<BaseParser::LoopTargetContext>(0);
}

BaseParser::GraphIDContext* BaseParser::ForeachStatementContext::graphID() {
  return getRuleContext<BaseParser::GraphIDContext>(0);
}

BaseParser::BlockContext* BaseParser::ForeachStatementContext::block() {
  return getRuleContext<BaseParser::BlockContext>(0);
}


size_t BaseParser::ForeachStatementContext::getRuleIndex() const {
  return BaseParser::RuleForeachStatement;
}

void BaseParser::ForeachStatementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterForeachStatement(this);
}

void BaseParser::ForeachStatementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitForeachStatement(this);
}


std::any BaseParser::ForeachStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitForeachStatement(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::ForeachStatementContext* BaseParser::foreachStatement() {
  ForeachStatementContext *_localctx = _tracker.createInstance<ForeachStatementContext>(_ctx, getState());
  enterRule(_localctx, 54, BaseParser::RuleForeachStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(479);
    match(BaseParser::T__32);
    setState(480);
    match(BaseParser::T__33);
    setState(481);
    loopTarget();
    setState(482);
    match(BaseParser::T__21);
    setState(483);
    graphID();
    setState(484);
    block();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- LoopTargetContext ------------------------------------------------------------------

BaseParser::LoopTargetContext::LoopTargetContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t BaseParser::LoopTargetContext::getRuleIndex() const {
  return BaseParser::RuleLoopTarget;
}

void BaseParser::LoopTargetContext::copyFrom(LoopTargetContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- ForEachVertexContext ------------------------------------------------------------------

tree::TerminalNode* BaseParser::ForEachVertexContext::ID() {
  return getToken(BaseParser::ID, 0);
}

BaseParser::ForEachVertexContext::ForEachVertexContext(LoopTargetContext *ctx) { copyFrom(ctx); }

void BaseParser::ForEachVertexContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterForEachVertex(this);
}
void BaseParser::ForEachVertexContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitForEachVertex(this);
}

std::any BaseParser::ForEachVertexContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitForEachVertex(this);
  else
    return visitor->visitChildren(this);
}
//----------------- ForEachAdjContext ------------------------------------------------------------------

tree::TerminalNode* BaseParser::ForEachAdjContext::ID() {
  return getToken(BaseParser::ID, 0);
}

tree::TerminalNode* BaseParser::ForEachAdjContext::OF() {
  return getToken(BaseParser::OF, 0);
}

BaseParser::ExprContext* BaseParser::ForEachAdjContext::expr() {
  return getRuleContext<BaseParser::ExprContext>(0);
}

BaseParser::ForEachAdjContext::ForEachAdjContext(LoopTargetContext *ctx) { copyFrom(ctx); }

void BaseParser::ForEachAdjContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterForEachAdj(this);
}
void BaseParser::ForEachAdjContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitForEachAdj(this);
}

std::any BaseParser::ForEachAdjContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitForEachAdj(this);
  else
    return visitor->visitChildren(this);
}
//----------------- ForEachPlainContext ------------------------------------------------------------------

tree::TerminalNode* BaseParser::ForEachPlainContext::ID() {
  return getToken(BaseParser::ID, 0);
}

BaseParser::ForEachPlainContext::ForEachPlainContext(LoopTargetContext *ctx) { copyFrom(ctx); }

void BaseParser::ForEachPlainContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterForEachPlain(this);
}
void BaseParser::ForEachPlainContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitForEachPlain(this);
}

std::any BaseParser::ForEachPlainContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitForEachPlain(this);
  else
    return visitor->visitChildren(this);
}
//----------------- ForEachInAdjContext ------------------------------------------------------------------

tree::TerminalNode* BaseParser::ForEachInAdjContext::ID() {
  return getToken(BaseParser::ID, 0);
}

tree::TerminalNode* BaseParser::ForEachInAdjContext::OF() {
  return getToken(BaseParser::OF, 0);
}

BaseParser::ExprContext* BaseParser::ForEachInAdjContext::expr() {
  return getRuleContext<BaseParser::ExprContext>(0);
}

BaseParser::ForEachInAdjContext::ForEachInAdjContext(LoopTargetContext *ctx) { copyFrom(ctx); }

void BaseParser::ForEachInAdjContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterForEachInAdj(this);
}
void BaseParser::ForEachInAdjContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitForEachInAdj(this);
}

std::any BaseParser::ForEachInAdjContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitForEachInAdj(this);
  else
    return visitor->visitChildren(this);
}
//----------------- ForEachElementContext ------------------------------------------------------------------

tree::TerminalNode* BaseParser::ForEachElementContext::ID() {
  return getToken(BaseParser::ID, 0);
}

BaseParser::ForEachElementContext::ForEachElementContext(LoopTargetContext *ctx) { copyFrom(ctx); }

void BaseParser::ForEachElementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterForEachElement(this);
}
void BaseParser::ForEachElementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitForEachElement(this);
}

std::any BaseParser::ForEachElementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitForEachElement(this);
  else
    return visitor->visitChildren(this);
}
//----------------- ForEachOutAdjContext ------------------------------------------------------------------

tree::TerminalNode* BaseParser::ForEachOutAdjContext::ID() {
  return getToken(BaseParser::ID, 0);
}

tree::TerminalNode* BaseParser::ForEachOutAdjContext::OF() {
  return getToken(BaseParser::OF, 0);
}

BaseParser::ExprContext* BaseParser::ForEachOutAdjContext::expr() {
  return getRuleContext<BaseParser::ExprContext>(0);
}

BaseParser::ForEachOutAdjContext::ForEachOutAdjContext(LoopTargetContext *ctx) { copyFrom(ctx); }

void BaseParser::ForEachOutAdjContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterForEachOutAdj(this);
}
void BaseParser::ForEachOutAdjContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitForEachOutAdj(this);
}

std::any BaseParser::ForEachOutAdjContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitForEachOutAdj(this);
  else
    return visitor->visitChildren(this);
}
//----------------- ForEachEdgeContext ------------------------------------------------------------------

std::vector<tree::TerminalNode *> BaseParser::ForEachEdgeContext::ID() {
  return getTokens(BaseParser::ID);
}

tree::TerminalNode* BaseParser::ForEachEdgeContext::ID(size_t i) {
  return getToken(BaseParser::ID, i);
}

BaseParser::ForEachEdgeContext::ForEachEdgeContext(LoopTargetContext *ctx) { copyFrom(ctx); }

void BaseParser::ForEachEdgeContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterForEachEdge(this);
}
void BaseParser::ForEachEdgeContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitForEachEdge(this);
}

std::any BaseParser::ForEachEdgeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitForEachEdge(this);
  else
    return visitor->visitChildren(this);
}
BaseParser::LoopTargetContext* BaseParser::loopTarget() {
  LoopTargetContext *_localctx = _tracker.createInstance<LoopTargetContext>(_ctx, getState());
  enterRule(_localctx, 56, BaseParser::RuleLoopTarget);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(509);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case BaseParser::T__30: {
        _localctx = _tracker.createInstance<BaseParser::ForEachVertexContext>(_localctx);
        enterOuterAlt(_localctx, 1);
        setState(486);
        match(BaseParser::T__30);
        setState(487);
        match(BaseParser::ID);
        break;
      }

      case BaseParser::T__28: {
        _localctx = _tracker.createInstance<BaseParser::ForEachEdgeContext>(_localctx);
        enterOuterAlt(_localctx, 2);
        setState(488);
        match(BaseParser::T__28);
        setState(489);
        match(BaseParser::ID);
        setState(490);
        match(BaseParser::T__3);
        setState(491);
        match(BaseParser::ID);
        break;
      }

      case BaseParser::T__34: {
        _localctx = _tracker.createInstance<BaseParser::ForEachOutAdjContext>(_localctx);
        enterOuterAlt(_localctx, 3);
        setState(492);
        match(BaseParser::T__34);
        setState(493);
        match(BaseParser::T__35);
        setState(494);
        match(BaseParser::ID);
        setState(495);
        match(BaseParser::OF);
        setState(496);
        expr(0);
        break;
      }

      case BaseParser::T__21: {
        _localctx = _tracker.createInstance<BaseParser::ForEachInAdjContext>(_localctx);
        enterOuterAlt(_localctx, 4);
        setState(497);
        match(BaseParser::T__21);
        setState(498);
        match(BaseParser::T__35);
        setState(499);
        match(BaseParser::ID);
        setState(500);
        match(BaseParser::OF);
        setState(501);
        expr(0);
        break;
      }

      case BaseParser::T__35: {
        _localctx = _tracker.createInstance<BaseParser::ForEachAdjContext>(_localctx);
        enterOuterAlt(_localctx, 5);
        setState(502);
        match(BaseParser::T__35);
        setState(503);
        match(BaseParser::ID);
        setState(504);
        match(BaseParser::OF);
        setState(505);
        expr(0);
        break;
      }

      case BaseParser::T__36: {
        _localctx = _tracker.createInstance<BaseParser::ForEachElementContext>(_localctx);
        enterOuterAlt(_localctx, 6);
        setState(506);
        match(BaseParser::T__36);
        setState(507);
        match(BaseParser::ID);
        break;
      }

      case BaseParser::ID: {
        _localctx = _tracker.createInstance<BaseParser::ForEachPlainContext>(_localctx);
        enterOuterAlt(_localctx, 7);
        setState(508);
        match(BaseParser::ID);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- WhileStatementContext ------------------------------------------------------------------

BaseParser::WhileStatementContext::WhileStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

BaseParser::ConditionContext* BaseParser::WhileStatementContext::condition() {
  return getRuleContext<BaseParser::ConditionContext>(0);
}

BaseParser::BlockContext* BaseParser::WhileStatementContext::block() {
  return getRuleContext<BaseParser::BlockContext>(0);
}


size_t BaseParser::WhileStatementContext::getRuleIndex() const {
  return BaseParser::RuleWhileStatement;
}

void BaseParser::WhileStatementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterWhileStatement(this);
}

void BaseParser::WhileStatementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitWhileStatement(this);
}


std::any BaseParser::WhileStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitWhileStatement(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::WhileStatementContext* BaseParser::whileStatement() {
  WhileStatementContext *_localctx = _tracker.createInstance<WhileStatementContext>(_ctx, getState());
  enterRule(_localctx, 58, BaseParser::RuleWhileStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(511);
    match(BaseParser::T__37);
    setState(512);
    match(BaseParser::T__2);
    setState(513);
    condition(0);
    setState(514);
    match(BaseParser::T__4);
    setState(515);
    block();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- NodeEdgeOperationContext ------------------------------------------------------------------

BaseParser::NodeEdgeOperationContext::NodeEdgeOperationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

BaseParser::AddOperationContext* BaseParser::NodeEdgeOperationContext::addOperation() {
  return getRuleContext<BaseParser::AddOperationContext>(0);
}

BaseParser::RemoveOperationContext* BaseParser::NodeEdgeOperationContext::removeOperation() {
  return getRuleContext<BaseParser::RemoveOperationContext>(0);
}


size_t BaseParser::NodeEdgeOperationContext::getRuleIndex() const {
  return BaseParser::RuleNodeEdgeOperation;
}

void BaseParser::NodeEdgeOperationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterNodeEdgeOperation(this);
}

void BaseParser::NodeEdgeOperationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitNodeEdgeOperation(this);
}


std::any BaseParser::NodeEdgeOperationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitNodeEdgeOperation(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::NodeEdgeOperationContext* BaseParser::nodeEdgeOperation() {
  NodeEdgeOperationContext *_localctx = _tracker.createInstance<NodeEdgeOperationContext>(_ctx, getState());
  enterRule(_localctx, 60, BaseParser::RuleNodeEdgeOperation);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(519);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case BaseParser::T__17: {
        enterOuterAlt(_localctx, 1);
        setState(517);
        addOperation();
        break;
      }

      case BaseParser::T__18: {
        enterOuterAlt(_localctx, 2);
        setState(518);
        removeOperation();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AddOperationContext ------------------------------------------------------------------

BaseParser::AddOperationContext::AddOperationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

BaseParser::AddTargetsContext* BaseParser::AddOperationContext::addTargets() {
  return getRuleContext<BaseParser::AddTargetsContext>(0);
}

BaseParser::GraphIDContext* BaseParser::AddOperationContext::graphID() {
  return getRuleContext<BaseParser::GraphIDContext>(0);
}


size_t BaseParser::AddOperationContext::getRuleIndex() const {
  return BaseParser::RuleAddOperation;
}

void BaseParser::AddOperationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterAddOperation(this);
}

void BaseParser::AddOperationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitAddOperation(this);
}


std::any BaseParser::AddOperationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitAddOperation(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::AddOperationContext* BaseParser::addOperation() {
  AddOperationContext *_localctx = _tracker.createInstance<AddOperationContext>(_ctx, getState());
  enterRule(_localctx, 62, BaseParser::RuleAddOperation);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(521);
    match(BaseParser::T__17);
    setState(522);
    addTargets();
    setState(523);
    match(BaseParser::T__38);
    setState(524);
    graphID();
    setState(525);
    match(BaseParser::T__0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- RemoveOperationContext ------------------------------------------------------------------

BaseParser::RemoveOperationContext::RemoveOperationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

BaseParser::RemoveTargetsContext* BaseParser::RemoveOperationContext::removeTargets() {
  return getRuleContext<BaseParser::RemoveTargetsContext>(0);
}

BaseParser::GraphIDContext* BaseParser::RemoveOperationContext::graphID() {
  return getRuleContext<BaseParser::GraphIDContext>(0);
}


size_t BaseParser::RemoveOperationContext::getRuleIndex() const {
  return BaseParser::RuleRemoveOperation;
}

void BaseParser::RemoveOperationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRemoveOperation(this);
}

void BaseParser::RemoveOperationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRemoveOperation(this);
}


std::any BaseParser::RemoveOperationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitRemoveOperation(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::RemoveOperationContext* BaseParser::removeOperation() {
  RemoveOperationContext *_localctx = _tracker.createInstance<RemoveOperationContext>(_ctx, getState());
  enterRule(_localctx, 64, BaseParser::RuleRemoveOperation);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(527);
    match(BaseParser::T__18);
    setState(528);
    removeTargets();
    setState(529);
    match(BaseParser::T__39);
    setState(530);
    graphID();
    setState(531);
    match(BaseParser::T__0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AddTargetsContext ------------------------------------------------------------------

BaseParser::AddTargetsContext::AddTargetsContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

BaseParser::NodeIDContext* BaseParser::AddTargetsContext::nodeID() {
  return getRuleContext<BaseParser::NodeIDContext>(0);
}

BaseParser::EdgeContext* BaseParser::AddTargetsContext::edge() {
  return getRuleContext<BaseParser::EdgeContext>(0);
}

BaseParser::NodeListContext* BaseParser::AddTargetsContext::nodeList() {
  return getRuleContext<BaseParser::NodeListContext>(0);
}

BaseParser::EdgeListContext* BaseParser::AddTargetsContext::edgeList() {
  return getRuleContext<BaseParser::EdgeListContext>(0);
}


size_t BaseParser::AddTargetsContext::getRuleIndex() const {
  return BaseParser::RuleAddTargets;
}

void BaseParser::AddTargetsContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterAddTargets(this);
}

void BaseParser::AddTargetsContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitAddTargets(this);
}


std::any BaseParser::AddTargetsContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitAddTargets(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::AddTargetsContext* BaseParser::addTargets() {
  AddTargetsContext *_localctx = _tracker.createInstance<AddTargetsContext>(_ctx, getState());
  enterRule(_localctx, 66, BaseParser::RuleAddTargets);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(537);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 42, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(533);
      nodeID();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(534);
      edge();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(535);
      nodeList();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(536);
      edgeList();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- RemoveTargetsContext ------------------------------------------------------------------

BaseParser::RemoveTargetsContext::RemoveTargetsContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

BaseParser::NodeIDContext* BaseParser::RemoveTargetsContext::nodeID() {
  return getRuleContext<BaseParser::NodeIDContext>(0);
}

BaseParser::EdgeContext* BaseParser::RemoveTargetsContext::edge() {
  return getRuleContext<BaseParser::EdgeContext>(0);
}

BaseParser::NodeListContext* BaseParser::RemoveTargetsContext::nodeList() {
  return getRuleContext<BaseParser::NodeListContext>(0);
}

BaseParser::EdgeListContext* BaseParser::RemoveTargetsContext::edgeList() {
  return getRuleContext<BaseParser::EdgeListContext>(0);
}


size_t BaseParser::RemoveTargetsContext::getRuleIndex() const {
  return BaseParser::RuleRemoveTargets;
}

void BaseParser::RemoveTargetsContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRemoveTargets(this);
}

void BaseParser::RemoveTargetsContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRemoveTargets(this);
}


std::any BaseParser::RemoveTargetsContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitRemoveTargets(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::RemoveTargetsContext* BaseParser::removeTargets() {
  RemoveTargetsContext *_localctx = _tracker.createInstance<RemoveTargetsContext>(_ctx, getState());
  enterRule(_localctx, 68, BaseParser::RuleRemoveTargets);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(543);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 43, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(539);
      nodeID();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(540);
      edge();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(541);
      nodeList();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(542);
      edgeList();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- QueryStatementContext ------------------------------------------------------------------

BaseParser::QueryStatementContext::QueryStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* BaseParser::QueryStatementContext::ID() {
  return getToken(BaseParser::ID, 0);
}

tree::TerminalNode* BaseParser::QueryStatementContext::STRING() {
  return getToken(BaseParser::STRING, 0);
}

tree::TerminalNode* BaseParser::QueryStatementContext::OF() {
  return getToken(BaseParser::OF, 0);
}

BaseParser::GraphIDContext* BaseParser::QueryStatementContext::graphID() {
  return getRuleContext<BaseParser::GraphIDContext>(0);
}

tree::TerminalNode* BaseParser::QueryStatementContext::INT() {
  return getToken(BaseParser::INT, 0);
}


size_t BaseParser::QueryStatementContext::getRuleIndex() const {
  return BaseParser::RuleQueryStatement;
}

void BaseParser::QueryStatementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterQueryStatement(this);
}

void BaseParser::QueryStatementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitQueryStatement(this);
}


std::any BaseParser::QueryStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitQueryStatement(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::QueryStatementContext* BaseParser::queryStatement() {
  QueryStatementContext *_localctx = _tracker.createInstance<QueryStatementContext>(_ctx, getState());
  enterRule(_localctx, 70, BaseParser::RuleQueryStatement);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(545);
    match(BaseParser::T__40);
    setState(546);
    match(BaseParser::ID);
    setState(547);
    match(BaseParser::T__8);
    setState(548);
    match(BaseParser::STRING);
    setState(550);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == BaseParser::INT) {
      setState(549);
      match(BaseParser::INT);
    }
    setState(552);
    match(BaseParser::OF);
    setState(553);
    graphID();
    setState(554);
    match(BaseParser::T__0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ShowgraphContext ------------------------------------------------------------------

BaseParser::ShowgraphContext::ShowgraphContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

BaseParser::GraphIDContext* BaseParser::ShowgraphContext::graphID() {
  return getRuleContext<BaseParser::GraphIDContext>(0);
}


size_t BaseParser::ShowgraphContext::getRuleIndex() const {
  return BaseParser::RuleShowgraph;
}

void BaseParser::ShowgraphContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterShowgraph(this);
}

void BaseParser::ShowgraphContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitShowgraph(this);
}


std::any BaseParser::ShowgraphContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitShowgraph(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::ShowgraphContext* BaseParser::showgraph() {
  ShowgraphContext *_localctx = _tracker.createInstance<ShowgraphContext>(_ctx, getState());
  enterRule(_localctx, 72, BaseParser::RuleShowgraph);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(556);
    match(BaseParser::T__41);
    setState(557);
    graphID();
    setState(558);
    match(BaseParser::T__0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FunctionContext ------------------------------------------------------------------

BaseParser::FunctionContext::FunctionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

BaseParser::ReturnTypeContext* BaseParser::FunctionContext::returnType() {
  return getRuleContext<BaseParser::ReturnTypeContext>(0);
}

tree::TerminalNode* BaseParser::FunctionContext::ID() {
  return getToken(BaseParser::ID, 0);
}

BaseParser::ParamListContext* BaseParser::FunctionContext::paramList() {
  return getRuleContext<BaseParser::ParamListContext>(0);
}

BaseParser::BlockContext* BaseParser::FunctionContext::block() {
  return getRuleContext<BaseParser::BlockContext>(0);
}


size_t BaseParser::FunctionContext::getRuleIndex() const {
  return BaseParser::RuleFunction;
}

void BaseParser::FunctionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterFunction(this);
}

void BaseParser::FunctionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitFunction(this);
}


std::any BaseParser::FunctionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitFunction(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::FunctionContext* BaseParser::function() {
  FunctionContext *_localctx = _tracker.createInstance<FunctionContext>(_ctx, getState());
  enterRule(_localctx, 74, BaseParser::RuleFunction);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(560);
    match(BaseParser::T__42);
    setState(561);
    returnType();
    setState(562);
    match(BaseParser::ID);
    setState(563);
    paramList();
    setState(564);
    block();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ReturnTypeContext ------------------------------------------------------------------

BaseParser::ReturnTypeContext::ReturnTypeContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* BaseParser::ReturnTypeContext::GRAPH() {
  return getToken(BaseParser::GRAPH, 0);
}

tree::TerminalNode* BaseParser::ReturnTypeContext::SET() {
  return getToken(BaseParser::SET, 0);
}


size_t BaseParser::ReturnTypeContext::getRuleIndex() const {
  return BaseParser::RuleReturnType;
}

void BaseParser::ReturnTypeContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterReturnType(this);
}

void BaseParser::ReturnTypeContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitReturnType(this);
}


std::any BaseParser::ReturnTypeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitReturnType(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::ReturnTypeContext* BaseParser::returnType() {
  ReturnTypeContext *_localctx = _tracker.createInstance<ReturnTypeContext>(_ctx, getState());
  enterRule(_localctx, 76, BaseParser::RuleReturnType);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(566);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 5765152883485966336) != 0))) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ParamListContext ------------------------------------------------------------------

BaseParser::ParamListContext::ParamListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<BaseParser::ParamContext *> BaseParser::ParamListContext::param() {
  return getRuleContexts<BaseParser::ParamContext>();
}

BaseParser::ParamContext* BaseParser::ParamListContext::param(size_t i) {
  return getRuleContext<BaseParser::ParamContext>(i);
}


size_t BaseParser::ParamListContext::getRuleIndex() const {
  return BaseParser::RuleParamList;
}

void BaseParser::ParamListContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterParamList(this);
}

void BaseParser::ParamListContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitParamList(this);
}


std::any BaseParser::ParamListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitParamList(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::ParamListContext* BaseParser::paramList() {
  ParamListContext *_localctx = _tracker.createInstance<ParamListContext>(_ctx, getState());
  enterRule(_localctx, 78, BaseParser::RuleParamList);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(568);
    match(BaseParser::T__2);
    setState(577);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 5765012145997611008) != 0)) {
      setState(569);
      param();
      setState(574);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == BaseParser::T__3) {
        setState(570);
        match(BaseParser::T__3);
        setState(571);
        param();
        setState(576);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
    }
    setState(579);
    match(BaseParser::T__4);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ParamContext ------------------------------------------------------------------

BaseParser::ParamContext::ParamContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

BaseParser::TypeContext* BaseParser::ParamContext::type() {
  return getRuleContext<BaseParser::TypeContext>(0);
}

tree::TerminalNode* BaseParser::ParamContext::ID() {
  return getToken(BaseParser::ID, 0);
}


size_t BaseParser::ParamContext::getRuleIndex() const {
  return BaseParser::RuleParam;
}

void BaseParser::ParamContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterParam(this);
}

void BaseParser::ParamContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitParam(this);
}


std::any BaseParser::ParamContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitParam(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::ParamContext* BaseParser::param() {
  ParamContext *_localctx = _tracker.createInstance<ParamContext>(_ctx, getState());
  enterRule(_localctx, 80, BaseParser::RuleParam);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(581);
    type();
    setState(582);
    match(BaseParser::ID);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- TypeContext ------------------------------------------------------------------

BaseParser::TypeContext::TypeContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* BaseParser::TypeContext::GRAPH() {
  return getToken(BaseParser::GRAPH, 0);
}

tree::TerminalNode* BaseParser::TypeContext::SET() {
  return getToken(BaseParser::SET, 0);
}


size_t BaseParser::TypeContext::getRuleIndex() const {
  return BaseParser::RuleType;
}

void BaseParser::TypeContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterType(this);
}

void BaseParser::TypeContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitType(this);
}


std::any BaseParser::TypeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitType(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::TypeContext* BaseParser::type() {
  TypeContext *_localctx = _tracker.createInstance<TypeContext>(_ctx, getState());
  enterRule(_localctx, 82, BaseParser::RuleType);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(584);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 5765012145997611008) != 0))) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FunctionCallContext ------------------------------------------------------------------

BaseParser::FunctionCallContext::FunctionCallContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

BaseParser::FunctionNameContext* BaseParser::FunctionCallContext::functionName() {
  return getRuleContext<BaseParser::FunctionNameContext>(0);
}

BaseParser::ArgumentListContext* BaseParser::FunctionCallContext::argumentList() {
  return getRuleContext<BaseParser::ArgumentListContext>(0);
}


size_t BaseParser::FunctionCallContext::getRuleIndex() const {
  return BaseParser::RuleFunctionCall;
}

void BaseParser::FunctionCallContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterFunctionCall(this);
}

void BaseParser::FunctionCallContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitFunctionCall(this);
}


std::any BaseParser::FunctionCallContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitFunctionCall(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::FunctionCallContext* BaseParser::functionCall() {
  FunctionCallContext *_localctx = _tracker.createInstance<FunctionCallContext>(_ctx, getState());
  enterRule(_localctx, 84, BaseParser::RuleFunctionCall);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(586);
    functionName();
    setState(587);
    match(BaseParser::T__2);
    setState(589);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 864691128522244168) != 0) || ((((_la - 65) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 65)) & 7340641) != 0)) {
      setState(588);
      argumentList();
    }
    setState(591);
    match(BaseParser::T__4);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FunctionNameContext ------------------------------------------------------------------

BaseParser::FunctionNameContext::FunctionNameContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* BaseParser::FunctionNameContext::ID() {
  return getToken(BaseParser::ID, 0);
}


size_t BaseParser::FunctionNameContext::getRuleIndex() const {
  return BaseParser::RuleFunctionName;
}

void BaseParser::FunctionNameContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterFunctionName(this);
}

void BaseParser::FunctionNameContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitFunctionName(this);
}


std::any BaseParser::FunctionNameContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitFunctionName(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::FunctionNameContext* BaseParser::functionName() {
  FunctionNameContext *_localctx = _tracker.createInstance<FunctionNameContext>(_ctx, getState());
  enterRule(_localctx, 86, BaseParser::RuleFunctionName);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(593);
    _la = _input->LA(1);
    if (!(_la == BaseParser::T__25

    || _la == BaseParser::ID)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ArgumentListContext ------------------------------------------------------------------

BaseParser::ArgumentListContext::ArgumentListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<BaseParser::ExprContext *> BaseParser::ArgumentListContext::expr() {
  return getRuleContexts<BaseParser::ExprContext>();
}

BaseParser::ExprContext* BaseParser::ArgumentListContext::expr(size_t i) {
  return getRuleContext<BaseParser::ExprContext>(i);
}


size_t BaseParser::ArgumentListContext::getRuleIndex() const {
  return BaseParser::RuleArgumentList;
}

void BaseParser::ArgumentListContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterArgumentList(this);
}

void BaseParser::ArgumentListContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitArgumentList(this);
}


std::any BaseParser::ArgumentListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitArgumentList(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::ArgumentListContext* BaseParser::argumentList() {
  ArgumentListContext *_localctx = _tracker.createInstance<ArgumentListContext>(_ctx, getState());
  enterRule(_localctx, 88, BaseParser::RuleArgumentList);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(595);
    expr(0);
    setState(600);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == BaseParser::T__3) {
      setState(596);
      match(BaseParser::T__3);
      setState(597);
      expr(0);
      setState(602);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- SleepStatementContext ------------------------------------------------------------------

BaseParser::SleepStatementContext::SleepStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

BaseParser::ExprContext* BaseParser::SleepStatementContext::expr() {
  return getRuleContext<BaseParser::ExprContext>(0);
}


size_t BaseParser::SleepStatementContext::getRuleIndex() const {
  return BaseParser::RuleSleepStatement;
}

void BaseParser::SleepStatementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterSleepStatement(this);
}

void BaseParser::SleepStatementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitSleepStatement(this);
}


std::any BaseParser::SleepStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitSleepStatement(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::SleepStatementContext* BaseParser::sleepStatement() {
  SleepStatementContext *_localctx = _tracker.createInstance<SleepStatementContext>(_ctx, getState());
  enterRule(_localctx, 90, BaseParser::RuleSleepStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(603);
    match(BaseParser::T__48);
    setState(604);
    match(BaseParser::T__2);
    setState(605);
    expr(0);
    setState(606);
    match(BaseParser::T__4);
    setState(607);
    match(BaseParser::T__0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- BlockContext ------------------------------------------------------------------

BaseParser::BlockContext::BlockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<BaseParser::StatementContext *> BaseParser::BlockContext::statement() {
  return getRuleContexts<BaseParser::StatementContext>();
}

BaseParser::StatementContext* BaseParser::BlockContext::statement(size_t i) {
  return getRuleContext<BaseParser::StatementContext>(i);
}

std::vector<BaseParser::ReturnStatementContext *> BaseParser::BlockContext::returnStatement() {
  return getRuleContexts<BaseParser::ReturnStatementContext>();
}

BaseParser::ReturnStatementContext* BaseParser::BlockContext::returnStatement(size_t i) {
  return getRuleContext<BaseParser::ReturnStatementContext>(i);
}

std::vector<BaseParser::BreakStatementContext *> BaseParser::BlockContext::breakStatement() {
  return getRuleContexts<BaseParser::BreakStatementContext>();
}

BaseParser::BreakStatementContext* BaseParser::BlockContext::breakStatement(size_t i) {
  return getRuleContext<BaseParser::BreakStatementContext>(i);
}

std::vector<BaseParser::ContinueStatementContext *> BaseParser::BlockContext::continueStatement() {
  return getRuleContexts<BaseParser::ContinueStatementContext>();
}

BaseParser::ContinueStatementContext* BaseParser::BlockContext::continueStatement(size_t i) {
  return getRuleContext<BaseParser::ContinueStatementContext>(i);
}


size_t BaseParser::BlockContext::getRuleIndex() const {
  return BaseParser::RuleBlock;
}

void BaseParser::BlockContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterBlock(this);
}

void BaseParser::BlockContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitBlock(this);
}


std::any BaseParser::BlockContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitBlock(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::BlockContext* BaseParser::block() {
  BlockContext *_localctx = _tracker.createInstance<BlockContext>(_ctx, getState());
  enterRule(_localctx, 92, BaseParser::RuleBlock);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(622);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 51, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(609);
      match(BaseParser::T__5);
      setState(616);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while ((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 5782470475160223750) != 0) || _la == BaseParser::ID) {
        setState(614);
        _errHandler->sync(this);
        switch (_input->LA(1)) {
          case BaseParser::T__0:
          case BaseParser::T__1:
          case BaseParser::T__17:
          case BaseParser::T__18:
          case BaseParser::T__19:
          case BaseParser::T__25:
          case BaseParser::T__28:
          case BaseParser::T__30:
          case BaseParser::T__32:
          case BaseParser::T__37:
          case BaseParser::T__40:
          case BaseParser::T__41:
          case BaseParser::T__43:
          case BaseParser::T__44:
          case BaseParser::T__45:
          case BaseParser::T__47:
          case BaseParser::T__48:
          case BaseParser::T__52:
          case BaseParser::GRAPH:
          case BaseParser::SET:
          case BaseParser::ID: {
            setState(610);
            statement();
            break;
          }

          case BaseParser::T__49: {
            setState(611);
            returnStatement();
            break;
          }

          case BaseParser::T__50: {
            setState(612);
            breakStatement();
            break;
          }

          case BaseParser::T__51: {
            setState(613);
            continueStatement();
            break;
          }

        default:
          throw NoViableAltException(this);
        }
        setState(618);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
      setState(619);
      match(BaseParser::T__6);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(620);
      match(BaseParser::T__5);
      setState(621);
      match(BaseParser::T__6);
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ReturnStatementContext ------------------------------------------------------------------

BaseParser::ReturnStatementContext::ReturnStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

BaseParser::ExprContext* BaseParser::ReturnStatementContext::expr() {
  return getRuleContext<BaseParser::ExprContext>(0);
}


size_t BaseParser::ReturnStatementContext::getRuleIndex() const {
  return BaseParser::RuleReturnStatement;
}

void BaseParser::ReturnStatementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterReturnStatement(this);
}

void BaseParser::ReturnStatementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitReturnStatement(this);
}


std::any BaseParser::ReturnStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitReturnStatement(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::ReturnStatementContext* BaseParser::returnStatement() {
  ReturnStatementContext *_localctx = _tracker.createInstance<ReturnStatementContext>(_ctx, getState());
  enterRule(_localctx, 94, BaseParser::RuleReturnStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(624);
    match(BaseParser::T__49);
    setState(625);
    expr(0);
    setState(626);
    match(BaseParser::T__0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- BreakStatementContext ------------------------------------------------------------------

BaseParser::BreakStatementContext::BreakStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t BaseParser::BreakStatementContext::getRuleIndex() const {
  return BaseParser::RuleBreakStatement;
}

void BaseParser::BreakStatementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterBreakStatement(this);
}

void BaseParser::BreakStatementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitBreakStatement(this);
}


std::any BaseParser::BreakStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitBreakStatement(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::BreakStatementContext* BaseParser::breakStatement() {
  BreakStatementContext *_localctx = _tracker.createInstance<BreakStatementContext>(_ctx, getState());
  enterRule(_localctx, 96, BaseParser::RuleBreakStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(628);
    match(BaseParser::T__50);
    setState(629);
    match(BaseParser::T__0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ContinueStatementContext ------------------------------------------------------------------

BaseParser::ContinueStatementContext::ContinueStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t BaseParser::ContinueStatementContext::getRuleIndex() const {
  return BaseParser::RuleContinueStatement;
}

void BaseParser::ContinueStatementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterContinueStatement(this);
}

void BaseParser::ContinueStatementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitContinueStatement(this);
}


std::any BaseParser::ContinueStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitContinueStatement(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::ContinueStatementContext* BaseParser::continueStatement() {
  ContinueStatementContext *_localctx = _tracker.createInstance<ContinueStatementContext>(_ctx, getState());
  enterRule(_localctx, 98, BaseParser::RuleContinueStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(631);
    match(BaseParser::T__51);
    setState(632);
    match(BaseParser::T__0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- PrintStatementContext ------------------------------------------------------------------

BaseParser::PrintStatementContext::PrintStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

BaseParser::PrintExprContext* BaseParser::PrintStatementContext::printExpr() {
  return getRuleContext<BaseParser::PrintExprContext>(0);
}

BaseParser::PrintArrayStatementContext* BaseParser::PrintStatementContext::printArrayStatement() {
  return getRuleContext<BaseParser::PrintArrayStatementContext>(0);
}

BaseParser::PrintgraphContext* BaseParser::PrintStatementContext::printgraph() {
  return getRuleContext<BaseParser::PrintgraphContext>(0);
}


size_t BaseParser::PrintStatementContext::getRuleIndex() const {
  return BaseParser::RulePrintStatement;
}

void BaseParser::PrintStatementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterPrintStatement(this);
}

void BaseParser::PrintStatementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitPrintStatement(this);
}


std::any BaseParser::PrintStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitPrintStatement(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::PrintStatementContext* BaseParser::printStatement() {
  PrintStatementContext *_localctx = _tracker.createInstance<PrintStatementContext>(_ctx, getState());
  enterRule(_localctx, 100, BaseParser::RulePrintStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(640);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 52, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(634);
      match(BaseParser::T__52);
      setState(635);
      printExpr(0);
      setState(636);
      match(BaseParser::T__0);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(638);
      printArrayStatement();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(639);
      printgraph();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- PrintExprContext ------------------------------------------------------------------

BaseParser::PrintExprContext::PrintExprContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* BaseParser::PrintExprContext::STRING() {
  return getToken(BaseParser::STRING, 0);
}

BaseParser::ExprContext* BaseParser::PrintExprContext::expr() {
  return getRuleContext<BaseParser::ExprContext>(0);
}

std::vector<BaseParser::PrintExprContext *> BaseParser::PrintExprContext::printExpr() {
  return getRuleContexts<BaseParser::PrintExprContext>();
}

BaseParser::PrintExprContext* BaseParser::PrintExprContext::printExpr(size_t i) {
  return getRuleContext<BaseParser::PrintExprContext>(i);
}

tree::TerminalNode* BaseParser::PrintExprContext::PLUS() {
  return getToken(BaseParser::PLUS, 0);
}


size_t BaseParser::PrintExprContext::getRuleIndex() const {
  return BaseParser::RulePrintExpr;
}

void BaseParser::PrintExprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterPrintExpr(this);
}

void BaseParser::PrintExprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitPrintExpr(this);
}


std::any BaseParser::PrintExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitPrintExpr(this);
  else
    return visitor->visitChildren(this);
}


BaseParser::PrintExprContext* BaseParser::printExpr() {
   return printExpr(0);
}

BaseParser::PrintExprContext* BaseParser::printExpr(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  BaseParser::PrintExprContext *_localctx = _tracker.createInstance<PrintExprContext>(_ctx, parentState);
  BaseParser::PrintExprContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 102;
  enterRecursionRule(_localctx, 102, BaseParser::RulePrintExpr, precedence);

    

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(645);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case BaseParser::STRING: {
        setState(643);
        match(BaseParser::STRING);
        break;
      }

      case BaseParser::T__2:
      case BaseParser::T__5:
      case BaseParser::T__25:
      case BaseParser::T__57:
      case BaseParser::T__58:
      case BaseParser::NOT:
      case BaseParser::TRUE:
      case BaseParser::FALSE:
      case BaseParser::MINUS:
      case BaseParser::ID:
      case BaseParser::INT:
      case BaseParser::REAL: {
        setState(644);
        expr(0);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    _ctx->stop = _input->LT(-1);
    setState(652);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 54, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        _localctx = _tracker.createInstance<PrintExprContext>(parentContext, parentState);
        pushNewRecursionContext(_localctx, startState, RulePrintExpr);
        setState(647);

        if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
        setState(648);
        match(BaseParser::PLUS);
        setState(649);
        printExpr(2); 
      }
      setState(654);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 54, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

//----------------- PrintArrayStatementContext ------------------------------------------------------------------

BaseParser::PrintArrayStatementContext::PrintArrayStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* BaseParser::PrintArrayStatementContext::ID() {
  return getToken(BaseParser::ID, 0);
}

BaseParser::ExprContext* BaseParser::PrintArrayStatementContext::expr() {
  return getRuleContext<BaseParser::ExprContext>(0);
}


size_t BaseParser::PrintArrayStatementContext::getRuleIndex() const {
  return BaseParser::RulePrintArrayStatement;
}

void BaseParser::PrintArrayStatementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterPrintArrayStatement(this);
}

void BaseParser::PrintArrayStatementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitPrintArrayStatement(this);
}


std::any BaseParser::PrintArrayStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitPrintArrayStatement(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::PrintArrayStatementContext* BaseParser::printArrayStatement() {
  PrintArrayStatementContext *_localctx = _tracker.createInstance<PrintArrayStatementContext>(_ctx, getState());
  enterRule(_localctx, 104, BaseParser::RulePrintArrayStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(655);
    match(BaseParser::T__52);
    setState(656);
    match(BaseParser::ID);
    setState(657);
    match(BaseParser::T__22);
    setState(658);
    expr(0);
    setState(659);
    match(BaseParser::T__24);
    setState(660);
    match(BaseParser::T__0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- PrintgraphContext ------------------------------------------------------------------

BaseParser::PrintgraphContext::PrintgraphContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t BaseParser::PrintgraphContext::getRuleIndex() const {
  return BaseParser::RulePrintgraph;
}

void BaseParser::PrintgraphContext::copyFrom(PrintgraphContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- NodePrintContext ------------------------------------------------------------------

tree::TerminalNode* BaseParser::NodePrintContext::NODE() {
  return getToken(BaseParser::NODE, 0);
}

tree::TerminalNode* BaseParser::NodePrintContext::OF() {
  return getToken(BaseParser::OF, 0);
}

BaseParser::GraphIDContext* BaseParser::NodePrintContext::graphID() {
  return getRuleContext<BaseParser::GraphIDContext>(0);
}

BaseParser::NodePrintContext::NodePrintContext(PrintgraphContext *ctx) { copyFrom(ctx); }

void BaseParser::NodePrintContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterNodePrint(this);
}
void BaseParser::NodePrintContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitNodePrint(this);
}

std::any BaseParser::NodePrintContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitNodePrint(this);
  else
    return visitor->visitChildren(this);
}
//----------------- GraphPrintContext ------------------------------------------------------------------

tree::TerminalNode* BaseParser::GraphPrintContext::GRAPH() {
  return getToken(BaseParser::GRAPH, 0);
}

tree::TerminalNode* BaseParser::GraphPrintContext::OF() {
  return getToken(BaseParser::OF, 0);
}

BaseParser::GraphIDContext* BaseParser::GraphPrintContext::graphID() {
  return getRuleContext<BaseParser::GraphIDContext>(0);
}

BaseParser::GraphPrintContext::GraphPrintContext(PrintgraphContext *ctx) { copyFrom(ctx); }

void BaseParser::GraphPrintContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterGraphPrint(this);
}
void BaseParser::GraphPrintContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitGraphPrint(this);
}

std::any BaseParser::GraphPrintContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitGraphPrint(this);
  else
    return visitor->visitChildren(this);
}
//----------------- EdgePrintContext ------------------------------------------------------------------

tree::TerminalNode* BaseParser::EdgePrintContext::EDGE() {
  return getToken(BaseParser::EDGE, 0);
}

tree::TerminalNode* BaseParser::EdgePrintContext::OF() {
  return getToken(BaseParser::OF, 0);
}

BaseParser::GraphIDContext* BaseParser::EdgePrintContext::graphID() {
  return getRuleContext<BaseParser::GraphIDContext>(0);
}

BaseParser::EdgePrintContext::EdgePrintContext(PrintgraphContext *ctx) { copyFrom(ctx); }

void BaseParser::EdgePrintContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterEdgePrint(this);
}
void BaseParser::EdgePrintContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitEdgePrint(this);
}

std::any BaseParser::EdgePrintContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitEdgePrint(this);
  else
    return visitor->visitChildren(this);
}
BaseParser::PrintgraphContext* BaseParser::printgraph() {
  PrintgraphContext *_localctx = _tracker.createInstance<PrintgraphContext>(_ctx, getState());
  enterRule(_localctx, 106, BaseParser::RulePrintgraph);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(680);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 55, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<BaseParser::EdgePrintContext>(_localctx);
      enterOuterAlt(_localctx, 1);
      setState(662);
      match(BaseParser::T__52);
      setState(663);
      match(BaseParser::EDGE);
      setState(664);
      match(BaseParser::OF);
      setState(665);
      graphID();
      setState(666);
      match(BaseParser::T__0);
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<BaseParser::NodePrintContext>(_localctx);
      enterOuterAlt(_localctx, 2);
      setState(668);
      match(BaseParser::T__52);
      setState(669);
      match(BaseParser::NODE);
      setState(670);
      match(BaseParser::OF);
      setState(671);
      graphID();
      setState(672);
      match(BaseParser::T__0);
      break;
    }

    case 3: {
      _localctx = _tracker.createInstance<BaseParser::GraphPrintContext>(_localctx);
      enterOuterAlt(_localctx, 3);
      setState(674);
      match(BaseParser::T__52);
      setState(675);
      match(BaseParser::GRAPH);
      setState(676);
      match(BaseParser::OF);
      setState(677);
      graphID();
      setState(678);
      match(BaseParser::T__0);
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ExprContext ------------------------------------------------------------------

BaseParser::ExprContext::ExprContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t BaseParser::ExprContext::getRuleIndex() const {
  return BaseParser::RuleExpr;
}

void BaseParser::ExprContext::copyFrom(ExprContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- BoolTrueExprContext ------------------------------------------------------------------

tree::TerminalNode* BaseParser::BoolTrueExprContext::TRUE() {
  return getToken(BaseParser::TRUE, 0);
}

BaseParser::BoolTrueExprContext::BoolTrueExprContext(ExprContext *ctx) { copyFrom(ctx); }

void BaseParser::BoolTrueExprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterBoolTrueExpr(this);
}
void BaseParser::BoolTrueExprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitBoolTrueExpr(this);
}

std::any BaseParser::BoolTrueExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitBoolTrueExpr(this);
  else
    return visitor->visitChildren(this);
}
//----------------- TimerExprContext ------------------------------------------------------------------

BaseParser::TimerExprContext::TimerExprContext(ExprContext *ctx) { copyFrom(ctx); }

void BaseParser::TimerExprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterTimerExpr(this);
}
void BaseParser::TimerExprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitTimerExpr(this);
}

std::any BaseParser::TimerExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitTimerExpr(this);
  else
    return visitor->visitChildren(this);
}
//----------------- IdExprContext ------------------------------------------------------------------

tree::TerminalNode* BaseParser::IdExprContext::ID() {
  return getToken(BaseParser::ID, 0);
}

BaseParser::IdExprContext::IdExprContext(ExprContext *ctx) { copyFrom(ctx); }

void BaseParser::IdExprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterIdExpr(this);
}
void BaseParser::IdExprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitIdExpr(this);
}

std::any BaseParser::IdExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitIdExpr(this);
  else
    return visitor->visitChildren(this);
}
//----------------- BoolFalseExprContext ------------------------------------------------------------------

tree::TerminalNode* BaseParser::BoolFalseExprContext::FALSE() {
  return getToken(BaseParser::FALSE, 0);
}

BaseParser::BoolFalseExprContext::BoolFalseExprContext(ExprContext *ctx) { copyFrom(ctx); }

void BaseParser::BoolFalseExprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterBoolFalseExpr(this);
}
void BaseParser::BoolFalseExprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitBoolFalseExpr(this);
}

std::any BaseParser::BoolFalseExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitBoolFalseExpr(this);
  else
    return visitor->visitChildren(this);
}
//----------------- ArrayAccessExprContext ------------------------------------------------------------------

tree::TerminalNode* BaseParser::ArrayAccessExprContext::ID() {
  return getToken(BaseParser::ID, 0);
}

BaseParser::ExprContext* BaseParser::ArrayAccessExprContext::expr() {
  return getRuleContext<BaseParser::ExprContext>(0);
}

BaseParser::ArrayAccessExprContext::ArrayAccessExprContext(ExprContext *ctx) { copyFrom(ctx); }

void BaseParser::ArrayAccessExprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterArrayAccessExpr(this);
}
void BaseParser::ArrayAccessExprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitArrayAccessExpr(this);
}

std::any BaseParser::ArrayAccessExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitArrayAccessExpr(this);
  else
    return visitor->visitChildren(this);
}
//----------------- SetLitExprContext ------------------------------------------------------------------

BaseParser::SetInitializerContext* BaseParser::SetLitExprContext::setInitializer() {
  return getRuleContext<BaseParser::SetInitializerContext>(0);
}

BaseParser::SetLitExprContext::SetLitExprContext(ExprContext *ctx) { copyFrom(ctx); }

void BaseParser::SetLitExprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterSetLitExpr(this);
}
void BaseParser::SetLitExprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitSetLitExpr(this);
}

std::any BaseParser::SetLitExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitSetLitExpr(this);
  else
    return visitor->visitChildren(this);
}
//----------------- SetSizeExprContext ------------------------------------------------------------------

tree::TerminalNode* BaseParser::SetSizeExprContext::ID() {
  return getToken(BaseParser::ID, 0);
}

BaseParser::SetSizeExprContext::SetSizeExprContext(ExprContext *ctx) { copyFrom(ctx); }

void BaseParser::SetSizeExprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterSetSizeExpr(this);
}
void BaseParser::SetSizeExprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitSetSizeExpr(this);
}

std::any BaseParser::SetSizeExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitSetSizeExpr(this);
  else
    return visitor->visitChildren(this);
}
//----------------- Array2DAccessExprContext ------------------------------------------------------------------

tree::TerminalNode* BaseParser::Array2DAccessExprContext::ID() {
  return getToken(BaseParser::ID, 0);
}

std::vector<BaseParser::ExprContext *> BaseParser::Array2DAccessExprContext::expr() {
  return getRuleContexts<BaseParser::ExprContext>();
}

BaseParser::ExprContext* BaseParser::Array2DAccessExprContext::expr(size_t i) {
  return getRuleContext<BaseParser::ExprContext>(i);
}

BaseParser::Array2DAccessExprContext::Array2DAccessExprContext(ExprContext *ctx) { copyFrom(ctx); }

void BaseParser::Array2DAccessExprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterArray2DAccessExpr(this);
}
void BaseParser::Array2DAccessExprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitArray2DAccessExpr(this);
}

std::any BaseParser::Array2DAccessExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitArray2DAccessExpr(this);
  else
    return visitor->visitChildren(this);
}
//----------------- ArrayPrintContext ------------------------------------------------------------------

tree::TerminalNode* BaseParser::ArrayPrintContext::ID() {
  return getToken(BaseParser::ID, 0);
}

BaseParser::ArrayPrintContext::ArrayPrintContext(ExprContext *ctx) { copyFrom(ctx); }

void BaseParser::ArrayPrintContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterArrayPrint(this);
}
void BaseParser::ArrayPrintContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitArrayPrint(this);
}

std::any BaseParser::ArrayPrintContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitArrayPrint(this);
  else
    return visitor->visitChildren(this);
}
//----------------- InfExprContext ------------------------------------------------------------------

BaseParser::InfExprContext::InfExprContext(ExprContext *ctx) { copyFrom(ctx); }

void BaseParser::InfExprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterInfExpr(this);
}
void BaseParser::InfExprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitInfExpr(this);
}

std::any BaseParser::InfExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitInfExpr(this);
  else
    return visitor->visitChildren(this);
}
//----------------- FuncExprContext ------------------------------------------------------------------

BaseParser::FunctionCallContext* BaseParser::FuncExprContext::functionCall() {
  return getRuleContext<BaseParser::FunctionCallContext>(0);
}

BaseParser::FuncExprContext::FuncExprContext(ExprContext *ctx) { copyFrom(ctx); }

void BaseParser::FuncExprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterFuncExpr(this);
}
void BaseParser::FuncExprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitFuncExpr(this);
}

std::any BaseParser::FuncExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitFuncExpr(this);
  else
    return visitor->visitChildren(this);
}
//----------------- SetContainsExprContext ------------------------------------------------------------------

BaseParser::SetTargetContext* BaseParser::SetContainsExprContext::setTarget() {
  return getRuleContext<BaseParser::SetTargetContext>(0);
}

BaseParser::ExprContext* BaseParser::SetContainsExprContext::expr() {
  return getRuleContext<BaseParser::ExprContext>(0);
}

BaseParser::SetContainsExprContext::SetContainsExprContext(ExprContext *ctx) { copyFrom(ctx); }

void BaseParser::SetContainsExprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterSetContainsExpr(this);
}
void BaseParser::SetContainsExprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitSetContainsExpr(this);
}

std::any BaseParser::SetContainsExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitSetContainsExpr(this);
  else
    return visitor->visitChildren(this);
}
//----------------- MulDivExprContext ------------------------------------------------------------------

std::vector<BaseParser::ExprContext *> BaseParser::MulDivExprContext::expr() {
  return getRuleContexts<BaseParser::ExprContext>();
}

BaseParser::ExprContext* BaseParser::MulDivExprContext::expr(size_t i) {
  return getRuleContext<BaseParser::ExprContext>(i);
}

tree::TerminalNode* BaseParser::MulDivExprContext::TIMES() {
  return getToken(BaseParser::TIMES, 0);
}

tree::TerminalNode* BaseParser::MulDivExprContext::DIVIDE() {
  return getToken(BaseParser::DIVIDE, 0);
}

tree::TerminalNode* BaseParser::MulDivExprContext::MODULO() {
  return getToken(BaseParser::MODULO, 0);
}

BaseParser::MulDivExprContext::MulDivExprContext(ExprContext *ctx) { copyFrom(ctx); }

void BaseParser::MulDivExprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterMulDivExpr(this);
}
void BaseParser::MulDivExprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitMulDivExpr(this);
}

std::any BaseParser::MulDivExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitMulDivExpr(this);
  else
    return visitor->visitChildren(this);
}
//----------------- NotExprContext ------------------------------------------------------------------

tree::TerminalNode* BaseParser::NotExprContext::NOT() {
  return getToken(BaseParser::NOT, 0);
}

BaseParser::ExprContext* BaseParser::NotExprContext::expr() {
  return getRuleContext<BaseParser::ExprContext>(0);
}

BaseParser::NotExprContext::NotExprContext(ExprContext *ctx) { copyFrom(ctx); }

void BaseParser::NotExprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterNotExpr(this);
}
void BaseParser::NotExprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitNotExpr(this);
}

std::any BaseParser::NotExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitNotExpr(this);
  else
    return visitor->visitChildren(this);
}
//----------------- IntExprContext ------------------------------------------------------------------

tree::TerminalNode* BaseParser::IntExprContext::INT() {
  return getToken(BaseParser::INT, 0);
}

BaseParser::IntExprContext::IntExprContext(ExprContext *ctx) { copyFrom(ctx); }

void BaseParser::IntExprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterIntExpr(this);
}
void BaseParser::IntExprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitIntExpr(this);
}

std::any BaseParser::IntExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitIntExpr(this);
  else
    return visitor->visitChildren(this);
}
//----------------- SetPopExprContext ------------------------------------------------------------------

tree::TerminalNode* BaseParser::SetPopExprContext::ID() {
  return getToken(BaseParser::ID, 0);
}

BaseParser::SetPopExprContext::SetPopExprContext(ExprContext *ctx) { copyFrom(ctx); }

void BaseParser::SetPopExprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterSetPopExpr(this);
}
void BaseParser::SetPopExprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitSetPopExpr(this);
}

std::any BaseParser::SetPopExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitSetPopExpr(this);
  else
    return visitor->visitChildren(this);
}
//----------------- ParenExprContext ------------------------------------------------------------------

BaseParser::ExprContext* BaseParser::ParenExprContext::expr() {
  return getRuleContext<BaseParser::ExprContext>(0);
}

BaseParser::ParenExprContext::ParenExprContext(ExprContext *ctx) { copyFrom(ctx); }

void BaseParser::ParenExprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterParenExpr(this);
}
void BaseParser::ParenExprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitParenExpr(this);
}

std::any BaseParser::ParenExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitParenExpr(this);
  else
    return visitor->visitChildren(this);
}
//----------------- RealExprContext ------------------------------------------------------------------

tree::TerminalNode* BaseParser::RealExprContext::REAL() {
  return getToken(BaseParser::REAL, 0);
}

BaseParser::RealExprContext::RealExprContext(ExprContext *ctx) { copyFrom(ctx); }

void BaseParser::RealExprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRealExpr(this);
}
void BaseParser::RealExprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRealExpr(this);
}

std::any BaseParser::RealExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitRealExpr(this);
  else
    return visitor->visitChildren(this);
}
//----------------- LogicalExprContext ------------------------------------------------------------------

std::vector<BaseParser::ExprContext *> BaseParser::LogicalExprContext::expr() {
  return getRuleContexts<BaseParser::ExprContext>();
}

BaseParser::ExprContext* BaseParser::LogicalExprContext::expr(size_t i) {
  return getRuleContext<BaseParser::ExprContext>(i);
}

tree::TerminalNode* BaseParser::LogicalExprContext::AND() {
  return getToken(BaseParser::AND, 0);
}

tree::TerminalNode* BaseParser::LogicalExprContext::OR() {
  return getToken(BaseParser::OR, 0);
}

BaseParser::LogicalExprContext::LogicalExprContext(ExprContext *ctx) { copyFrom(ctx); }

void BaseParser::LogicalExprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterLogicalExpr(this);
}
void BaseParser::LogicalExprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitLogicalExpr(this);
}

std::any BaseParser::LogicalExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitLogicalExpr(this);
  else
    return visitor->visitChildren(this);
}
//----------------- AddSubExprContext ------------------------------------------------------------------

std::vector<BaseParser::ExprContext *> BaseParser::AddSubExprContext::expr() {
  return getRuleContexts<BaseParser::ExprContext>();
}

BaseParser::ExprContext* BaseParser::AddSubExprContext::expr(size_t i) {
  return getRuleContext<BaseParser::ExprContext>(i);
}

tree::TerminalNode* BaseParser::AddSubExprContext::PLUS() {
  return getToken(BaseParser::PLUS, 0);
}

tree::TerminalNode* BaseParser::AddSubExprContext::MINUS() {
  return getToken(BaseParser::MINUS, 0);
}

BaseParser::AddSubExprContext::AddSubExprContext(ExprContext *ctx) { copyFrom(ctx); }

void BaseParser::AddSubExprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterAddSubExpr(this);
}
void BaseParser::AddSubExprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitAddSubExpr(this);
}

std::any BaseParser::AddSubExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitAddSubExpr(this);
  else
    return visitor->visitChildren(this);
}
//----------------- UnaryMinusExprContext ------------------------------------------------------------------

tree::TerminalNode* BaseParser::UnaryMinusExprContext::MINUS() {
  return getToken(BaseParser::MINUS, 0);
}

BaseParser::ExprContext* BaseParser::UnaryMinusExprContext::expr() {
  return getRuleContext<BaseParser::ExprContext>(0);
}

BaseParser::UnaryMinusExprContext::UnaryMinusExprContext(ExprContext *ctx) { copyFrom(ctx); }

void BaseParser::UnaryMinusExprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnaryMinusExpr(this);
}
void BaseParser::UnaryMinusExprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnaryMinusExpr(this);
}

std::any BaseParser::UnaryMinusExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitUnaryMinusExpr(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::ExprContext* BaseParser::expr() {
   return expr(0);
}

BaseParser::ExprContext* BaseParser::expr(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  BaseParser::ExprContext *_localctx = _tracker.createInstance<ExprContext>(_ctx, parentState);
  BaseParser::ExprContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 108;
  enterRecursionRule(_localctx, 108, BaseParser::RuleExpr, precedence);

    size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(734);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 56, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<UnaryMinusExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;

      setState(683);
      match(BaseParser::MINUS);
      setState(684);
      expr(18);
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<NotExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(685);
      match(BaseParser::NOT);
      setState(686);
      expr(17);
      break;
    }

    case 3: {
      _localctx = _tracker.createInstance<FuncExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(687);
      functionCall();
      break;
    }

    case 4: {
      _localctx = _tracker.createInstance<IntExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(688);
      match(BaseParser::INT);
      break;
    }

    case 5: {
      _localctx = _tracker.createInstance<IdExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(689);
      match(BaseParser::ID);
      break;
    }

    case 6: {
      _localctx = _tracker.createInstance<ParenExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(690);
      match(BaseParser::T__2);
      setState(691);
      expr(0);
      setState(692);
      match(BaseParser::T__4);
      break;
    }

    case 7: {
      _localctx = _tracker.createInstance<Array2DAccessExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(694);
      match(BaseParser::ID);
      setState(695);
      match(BaseParser::T__22);
      setState(696);
      expr(0);
      setState(697);
      match(BaseParser::T__24);
      setState(698);
      match(BaseParser::T__22);
      setState(699);
      expr(0);
      setState(700);
      match(BaseParser::T__24);
      break;
    }

    case 8: {
      _localctx = _tracker.createInstance<ArrayAccessExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(702);
      match(BaseParser::ID);
      setState(703);
      match(BaseParser::T__22);
      setState(704);
      expr(0);
      setState(705);
      match(BaseParser::T__24);
      break;
    }

    case 9: {
      _localctx = _tracker.createInstance<SetContainsExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(707);
      setTarget();
      setState(708);
      match(BaseParser::T__16);
      setState(709);
      match(BaseParser::T__53);
      setState(710);
      match(BaseParser::T__2);
      setState(711);
      expr(0);
      setState(712);
      match(BaseParser::T__4);
      break;
    }

    case 10: {
      _localctx = _tracker.createInstance<SetSizeExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(714);
      match(BaseParser::ID);
      setState(715);
      match(BaseParser::T__16);
      setState(716);
      match(BaseParser::T__54);
      setState(717);
      match(BaseParser::T__2);
      setState(718);
      match(BaseParser::T__4);
      break;
    }

    case 11: {
      _localctx = _tracker.createInstance<SetPopExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(719);
      match(BaseParser::ID);
      setState(720);
      match(BaseParser::T__16);
      setState(721);
      match(BaseParser::T__55);
      setState(722);
      match(BaseParser::T__2);
      setState(723);
      match(BaseParser::T__4);
      break;
    }

    case 12: {
      _localctx = _tracker.createInstance<BoolTrueExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(724);
      match(BaseParser::TRUE);
      break;
    }

    case 13: {
      _localctx = _tracker.createInstance<BoolFalseExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(725);
      match(BaseParser::FALSE);
      break;
    }

    case 14: {
      _localctx = _tracker.createInstance<ArrayPrintContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(726);
      match(BaseParser::ID);
      setState(727);
      match(BaseParser::T__56);
      break;
    }

    case 15: {
      _localctx = _tracker.createInstance<RealExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(728);
      match(BaseParser::REAL);
      break;
    }

    case 16: {
      _localctx = _tracker.createInstance<SetLitExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(729);
      setInitializer();
      break;
    }

    case 17: {
      _localctx = _tracker.createInstance<InfExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(730);
      match(BaseParser::T__57);
      break;
    }

    case 18: {
      _localctx = _tracker.createInstance<TimerExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(731);
      match(BaseParser::T__58);
      setState(732);
      match(BaseParser::T__2);
      setState(733);
      match(BaseParser::T__4);
      break;
    }

    default:
      break;
    }
    _ctx->stop = _input->LT(-1);
    setState(747);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 58, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(745);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 57, _ctx)) {
        case 1: {
          auto newContext = _tracker.createInstance<LogicalExprContext>(_tracker.createInstance<ExprContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpr);
          setState(736);

          if (!(precpred(_ctx, 21))) throw FailedPredicateException(this, "precpred(_ctx, 21)");
          setState(737);
          _la = _input->LA(1);
          if (!(_la == BaseParser::AND

          || _la == BaseParser::OR)) {
          _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(738);
          expr(22);
          break;
        }

        case 2: {
          auto newContext = _tracker.createInstance<MulDivExprContext>(_tracker.createInstance<ExprContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpr);
          setState(739);

          if (!(precpred(_ctx, 20))) throw FailedPredicateException(this, "precpred(_ctx, 20)");
          setState(740);
          _la = _input->LA(1);
          if (!(((((_la - 66) & ~ 0x3fULL) == 0) &&
            ((1ULL << (_la - 66)) & 1537) != 0))) {
          _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(741);
          expr(21);
          break;
        }

        case 3: {
          auto newContext = _tracker.createInstance<AddSubExprContext>(_tracker.createInstance<ExprContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpr);
          setState(742);

          if (!(precpred(_ctx, 19))) throw FailedPredicateException(this, "precpred(_ctx, 19)");
          setState(743);
          _la = _input->LA(1);
          if (!(_la == BaseParser::PLUS

          || _la == BaseParser::MINUS)) {
          _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(744);
          expr(20);
          break;
        }

        default:
          break;
        } 
      }
      setState(749);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 58, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

//----------------- ArrayDeclaratorContext ------------------------------------------------------------------

BaseParser::ArrayDeclaratorContext::ArrayDeclaratorContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t BaseParser::ArrayDeclaratorContext::getRuleIndex() const {
  return BaseParser::RuleArrayDeclarator;
}

void BaseParser::ArrayDeclaratorContext::copyFrom(ArrayDeclaratorContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- Sized2DArrayContext ------------------------------------------------------------------

tree::TerminalNode* BaseParser::Sized2DArrayContext::ID() {
  return getToken(BaseParser::ID, 0);
}

std::vector<BaseParser::ExprContext *> BaseParser::Sized2DArrayContext::expr() {
  return getRuleContexts<BaseParser::ExprContext>();
}

BaseParser::ExprContext* BaseParser::Sized2DArrayContext::expr(size_t i) {
  return getRuleContext<BaseParser::ExprContext>(i);
}

BaseParser::Sized2DArrayContext::Sized2DArrayContext(ArrayDeclaratorContext *ctx) { copyFrom(ctx); }

void BaseParser::Sized2DArrayContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterSized2DArray(this);
}
void BaseParser::Sized2DArrayContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitSized2DArray(this);
}

std::any BaseParser::Sized2DArrayContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitSized2DArray(this);
  else
    return visitor->visitChildren(this);
}
//----------------- SizedArrayContext ------------------------------------------------------------------

tree::TerminalNode* BaseParser::SizedArrayContext::ID() {
  return getToken(BaseParser::ID, 0);
}

BaseParser::ExprContext* BaseParser::SizedArrayContext::expr() {
  return getRuleContext<BaseParser::ExprContext>(0);
}

BaseParser::SizedArrayContext::SizedArrayContext(ArrayDeclaratorContext *ctx) { copyFrom(ctx); }

void BaseParser::SizedArrayContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterSizedArray(this);
}
void BaseParser::SizedArrayContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitSizedArray(this);
}

std::any BaseParser::SizedArrayContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitSizedArray(this);
  else
    return visitor->visitChildren(this);
}
//----------------- UnsizedArrayContext ------------------------------------------------------------------

tree::TerminalNode* BaseParser::UnsizedArrayContext::ID() {
  return getToken(BaseParser::ID, 0);
}

BaseParser::UnsizedArrayContext::UnsizedArrayContext(ArrayDeclaratorContext *ctx) { copyFrom(ctx); }

void BaseParser::UnsizedArrayContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnsizedArray(this);
}
void BaseParser::UnsizedArrayContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnsizedArray(this);
}

std::any BaseParser::UnsizedArrayContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitUnsizedArray(this);
  else
    return visitor->visitChildren(this);
}
BaseParser::ArrayDeclaratorContext* BaseParser::arrayDeclarator() {
  ArrayDeclaratorContext *_localctx = _tracker.createInstance<ArrayDeclaratorContext>(_ctx, getState());
  enterRule(_localctx, 110, BaseParser::RuleArrayDeclarator);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(766);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 59, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<BaseParser::Sized2DArrayContext>(_localctx);
      enterOuterAlt(_localctx, 1);
      setState(750);
      match(BaseParser::ID);
      setState(751);
      match(BaseParser::T__22);
      setState(752);
      expr(0);
      setState(753);
      match(BaseParser::T__24);
      setState(754);
      match(BaseParser::T__22);
      setState(755);
      expr(0);
      setState(756);
      match(BaseParser::T__24);
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<BaseParser::SizedArrayContext>(_localctx);
      enterOuterAlt(_localctx, 2);
      setState(758);
      match(BaseParser::ID);
      setState(759);
      match(BaseParser::T__22);
      setState(760);
      expr(0);
      setState(761);
      match(BaseParser::T__24);
      break;
    }

    case 3: {
      _localctx = _tracker.createInstance<BaseParser::UnsizedArrayContext>(_localctx);
      enterOuterAlt(_localctx, 3);
      setState(763);
      match(BaseParser::ID);
      setState(764);
      match(BaseParser::T__22);
      setState(765);
      match(BaseParser::T__24);
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ArrayInitializerContext ------------------------------------------------------------------

BaseParser::ArrayInitializerContext::ArrayInitializerContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<BaseParser::ExprContext *> BaseParser::ArrayInitializerContext::expr() {
  return getRuleContexts<BaseParser::ExprContext>();
}

BaseParser::ExprContext* BaseParser::ArrayInitializerContext::expr(size_t i) {
  return getRuleContext<BaseParser::ExprContext>(i);
}


size_t BaseParser::ArrayInitializerContext::getRuleIndex() const {
  return BaseParser::RuleArrayInitializer;
}

void BaseParser::ArrayInitializerContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterArrayInitializer(this);
}

void BaseParser::ArrayInitializerContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitArrayInitializer(this);
}


std::any BaseParser::ArrayInitializerContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitArrayInitializer(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::ArrayInitializerContext* BaseParser::arrayInitializer() {
  ArrayInitializerContext *_localctx = _tracker.createInstance<ArrayInitializerContext>(_ctx, getState());
  enterRule(_localctx, 112, BaseParser::RuleArrayInitializer);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(768);
    match(BaseParser::T__22);
    setState(769);
    expr(0);
    setState(774);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == BaseParser::T__3) {
      setState(770);
      match(BaseParser::T__3);
      setState(771);
      expr(0);
      setState(776);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(777);
    match(BaseParser::T__24);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AssignmentStatementContext ------------------------------------------------------------------

BaseParser::AssignmentStatementContext::AssignmentStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* BaseParser::AssignmentStatementContext::ID() {
  return getToken(BaseParser::ID, 0);
}

BaseParser::ExprContext* BaseParser::AssignmentStatementContext::expr() {
  return getRuleContext<BaseParser::ExprContext>(0);
}


size_t BaseParser::AssignmentStatementContext::getRuleIndex() const {
  return BaseParser::RuleAssignmentStatement;
}

void BaseParser::AssignmentStatementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterAssignmentStatement(this);
}

void BaseParser::AssignmentStatementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitAssignmentStatement(this);
}


std::any BaseParser::AssignmentStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitAssignmentStatement(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::AssignmentStatementContext* BaseParser::assignmentStatement() {
  AssignmentStatementContext *_localctx = _tracker.createInstance<AssignmentStatementContext>(_ctx, getState());
  enterRule(_localctx, 114, BaseParser::RuleAssignmentStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(786);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 61, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(779);
      match(BaseParser::ID);
      setState(780);
      match(BaseParser::T__15);
      setState(781);
      expr(0);
      setState(782);
      match(BaseParser::T__0);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(784);
      match(BaseParser::ID);
      setState(785);
      match(BaseParser::T__0);
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ArrayAssignStatementContext ------------------------------------------------------------------

BaseParser::ArrayAssignStatementContext::ArrayAssignStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t BaseParser::ArrayAssignStatementContext::getRuleIndex() const {
  return BaseParser::RuleArrayAssignStatement;
}

void BaseParser::ArrayAssignStatementContext::copyFrom(ArrayAssignStatementContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- Array2DAssignStmtContext ------------------------------------------------------------------

tree::TerminalNode* BaseParser::Array2DAssignStmtContext::ID() {
  return getToken(BaseParser::ID, 0);
}

std::vector<BaseParser::ExprContext *> BaseParser::Array2DAssignStmtContext::expr() {
  return getRuleContexts<BaseParser::ExprContext>();
}

BaseParser::ExprContext* BaseParser::Array2DAssignStmtContext::expr(size_t i) {
  return getRuleContext<BaseParser::ExprContext>(i);
}

BaseParser::Array2DAssignStmtContext::Array2DAssignStmtContext(ArrayAssignStatementContext *ctx) { copyFrom(ctx); }

void BaseParser::Array2DAssignStmtContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterArray2DAssignStmt(this);
}
void BaseParser::Array2DAssignStmtContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitArray2DAssignStmt(this);
}

std::any BaseParser::Array2DAssignStmtContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitArray2DAssignStmt(this);
  else
    return visitor->visitChildren(this);
}
//----------------- ArrayAssignStmtContext ------------------------------------------------------------------

tree::TerminalNode* BaseParser::ArrayAssignStmtContext::ID() {
  return getToken(BaseParser::ID, 0);
}

std::vector<BaseParser::ExprContext *> BaseParser::ArrayAssignStmtContext::expr() {
  return getRuleContexts<BaseParser::ExprContext>();
}

BaseParser::ExprContext* BaseParser::ArrayAssignStmtContext::expr(size_t i) {
  return getRuleContext<BaseParser::ExprContext>(i);
}

BaseParser::ArrayAssignStmtContext::ArrayAssignStmtContext(ArrayAssignStatementContext *ctx) { copyFrom(ctx); }

void BaseParser::ArrayAssignStmtContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterArrayAssignStmt(this);
}
void BaseParser::ArrayAssignStmtContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitArrayAssignStmt(this);
}

std::any BaseParser::ArrayAssignStmtContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitArrayAssignStmt(this);
  else
    return visitor->visitChildren(this);
}
BaseParser::ArrayAssignStatementContext* BaseParser::arrayAssignStatement() {
  ArrayAssignStatementContext *_localctx = _tracker.createInstance<ArrayAssignStatementContext>(_ctx, getState());
  enterRule(_localctx, 116, BaseParser::RuleArrayAssignStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(807);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 62, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<BaseParser::Array2DAssignStmtContext>(_localctx);
      enterOuterAlt(_localctx, 1);
      setState(788);
      match(BaseParser::ID);
      setState(789);
      match(BaseParser::T__22);
      setState(790);
      expr(0);
      setState(791);
      match(BaseParser::T__24);
      setState(792);
      match(BaseParser::T__22);
      setState(793);
      expr(0);
      setState(794);
      match(BaseParser::T__24);
      setState(795);
      match(BaseParser::T__15);
      setState(796);
      expr(0);
      setState(797);
      match(BaseParser::T__0);
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<BaseParser::ArrayAssignStmtContext>(_localctx);
      enterOuterAlt(_localctx, 2);
      setState(799);
      match(BaseParser::ID);
      setState(800);
      match(BaseParser::T__22);
      setState(801);
      expr(0);
      setState(802);
      match(BaseParser::T__24);
      setState(803);
      match(BaseParser::T__15);
      setState(804);
      expr(0);
      setState(805);
      match(BaseParser::T__0);
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- WeightsContext ------------------------------------------------------------------

BaseParser::WeightsContext::WeightsContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* BaseParser::WeightsContext::TRUE() {
  return getToken(BaseParser::TRUE, 0);
}

tree::TerminalNode* BaseParser::WeightsContext::FALSE() {
  return getToken(BaseParser::FALSE, 0);
}


size_t BaseParser::WeightsContext::getRuleIndex() const {
  return BaseParser::RuleWeights;
}

void BaseParser::WeightsContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterWeights(this);
}

void BaseParser::WeightsContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitWeights(this);
}


std::any BaseParser::WeightsContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitWeights(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::WeightsContext* BaseParser::weights() {
  WeightsContext *_localctx = _tracker.createInstance<WeightsContext>(_ctx, getState());
  enterRule(_localctx, 118, BaseParser::RuleWeights);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(809);
    _la = _input->LA(1);
    if (!(_la == BaseParser::TRUE

    || _la == BaseParser::FALSE)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

bool BaseParser::sempred(RuleContext *context, size_t ruleIndex, size_t predicateIndex) {
  switch (ruleIndex) {
    case 19: return setExprSempred(antlrcpp::downCast<SetExprContext *>(context), predicateIndex);
    case 22: return conditionSempred(antlrcpp::downCast<ConditionContext *>(context), predicateIndex);
    case 25: return graphConditionSempred(antlrcpp::downCast<GraphConditionContext *>(context), predicateIndex);
    case 51: return printExprSempred(antlrcpp::downCast<PrintExprContext *>(context), predicateIndex);
    case 54: return exprSempred(antlrcpp::downCast<ExprContext *>(context), predicateIndex);

  default:
    break;
  }
  return true;
}

bool BaseParser::setExprSempred(SetExprContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 0: return precpred(_ctx, 7);
    case 1: return precpred(_ctx, 6);

  default:
    break;
  }
  return true;
}

bool BaseParser::conditionSempred(ConditionContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 2: return precpred(_ctx, 6);
    case 3: return precpred(_ctx, 5);

  default:
    break;
  }
  return true;
}

bool BaseParser::graphConditionSempred(GraphConditionContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 4: return precpred(_ctx, 8);
    case 5: return precpred(_ctx, 7);

  default:
    break;
  }
  return true;
}

bool BaseParser::printExprSempred(PrintExprContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 6: return precpred(_ctx, 1);

  default:
    break;
  }
  return true;
}

bool BaseParser::exprSempred(ExprContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 7: return precpred(_ctx, 21);
    case 8: return precpred(_ctx, 20);
    case 9: return precpred(_ctx, 19);

  default:
    break;
  }
  return true;
}

void BaseParser::initialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  baseParserInitialize();
#else
  ::antlr4::internal::call_once(baseParserOnceFlag, baseParserInitialize);
#endif
}
