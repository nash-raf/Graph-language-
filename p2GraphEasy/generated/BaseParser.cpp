
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
      "removeTargets", "queryStatement", "showgraph", "drawgraph", "drawOption", 
      "vertexDrawOption", "edgeDrawOption", "colorMapping", "continuousMapping", 
      "function", "returnType", "paramList", "param", "type", "functionCall", 
      "functionName", "argumentList", "sleepStatement", "block", "returnStatement", 
      "breakStatement", "continueStatement", "printStatement", "printExpr", 
      "printArrayStatement", "printgraph", "expr", "arrayDeclarator", "arrayInitializer", 
      "assignmentStatement", "arrayAssignStatement", "weights"
    },
    std::vector<std::string>{
      "", "';'", "'swap'", "'('", "','", "')'", "'{'", "'}'", "'directed'", 
      "':'", "'true'", "'false'", "'nodes:'", "'edges:'", "'file'", "'->'", 
      "'='", "'.'", "'add'", "'remove'", "'if'", "'else'", "'in'", "'['", 
      "'where'", "']'", "'degree'", "'connected'", "'with'", "'edge'", "'has'", 
      "'vertex'", "'cycle'", "'for'", "'each'", "'out'", "'neighbor'", "'element'", 
      "'while'", "'to'", "'from'", "'query'", "'show'", "'draw'", "'layout'", 
      "'labels'", "'color'", "'size'", "'categorical'", "'continuous'", 
      "'fn'", "'int'", "'real'", "'bool'", "'void'", "'string'", "'sleep'", 
      "'return'", "'break'", "'continue'", "'print'", "'contains'", "'pop'", 
      "'[]'", "'INF'", "'timer'", "'graph'", "'weights'", "'set'", "'union'", 
      "'intersect'", "'!'", "'%'", "'edges'", "'nodes'", "'vertices'", "'TRUE'", 
      "'FALSE'", "'of'", "'+'", "'-'", "'*'", "'/'", "'&&'", "'||'", "'=='", 
      "'!='", "'<'", "'>'", "'<='", "'>='"
    },
    std::vector<std::string>{
      "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", 
      "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", 
      "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", 
      "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "GRAPH", 
      "WEIGHTS", "SET", "UNION", "INTERSECT", "NOT", "MODULO", "EDGE", "NODE", 
      "VERTICES", "TRUE", "FALSE", "OF", "PLUS", "MINUS", "TIMES", "DIVIDE", 
      "AND", "OR", "EQUAL", "NOTEQUAL", "LESSTHAN", "GREATERTHAN", "LESSEQUAL", 
      "GREATEREQUAL", "ID", "INT", "REAL", "STRING", "Comment", "WS"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,1,96,899,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,2,
  	7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,7,
  	14,2,15,7,15,2,16,7,16,2,17,7,17,2,18,7,18,2,19,7,19,2,20,7,20,2,21,7,
  	21,2,22,7,22,2,23,7,23,2,24,7,24,2,25,7,25,2,26,7,26,2,27,7,27,2,28,7,
  	28,2,29,7,29,2,30,7,30,2,31,7,31,2,32,7,32,2,33,7,33,2,34,7,34,2,35,7,
  	35,2,36,7,36,2,37,7,37,2,38,7,38,2,39,7,39,2,40,7,40,2,41,7,41,2,42,7,
  	42,2,43,7,43,2,44,7,44,2,45,7,45,2,46,7,46,2,47,7,47,2,48,7,48,2,49,7,
  	49,2,50,7,50,2,51,7,51,2,52,7,52,2,53,7,53,2,54,7,54,2,55,7,55,2,56,7,
  	56,2,57,7,57,2,58,7,58,2,59,7,59,2,60,7,60,2,61,7,61,2,62,7,62,2,63,7,
  	63,2,64,7,64,2,65,7,65,1,0,1,0,5,0,135,8,0,10,0,12,0,138,9,0,1,0,1,0,
  	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  	1,1,1,1,1,1,1,1,1,3,1,164,8,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,3,1,3,
  	1,3,1,3,5,3,178,8,3,10,3,12,3,181,9,3,1,3,3,3,184,8,3,1,3,5,3,187,8,3,
  	10,3,12,3,190,9,3,1,3,3,3,193,8,3,1,3,5,3,196,8,3,10,3,12,3,199,9,3,1,
  	3,1,3,5,3,203,8,3,10,3,12,3,206,9,3,1,3,1,3,1,3,1,3,1,3,1,3,1,3,5,3,215,
  	8,3,10,3,12,3,218,9,3,1,3,3,3,221,8,3,1,3,5,3,224,8,3,10,3,12,3,227,9,
  	3,1,3,3,3,230,8,3,1,3,5,3,233,8,3,10,3,12,3,236,9,3,1,3,1,3,1,3,3,3,241,
  	8,3,1,4,1,4,1,4,1,4,1,4,1,5,1,5,1,6,1,6,1,6,1,6,1,7,1,7,1,7,3,7,257,8,
  	7,1,7,1,7,1,8,1,8,1,8,5,8,264,8,8,10,8,12,8,267,9,8,1,9,1,9,1,9,5,9,272,
  	8,9,10,9,12,9,275,9,9,1,10,1,10,1,11,1,11,1,12,1,12,1,12,1,13,1,13,1,
  	13,1,13,1,14,1,14,1,14,1,14,3,14,292,8,14,1,14,1,14,1,14,1,14,1,14,1,
  	14,3,14,300,8,14,1,14,1,14,3,14,304,8,14,1,15,1,15,1,15,1,15,1,15,1,15,
  	1,15,1,15,1,15,1,15,1,15,1,15,1,15,1,15,1,15,3,15,321,8,15,1,16,1,16,
  	1,16,1,16,5,16,327,8,16,10,16,12,16,330,9,16,3,16,332,8,16,1,16,1,16,
  	1,17,1,17,1,17,1,17,1,17,1,18,1,18,1,18,1,18,1,18,1,18,1,18,1,18,1,18,
  	3,18,350,8,18,1,19,1,19,1,19,1,19,1,19,1,19,1,19,1,19,1,19,1,19,1,19,
  	1,19,1,19,1,19,1,19,3,19,367,8,19,1,19,1,19,1,19,1,19,1,19,1,19,5,19,
  	375,8,19,10,19,12,19,378,9,19,1,20,1,20,1,20,1,20,1,20,1,20,1,20,1,20,
  	1,20,1,20,1,20,1,20,1,20,1,20,1,20,1,20,3,20,396,8,20,1,21,1,21,1,21,
  	1,21,1,21,1,21,1,21,1,21,1,21,3,21,407,8,21,1,22,1,22,1,22,1,22,1,22,
  	1,22,1,22,1,22,1,22,1,22,1,22,1,22,1,22,1,22,3,22,423,8,22,1,22,1,22,
  	1,22,1,22,1,22,1,22,5,22,431,8,22,10,22,12,22,434,9,22,1,23,3,23,437,
  	8,23,1,23,1,23,1,23,1,23,1,23,1,23,3,23,445,8,23,1,23,1,23,1,23,1,24,
  	1,24,1,24,5,24,453,8,24,10,24,12,24,456,9,24,1,25,1,25,1,25,1,25,1,25,
  	1,25,1,25,1,25,1,25,1,25,1,25,1,25,1,25,1,25,1,25,1,25,1,25,1,25,3,25,
  	476,8,25,1,25,1,25,1,25,1,25,1,25,1,25,5,25,484,8,25,10,25,12,25,487,
  	9,25,1,26,1,26,3,26,491,8,26,1,27,1,27,1,27,1,27,1,27,1,27,1,27,1,28,
  	1,28,1,28,1,28,1,28,1,28,1,28,1,28,1,28,1,28,1,28,1,28,1,28,1,28,1,28,
  	1,28,1,28,1,28,1,28,1,28,1,28,1,28,1,28,3,28,523,8,28,1,29,1,29,1,29,
  	1,29,1,29,1,29,1,30,1,30,3,30,533,8,30,1,31,1,31,1,31,1,31,1,31,1,31,
  	1,32,1,32,1,32,1,32,1,32,1,32,1,33,1,33,1,33,1,33,3,33,551,8,33,1,34,
  	1,34,1,34,1,34,3,34,557,8,34,1,35,1,35,1,35,1,35,1,35,3,35,564,8,35,1,
  	35,1,35,1,35,1,35,1,36,1,36,1,36,1,36,1,37,1,37,1,37,1,37,1,37,1,37,5,
  	37,580,8,37,10,37,12,37,583,9,37,1,37,3,37,586,8,37,1,37,1,37,1,38,1,
  	38,1,38,1,38,1,38,1,38,1,38,5,38,597,8,38,10,38,12,38,600,9,38,1,38,1,
  	38,1,38,1,38,5,38,606,8,38,10,38,12,38,609,9,38,1,38,3,38,612,8,38,1,
  	39,1,39,1,39,1,39,1,39,1,39,1,39,1,39,1,39,1,39,1,39,1,39,1,39,1,39,1,
  	39,3,39,629,8,39,1,40,1,40,1,40,1,40,1,40,1,41,1,41,1,41,1,41,1,41,3,
  	41,641,8,41,1,42,1,42,1,42,1,42,1,42,1,43,1,43,1,43,1,43,1,43,1,43,1,
  	44,1,44,1,45,1,45,1,45,1,45,5,45,660,8,45,10,45,12,45,663,9,45,3,45,665,
  	8,45,1,45,1,45,1,46,1,46,1,46,1,47,1,47,1,48,1,48,1,48,3,48,677,8,48,
  	1,48,1,48,1,49,1,49,1,50,1,50,1,50,5,50,686,8,50,10,50,12,50,689,9,50,
  	1,51,1,51,1,51,1,51,1,51,1,51,1,52,1,52,1,52,1,52,1,52,5,52,702,8,52,
  	10,52,12,52,705,9,52,1,52,1,52,1,52,3,52,710,8,52,1,53,1,53,1,53,1,53,
  	1,54,1,54,1,54,1,55,1,55,1,55,1,56,1,56,1,56,1,56,1,56,1,56,3,56,728,
  	8,56,1,57,1,57,1,57,3,57,733,8,57,1,57,1,57,1,57,5,57,738,8,57,10,57,
  	12,57,741,9,57,1,58,1,58,1,58,1,58,1,58,1,58,1,58,1,59,1,59,1,59,1,59,
  	1,59,1,59,1,59,1,59,1,59,1,59,1,59,1,59,1,59,1,59,1,59,1,59,1,59,1,59,
  	3,59,768,8,59,1,60,1,60,1,60,1,60,1,60,1,60,1,60,1,60,1,60,1,60,1,60,
  	1,60,1,60,1,60,1,60,1,60,1,60,1,60,1,60,1,60,1,60,1,60,1,60,1,60,1,60,
  	1,60,1,60,1,60,1,60,1,60,1,60,1,60,1,60,1,60,1,60,1,60,1,60,1,60,1,60,
  	1,60,1,60,1,60,1,60,1,60,1,60,1,60,1,60,1,60,1,60,1,60,1,60,1,60,3,60,
  	822,8,60,1,60,1,60,1,60,1,60,1,60,1,60,1,60,1,60,1,60,5,60,833,8,60,10,
  	60,12,60,836,9,60,1,61,1,61,1,61,1,61,1,61,1,61,1,61,1,61,1,61,1,61,1,
  	61,1,61,1,61,1,61,1,61,1,61,3,61,854,8,61,1,62,1,62,1,62,1,62,5,62,860,
  	8,62,10,62,12,62,863,9,62,1,62,1,62,1,63,1,63,1,63,1,63,1,63,1,63,1,63,
  	3,63,874,8,63,1,64,1,64,1,64,1,64,1,64,1,64,1,64,1,64,1,64,1,64,1,64,
  	1,64,1,64,1,64,1,64,1,64,1,64,1,64,1,64,3,64,895,8,64,1,65,1,65,1,65,
  	0,5,38,44,50,114,120,66,0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,
  	34,36,38,40,42,44,46,48,50,52,54,56,58,60,62,64,66,68,70,72,74,76,78,
  	80,82,84,86,88,90,92,94,96,98,100,102,104,106,108,110,112,114,116,118,
  	120,122,124,126,128,130,0,10,2,0,10,11,76,77,1,0,74,75,1,0,85,90,1,0,
  	83,84,5,0,29,29,31,31,51,55,66,66,68,68,6,0,29,29,31,31,51,53,55,55,66,
  	66,68,68,2,0,26,26,91,91,2,0,72,72,81,82,1,0,79,80,1,0,76,77,965,0,136,
  	1,0,0,0,2,163,1,0,0,0,4,165,1,0,0,0,6,240,1,0,0,0,8,242,1,0,0,0,10,247,
  	1,0,0,0,12,249,1,0,0,0,14,253,1,0,0,0,16,260,1,0,0,0,18,268,1,0,0,0,20,
  	276,1,0,0,0,22,278,1,0,0,0,24,280,1,0,0,0,26,283,1,0,0,0,28,303,1,0,0,
  	0,30,320,1,0,0,0,32,322,1,0,0,0,34,335,1,0,0,0,36,349,1,0,0,0,38,366,
  	1,0,0,0,40,395,1,0,0,0,42,397,1,0,0,0,44,422,1,0,0,0,46,436,1,0,0,0,48,
  	449,1,0,0,0,50,475,1,0,0,0,52,490,1,0,0,0,54,492,1,0,0,0,56,522,1,0,0,
  	0,58,524,1,0,0,0,60,532,1,0,0,0,62,534,1,0,0,0,64,540,1,0,0,0,66,550,
  	1,0,0,0,68,556,1,0,0,0,70,558,1,0,0,0,72,569,1,0,0,0,74,573,1,0,0,0,76,
  	611,1,0,0,0,78,628,1,0,0,0,80,630,1,0,0,0,82,640,1,0,0,0,84,642,1,0,0,
  	0,86,647,1,0,0,0,88,653,1,0,0,0,90,655,1,0,0,0,92,668,1,0,0,0,94,671,
  	1,0,0,0,96,673,1,0,0,0,98,680,1,0,0,0,100,682,1,0,0,0,102,690,1,0,0,0,
  	104,709,1,0,0,0,106,711,1,0,0,0,108,715,1,0,0,0,110,718,1,0,0,0,112,727,
  	1,0,0,0,114,732,1,0,0,0,116,742,1,0,0,0,118,767,1,0,0,0,120,821,1,0,0,
  	0,122,853,1,0,0,0,124,855,1,0,0,0,126,873,1,0,0,0,128,894,1,0,0,0,130,
  	896,1,0,0,0,132,135,3,2,1,0,133,135,3,86,43,0,134,132,1,0,0,0,134,133,
  	1,0,0,0,135,138,1,0,0,0,136,134,1,0,0,0,136,137,1,0,0,0,137,139,1,0,0,
  	0,138,136,1,0,0,0,139,140,5,0,0,1,140,1,1,0,0,0,141,164,3,6,3,0,142,164,
  	3,42,21,0,143,164,3,112,56,0,144,164,3,58,29,0,145,164,3,54,27,0,146,
  	164,3,28,14,0,147,164,3,30,15,0,148,149,3,96,48,0,149,150,5,1,0,0,150,
  	164,1,0,0,0,151,164,3,102,51,0,152,164,3,46,23,0,153,164,3,128,64,0,154,
  	164,3,126,63,0,155,164,3,70,35,0,156,164,3,72,36,0,157,164,3,74,37,0,
  	158,164,3,60,30,0,159,164,3,34,17,0,160,164,3,40,20,0,161,164,3,4,2,0,
  	162,164,5,1,0,0,163,141,1,0,0,0,163,142,1,0,0,0,163,143,1,0,0,0,163,144,
  	1,0,0,0,163,145,1,0,0,0,163,146,1,0,0,0,163,147,1,0,0,0,163,148,1,0,0,
  	0,163,151,1,0,0,0,163,152,1,0,0,0,163,153,1,0,0,0,163,154,1,0,0,0,163,
  	155,1,0,0,0,163,156,1,0,0,0,163,157,1,0,0,0,163,158,1,0,0,0,163,159,1,
  	0,0,0,163,160,1,0,0,0,163,161,1,0,0,0,163,162,1,0,0,0,164,3,1,0,0,0,165,
  	166,5,2,0,0,166,167,5,3,0,0,167,168,5,91,0,0,168,169,5,4,0,0,169,170,
  	5,91,0,0,170,171,5,5,0,0,171,172,5,1,0,0,172,5,1,0,0,0,173,174,5,66,0,
  	0,174,175,3,20,10,0,175,179,5,6,0,0,176,178,3,8,4,0,177,176,1,0,0,0,178,
  	181,1,0,0,0,179,177,1,0,0,0,179,180,1,0,0,0,180,183,1,0,0,0,181,179,1,
  	0,0,0,182,184,3,12,6,0,183,182,1,0,0,0,183,184,1,0,0,0,184,188,1,0,0,
  	0,185,187,3,8,4,0,186,185,1,0,0,0,187,190,1,0,0,0,188,186,1,0,0,0,188,
  	189,1,0,0,0,189,192,1,0,0,0,190,188,1,0,0,0,191,193,3,14,7,0,192,191,
  	1,0,0,0,192,193,1,0,0,0,193,197,1,0,0,0,194,196,3,8,4,0,195,194,1,0,0,
  	0,196,199,1,0,0,0,197,195,1,0,0,0,197,198,1,0,0,0,198,200,1,0,0,0,199,
  	197,1,0,0,0,200,204,5,76,0,0,201,203,3,8,4,0,202,201,1,0,0,0,203,206,
  	1,0,0,0,204,202,1,0,0,0,204,205,1,0,0,0,205,207,1,0,0,0,206,204,1,0,0,
  	0,207,208,5,7,0,0,208,209,5,1,0,0,209,241,1,0,0,0,210,211,5,66,0,0,211,
  	212,3,20,10,0,212,216,5,6,0,0,213,215,3,8,4,0,214,213,1,0,0,0,215,218,
  	1,0,0,0,216,214,1,0,0,0,216,217,1,0,0,0,217,220,1,0,0,0,218,216,1,0,0,
  	0,219,221,3,12,6,0,220,219,1,0,0,0,220,221,1,0,0,0,221,225,1,0,0,0,222,
  	224,3,8,4,0,223,222,1,0,0,0,224,227,1,0,0,0,225,223,1,0,0,0,225,226,1,
  	0,0,0,226,229,1,0,0,0,227,225,1,0,0,0,228,230,3,14,7,0,229,228,1,0,0,
  	0,229,230,1,0,0,0,230,234,1,0,0,0,231,233,3,8,4,0,232,231,1,0,0,0,233,
  	236,1,0,0,0,234,232,1,0,0,0,234,235,1,0,0,0,235,237,1,0,0,0,236,234,1,
  	0,0,0,237,238,5,7,0,0,238,239,5,1,0,0,239,241,1,0,0,0,240,173,1,0,0,0,
  	240,210,1,0,0,0,241,7,1,0,0,0,242,243,5,8,0,0,243,244,5,9,0,0,244,245,
  	3,10,5,0,245,246,5,1,0,0,246,9,1,0,0,0,247,248,7,0,0,0,248,11,1,0,0,0,
  	249,250,5,12,0,0,250,251,3,16,8,0,251,252,5,1,0,0,252,13,1,0,0,0,253,
  	256,5,13,0,0,254,257,3,18,9,0,255,257,3,24,12,0,256,254,1,0,0,0,256,255,
  	1,0,0,0,257,258,1,0,0,0,258,259,5,1,0,0,259,15,1,0,0,0,260,265,3,22,11,
  	0,261,262,5,4,0,0,262,264,3,22,11,0,263,261,1,0,0,0,264,267,1,0,0,0,265,
  	263,1,0,0,0,265,266,1,0,0,0,266,17,1,0,0,0,267,265,1,0,0,0,268,273,3,
  	26,13,0,269,270,5,4,0,0,270,272,3,26,13,0,271,269,1,0,0,0,272,275,1,0,
  	0,0,273,271,1,0,0,0,273,274,1,0,0,0,274,19,1,0,0,0,275,273,1,0,0,0,276,
  	277,5,91,0,0,277,21,1,0,0,0,278,279,5,92,0,0,279,23,1,0,0,0,280,281,5,
  	14,0,0,281,282,5,94,0,0,282,25,1,0,0,0,283,284,3,22,11,0,284,285,5,15,
  	0,0,285,286,3,22,11,0,286,27,1,0,0,0,287,288,3,94,47,0,288,291,5,91,0,
  	0,289,290,5,16,0,0,290,292,3,120,60,0,291,289,1,0,0,0,291,292,1,0,0,0,
  	292,293,1,0,0,0,293,294,5,1,0,0,294,304,1,0,0,0,295,296,3,94,47,0,296,
  	299,3,122,61,0,297,298,5,16,0,0,298,300,3,124,62,0,299,297,1,0,0,0,299,
  	300,1,0,0,0,300,301,1,0,0,0,301,302,5,1,0,0,302,304,1,0,0,0,303,287,1,
  	0,0,0,303,295,1,0,0,0,304,29,1,0,0,0,305,306,5,68,0,0,306,307,5,91,0,
  	0,307,321,5,1,0,0,308,309,5,68,0,0,309,310,5,91,0,0,310,311,5,16,0,0,
  	311,312,3,32,16,0,312,313,5,1,0,0,313,321,1,0,0,0,314,315,5,68,0,0,315,
  	316,5,91,0,0,316,317,5,16,0,0,317,318,3,38,19,0,318,319,5,1,0,0,319,321,
  	1,0,0,0,320,305,1,0,0,0,320,308,1,0,0,0,320,314,1,0,0,0,321,31,1,0,0,
  	0,322,331,5,6,0,0,323,328,3,120,60,0,324,325,5,4,0,0,325,327,3,120,60,
  	0,326,324,1,0,0,0,327,330,1,0,0,0,328,326,1,0,0,0,328,329,1,0,0,0,329,
  	332,1,0,0,0,330,328,1,0,0,0,331,323,1,0,0,0,331,332,1,0,0,0,332,333,1,
  	0,0,0,333,334,5,7,0,0,334,33,1,0,0,0,335,336,5,91,0,0,336,337,5,16,0,
  	0,337,338,3,38,19,0,338,339,5,1,0,0,339,35,1,0,0,0,340,350,5,91,0,0,341,
  	342,3,20,10,0,342,343,5,17,0,0,343,344,7,1,0,0,344,350,1,0,0,0,345,346,
  	3,20,10,0,346,347,5,17,0,0,347,348,5,73,0,0,348,350,1,0,0,0,349,340,1,
  	0,0,0,349,341,1,0,0,0,349,345,1,0,0,0,350,37,1,0,0,0,351,352,6,19,-1,
  	0,352,353,3,20,10,0,353,354,5,17,0,0,354,355,7,1,0,0,355,367,1,0,0,0,
  	356,357,3,20,10,0,357,358,5,17,0,0,358,359,5,73,0,0,359,367,1,0,0,0,360,
  	367,5,91,0,0,361,367,3,32,16,0,362,363,5,3,0,0,363,364,3,38,19,0,364,
  	365,5,5,0,0,365,367,1,0,0,0,366,351,1,0,0,0,366,356,1,0,0,0,366,360,1,
  	0,0,0,366,361,1,0,0,0,366,362,1,0,0,0,367,376,1,0,0,0,368,369,10,7,0,
  	0,369,370,5,69,0,0,370,375,3,38,19,8,371,372,10,6,0,0,372,373,5,70,0,
  	0,373,375,3,38,19,7,374,368,1,0,0,0,374,371,1,0,0,0,375,378,1,0,0,0,376,
  	374,1,0,0,0,376,377,1,0,0,0,377,39,1,0,0,0,378,376,1,0,0,0,379,380,3,
  	36,18,0,380,381,5,17,0,0,381,382,5,18,0,0,382,383,5,3,0,0,383,384,3,120,
  	60,0,384,385,5,5,0,0,385,386,5,1,0,0,386,396,1,0,0,0,387,388,3,36,18,
  	0,388,389,5,17,0,0,389,390,5,19,0,0,390,391,5,3,0,0,391,392,3,120,60,
  	0,392,393,5,5,0,0,393,394,5,1,0,0,394,396,1,0,0,0,395,379,1,0,0,0,395,
  	387,1,0,0,0,396,41,1,0,0,0,397,398,5,20,0,0,398,399,5,3,0,0,399,400,3,
  	44,22,0,400,401,5,5,0,0,401,406,3,104,52,0,402,403,5,21,0,0,403,407,3,
  	42,21,0,404,405,5,21,0,0,405,407,3,104,52,0,406,402,1,0,0,0,406,404,1,
  	0,0,0,406,407,1,0,0,0,407,43,1,0,0,0,408,409,6,22,-1,0,409,410,3,120,
  	60,0,410,411,7,2,0,0,411,412,3,120,60,0,412,423,1,0,0,0,413,414,3,22,
  	11,0,414,415,5,22,0,0,415,416,3,20,10,0,416,423,1,0,0,0,417,418,3,26,
  	13,0,418,419,5,22,0,0,419,420,3,20,10,0,420,423,1,0,0,0,421,423,3,120,
  	60,0,422,408,1,0,0,0,422,413,1,0,0,0,422,417,1,0,0,0,422,421,1,0,0,0,
  	423,432,1,0,0,0,424,425,10,6,0,0,425,426,5,83,0,0,426,431,3,44,22,7,427,
  	428,10,5,0,0,428,429,5,84,0,0,429,431,3,44,22,6,430,424,1,0,0,0,430,427,
  	1,0,0,0,431,434,1,0,0,0,432,430,1,0,0,0,432,433,1,0,0,0,433,45,1,0,0,
  	0,434,432,1,0,0,0,435,437,5,66,0,0,436,435,1,0,0,0,436,437,1,0,0,0,437,
  	438,1,0,0,0,438,439,5,91,0,0,439,440,5,16,0,0,440,441,5,23,0,0,441,444,
  	3,48,24,0,442,443,5,24,0,0,443,445,3,50,25,0,444,442,1,0,0,0,444,445,
  	1,0,0,0,445,446,1,0,0,0,446,447,5,25,0,0,447,448,5,1,0,0,448,47,1,0,0,
  	0,449,454,3,20,10,0,450,451,7,3,0,0,451,453,3,20,10,0,452,450,1,0,0,0,
  	453,456,1,0,0,0,454,452,1,0,0,0,454,455,1,0,0,0,455,49,1,0,0,0,456,454,
  	1,0,0,0,457,458,6,25,-1,0,458,459,5,26,0,0,459,460,7,2,0,0,460,476,5,
  	92,0,0,461,462,5,27,0,0,462,463,5,28,0,0,463,476,3,22,11,0,464,465,5,
  	29,0,0,465,466,5,30,0,0,466,476,3,120,60,0,467,468,5,31,0,0,468,469,5,
  	22,0,0,469,476,5,91,0,0,470,476,5,32,0,0,471,472,5,3,0,0,472,473,3,50,
  	25,0,473,474,5,5,0,0,474,476,1,0,0,0,475,457,1,0,0,0,475,461,1,0,0,0,
  	475,464,1,0,0,0,475,467,1,0,0,0,475,470,1,0,0,0,475,471,1,0,0,0,476,485,
  	1,0,0,0,477,478,10,8,0,0,478,479,5,83,0,0,479,484,3,50,25,9,480,481,10,
  	7,0,0,481,482,5,84,0,0,482,484,3,50,25,8,483,477,1,0,0,0,483,480,1,0,
  	0,0,484,487,1,0,0,0,485,483,1,0,0,0,485,486,1,0,0,0,486,51,1,0,0,0,487,
  	485,1,0,0,0,488,491,3,54,27,0,489,491,3,58,29,0,490,488,1,0,0,0,490,489,
  	1,0,0,0,491,53,1,0,0,0,492,493,5,33,0,0,493,494,5,34,0,0,494,495,3,56,
  	28,0,495,496,5,22,0,0,496,497,3,20,10,0,497,498,3,104,52,0,498,55,1,0,
  	0,0,499,500,5,31,0,0,500,523,5,91,0,0,501,502,5,29,0,0,502,503,5,91,0,
  	0,503,504,5,4,0,0,504,523,5,91,0,0,505,506,5,35,0,0,506,507,5,36,0,0,
  	507,508,5,91,0,0,508,509,5,78,0,0,509,523,3,120,60,0,510,511,5,22,0,0,
  	511,512,5,36,0,0,512,513,5,91,0,0,513,514,5,78,0,0,514,523,3,120,60,0,
  	515,516,5,36,0,0,516,517,5,91,0,0,517,518,5,78,0,0,518,523,3,120,60,0,
  	519,520,5,37,0,0,520,523,5,91,0,0,521,523,5,91,0,0,522,499,1,0,0,0,522,
  	501,1,0,0,0,522,505,1,0,0,0,522,510,1,0,0,0,522,515,1,0,0,0,522,519,1,
  	0,0,0,522,521,1,0,0,0,523,57,1,0,0,0,524,525,5,38,0,0,525,526,5,3,0,0,
  	526,527,3,44,22,0,527,528,5,5,0,0,528,529,3,104,52,0,529,59,1,0,0,0,530,
  	533,3,62,31,0,531,533,3,64,32,0,532,530,1,0,0,0,532,531,1,0,0,0,533,61,
  	1,0,0,0,534,535,5,18,0,0,535,536,3,66,33,0,536,537,5,39,0,0,537,538,3,
  	20,10,0,538,539,5,1,0,0,539,63,1,0,0,0,540,541,5,19,0,0,541,542,3,68,
  	34,0,542,543,5,40,0,0,543,544,3,20,10,0,544,545,5,1,0,0,545,65,1,0,0,
  	0,546,551,3,22,11,0,547,551,3,26,13,0,548,551,3,16,8,0,549,551,3,18,9,
  	0,550,546,1,0,0,0,550,547,1,0,0,0,550,548,1,0,0,0,550,549,1,0,0,0,551,
  	67,1,0,0,0,552,557,3,22,11,0,553,557,3,26,13,0,554,557,3,16,8,0,555,557,
  	3,18,9,0,556,552,1,0,0,0,556,553,1,0,0,0,556,554,1,0,0,0,556,555,1,0,
  	0,0,557,69,1,0,0,0,558,559,5,41,0,0,559,560,5,91,0,0,560,561,5,9,0,0,
  	561,563,5,94,0,0,562,564,5,92,0,0,563,562,1,0,0,0,563,564,1,0,0,0,564,
  	565,1,0,0,0,565,566,5,78,0,0,566,567,3,20,10,0,567,568,5,1,0,0,568,71,
  	1,0,0,0,569,570,5,42,0,0,570,571,3,20,10,0,571,572,5,1,0,0,572,73,1,0,
  	0,0,573,574,5,43,0,0,574,575,3,20,10,0,575,576,5,39,0,0,576,585,5,94,
  	0,0,577,581,5,6,0,0,578,580,3,76,38,0,579,578,1,0,0,0,580,583,1,0,0,0,
  	581,579,1,0,0,0,581,582,1,0,0,0,582,584,1,0,0,0,583,581,1,0,0,0,584,586,
  	5,7,0,0,585,577,1,0,0,0,585,586,1,0,0,0,586,587,1,0,0,0,587,588,5,1,0,
  	0,588,75,1,0,0,0,589,590,5,44,0,0,590,591,5,9,0,0,591,592,5,94,0,0,592,
  	612,5,1,0,0,593,594,5,75,0,0,594,598,5,6,0,0,595,597,3,78,39,0,596,595,
  	1,0,0,0,597,600,1,0,0,0,598,596,1,0,0,0,598,599,1,0,0,0,599,601,1,0,0,
  	0,600,598,1,0,0,0,601,612,5,7,0,0,602,603,5,73,0,0,603,607,5,6,0,0,604,
  	606,3,80,40,0,605,604,1,0,0,0,606,609,1,0,0,0,607,605,1,0,0,0,607,608,
  	1,0,0,0,608,610,1,0,0,0,609,607,1,0,0,0,610,612,5,7,0,0,611,589,1,0,0,
  	0,611,593,1,0,0,0,611,602,1,0,0,0,612,77,1,0,0,0,613,614,5,45,0,0,614,
  	615,5,9,0,0,615,616,3,10,5,0,616,617,5,1,0,0,617,629,1,0,0,0,618,619,
  	5,46,0,0,619,620,5,9,0,0,620,621,3,82,41,0,621,622,5,1,0,0,622,629,1,
  	0,0,0,623,624,5,47,0,0,624,625,5,9,0,0,625,626,3,84,42,0,626,627,5,1,
  	0,0,627,629,1,0,0,0,628,613,1,0,0,0,628,618,1,0,0,0,628,623,1,0,0,0,629,
  	79,1,0,0,0,630,631,5,45,0,0,631,632,5,9,0,0,632,633,3,10,5,0,633,634,
  	5,1,0,0,634,81,1,0,0,0,635,636,5,48,0,0,636,637,5,3,0,0,637,638,5,91,
  	0,0,638,641,5,5,0,0,639,641,3,84,42,0,640,635,1,0,0,0,640,639,1,0,0,0,
  	641,83,1,0,0,0,642,643,5,49,0,0,643,644,5,3,0,0,644,645,5,91,0,0,645,
  	646,5,5,0,0,646,85,1,0,0,0,647,648,5,50,0,0,648,649,3,88,44,0,649,650,
  	5,91,0,0,650,651,3,90,45,0,651,652,3,104,52,0,652,87,1,0,0,0,653,654,
  	7,4,0,0,654,89,1,0,0,0,655,664,5,3,0,0,656,661,3,92,46,0,657,658,5,4,
  	0,0,658,660,3,92,46,0,659,657,1,0,0,0,660,663,1,0,0,0,661,659,1,0,0,0,
  	661,662,1,0,0,0,662,665,1,0,0,0,663,661,1,0,0,0,664,656,1,0,0,0,664,665,
  	1,0,0,0,665,666,1,0,0,0,666,667,5,5,0,0,667,91,1,0,0,0,668,669,3,94,47,
  	0,669,670,5,91,0,0,670,93,1,0,0,0,671,672,7,5,0,0,672,95,1,0,0,0,673,
  	674,3,98,49,0,674,676,5,3,0,0,675,677,3,100,50,0,676,675,1,0,0,0,676,
  	677,1,0,0,0,677,678,1,0,0,0,678,679,5,5,0,0,679,97,1,0,0,0,680,681,7,
  	6,0,0,681,99,1,0,0,0,682,687,3,120,60,0,683,684,5,4,0,0,684,686,3,120,
  	60,0,685,683,1,0,0,0,686,689,1,0,0,0,687,685,1,0,0,0,687,688,1,0,0,0,
  	688,101,1,0,0,0,689,687,1,0,0,0,690,691,5,56,0,0,691,692,5,3,0,0,692,
  	693,3,120,60,0,693,694,5,5,0,0,694,695,5,1,0,0,695,103,1,0,0,0,696,703,
  	5,6,0,0,697,702,3,2,1,0,698,702,3,106,53,0,699,702,3,108,54,0,700,702,
  	3,110,55,0,701,697,1,0,0,0,701,698,1,0,0,0,701,699,1,0,0,0,701,700,1,
  	0,0,0,702,705,1,0,0,0,703,701,1,0,0,0,703,704,1,0,0,0,704,706,1,0,0,0,
  	705,703,1,0,0,0,706,710,5,7,0,0,707,708,5,6,0,0,708,710,5,7,0,0,709,696,
  	1,0,0,0,709,707,1,0,0,0,710,105,1,0,0,0,711,712,5,57,0,0,712,713,3,120,
  	60,0,713,714,5,1,0,0,714,107,1,0,0,0,715,716,5,58,0,0,716,717,5,1,0,0,
  	717,109,1,0,0,0,718,719,5,59,0,0,719,720,5,1,0,0,720,111,1,0,0,0,721,
  	722,5,60,0,0,722,723,3,114,57,0,723,724,5,1,0,0,724,728,1,0,0,0,725,728,
  	3,116,58,0,726,728,3,118,59,0,727,721,1,0,0,0,727,725,1,0,0,0,727,726,
  	1,0,0,0,728,113,1,0,0,0,729,730,6,57,-1,0,730,733,5,94,0,0,731,733,3,
  	120,60,0,732,729,1,0,0,0,732,731,1,0,0,0,733,739,1,0,0,0,734,735,10,1,
  	0,0,735,736,5,79,0,0,736,738,3,114,57,2,737,734,1,0,0,0,738,741,1,0,0,
  	0,739,737,1,0,0,0,739,740,1,0,0,0,740,115,1,0,0,0,741,739,1,0,0,0,742,
  	743,5,60,0,0,743,744,5,91,0,0,744,745,5,23,0,0,745,746,3,120,60,0,746,
  	747,5,25,0,0,747,748,5,1,0,0,748,117,1,0,0,0,749,750,5,60,0,0,750,751,
  	5,73,0,0,751,752,5,78,0,0,752,753,3,20,10,0,753,754,5,1,0,0,754,768,1,
  	0,0,0,755,756,5,60,0,0,756,757,5,74,0,0,757,758,5,78,0,0,758,759,3,20,
  	10,0,759,760,5,1,0,0,760,768,1,0,0,0,761,762,5,60,0,0,762,763,5,66,0,
  	0,763,764,5,78,0,0,764,765,3,20,10,0,765,766,5,1,0,0,766,768,1,0,0,0,
  	767,749,1,0,0,0,767,755,1,0,0,0,767,761,1,0,0,0,768,119,1,0,0,0,769,770,
  	6,60,-1,0,770,771,5,80,0,0,771,822,3,120,60,18,772,773,5,71,0,0,773,822,
  	3,120,60,17,774,822,3,96,48,0,775,822,5,92,0,0,776,822,5,91,0,0,777,778,
  	5,3,0,0,778,779,3,120,60,0,779,780,5,5,0,0,780,822,1,0,0,0,781,782,5,
  	91,0,0,782,783,5,23,0,0,783,784,3,120,60,0,784,785,5,25,0,0,785,786,5,
  	23,0,0,786,787,3,120,60,0,787,788,5,25,0,0,788,822,1,0,0,0,789,790,5,
  	91,0,0,790,791,5,23,0,0,791,792,3,120,60,0,792,793,5,25,0,0,793,822,1,
  	0,0,0,794,795,3,36,18,0,795,796,5,17,0,0,796,797,5,61,0,0,797,798,5,3,
  	0,0,798,799,3,120,60,0,799,800,5,5,0,0,800,822,1,0,0,0,801,802,5,91,0,
  	0,802,803,5,17,0,0,803,804,5,47,0,0,804,805,5,3,0,0,805,822,5,5,0,0,806,
  	807,5,91,0,0,807,808,5,17,0,0,808,809,5,62,0,0,809,810,5,3,0,0,810,822,
  	5,5,0,0,811,822,5,76,0,0,812,822,5,77,0,0,813,814,5,91,0,0,814,822,5,
  	63,0,0,815,822,5,93,0,0,816,822,3,32,16,0,817,822,5,64,0,0,818,819,5,
  	65,0,0,819,820,5,3,0,0,820,822,5,5,0,0,821,769,1,0,0,0,821,772,1,0,0,
  	0,821,774,1,0,0,0,821,775,1,0,0,0,821,776,1,0,0,0,821,777,1,0,0,0,821,
  	781,1,0,0,0,821,789,1,0,0,0,821,794,1,0,0,0,821,801,1,0,0,0,821,806,1,
  	0,0,0,821,811,1,0,0,0,821,812,1,0,0,0,821,813,1,0,0,0,821,815,1,0,0,0,
  	821,816,1,0,0,0,821,817,1,0,0,0,821,818,1,0,0,0,822,834,1,0,0,0,823,824,
  	10,21,0,0,824,825,7,3,0,0,825,833,3,120,60,22,826,827,10,20,0,0,827,828,
  	7,7,0,0,828,833,3,120,60,21,829,830,10,19,0,0,830,831,7,8,0,0,831,833,
  	3,120,60,20,832,823,1,0,0,0,832,826,1,0,0,0,832,829,1,0,0,0,833,836,1,
  	0,0,0,834,832,1,0,0,0,834,835,1,0,0,0,835,121,1,0,0,0,836,834,1,0,0,0,
  	837,838,5,91,0,0,838,839,5,23,0,0,839,840,3,120,60,0,840,841,5,25,0,0,
  	841,842,5,23,0,0,842,843,3,120,60,0,843,844,5,25,0,0,844,854,1,0,0,0,
  	845,846,5,91,0,0,846,847,5,23,0,0,847,848,3,120,60,0,848,849,5,25,0,0,
  	849,854,1,0,0,0,850,851,5,91,0,0,851,852,5,23,0,0,852,854,5,25,0,0,853,
  	837,1,0,0,0,853,845,1,0,0,0,853,850,1,0,0,0,854,123,1,0,0,0,855,856,5,
  	23,0,0,856,861,3,120,60,0,857,858,5,4,0,0,858,860,3,120,60,0,859,857,
  	1,0,0,0,860,863,1,0,0,0,861,859,1,0,0,0,861,862,1,0,0,0,862,864,1,0,0,
  	0,863,861,1,0,0,0,864,865,5,25,0,0,865,125,1,0,0,0,866,867,5,91,0,0,867,
  	868,5,16,0,0,868,869,3,120,60,0,869,870,5,1,0,0,870,874,1,0,0,0,871,872,
  	5,91,0,0,872,874,5,1,0,0,873,866,1,0,0,0,873,871,1,0,0,0,874,127,1,0,
  	0,0,875,876,5,91,0,0,876,877,5,23,0,0,877,878,3,120,60,0,878,879,5,25,
  	0,0,879,880,5,23,0,0,880,881,3,120,60,0,881,882,5,25,0,0,882,883,5,16,
  	0,0,883,884,3,120,60,0,884,885,5,1,0,0,885,895,1,0,0,0,886,887,5,91,0,
  	0,887,888,5,23,0,0,888,889,3,120,60,0,889,890,5,25,0,0,890,891,5,16,0,
  	0,891,892,3,120,60,0,892,893,5,1,0,0,893,895,1,0,0,0,894,875,1,0,0,0,
  	894,886,1,0,0,0,895,129,1,0,0,0,896,897,7,9,0,0,897,131,1,0,0,0,70,134,
  	136,163,179,183,188,192,197,204,216,220,225,229,234,240,256,265,273,291,
  	299,303,320,328,331,349,366,374,376,395,406,422,430,432,436,444,454,475,
  	483,485,490,522,532,550,556,563,581,585,598,607,611,628,640,661,664,676,
  	687,701,703,709,727,732,739,767,821,832,834,853,861,873,894
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
    setState(136);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 1277912073650307078) != 0) || ((((_la - 66) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 66)) & 33554437) != 0)) {
      setState(134);
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
        case BaseParser::T__42:
        case BaseParser::T__50:
        case BaseParser::T__51:
        case BaseParser::T__52:
        case BaseParser::T__54:
        case BaseParser::T__55:
        case BaseParser::T__59:
        case BaseParser::GRAPH:
        case BaseParser::SET:
        case BaseParser::ID: {
          setState(132);
          statement();
          break;
        }

        case BaseParser::T__49: {
          setState(133);
          function();
          break;
        }

      default:
        throw NoViableAltException(this);
      }
      setState(138);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(139);
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

BaseParser::DrawgraphContext* BaseParser::StatementContext::drawgraph() {
  return getRuleContext<BaseParser::DrawgraphContext>(0);
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
    setState(163);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 2, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(141);
      graphDef();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(142);
      conditionalStatement();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(143);
      printStatement();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(144);
      whileStatement();
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(145);
      foreachStatement();
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(146);
      varDecl();
      break;
    }

    case 7: {
      enterOuterAlt(_localctx, 7);
      setState(147);
      setDecl();
      break;
    }

    case 8: {
      enterOuterAlt(_localctx, 8);
      setState(148);
      functionCall();
      setState(149);
      match(BaseParser::T__0);
      break;
    }

    case 9: {
      enterOuterAlt(_localctx, 9);
      setState(151);
      sleepStatement();
      break;
    }

    case 10: {
      enterOuterAlt(_localctx, 10);
      setState(152);
      graphComprehension();
      break;
    }

    case 11: {
      enterOuterAlt(_localctx, 11);
      setState(153);
      arrayAssignStatement();
      break;
    }

    case 12: {
      enterOuterAlt(_localctx, 12);
      setState(154);
      assignmentStatement();
      break;
    }

    case 13: {
      enterOuterAlt(_localctx, 13);
      setState(155);
      queryStatement();
      break;
    }

    case 14: {
      enterOuterAlt(_localctx, 14);
      setState(156);
      showgraph();
      break;
    }

    case 15: {
      enterOuterAlt(_localctx, 15);
      setState(157);
      drawgraph();
      break;
    }

    case 16: {
      enterOuterAlt(_localctx, 16);
      setState(158);
      nodeEdgeOperation();
      break;
    }

    case 17: {
      enterOuterAlt(_localctx, 17);
      setState(159);
      setOperation();
      break;
    }

    case 18: {
      enterOuterAlt(_localctx, 18);
      setState(160);
      setMethodCall();
      break;
    }

    case 19: {
      enterOuterAlt(_localctx, 19);
      setState(161);
      swapStatement();
      break;
    }

    case 20: {
      enterOuterAlt(_localctx, 20);
      setState(162);
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
    setState(165);
    match(BaseParser::T__1);
    setState(166);
    match(BaseParser::T__2);
    setState(167);
    match(BaseParser::ID);
    setState(168);
    match(BaseParser::T__3);
    setState(169);
    match(BaseParser::ID);
    setState(170);
    match(BaseParser::T__4);
    setState(171);
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
    setState(240);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 14, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<BaseParser::WeightedGraphDefContext>(_localctx);
      enterOuterAlt(_localctx, 1);
      setState(173);
      match(BaseParser::GRAPH);
      setState(174);
      graphID();
      setState(175);
      match(BaseParser::T__5);
      setState(179);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 3, _ctx);
      while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
        if (alt == 1) {
          setState(176);
          graphProperty(); 
        }
        setState(181);
        _errHandler->sync(this);
        alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 3, _ctx);
      }
      setState(183);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == BaseParser::T__11) {
        setState(182);
        nodes();
      }
      setState(188);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 5, _ctx);
      while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
        if (alt == 1) {
          setState(185);
          graphProperty(); 
        }
        setState(190);
        _errHandler->sync(this);
        alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 5, _ctx);
      }
      setState(192);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == BaseParser::T__12) {
        setState(191);
        edges();
      }
      setState(197);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == BaseParser::T__7) {
        setState(194);
        graphProperty();
        setState(199);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
      setState(200);
      match(BaseParser::TRUE);
      setState(204);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == BaseParser::T__7) {
        setState(201);
        graphProperty();
        setState(206);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
      setState(207);
      match(BaseParser::T__6);
      setState(208);
      match(BaseParser::T__0);
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<BaseParser::UnweightedGraphDefContext>(_localctx);
      enterOuterAlt(_localctx, 2);
      setState(210);
      match(BaseParser::GRAPH);
      setState(211);
      graphID();
      setState(212);
      match(BaseParser::T__5);
      setState(216);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 9, _ctx);
      while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
        if (alt == 1) {
          setState(213);
          graphProperty(); 
        }
        setState(218);
        _errHandler->sync(this);
        alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 9, _ctx);
      }
      setState(220);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == BaseParser::T__11) {
        setState(219);
        nodes();
      }
      setState(225);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 11, _ctx);
      while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
        if (alt == 1) {
          setState(222);
          graphProperty(); 
        }
        setState(227);
        _errHandler->sync(this);
        alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 11, _ctx);
      }
      setState(229);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == BaseParser::T__12) {
        setState(228);
        edges();
      }
      setState(234);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == BaseParser::T__7) {
        setState(231);
        graphProperty();
        setState(236);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
      setState(237);
      match(BaseParser::T__6);
      setState(238);
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
    setState(242);
    match(BaseParser::T__7);
    setState(243);
    match(BaseParser::T__8);
    setState(244);
    boolLiteral();
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
    setState(247);
    _la = _input->LA(1);
    if (!(_la == BaseParser::T__9

    || _la == BaseParser::T__10 || _la == BaseParser::TRUE

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
    setState(249);
    match(BaseParser::T__11);
    setState(250);
    nodeList();
    setState(251);
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
    setState(253);
    match(BaseParser::T__12);
    setState(256);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case BaseParser::INT: {
        setState(254);
        edgeList();
        break;
      }

      case BaseParser::T__13: {
        setState(255);
        fileEdgeList();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    setState(258);
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
    setState(260);
    nodeID();
    setState(265);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == BaseParser::T__3) {
      setState(261);
      match(BaseParser::T__3);
      setState(262);
      nodeID();
      setState(267);
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
    setState(268);
    edge();
    setState(273);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == BaseParser::T__3) {
      setState(269);
      match(BaseParser::T__3);
      setState(270);
      edge();
      setState(275);
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
    setState(276);
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
    setState(278);
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
    setState(280);
    match(BaseParser::T__13);
    setState(281);
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
    setState(283);
    nodeID();
    setState(284);
    match(BaseParser::T__14);
    setState(285);
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
    setState(303);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 20, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<BaseParser::SimpleDeclarationContext>(_localctx);
      enterOuterAlt(_localctx, 1);
      setState(287);
      type();
      setState(288);
      match(BaseParser::ID);
      setState(291);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == BaseParser::T__15) {
        setState(289);
        match(BaseParser::T__15);
        setState(290);
        expr(0);
      }
      setState(293);
      match(BaseParser::T__0);
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<BaseParser::ArrayDeclarationContext>(_localctx);
      enterOuterAlt(_localctx, 2);
      setState(295);
      type();
      setState(296);
      arrayDeclarator();
      setState(299);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == BaseParser::T__15) {
        setState(297);
        match(BaseParser::T__15);
        setState(298);
        arrayInitializer();
      }
      setState(301);
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
    setState(320);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 21, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(305);
      match(BaseParser::SET);
      setState(306);
      match(BaseParser::ID);
      setState(307);
      match(BaseParser::T__0);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(308);
      match(BaseParser::SET);
      setState(309);
      match(BaseParser::ID);
      setState(310);
      match(BaseParser::T__15);
      setState(311);
      setInitializer();
      setState(312);
      match(BaseParser::T__0);
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(314);
      match(BaseParser::SET);
      setState(315);
      match(BaseParser::ID);
      setState(316);
      match(BaseParser::T__15);
      setState(317);
      setExpr(0);
      setState(318);
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
    setState(322);
    match(BaseParser::T__5);
    setState(331);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 67108936) != 0) || ((((_la - 64) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 64)) & 939602051) != 0)) {
      setState(323);
      expr(0);
      setState(328);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == BaseParser::T__3) {
        setState(324);
        match(BaseParser::T__3);
        setState(325);
        expr(0);
        setState(330);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
    }
    setState(333);
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
    setState(335);
    match(BaseParser::ID);
    setState(336);
    match(BaseParser::T__15);
    setState(337);
    setExpr(0);
    setState(338);
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
    setState(349);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 24, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(340);
      match(BaseParser::ID);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(341);
      graphID();
      setState(342);
      match(BaseParser::T__16);
      setState(343);
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
      setState(345);
      graphID();
      setState(346);
      match(BaseParser::T__16);
      setState(347);
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
    setState(366);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 25, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<GraphNodesSetContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;

      setState(352);
      graphID();
      setState(353);
      match(BaseParser::T__16);
      setState(354);
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
      setState(356);
      graphID();
      setState(357);
      match(BaseParser::T__16);
      setState(358);
      match(BaseParser::EDGE);
      break;
    }

    case 3: {
      _localctx = _tracker.createInstance<SetIdContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(360);
      match(BaseParser::ID);
      break;
    }

    case 4: {
      _localctx = _tracker.createInstance<SetLiteralContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(361);
      setInitializer();
      break;
    }

    case 5: {
      _localctx = _tracker.createInstance<ParenSetContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(362);
      match(BaseParser::T__2);
      setState(363);
      setExpr(0);
      setState(364);
      match(BaseParser::T__4);
      break;
    }

    default:
      break;
    }
    _ctx->stop = _input->LT(-1);
    setState(376);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 27, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(374);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 26, _ctx)) {
        case 1: {
          auto newContext = _tracker.createInstance<SetUnionContext>(_tracker.createInstance<SetExprContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleSetExpr);
          setState(368);

          if (!(precpred(_ctx, 7))) throw FailedPredicateException(this, "precpred(_ctx, 7)");
          setState(369);
          match(BaseParser::UNION);
          setState(370);
          setExpr(8);
          break;
        }

        case 2: {
          auto newContext = _tracker.createInstance<SetIntersectContext>(_tracker.createInstance<SetExprContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleSetExpr);
          setState(371);

          if (!(precpred(_ctx, 6))) throw FailedPredicateException(this, "precpred(_ctx, 6)");
          setState(372);
          match(BaseParser::INTERSECT);
          setState(373);
          setExpr(7);
          break;
        }

        default:
          break;
        } 
      }
      setState(378);
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
    setState(395);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 28, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<BaseParser::SetAddMethodContext>(_localctx);
      enterOuterAlt(_localctx, 1);
      setState(379);
      setTarget();
      setState(380);
      match(BaseParser::T__16);
      setState(381);
      match(BaseParser::T__17);
      setState(382);
      match(BaseParser::T__2);
      setState(383);
      expr(0);
      setState(384);
      match(BaseParser::T__4);
      setState(385);
      match(BaseParser::T__0);
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<BaseParser::SetRemoveMethodContext>(_localctx);
      enterOuterAlt(_localctx, 2);
      setState(387);
      setTarget();
      setState(388);
      match(BaseParser::T__16);
      setState(389);
      match(BaseParser::T__18);
      setState(390);
      match(BaseParser::T__2);
      setState(391);
      expr(0);
      setState(392);
      match(BaseParser::T__4);
      setState(393);
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
    setState(397);
    match(BaseParser::T__19);
    setState(398);
    match(BaseParser::T__2);
    setState(399);
    condition(0);
    setState(400);
    match(BaseParser::T__4);
    setState(401);
    block();
    setState(406);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 29, _ctx)) {
    case 1: {
      setState(402);
      match(BaseParser::T__20);
      setState(403);
      conditionalStatement();
      break;
    }

    case 2: {
      setState(404);
      match(BaseParser::T__20);
      setState(405);
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
    setState(422);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 30, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<RelationalContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;

      setState(409);
      expr(0);
      setState(410);
      _la = _input->LA(1);
      if (!(((((_la - 85) & ~ 0x3fULL) == 0) &&
        ((1ULL << (_la - 85)) & 63) != 0))) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(411);
      expr(0);
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<NodeCheckContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(413);
      nodeID();
      setState(414);
      match(BaseParser::T__21);
      setState(415);
      graphID();
      break;
    }

    case 3: {
      _localctx = _tracker.createInstance<EdgeCheckContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(417);
      edge();
      setState(418);
      match(BaseParser::T__21);
      setState(419);
      graphID();
      break;
    }

    case 4: {
      _localctx = _tracker.createInstance<ExprConditionContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(421);
      expr(0);
      break;
    }

    default:
      break;
    }
    _ctx->stop = _input->LT(-1);
    setState(432);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 32, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(430);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 31, _ctx)) {
        case 1: {
          auto newContext = _tracker.createInstance<LogicalAndContext>(_tracker.createInstance<ConditionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleCondition);
          setState(424);

          if (!(precpred(_ctx, 6))) throw FailedPredicateException(this, "precpred(_ctx, 6)");
          setState(425);
          match(BaseParser::AND);
          setState(426);
          condition(7);
          break;
        }

        case 2: {
          auto newContext = _tracker.createInstance<LogicalOrContext>(_tracker.createInstance<ConditionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleCondition);
          setState(427);

          if (!(precpred(_ctx, 5))) throw FailedPredicateException(this, "precpred(_ctx, 5)");
          setState(428);
          match(BaseParser::OR);
          setState(429);
          condition(6);
          break;
        }

        default:
          break;
        } 
      }
      setState(434);
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
    setState(436);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == BaseParser::GRAPH) {
      setState(435);
      match(BaseParser::GRAPH);
    }
    setState(438);
    match(BaseParser::ID);
    setState(439);
    match(BaseParser::T__15);
    setState(440);
    match(BaseParser::T__22);
    setState(441);
    graphExpr();
    setState(444);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == BaseParser::T__23) {
      setState(442);
      match(BaseParser::T__23);
      setState(443);
      graphCondition(0);
    }
    setState(446);
    match(BaseParser::T__24);
    setState(447);
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
    setState(449);
    graphID();
    setState(454);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == BaseParser::AND

    || _la == BaseParser::OR) {
      setState(450);
      _la = _input->LA(1);
      if (!(_la == BaseParser::AND

      || _la == BaseParser::OR)) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(451);
      graphID();
      setState(456);
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
    setState(475);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case BaseParser::T__25: {
        _localctx = _tracker.createInstance<DegreeConditionContext>(_localctx);
        _ctx = _localctx;
        previousContext = _localctx;

        setState(458);
        match(BaseParser::T__25);
        setState(459);
        _la = _input->LA(1);
        if (!(((((_la - 85) & ~ 0x3fULL) == 0) &&
          ((1ULL << (_la - 85)) & 63) != 0))) {
        _errHandler->recoverInline(this);
        }
        else {
          _errHandler->reportMatch(this);
          consume();
        }
        setState(460);
        match(BaseParser::INT);
        break;
      }

      case BaseParser::T__26: {
        _localctx = _tracker.createInstance<ConnectedConditionContext>(_localctx);
        _ctx = _localctx;
        previousContext = _localctx;
        setState(461);
        match(BaseParser::T__26);
        setState(462);
        match(BaseParser::T__27);
        setState(463);
        nodeID();
        break;
      }

      case BaseParser::T__28: {
        _localctx = _tracker.createInstance<EdgeHasConditionContext>(_localctx);
        _ctx = _localctx;
        previousContext = _localctx;
        setState(464);
        match(BaseParser::T__28);
        setState(465);
        match(BaseParser::T__29);
        setState(466);
        expr(0);
        break;
      }

      case BaseParser::T__30: {
        _localctx = _tracker.createInstance<VertexInSetConditionContext>(_localctx);
        _ctx = _localctx;
        previousContext = _localctx;
        setState(467);
        match(BaseParser::T__30);
        setState(468);
        match(BaseParser::T__21);
        setState(469);
        match(BaseParser::ID);
        break;
      }

      case BaseParser::T__31: {
        _localctx = _tracker.createInstance<CycleConditionContext>(_localctx);
        _ctx = _localctx;
        previousContext = _localctx;
        setState(470);
        match(BaseParser::T__31);
        break;
      }

      case BaseParser::T__2: {
        _localctx = _tracker.createInstance<ParenGraphConditionContext>(_localctx);
        _ctx = _localctx;
        previousContext = _localctx;
        setState(471);
        match(BaseParser::T__2);
        setState(472);
        graphCondition(0);
        setState(473);
        match(BaseParser::T__4);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    _ctx->stop = _input->LT(-1);
    setState(485);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 38, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(483);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 37, _ctx)) {
        case 1: {
          auto newContext = _tracker.createInstance<GraphLogicalAndContext>(_tracker.createInstance<GraphConditionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleGraphCondition);
          setState(477);

          if (!(precpred(_ctx, 8))) throw FailedPredicateException(this, "precpred(_ctx, 8)");
          setState(478);
          match(BaseParser::AND);
          setState(479);
          graphCondition(9);
          break;
        }

        case 2: {
          auto newContext = _tracker.createInstance<GraphLogicalOrContext>(_tracker.createInstance<GraphConditionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleGraphCondition);
          setState(480);

          if (!(precpred(_ctx, 7))) throw FailedPredicateException(this, "precpred(_ctx, 7)");
          setState(481);
          match(BaseParser::OR);
          setState(482);
          graphCondition(8);
          break;
        }

        default:
          break;
        } 
      }
      setState(487);
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
    setState(490);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case BaseParser::T__32: {
        enterOuterAlt(_localctx, 1);
        setState(488);
        foreachStatement();
        break;
      }

      case BaseParser::T__37: {
        enterOuterAlt(_localctx, 2);
        setState(489);
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
    setState(492);
    match(BaseParser::T__32);
    setState(493);
    match(BaseParser::T__33);
    setState(494);
    loopTarget();
    setState(495);
    match(BaseParser::T__21);
    setState(496);
    graphID();
    setState(497);
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
    setState(522);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case BaseParser::T__30: {
        _localctx = _tracker.createInstance<BaseParser::ForEachVertexContext>(_localctx);
        enterOuterAlt(_localctx, 1);
        setState(499);
        match(BaseParser::T__30);
        setState(500);
        match(BaseParser::ID);
        break;
      }

      case BaseParser::T__28: {
        _localctx = _tracker.createInstance<BaseParser::ForEachEdgeContext>(_localctx);
        enterOuterAlt(_localctx, 2);
        setState(501);
        match(BaseParser::T__28);
        setState(502);
        match(BaseParser::ID);
        setState(503);
        match(BaseParser::T__3);
        setState(504);
        match(BaseParser::ID);
        break;
      }

      case BaseParser::T__34: {
        _localctx = _tracker.createInstance<BaseParser::ForEachOutAdjContext>(_localctx);
        enterOuterAlt(_localctx, 3);
        setState(505);
        match(BaseParser::T__34);
        setState(506);
        match(BaseParser::T__35);
        setState(507);
        match(BaseParser::ID);
        setState(508);
        match(BaseParser::OF);
        setState(509);
        expr(0);
        break;
      }

      case BaseParser::T__21: {
        _localctx = _tracker.createInstance<BaseParser::ForEachInAdjContext>(_localctx);
        enterOuterAlt(_localctx, 4);
        setState(510);
        match(BaseParser::T__21);
        setState(511);
        match(BaseParser::T__35);
        setState(512);
        match(BaseParser::ID);
        setState(513);
        match(BaseParser::OF);
        setState(514);
        expr(0);
        break;
      }

      case BaseParser::T__35: {
        _localctx = _tracker.createInstance<BaseParser::ForEachAdjContext>(_localctx);
        enterOuterAlt(_localctx, 5);
        setState(515);
        match(BaseParser::T__35);
        setState(516);
        match(BaseParser::ID);
        setState(517);
        match(BaseParser::OF);
        setState(518);
        expr(0);
        break;
      }

      case BaseParser::T__36: {
        _localctx = _tracker.createInstance<BaseParser::ForEachElementContext>(_localctx);
        enterOuterAlt(_localctx, 6);
        setState(519);
        match(BaseParser::T__36);
        setState(520);
        match(BaseParser::ID);
        break;
      }

      case BaseParser::ID: {
        _localctx = _tracker.createInstance<BaseParser::ForEachPlainContext>(_localctx);
        enterOuterAlt(_localctx, 7);
        setState(521);
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
    setState(524);
    match(BaseParser::T__37);
    setState(525);
    match(BaseParser::T__2);
    setState(526);
    condition(0);
    setState(527);
    match(BaseParser::T__4);
    setState(528);
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
    setState(532);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case BaseParser::T__17: {
        enterOuterAlt(_localctx, 1);
        setState(530);
        addOperation();
        break;
      }

      case BaseParser::T__18: {
        enterOuterAlt(_localctx, 2);
        setState(531);
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
    setState(534);
    match(BaseParser::T__17);
    setState(535);
    addTargets();
    setState(536);
    match(BaseParser::T__38);
    setState(537);
    graphID();
    setState(538);
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
    setState(540);
    match(BaseParser::T__18);
    setState(541);
    removeTargets();
    setState(542);
    match(BaseParser::T__39);
    setState(543);
    graphID();
    setState(544);
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
    setState(550);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 42, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(546);
      nodeID();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(547);
      edge();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(548);
      nodeList();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(549);
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
    setState(556);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 43, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(552);
      nodeID();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(553);
      edge();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(554);
      nodeList();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(555);
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
    setState(558);
    match(BaseParser::T__40);
    setState(559);
    match(BaseParser::ID);
    setState(560);
    match(BaseParser::T__8);
    setState(561);
    match(BaseParser::STRING);
    setState(563);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == BaseParser::INT) {
      setState(562);
      match(BaseParser::INT);
    }
    setState(565);
    match(BaseParser::OF);
    setState(566);
    graphID();
    setState(567);
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
    setState(569);
    match(BaseParser::T__41);
    setState(570);
    graphID();
    setState(571);
    match(BaseParser::T__0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- DrawgraphContext ------------------------------------------------------------------

BaseParser::DrawgraphContext::DrawgraphContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

BaseParser::GraphIDContext* BaseParser::DrawgraphContext::graphID() {
  return getRuleContext<BaseParser::GraphIDContext>(0);
}

tree::TerminalNode* BaseParser::DrawgraphContext::STRING() {
  return getToken(BaseParser::STRING, 0);
}

std::vector<BaseParser::DrawOptionContext *> BaseParser::DrawgraphContext::drawOption() {
  return getRuleContexts<BaseParser::DrawOptionContext>();
}

BaseParser::DrawOptionContext* BaseParser::DrawgraphContext::drawOption(size_t i) {
  return getRuleContext<BaseParser::DrawOptionContext>(i);
}


size_t BaseParser::DrawgraphContext::getRuleIndex() const {
  return BaseParser::RuleDrawgraph;
}

void BaseParser::DrawgraphContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterDrawgraph(this);
}

void BaseParser::DrawgraphContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitDrawgraph(this);
}


std::any BaseParser::DrawgraphContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitDrawgraph(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::DrawgraphContext* BaseParser::drawgraph() {
  DrawgraphContext *_localctx = _tracker.createInstance<DrawgraphContext>(_ctx, getState());
  enterRule(_localctx, 74, BaseParser::RuleDrawgraph);
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
    setState(573);
    match(BaseParser::T__42);
    setState(574);
    graphID();
    setState(575);
    match(BaseParser::T__38);
    setState(576);
    match(BaseParser::STRING);
    setState(585);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == BaseParser::T__5) {
      setState(577);
      match(BaseParser::T__5);
      setState(581);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (((((_la - 44) & ~ 0x3fULL) == 0) &&
        ((1ULL << (_la - 44)) & 2684354561) != 0)) {
        setState(578);
        drawOption();
        setState(583);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
      setState(584);
      match(BaseParser::T__6);
    }
    setState(587);
    match(BaseParser::T__0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- DrawOptionContext ------------------------------------------------------------------

BaseParser::DrawOptionContext::DrawOptionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* BaseParser::DrawOptionContext::STRING() {
  return getToken(BaseParser::STRING, 0);
}

tree::TerminalNode* BaseParser::DrawOptionContext::VERTICES() {
  return getToken(BaseParser::VERTICES, 0);
}

std::vector<BaseParser::VertexDrawOptionContext *> BaseParser::DrawOptionContext::vertexDrawOption() {
  return getRuleContexts<BaseParser::VertexDrawOptionContext>();
}

BaseParser::VertexDrawOptionContext* BaseParser::DrawOptionContext::vertexDrawOption(size_t i) {
  return getRuleContext<BaseParser::VertexDrawOptionContext>(i);
}

tree::TerminalNode* BaseParser::DrawOptionContext::EDGE() {
  return getToken(BaseParser::EDGE, 0);
}

std::vector<BaseParser::EdgeDrawOptionContext *> BaseParser::DrawOptionContext::edgeDrawOption() {
  return getRuleContexts<BaseParser::EdgeDrawOptionContext>();
}

BaseParser::EdgeDrawOptionContext* BaseParser::DrawOptionContext::edgeDrawOption(size_t i) {
  return getRuleContext<BaseParser::EdgeDrawOptionContext>(i);
}


size_t BaseParser::DrawOptionContext::getRuleIndex() const {
  return BaseParser::RuleDrawOption;
}

void BaseParser::DrawOptionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterDrawOption(this);
}

void BaseParser::DrawOptionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitDrawOption(this);
}


std::any BaseParser::DrawOptionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitDrawOption(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::DrawOptionContext* BaseParser::drawOption() {
  DrawOptionContext *_localctx = _tracker.createInstance<DrawOptionContext>(_ctx, getState());
  enterRule(_localctx, 76, BaseParser::RuleDrawOption);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(611);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case BaseParser::T__43: {
        enterOuterAlt(_localctx, 1);
        setState(589);
        match(BaseParser::T__43);
        setState(590);
        match(BaseParser::T__8);
        setState(591);
        match(BaseParser::STRING);
        setState(592);
        match(BaseParser::T__0);
        break;
      }

      case BaseParser::VERTICES: {
        enterOuterAlt(_localctx, 2);
        setState(593);
        match(BaseParser::VERTICES);
        setState(594);
        match(BaseParser::T__5);
        setState(598);
        _errHandler->sync(this);
        _la = _input->LA(1);
        while ((((_la & ~ 0x3fULL) == 0) &&
          ((1ULL << _la) & 246290604621824) != 0)) {
          setState(595);
          vertexDrawOption();
          setState(600);
          _errHandler->sync(this);
          _la = _input->LA(1);
        }
        setState(601);
        match(BaseParser::T__6);
        break;
      }

      case BaseParser::EDGE: {
        enterOuterAlt(_localctx, 3);
        setState(602);
        match(BaseParser::EDGE);
        setState(603);
        match(BaseParser::T__5);
        setState(607);
        _errHandler->sync(this);
        _la = _input->LA(1);
        while (_la == BaseParser::T__44) {
          setState(604);
          edgeDrawOption();
          setState(609);
          _errHandler->sync(this);
          _la = _input->LA(1);
        }
        setState(610);
        match(BaseParser::T__6);
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

//----------------- VertexDrawOptionContext ------------------------------------------------------------------

BaseParser::VertexDrawOptionContext::VertexDrawOptionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

BaseParser::BoolLiteralContext* BaseParser::VertexDrawOptionContext::boolLiteral() {
  return getRuleContext<BaseParser::BoolLiteralContext>(0);
}

BaseParser::ColorMappingContext* BaseParser::VertexDrawOptionContext::colorMapping() {
  return getRuleContext<BaseParser::ColorMappingContext>(0);
}

BaseParser::ContinuousMappingContext* BaseParser::VertexDrawOptionContext::continuousMapping() {
  return getRuleContext<BaseParser::ContinuousMappingContext>(0);
}


size_t BaseParser::VertexDrawOptionContext::getRuleIndex() const {
  return BaseParser::RuleVertexDrawOption;
}

void BaseParser::VertexDrawOptionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterVertexDrawOption(this);
}

void BaseParser::VertexDrawOptionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitVertexDrawOption(this);
}


std::any BaseParser::VertexDrawOptionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitVertexDrawOption(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::VertexDrawOptionContext* BaseParser::vertexDrawOption() {
  VertexDrawOptionContext *_localctx = _tracker.createInstance<VertexDrawOptionContext>(_ctx, getState());
  enterRule(_localctx, 78, BaseParser::RuleVertexDrawOption);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(628);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case BaseParser::T__44: {
        enterOuterAlt(_localctx, 1);
        setState(613);
        match(BaseParser::T__44);
        setState(614);
        match(BaseParser::T__8);
        setState(615);
        boolLiteral();
        setState(616);
        match(BaseParser::T__0);
        break;
      }

      case BaseParser::T__45: {
        enterOuterAlt(_localctx, 2);
        setState(618);
        match(BaseParser::T__45);
        setState(619);
        match(BaseParser::T__8);
        setState(620);
        colorMapping();
        setState(621);
        match(BaseParser::T__0);
        break;
      }

      case BaseParser::T__46: {
        enterOuterAlt(_localctx, 3);
        setState(623);
        match(BaseParser::T__46);
        setState(624);
        match(BaseParser::T__8);
        setState(625);
        continuousMapping();
        setState(626);
        match(BaseParser::T__0);
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

//----------------- EdgeDrawOptionContext ------------------------------------------------------------------

BaseParser::EdgeDrawOptionContext::EdgeDrawOptionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

BaseParser::BoolLiteralContext* BaseParser::EdgeDrawOptionContext::boolLiteral() {
  return getRuleContext<BaseParser::BoolLiteralContext>(0);
}


size_t BaseParser::EdgeDrawOptionContext::getRuleIndex() const {
  return BaseParser::RuleEdgeDrawOption;
}

void BaseParser::EdgeDrawOptionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterEdgeDrawOption(this);
}

void BaseParser::EdgeDrawOptionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitEdgeDrawOption(this);
}


std::any BaseParser::EdgeDrawOptionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitEdgeDrawOption(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::EdgeDrawOptionContext* BaseParser::edgeDrawOption() {
  EdgeDrawOptionContext *_localctx = _tracker.createInstance<EdgeDrawOptionContext>(_ctx, getState());
  enterRule(_localctx, 80, BaseParser::RuleEdgeDrawOption);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(630);
    match(BaseParser::T__44);
    setState(631);
    match(BaseParser::T__8);
    setState(632);
    boolLiteral();
    setState(633);
    match(BaseParser::T__0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ColorMappingContext ------------------------------------------------------------------

BaseParser::ColorMappingContext::ColorMappingContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* BaseParser::ColorMappingContext::ID() {
  return getToken(BaseParser::ID, 0);
}

BaseParser::ContinuousMappingContext* BaseParser::ColorMappingContext::continuousMapping() {
  return getRuleContext<BaseParser::ContinuousMappingContext>(0);
}


size_t BaseParser::ColorMappingContext::getRuleIndex() const {
  return BaseParser::RuleColorMapping;
}

void BaseParser::ColorMappingContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterColorMapping(this);
}

void BaseParser::ColorMappingContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitColorMapping(this);
}


std::any BaseParser::ColorMappingContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitColorMapping(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::ColorMappingContext* BaseParser::colorMapping() {
  ColorMappingContext *_localctx = _tracker.createInstance<ColorMappingContext>(_ctx, getState());
  enterRule(_localctx, 82, BaseParser::RuleColorMapping);

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
    switch (_input->LA(1)) {
      case BaseParser::T__47: {
        enterOuterAlt(_localctx, 1);
        setState(635);
        match(BaseParser::T__47);
        setState(636);
        match(BaseParser::T__2);
        setState(637);
        match(BaseParser::ID);
        setState(638);
        match(BaseParser::T__4);
        break;
      }

      case BaseParser::T__48: {
        enterOuterAlt(_localctx, 2);
        setState(639);
        continuousMapping();
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

//----------------- ContinuousMappingContext ------------------------------------------------------------------

BaseParser::ContinuousMappingContext::ContinuousMappingContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* BaseParser::ContinuousMappingContext::ID() {
  return getToken(BaseParser::ID, 0);
}


size_t BaseParser::ContinuousMappingContext::getRuleIndex() const {
  return BaseParser::RuleContinuousMapping;
}

void BaseParser::ContinuousMappingContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterContinuousMapping(this);
}

void BaseParser::ContinuousMappingContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitContinuousMapping(this);
}


std::any BaseParser::ContinuousMappingContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitContinuousMapping(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::ContinuousMappingContext* BaseParser::continuousMapping() {
  ContinuousMappingContext *_localctx = _tracker.createInstance<ContinuousMappingContext>(_ctx, getState());
  enterRule(_localctx, 84, BaseParser::RuleContinuousMapping);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(642);
    match(BaseParser::T__48);
    setState(643);
    match(BaseParser::T__2);
    setState(644);
    match(BaseParser::ID);
    setState(645);
    match(BaseParser::T__4);
   
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
  enterRule(_localctx, 86, BaseParser::RuleFunction);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(647);
    match(BaseParser::T__49);
    setState(648);
    returnType();
    setState(649);
    match(BaseParser::ID);
    setState(650);
    paramList();
    setState(651);
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
  enterRule(_localctx, 88, BaseParser::RuleReturnType);
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
    setState(653);
    _la = _input->LA(1);
    if (!(((((_la - 29) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 29)) & 687324790789) != 0))) {
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
  enterRule(_localctx, 90, BaseParser::RuleParamList);
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
    setState(655);
    match(BaseParser::T__2);
    setState(664);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (((((_la - 29) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 29)) & 687291236357) != 0)) {
      setState(656);
      param();
      setState(661);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == BaseParser::T__3) {
        setState(657);
        match(BaseParser::T__3);
        setState(658);
        param();
        setState(663);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
    }
    setState(666);
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
  enterRule(_localctx, 92, BaseParser::RuleParam);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(668);
    type();
    setState(669);
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
  enterRule(_localctx, 94, BaseParser::RuleType);
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
    setState(671);
    _la = _input->LA(1);
    if (!(((((_la - 29) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 29)) & 687291236357) != 0))) {
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
  enterRule(_localctx, 96, BaseParser::RuleFunctionCall);
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
    setState(673);
    functionName();
    setState(674);
    match(BaseParser::T__2);
    setState(676);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 67108936) != 0) || ((((_la - 64) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 64)) & 939602051) != 0)) {
      setState(675);
      argumentList();
    }
    setState(678);
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
  enterRule(_localctx, 98, BaseParser::RuleFunctionName);
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
    setState(680);
    _la = _input->LA(1);
    if (!(_la == BaseParser::T__25 || _la == BaseParser::ID)) {
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
  enterRule(_localctx, 100, BaseParser::RuleArgumentList);
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
    setState(682);
    expr(0);
    setState(687);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == BaseParser::T__3) {
      setState(683);
      match(BaseParser::T__3);
      setState(684);
      expr(0);
      setState(689);
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
  enterRule(_localctx, 102, BaseParser::RuleSleepStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(690);
    match(BaseParser::T__55);
    setState(691);
    match(BaseParser::T__2);
    setState(692);
    expr(0);
    setState(693);
    match(BaseParser::T__4);
    setState(694);
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
  enterRule(_localctx, 104, BaseParser::RuleBlock);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(709);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 58, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(696);
      match(BaseParser::T__5);
      setState(703);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while ((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 2285592490274455558) != 0) || ((((_la - 66) & ~ 0x3fULL) == 0) &&
        ((1ULL << (_la - 66)) & 33554437) != 0)) {
        setState(701);
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
          case BaseParser::T__42:
          case BaseParser::T__50:
          case BaseParser::T__51:
          case BaseParser::T__52:
          case BaseParser::T__54:
          case BaseParser::T__55:
          case BaseParser::T__59:
          case BaseParser::GRAPH:
          case BaseParser::SET:
          case BaseParser::ID: {
            setState(697);
            statement();
            break;
          }

          case BaseParser::T__56: {
            setState(698);
            returnStatement();
            break;
          }

          case BaseParser::T__57: {
            setState(699);
            breakStatement();
            break;
          }

          case BaseParser::T__58: {
            setState(700);
            continueStatement();
            break;
          }

        default:
          throw NoViableAltException(this);
        }
        setState(705);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
      setState(706);
      match(BaseParser::T__6);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(707);
      match(BaseParser::T__5);
      setState(708);
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
  enterRule(_localctx, 106, BaseParser::RuleReturnStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(711);
    match(BaseParser::T__56);
    setState(712);
    expr(0);
    setState(713);
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
  enterRule(_localctx, 108, BaseParser::RuleBreakStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(715);
    match(BaseParser::T__57);
    setState(716);
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
  enterRule(_localctx, 110, BaseParser::RuleContinueStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(718);
    match(BaseParser::T__58);
    setState(719);
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
  enterRule(_localctx, 112, BaseParser::RulePrintStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(727);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 59, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(721);
      match(BaseParser::T__59);
      setState(722);
      printExpr(0);
      setState(723);
      match(BaseParser::T__0);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(725);
      printArrayStatement();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(726);
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
  size_t startState = 114;
  enterRecursionRule(_localctx, 114, BaseParser::RulePrintExpr, precedence);

    

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
    setState(732);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case BaseParser::STRING: {
        setState(730);
        match(BaseParser::STRING);
        break;
      }

      case BaseParser::T__2:
      case BaseParser::T__5:
      case BaseParser::T__25:
      case BaseParser::T__63:
      case BaseParser::T__64:
      case BaseParser::NOT:
      case BaseParser::TRUE:
      case BaseParser::FALSE:
      case BaseParser::MINUS:
      case BaseParser::ID:
      case BaseParser::INT:
      case BaseParser::REAL: {
        setState(731);
        expr(0);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    _ctx->stop = _input->LT(-1);
    setState(739);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 61, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        _localctx = _tracker.createInstance<PrintExprContext>(parentContext, parentState);
        pushNewRecursionContext(_localctx, startState, RulePrintExpr);
        setState(734);

        if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
        setState(735);
        match(BaseParser::PLUS);
        setState(736);
        printExpr(2); 
      }
      setState(741);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 61, _ctx);
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
  enterRule(_localctx, 116, BaseParser::RulePrintArrayStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(742);
    match(BaseParser::T__59);
    setState(743);
    match(BaseParser::ID);
    setState(744);
    match(BaseParser::T__22);
    setState(745);
    expr(0);
    setState(746);
    match(BaseParser::T__24);
    setState(747);
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
  enterRule(_localctx, 118, BaseParser::RulePrintgraph);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(767);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 62, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<BaseParser::EdgePrintContext>(_localctx);
      enterOuterAlt(_localctx, 1);
      setState(749);
      match(BaseParser::T__59);
      setState(750);
      match(BaseParser::EDGE);
      setState(751);
      match(BaseParser::OF);
      setState(752);
      graphID();
      setState(753);
      match(BaseParser::T__0);
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<BaseParser::NodePrintContext>(_localctx);
      enterOuterAlt(_localctx, 2);
      setState(755);
      match(BaseParser::T__59);
      setState(756);
      match(BaseParser::NODE);
      setState(757);
      match(BaseParser::OF);
      setState(758);
      graphID();
      setState(759);
      match(BaseParser::T__0);
      break;
    }

    case 3: {
      _localctx = _tracker.createInstance<BaseParser::GraphPrintContext>(_localctx);
      enterOuterAlt(_localctx, 3);
      setState(761);
      match(BaseParser::T__59);
      setState(762);
      match(BaseParser::GRAPH);
      setState(763);
      match(BaseParser::OF);
      setState(764);
      graphID();
      setState(765);
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
  size_t startState = 120;
  enterRecursionRule(_localctx, 120, BaseParser::RuleExpr, precedence);

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
    setState(821);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 63, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<UnaryMinusExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;

      setState(770);
      match(BaseParser::MINUS);
      setState(771);
      expr(18);
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<NotExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(772);
      match(BaseParser::NOT);
      setState(773);
      expr(17);
      break;
    }

    case 3: {
      _localctx = _tracker.createInstance<FuncExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(774);
      functionCall();
      break;
    }

    case 4: {
      _localctx = _tracker.createInstance<IntExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(775);
      match(BaseParser::INT);
      break;
    }

    case 5: {
      _localctx = _tracker.createInstance<IdExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(776);
      match(BaseParser::ID);
      break;
    }

    case 6: {
      _localctx = _tracker.createInstance<ParenExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(777);
      match(BaseParser::T__2);
      setState(778);
      expr(0);
      setState(779);
      match(BaseParser::T__4);
      break;
    }

    case 7: {
      _localctx = _tracker.createInstance<Array2DAccessExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(781);
      match(BaseParser::ID);
      setState(782);
      match(BaseParser::T__22);
      setState(783);
      expr(0);
      setState(784);
      match(BaseParser::T__24);
      setState(785);
      match(BaseParser::T__22);
      setState(786);
      expr(0);
      setState(787);
      match(BaseParser::T__24);
      break;
    }

    case 8: {
      _localctx = _tracker.createInstance<ArrayAccessExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(789);
      match(BaseParser::ID);
      setState(790);
      match(BaseParser::T__22);
      setState(791);
      expr(0);
      setState(792);
      match(BaseParser::T__24);
      break;
    }

    case 9: {
      _localctx = _tracker.createInstance<SetContainsExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(794);
      setTarget();
      setState(795);
      match(BaseParser::T__16);
      setState(796);
      match(BaseParser::T__60);
      setState(797);
      match(BaseParser::T__2);
      setState(798);
      expr(0);
      setState(799);
      match(BaseParser::T__4);
      break;
    }

    case 10: {
      _localctx = _tracker.createInstance<SetSizeExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(801);
      match(BaseParser::ID);
      setState(802);
      match(BaseParser::T__16);
      setState(803);
      match(BaseParser::T__46);
      setState(804);
      match(BaseParser::T__2);
      setState(805);
      match(BaseParser::T__4);
      break;
    }

    case 11: {
      _localctx = _tracker.createInstance<SetPopExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(806);
      match(BaseParser::ID);
      setState(807);
      match(BaseParser::T__16);
      setState(808);
      match(BaseParser::T__61);
      setState(809);
      match(BaseParser::T__2);
      setState(810);
      match(BaseParser::T__4);
      break;
    }

    case 12: {
      _localctx = _tracker.createInstance<BoolTrueExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(811);
      match(BaseParser::TRUE);
      break;
    }

    case 13: {
      _localctx = _tracker.createInstance<BoolFalseExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(812);
      match(BaseParser::FALSE);
      break;
    }

    case 14: {
      _localctx = _tracker.createInstance<ArrayPrintContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(813);
      match(BaseParser::ID);
      setState(814);
      match(BaseParser::T__62);
      break;
    }

    case 15: {
      _localctx = _tracker.createInstance<RealExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(815);
      match(BaseParser::REAL);
      break;
    }

    case 16: {
      _localctx = _tracker.createInstance<SetLitExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(816);
      setInitializer();
      break;
    }

    case 17: {
      _localctx = _tracker.createInstance<InfExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(817);
      match(BaseParser::T__63);
      break;
    }

    case 18: {
      _localctx = _tracker.createInstance<TimerExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(818);
      match(BaseParser::T__64);
      setState(819);
      match(BaseParser::T__2);
      setState(820);
      match(BaseParser::T__4);
      break;
    }

    default:
      break;
    }
    _ctx->stop = _input->LT(-1);
    setState(834);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 65, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(832);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 64, _ctx)) {
        case 1: {
          auto newContext = _tracker.createInstance<LogicalExprContext>(_tracker.createInstance<ExprContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpr);
          setState(823);

          if (!(precpred(_ctx, 21))) throw FailedPredicateException(this, "precpred(_ctx, 21)");
          setState(824);
          _la = _input->LA(1);
          if (!(_la == BaseParser::AND

          || _la == BaseParser::OR)) {
          _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(825);
          expr(22);
          break;
        }

        case 2: {
          auto newContext = _tracker.createInstance<MulDivExprContext>(_tracker.createInstance<ExprContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpr);
          setState(826);

          if (!(precpred(_ctx, 20))) throw FailedPredicateException(this, "precpred(_ctx, 20)");
          setState(827);
          _la = _input->LA(1);
          if (!(((((_la - 72) & ~ 0x3fULL) == 0) &&
            ((1ULL << (_la - 72)) & 1537) != 0))) {
          _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(828);
          expr(21);
          break;
        }

        case 3: {
          auto newContext = _tracker.createInstance<AddSubExprContext>(_tracker.createInstance<ExprContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpr);
          setState(829);

          if (!(precpred(_ctx, 19))) throw FailedPredicateException(this, "precpred(_ctx, 19)");
          setState(830);
          _la = _input->LA(1);
          if (!(_la == BaseParser::PLUS

          || _la == BaseParser::MINUS)) {
          _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(831);
          expr(20);
          break;
        }

        default:
          break;
        } 
      }
      setState(836);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 65, _ctx);
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
  enterRule(_localctx, 122, BaseParser::RuleArrayDeclarator);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(853);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 66, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<BaseParser::Sized2DArrayContext>(_localctx);
      enterOuterAlt(_localctx, 1);
      setState(837);
      match(BaseParser::ID);
      setState(838);
      match(BaseParser::T__22);
      setState(839);
      expr(0);
      setState(840);
      match(BaseParser::T__24);
      setState(841);
      match(BaseParser::T__22);
      setState(842);
      expr(0);
      setState(843);
      match(BaseParser::T__24);
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<BaseParser::SizedArrayContext>(_localctx);
      enterOuterAlt(_localctx, 2);
      setState(845);
      match(BaseParser::ID);
      setState(846);
      match(BaseParser::T__22);
      setState(847);
      expr(0);
      setState(848);
      match(BaseParser::T__24);
      break;
    }

    case 3: {
      _localctx = _tracker.createInstance<BaseParser::UnsizedArrayContext>(_localctx);
      enterOuterAlt(_localctx, 3);
      setState(850);
      match(BaseParser::ID);
      setState(851);
      match(BaseParser::T__22);
      setState(852);
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
  enterRule(_localctx, 124, BaseParser::RuleArrayInitializer);
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
    setState(855);
    match(BaseParser::T__22);
    setState(856);
    expr(0);
    setState(861);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == BaseParser::T__3) {
      setState(857);
      match(BaseParser::T__3);
      setState(858);
      expr(0);
      setState(863);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(864);
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
  enterRule(_localctx, 126, BaseParser::RuleAssignmentStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(873);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 68, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(866);
      match(BaseParser::ID);
      setState(867);
      match(BaseParser::T__15);
      setState(868);
      expr(0);
      setState(869);
      match(BaseParser::T__0);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(871);
      match(BaseParser::ID);
      setState(872);
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
  enterRule(_localctx, 128, BaseParser::RuleArrayAssignStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(894);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 69, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<BaseParser::Array2DAssignStmtContext>(_localctx);
      enterOuterAlt(_localctx, 1);
      setState(875);
      match(BaseParser::ID);
      setState(876);
      match(BaseParser::T__22);
      setState(877);
      expr(0);
      setState(878);
      match(BaseParser::T__24);
      setState(879);
      match(BaseParser::T__22);
      setState(880);
      expr(0);
      setState(881);
      match(BaseParser::T__24);
      setState(882);
      match(BaseParser::T__15);
      setState(883);
      expr(0);
      setState(884);
      match(BaseParser::T__0);
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<BaseParser::ArrayAssignStmtContext>(_localctx);
      enterOuterAlt(_localctx, 2);
      setState(886);
      match(BaseParser::ID);
      setState(887);
      match(BaseParser::T__22);
      setState(888);
      expr(0);
      setState(889);
      match(BaseParser::T__24);
      setState(890);
      match(BaseParser::T__15);
      setState(891);
      expr(0);
      setState(892);
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
  enterRule(_localctx, 130, BaseParser::RuleWeights);
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
    setState(896);
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
    case 57: return printExprSempred(antlrcpp::downCast<PrintExprContext *>(context), predicateIndex);
    case 60: return exprSempred(antlrcpp::downCast<ExprContext *>(context), predicateIndex);

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
