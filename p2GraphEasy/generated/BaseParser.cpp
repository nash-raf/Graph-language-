
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
      "program", "statement", "graphDef", "nodes", "edges", "nodeList", 
      "edgeList", "graphID", "nodeID", "fileEdgeList", "edge", "varDecl", 
      "setDecl", "setInitializer", "setOperation", "setTarget", "setExpr", 
      "setMethodCall", "conditionalStatement", "condition", "graphComprehension", 
      "graphExpr", "graphCondition", "loopStatement", "foreachStatement", 
      "loopTarget", "whileStatement", "nodeEdgeOperation", "addOperation", 
      "removeOperation", "addTargets", "removeTargets", "queryStatement", 
      "showgraph", "function", "returnType", "paramList", "param", "type", 
      "functionCall", "argumentList", "sleepStatement", "block", "returnStatement", 
      "printStatement", "printExpr", "printArrayStatement", "printgraph", 
      "expr", "arrayDeclarator", "arrayInitializer", "assignmentStatement", 
      "arrayAssignStatement", "weights"
    },
    std::vector<std::string>{
      "", "';'", "'{'", "'}'", "'nodes:'", "'edges:'", "','", "'file'", 
      "'->'", "'='", "'.'", "'('", "')'", "'add'", "'remove'", "'if'", "'else'", 
      "'in'", "'['", "'where'", "']'", "'degree'", "'connected'", "'with'", 
      "'cycle'", "'for'", "'each'", "'vertex'", "'edge'", "'neighbor'", 
      "'element'", "'while'", "'to'", "'from'", "'query'", "':'", "'show'", 
      "'fn'", "'int'", "'real'", "'bool'", "'void'", "'string'", "'sleep'", 
      "'return'", "'print'", "'contains'", "'size'", "'[]'", "'timer'", 
      "'graph'", "'weights'", "'set'", "'union'", "'intersect'", "'!'", 
      "'%'", "'edges'", "'nodes'", "'TRUE'", "'FALSE'", "'of'", "'+'", "'-'", 
      "'*'", "'/'", "'&&'", "'||'", "'=='", "'!='", "'<'", "'>'", "'<='", 
      "'>='"
    },
    std::vector<std::string>{
      "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", 
      "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", 
      "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "GRAPH", 
      "WEIGHTS", "SET", "UNION", "INTERSECT", "NOT", "MODULO", "EDGE", "NODE", 
      "TRUE", "FALSE", "OF", "PLUS", "MINUS", "TIMES", "DIVIDE", "AND", 
      "OR", "EQUAL", "NOTEQUAL", "LESSTHAN", "GREATERTHAN", "LESSEQUAL", 
      "GREATEREQUAL", "ID", "INT", "REAL", "STRING", "Comment", "WS"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,1,79,676,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,2,
  	7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,7,
  	14,2,15,7,15,2,16,7,16,2,17,7,17,2,18,7,18,2,19,7,19,2,20,7,20,2,21,7,
  	21,2,22,7,22,2,23,7,23,2,24,7,24,2,25,7,25,2,26,7,26,2,27,7,27,2,28,7,
  	28,2,29,7,29,2,30,7,30,2,31,7,31,2,32,7,32,2,33,7,33,2,34,7,34,2,35,7,
  	35,2,36,7,36,2,37,7,37,2,38,7,38,2,39,7,39,2,40,7,40,2,41,7,41,2,42,7,
  	42,2,43,7,43,2,44,7,44,2,45,7,45,2,46,7,46,2,47,7,47,2,48,7,48,2,49,7,
  	49,2,50,7,50,2,51,7,51,2,52,7,52,2,53,7,53,1,0,1,0,5,0,111,8,0,10,0,12,
  	0,114,9,0,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  	1,1,1,1,1,1,1,1,1,1,1,1,1,1,3,1,138,8,1,1,2,1,2,1,2,1,2,3,2,144,8,2,1,
  	2,3,2,147,8,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,3,2,157,8,2,1,2,3,2,160,
  	8,2,1,2,1,2,1,2,3,2,165,8,2,1,3,1,3,1,3,1,3,1,4,1,4,1,4,3,4,174,8,4,1,
  	4,1,4,1,5,1,5,1,5,5,5,181,8,5,10,5,12,5,184,9,5,1,6,1,6,1,6,5,6,189,8,
  	6,10,6,12,6,192,9,6,1,7,1,7,1,8,1,8,1,9,1,9,1,9,1,10,1,10,1,10,1,10,1,
  	11,1,11,1,11,1,11,3,11,209,8,11,1,11,1,11,1,11,1,11,1,11,1,11,3,11,217,
  	8,11,1,11,1,11,3,11,221,8,11,1,12,1,12,1,12,1,12,1,12,1,12,1,12,1,12,
  	1,12,1,12,1,12,1,12,1,12,1,12,1,12,3,12,238,8,12,1,13,1,13,1,13,1,13,
  	5,13,244,8,13,10,13,12,13,247,9,13,3,13,249,8,13,1,13,1,13,1,14,1,14,
  	1,14,1,14,1,14,1,15,1,15,1,15,1,15,1,15,1,15,1,15,1,15,1,15,3,15,267,
  	8,15,1,16,1,16,1,16,1,16,1,16,1,16,1,16,1,16,1,16,1,16,1,16,1,16,1,16,
  	1,16,1,16,3,16,284,8,16,1,16,1,16,1,16,1,16,1,16,1,16,5,16,292,8,16,10,
  	16,12,16,295,9,16,1,17,1,17,1,17,1,17,1,17,1,17,1,17,1,17,1,17,1,17,1,
  	17,1,17,1,17,1,17,1,17,1,17,3,17,313,8,17,1,18,1,18,1,18,1,18,1,18,1,
  	18,1,18,1,18,1,18,3,18,324,8,18,1,19,1,19,1,19,1,19,1,19,1,19,1,19,1,
  	19,1,19,1,19,1,19,1,19,1,19,1,19,3,19,340,8,19,1,19,1,19,1,19,1,19,1,
  	19,1,19,5,19,348,8,19,10,19,12,19,351,9,19,1,20,1,20,1,20,1,20,1,20,1,
  	20,3,20,359,8,20,1,20,1,20,1,20,1,21,1,21,1,21,5,21,367,8,21,10,21,12,
  	21,370,9,21,1,22,1,22,1,22,1,22,1,22,1,22,1,22,1,22,1,22,1,22,1,22,1,
  	22,3,22,384,8,22,1,22,1,22,1,22,1,22,1,22,1,22,5,22,392,8,22,10,22,12,
  	22,395,9,22,1,23,1,23,3,23,399,8,23,1,24,1,24,1,24,1,24,1,24,1,24,1,24,
  	1,25,1,25,1,25,1,25,1,25,1,25,1,25,1,25,1,25,1,25,1,25,1,25,1,25,3,25,
  	421,8,25,1,26,1,26,1,26,1,26,1,26,1,26,1,27,1,27,3,27,431,8,27,1,28,1,
  	28,1,28,1,28,1,28,1,28,1,29,1,29,1,29,1,29,1,29,1,29,1,30,1,30,1,30,1,
  	30,3,30,449,8,30,1,31,1,31,1,31,1,31,3,31,455,8,31,1,32,1,32,1,32,1,32,
  	1,32,3,32,462,8,32,1,32,1,32,1,32,1,32,1,33,1,33,1,33,1,33,1,34,1,34,
  	1,34,1,34,1,34,1,34,1,35,1,35,1,36,1,36,1,36,1,36,5,36,484,8,36,10,36,
  	12,36,487,9,36,3,36,489,8,36,1,36,1,36,1,37,1,37,1,37,1,38,1,38,1,39,
  	1,39,1,39,3,39,501,8,39,1,39,1,39,1,40,1,40,1,40,5,40,508,8,40,10,40,
  	12,40,511,9,40,1,41,1,41,1,41,1,41,1,41,1,41,1,42,1,42,1,42,5,42,522,
  	8,42,10,42,12,42,525,9,42,1,42,1,42,1,42,3,42,530,8,42,1,43,1,43,1,43,
  	1,43,1,44,1,44,1,44,1,44,1,44,1,44,3,44,542,8,44,1,45,1,45,1,45,3,45,
  	547,8,45,1,45,1,45,1,45,5,45,552,8,45,10,45,12,45,555,9,45,1,46,1,46,
  	1,46,1,46,1,46,1,46,1,46,1,47,1,47,1,47,1,47,1,47,1,47,1,47,1,47,1,47,
  	1,47,1,47,1,47,1,47,1,47,1,47,1,47,1,47,1,47,3,47,582,8,47,1,48,1,48,
  	1,48,1,48,1,48,1,48,1,48,1,48,1,48,1,48,1,48,1,48,1,48,1,48,1,48,1,48,
  	1,48,1,48,1,48,1,48,1,48,1,48,1,48,1,48,1,48,1,48,1,48,1,48,1,48,1,48,
  	1,48,1,48,1,48,1,48,1,48,1,48,3,48,620,8,48,1,48,1,48,1,48,1,48,1,48,
  	1,48,1,48,1,48,1,48,5,48,631,8,48,10,48,12,48,634,9,48,1,49,1,49,1,49,
  	1,49,1,49,1,49,1,49,1,49,3,49,644,8,49,1,50,1,50,1,50,1,50,5,50,650,8,
  	50,10,50,12,50,653,9,50,1,50,1,50,1,51,1,51,1,51,1,51,1,51,1,51,1,51,
  	3,51,664,8,51,1,52,1,52,1,52,1,52,1,52,1,52,1,52,1,52,1,53,1,53,1,53,
  	0,5,32,38,44,90,96,54,0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,34,
  	36,38,40,42,44,46,48,50,52,54,56,58,60,62,64,66,68,70,72,74,76,78,80,
  	82,84,86,88,90,92,94,96,98,100,102,104,106,0,7,1,0,68,73,1,0,66,67,3,
  	0,27,28,38,42,50,50,4,0,27,28,38,40,42,42,50,50,2,0,56,56,64,65,1,0,62,
  	63,1,0,59,60,723,0,112,1,0,0,0,2,137,1,0,0,0,4,164,1,0,0,0,6,166,1,0,
  	0,0,8,170,1,0,0,0,10,177,1,0,0,0,12,185,1,0,0,0,14,193,1,0,0,0,16,195,
  	1,0,0,0,18,197,1,0,0,0,20,200,1,0,0,0,22,220,1,0,0,0,24,237,1,0,0,0,26,
  	239,1,0,0,0,28,252,1,0,0,0,30,266,1,0,0,0,32,283,1,0,0,0,34,312,1,0,0,
  	0,36,314,1,0,0,0,38,339,1,0,0,0,40,352,1,0,0,0,42,363,1,0,0,0,44,383,
  	1,0,0,0,46,398,1,0,0,0,48,400,1,0,0,0,50,420,1,0,0,0,52,422,1,0,0,0,54,
  	430,1,0,0,0,56,432,1,0,0,0,58,438,1,0,0,0,60,448,1,0,0,0,62,454,1,0,0,
  	0,64,456,1,0,0,0,66,467,1,0,0,0,68,471,1,0,0,0,70,477,1,0,0,0,72,479,
  	1,0,0,0,74,492,1,0,0,0,76,495,1,0,0,0,78,497,1,0,0,0,80,504,1,0,0,0,82,
  	512,1,0,0,0,84,529,1,0,0,0,86,531,1,0,0,0,88,541,1,0,0,0,90,546,1,0,0,
  	0,92,556,1,0,0,0,94,581,1,0,0,0,96,619,1,0,0,0,98,643,1,0,0,0,100,645,
  	1,0,0,0,102,663,1,0,0,0,104,665,1,0,0,0,106,673,1,0,0,0,108,111,3,2,1,
  	0,109,111,3,68,34,0,110,108,1,0,0,0,110,109,1,0,0,0,111,114,1,0,0,0,112,
  	110,1,0,0,0,112,113,1,0,0,0,113,115,1,0,0,0,114,112,1,0,0,0,115,116,5,
  	0,0,1,116,1,1,0,0,0,117,138,3,4,2,0,118,138,3,36,18,0,119,138,3,88,44,
  	0,120,138,3,52,26,0,121,138,3,48,24,0,122,138,3,22,11,0,123,138,3,24,
  	12,0,124,125,3,78,39,0,125,126,5,1,0,0,126,138,1,0,0,0,127,138,3,82,41,
  	0,128,138,3,40,20,0,129,138,3,104,52,0,130,138,3,102,51,0,131,138,3,64,
  	32,0,132,138,3,66,33,0,133,138,3,54,27,0,134,138,3,28,14,0,135,138,3,
  	34,17,0,136,138,5,1,0,0,137,117,1,0,0,0,137,118,1,0,0,0,137,119,1,0,0,
  	0,137,120,1,0,0,0,137,121,1,0,0,0,137,122,1,0,0,0,137,123,1,0,0,0,137,
  	124,1,0,0,0,137,127,1,0,0,0,137,128,1,0,0,0,137,129,1,0,0,0,137,130,1,
  	0,0,0,137,131,1,0,0,0,137,132,1,0,0,0,137,133,1,0,0,0,137,134,1,0,0,0,
  	137,135,1,0,0,0,137,136,1,0,0,0,138,3,1,0,0,0,139,140,5,50,0,0,140,141,
  	3,14,7,0,141,143,5,2,0,0,142,144,3,6,3,0,143,142,1,0,0,0,143,144,1,0,
  	0,0,144,146,1,0,0,0,145,147,3,8,4,0,146,145,1,0,0,0,146,147,1,0,0,0,147,
  	148,1,0,0,0,148,149,5,59,0,0,149,150,5,3,0,0,150,151,5,1,0,0,151,165,
  	1,0,0,0,152,153,5,50,0,0,153,154,3,14,7,0,154,156,5,2,0,0,155,157,3,6,
  	3,0,156,155,1,0,0,0,156,157,1,0,0,0,157,159,1,0,0,0,158,160,3,8,4,0,159,
  	158,1,0,0,0,159,160,1,0,0,0,160,161,1,0,0,0,161,162,5,3,0,0,162,163,5,
  	1,0,0,163,165,1,0,0,0,164,139,1,0,0,0,164,152,1,0,0,0,165,5,1,0,0,0,166,
  	167,5,4,0,0,167,168,3,10,5,0,168,169,5,1,0,0,169,7,1,0,0,0,170,173,5,
  	5,0,0,171,174,3,12,6,0,172,174,3,18,9,0,173,171,1,0,0,0,173,172,1,0,0,
  	0,174,175,1,0,0,0,175,176,5,1,0,0,176,9,1,0,0,0,177,182,3,16,8,0,178,
  	179,5,6,0,0,179,181,3,16,8,0,180,178,1,0,0,0,181,184,1,0,0,0,182,180,
  	1,0,0,0,182,183,1,0,0,0,183,11,1,0,0,0,184,182,1,0,0,0,185,190,3,20,10,
  	0,186,187,5,6,0,0,187,189,3,20,10,0,188,186,1,0,0,0,189,192,1,0,0,0,190,
  	188,1,0,0,0,190,191,1,0,0,0,191,13,1,0,0,0,192,190,1,0,0,0,193,194,5,
  	74,0,0,194,15,1,0,0,0,195,196,5,75,0,0,196,17,1,0,0,0,197,198,5,7,0,0,
  	198,199,5,77,0,0,199,19,1,0,0,0,200,201,3,16,8,0,201,202,5,8,0,0,202,
  	203,3,16,8,0,203,21,1,0,0,0,204,205,3,76,38,0,205,208,5,74,0,0,206,207,
  	5,9,0,0,207,209,3,96,48,0,208,206,1,0,0,0,208,209,1,0,0,0,209,210,1,0,
  	0,0,210,211,5,1,0,0,211,221,1,0,0,0,212,213,3,76,38,0,213,216,3,98,49,
  	0,214,215,5,9,0,0,215,217,3,100,50,0,216,214,1,0,0,0,216,217,1,0,0,0,
  	217,218,1,0,0,0,218,219,5,1,0,0,219,221,1,0,0,0,220,204,1,0,0,0,220,212,
  	1,0,0,0,221,23,1,0,0,0,222,223,5,52,0,0,223,224,5,74,0,0,224,238,5,1,
  	0,0,225,226,5,52,0,0,226,227,5,74,0,0,227,228,5,9,0,0,228,229,3,26,13,
  	0,229,230,5,1,0,0,230,238,1,0,0,0,231,232,5,52,0,0,232,233,5,74,0,0,233,
  	234,5,9,0,0,234,235,3,32,16,0,235,236,5,1,0,0,236,238,1,0,0,0,237,222,
  	1,0,0,0,237,225,1,0,0,0,237,231,1,0,0,0,238,25,1,0,0,0,239,248,5,2,0,
  	0,240,245,3,96,48,0,241,242,5,6,0,0,242,244,3,96,48,0,243,241,1,0,0,0,
  	244,247,1,0,0,0,245,243,1,0,0,0,245,246,1,0,0,0,246,249,1,0,0,0,247,245,
  	1,0,0,0,248,240,1,0,0,0,248,249,1,0,0,0,249,250,1,0,0,0,250,251,5,3,0,
  	0,251,27,1,0,0,0,252,253,5,74,0,0,253,254,5,9,0,0,254,255,3,32,16,0,255,
  	256,5,1,0,0,256,29,1,0,0,0,257,267,5,74,0,0,258,259,3,14,7,0,259,260,
  	5,10,0,0,260,261,5,58,0,0,261,267,1,0,0,0,262,263,3,14,7,0,263,264,5,
  	10,0,0,264,265,5,57,0,0,265,267,1,0,0,0,266,257,1,0,0,0,266,258,1,0,0,
  	0,266,262,1,0,0,0,267,31,1,0,0,0,268,269,6,16,-1,0,269,270,3,14,7,0,270,
  	271,5,10,0,0,271,272,5,58,0,0,272,284,1,0,0,0,273,274,3,14,7,0,274,275,
  	5,10,0,0,275,276,5,57,0,0,276,284,1,0,0,0,277,284,5,74,0,0,278,284,3,
  	26,13,0,279,280,5,11,0,0,280,281,3,32,16,0,281,282,5,12,0,0,282,284,1,
  	0,0,0,283,268,1,0,0,0,283,273,1,0,0,0,283,277,1,0,0,0,283,278,1,0,0,0,
  	283,279,1,0,0,0,284,293,1,0,0,0,285,286,10,7,0,0,286,287,5,53,0,0,287,
  	292,3,32,16,8,288,289,10,6,0,0,289,290,5,54,0,0,290,292,3,32,16,7,291,
  	285,1,0,0,0,291,288,1,0,0,0,292,295,1,0,0,0,293,291,1,0,0,0,293,294,1,
  	0,0,0,294,33,1,0,0,0,295,293,1,0,0,0,296,297,3,30,15,0,297,298,5,10,0,
  	0,298,299,5,13,0,0,299,300,5,11,0,0,300,301,3,96,48,0,301,302,5,12,0,
  	0,302,303,5,1,0,0,303,313,1,0,0,0,304,305,3,30,15,0,305,306,5,10,0,0,
  	306,307,5,14,0,0,307,308,5,11,0,0,308,309,3,96,48,0,309,310,5,12,0,0,
  	310,311,5,1,0,0,311,313,1,0,0,0,312,296,1,0,0,0,312,304,1,0,0,0,313,35,
  	1,0,0,0,314,315,5,15,0,0,315,316,5,11,0,0,316,317,3,38,19,0,317,318,5,
  	12,0,0,318,323,3,84,42,0,319,320,5,16,0,0,320,324,3,36,18,0,321,322,5,
  	16,0,0,322,324,3,84,42,0,323,319,1,0,0,0,323,321,1,0,0,0,323,324,1,0,
  	0,0,324,37,1,0,0,0,325,326,6,19,-1,0,326,327,3,96,48,0,327,328,7,0,0,
  	0,328,329,3,96,48,0,329,340,1,0,0,0,330,331,3,16,8,0,331,332,5,17,0,0,
  	332,333,3,14,7,0,333,340,1,0,0,0,334,335,3,20,10,0,335,336,5,17,0,0,336,
  	337,3,14,7,0,337,340,1,0,0,0,338,340,3,96,48,0,339,325,1,0,0,0,339,330,
  	1,0,0,0,339,334,1,0,0,0,339,338,1,0,0,0,340,349,1,0,0,0,341,342,10,6,
  	0,0,342,343,5,66,0,0,343,348,3,38,19,7,344,345,10,5,0,0,345,346,5,67,
  	0,0,346,348,3,38,19,6,347,341,1,0,0,0,347,344,1,0,0,0,348,351,1,0,0,0,
  	349,347,1,0,0,0,349,350,1,0,0,0,350,39,1,0,0,0,351,349,1,0,0,0,352,353,
  	5,74,0,0,353,354,5,9,0,0,354,355,5,18,0,0,355,358,3,42,21,0,356,357,5,
  	19,0,0,357,359,3,44,22,0,358,356,1,0,0,0,358,359,1,0,0,0,359,360,1,0,
  	0,0,360,361,5,20,0,0,361,362,5,1,0,0,362,41,1,0,0,0,363,368,3,14,7,0,
  	364,365,7,1,0,0,365,367,3,14,7,0,366,364,1,0,0,0,367,370,1,0,0,0,368,
  	366,1,0,0,0,368,369,1,0,0,0,369,43,1,0,0,0,370,368,1,0,0,0,371,372,6,
  	22,-1,0,372,373,5,21,0,0,373,374,7,0,0,0,374,384,5,75,0,0,375,376,5,22,
  	0,0,376,377,5,23,0,0,377,384,3,16,8,0,378,384,5,24,0,0,379,380,5,11,0,
  	0,380,381,3,44,22,0,381,382,5,12,0,0,382,384,1,0,0,0,383,371,1,0,0,0,
  	383,375,1,0,0,0,383,378,1,0,0,0,383,379,1,0,0,0,384,393,1,0,0,0,385,386,
  	10,6,0,0,386,387,5,66,0,0,387,392,3,44,22,7,388,389,10,5,0,0,389,390,
  	5,67,0,0,390,392,3,44,22,6,391,385,1,0,0,0,391,388,1,0,0,0,392,395,1,
  	0,0,0,393,391,1,0,0,0,393,394,1,0,0,0,394,45,1,0,0,0,395,393,1,0,0,0,
  	396,399,3,48,24,0,397,399,3,52,26,0,398,396,1,0,0,0,398,397,1,0,0,0,399,
  	47,1,0,0,0,400,401,5,25,0,0,401,402,5,26,0,0,402,403,3,50,25,0,403,404,
  	5,17,0,0,404,405,3,14,7,0,405,406,3,84,42,0,406,49,1,0,0,0,407,408,5,
  	27,0,0,408,421,5,74,0,0,409,410,5,28,0,0,410,411,5,74,0,0,411,412,5,6,
  	0,0,412,421,5,74,0,0,413,414,5,29,0,0,414,415,5,74,0,0,415,416,5,61,0,
  	0,416,421,3,96,48,0,417,418,5,30,0,0,418,421,5,74,0,0,419,421,5,74,0,
  	0,420,407,1,0,0,0,420,409,1,0,0,0,420,413,1,0,0,0,420,417,1,0,0,0,420,
  	419,1,0,0,0,421,51,1,0,0,0,422,423,5,31,0,0,423,424,5,11,0,0,424,425,
  	3,38,19,0,425,426,5,12,0,0,426,427,3,84,42,0,427,53,1,0,0,0,428,431,3,
  	56,28,0,429,431,3,58,29,0,430,428,1,0,0,0,430,429,1,0,0,0,431,55,1,0,
  	0,0,432,433,5,13,0,0,433,434,3,60,30,0,434,435,5,32,0,0,435,436,3,14,
  	7,0,436,437,5,1,0,0,437,57,1,0,0,0,438,439,5,14,0,0,439,440,3,62,31,0,
  	440,441,5,33,0,0,441,442,3,14,7,0,442,443,5,1,0,0,443,59,1,0,0,0,444,
  	449,3,16,8,0,445,449,3,20,10,0,446,449,3,10,5,0,447,449,3,12,6,0,448,
  	444,1,0,0,0,448,445,1,0,0,0,448,446,1,0,0,0,448,447,1,0,0,0,449,61,1,
  	0,0,0,450,455,3,16,8,0,451,455,3,20,10,0,452,455,3,10,5,0,453,455,3,12,
  	6,0,454,450,1,0,0,0,454,451,1,0,0,0,454,452,1,0,0,0,454,453,1,0,0,0,455,
  	63,1,0,0,0,456,457,5,34,0,0,457,458,5,74,0,0,458,459,5,35,0,0,459,461,
  	5,77,0,0,460,462,5,75,0,0,461,460,1,0,0,0,461,462,1,0,0,0,462,463,1,0,
  	0,0,463,464,5,61,0,0,464,465,3,14,7,0,465,466,5,1,0,0,466,65,1,0,0,0,
  	467,468,5,36,0,0,468,469,3,14,7,0,469,470,5,1,0,0,470,67,1,0,0,0,471,
  	472,5,37,0,0,472,473,3,70,35,0,473,474,5,74,0,0,474,475,3,72,36,0,475,
  	476,3,84,42,0,476,69,1,0,0,0,477,478,7,2,0,0,478,71,1,0,0,0,479,488,5,
  	11,0,0,480,485,3,74,37,0,481,482,5,6,0,0,482,484,3,74,37,0,483,481,1,
  	0,0,0,484,487,1,0,0,0,485,483,1,0,0,0,485,486,1,0,0,0,486,489,1,0,0,0,
  	487,485,1,0,0,0,488,480,1,0,0,0,488,489,1,0,0,0,489,490,1,0,0,0,490,491,
  	5,12,0,0,491,73,1,0,0,0,492,493,3,76,38,0,493,494,5,74,0,0,494,75,1,0,
  	0,0,495,496,7,3,0,0,496,77,1,0,0,0,497,498,5,74,0,0,498,500,5,11,0,0,
  	499,501,3,80,40,0,500,499,1,0,0,0,500,501,1,0,0,0,501,502,1,0,0,0,502,
  	503,5,12,0,0,503,79,1,0,0,0,504,509,3,96,48,0,505,506,5,6,0,0,506,508,
  	3,96,48,0,507,505,1,0,0,0,508,511,1,0,0,0,509,507,1,0,0,0,509,510,1,0,
  	0,0,510,81,1,0,0,0,511,509,1,0,0,0,512,513,5,43,0,0,513,514,5,11,0,0,
  	514,515,3,96,48,0,515,516,5,12,0,0,516,517,5,1,0,0,517,83,1,0,0,0,518,
  	523,5,2,0,0,519,522,3,2,1,0,520,522,3,86,43,0,521,519,1,0,0,0,521,520,
  	1,0,0,0,522,525,1,0,0,0,523,521,1,0,0,0,523,524,1,0,0,0,524,526,1,0,0,
  	0,525,523,1,0,0,0,526,530,5,3,0,0,527,528,5,2,0,0,528,530,5,3,0,0,529,
  	518,1,0,0,0,529,527,1,0,0,0,530,85,1,0,0,0,531,532,5,44,0,0,532,533,3,
  	96,48,0,533,534,5,1,0,0,534,87,1,0,0,0,535,536,5,45,0,0,536,537,3,90,
  	45,0,537,538,5,1,0,0,538,542,1,0,0,0,539,542,3,92,46,0,540,542,3,94,47,
  	0,541,535,1,0,0,0,541,539,1,0,0,0,541,540,1,0,0,0,542,89,1,0,0,0,543,
  	544,6,45,-1,0,544,547,5,77,0,0,545,547,3,96,48,0,546,543,1,0,0,0,546,
  	545,1,0,0,0,547,553,1,0,0,0,548,549,10,1,0,0,549,550,5,62,0,0,550,552,
  	3,90,45,2,551,548,1,0,0,0,552,555,1,0,0,0,553,551,1,0,0,0,553,554,1,0,
  	0,0,554,91,1,0,0,0,555,553,1,0,0,0,556,557,5,45,0,0,557,558,5,74,0,0,
  	558,559,5,18,0,0,559,560,3,96,48,0,560,561,5,20,0,0,561,562,5,1,0,0,562,
  	93,1,0,0,0,563,564,5,45,0,0,564,565,5,57,0,0,565,566,5,61,0,0,566,567,
  	3,14,7,0,567,568,5,1,0,0,568,582,1,0,0,0,569,570,5,45,0,0,570,571,5,58,
  	0,0,571,572,5,61,0,0,572,573,3,14,7,0,573,574,5,1,0,0,574,582,1,0,0,0,
  	575,576,5,45,0,0,576,577,5,50,0,0,577,578,5,61,0,0,578,579,3,14,7,0,579,
  	580,5,1,0,0,580,582,1,0,0,0,581,563,1,0,0,0,581,569,1,0,0,0,581,575,1,
  	0,0,0,582,95,1,0,0,0,583,584,6,48,-1,0,584,585,5,55,0,0,585,620,3,96,
  	48,14,586,620,3,78,39,0,587,620,5,75,0,0,588,620,5,74,0,0,589,590,5,11,
  	0,0,590,591,3,96,48,0,591,592,5,12,0,0,592,620,1,0,0,0,593,594,5,74,0,
  	0,594,595,5,18,0,0,595,596,3,96,48,0,596,597,5,20,0,0,597,620,1,0,0,0,
  	598,599,3,30,15,0,599,600,5,10,0,0,600,601,5,46,0,0,601,602,5,11,0,0,
  	602,603,3,96,48,0,603,604,5,12,0,0,604,620,1,0,0,0,605,606,5,74,0,0,606,
  	607,5,10,0,0,607,608,5,47,0,0,608,609,5,11,0,0,609,620,5,12,0,0,610,620,
  	5,59,0,0,611,620,5,60,0,0,612,613,5,74,0,0,613,620,5,48,0,0,614,620,5,
  	76,0,0,615,620,3,26,13,0,616,617,5,49,0,0,617,618,5,11,0,0,618,620,5,
  	12,0,0,619,583,1,0,0,0,619,586,1,0,0,0,619,587,1,0,0,0,619,588,1,0,0,
  	0,619,589,1,0,0,0,619,593,1,0,0,0,619,598,1,0,0,0,619,605,1,0,0,0,619,
  	610,1,0,0,0,619,611,1,0,0,0,619,612,1,0,0,0,619,614,1,0,0,0,619,615,1,
  	0,0,0,619,616,1,0,0,0,620,632,1,0,0,0,621,622,10,17,0,0,622,623,7,1,0,
  	0,623,631,3,96,48,18,624,625,10,16,0,0,625,626,7,4,0,0,626,631,3,96,48,
  	17,627,628,10,15,0,0,628,629,7,5,0,0,629,631,3,96,48,16,630,621,1,0,0,
  	0,630,624,1,0,0,0,630,627,1,0,0,0,631,634,1,0,0,0,632,630,1,0,0,0,632,
  	633,1,0,0,0,633,97,1,0,0,0,634,632,1,0,0,0,635,636,5,74,0,0,636,637,5,
  	18,0,0,637,638,3,96,48,0,638,639,5,20,0,0,639,644,1,0,0,0,640,641,5,74,
  	0,0,641,642,5,18,0,0,642,644,5,20,0,0,643,635,1,0,0,0,643,640,1,0,0,0,
  	644,99,1,0,0,0,645,646,5,18,0,0,646,651,3,96,48,0,647,648,5,6,0,0,648,
  	650,3,96,48,0,649,647,1,0,0,0,650,653,1,0,0,0,651,649,1,0,0,0,651,652,
  	1,0,0,0,652,654,1,0,0,0,653,651,1,0,0,0,654,655,5,20,0,0,655,101,1,0,
  	0,0,656,657,5,74,0,0,657,658,5,9,0,0,658,659,3,96,48,0,659,660,5,1,0,
  	0,660,664,1,0,0,0,661,662,5,74,0,0,662,664,5,1,0,0,663,656,1,0,0,0,663,
  	661,1,0,0,0,664,103,1,0,0,0,665,666,5,74,0,0,666,667,5,18,0,0,667,668,
  	3,96,48,0,668,669,5,20,0,0,669,670,5,9,0,0,670,671,3,96,48,0,671,672,
  	5,1,0,0,672,105,1,0,0,0,673,674,7,6,0,0,674,107,1,0,0,0,54,110,112,137,
  	143,146,156,159,164,173,182,190,208,216,220,237,245,248,266,283,291,293,
  	312,323,339,347,349,358,368,383,391,393,398,420,430,448,454,461,485,488,
  	500,509,521,523,529,541,546,553,581,619,630,632,643,651,663
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
    setState(112);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 5680028113231874) != 0) || _la == BaseParser::ID) {
      setState(110);
      _errHandler->sync(this);
      switch (_input->LA(1)) {
        case BaseParser::T__0:
        case BaseParser::T__12:
        case BaseParser::T__13:
        case BaseParser::T__14:
        case BaseParser::T__24:
        case BaseParser::T__26:
        case BaseParser::T__27:
        case BaseParser::T__30:
        case BaseParser::T__33:
        case BaseParser::T__35:
        case BaseParser::T__37:
        case BaseParser::T__38:
        case BaseParser::T__39:
        case BaseParser::T__41:
        case BaseParser::T__42:
        case BaseParser::T__44:
        case BaseParser::GRAPH:
        case BaseParser::SET:
        case BaseParser::ID: {
          setState(108);
          statement();
          break;
        }

        case BaseParser::T__36: {
          setState(109);
          function();
          break;
        }

      default:
        throw NoViableAltException(this);
      }
      setState(114);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(115);
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
    setState(137);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 2, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(117);
      graphDef();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(118);
      conditionalStatement();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(119);
      printStatement();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(120);
      whileStatement();
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(121);
      foreachStatement();
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(122);
      varDecl();
      break;
    }

    case 7: {
      enterOuterAlt(_localctx, 7);
      setState(123);
      setDecl();
      break;
    }

    case 8: {
      enterOuterAlt(_localctx, 8);
      setState(124);
      functionCall();
      setState(125);
      match(BaseParser::T__0);
      break;
    }

    case 9: {
      enterOuterAlt(_localctx, 9);
      setState(127);
      sleepStatement();
      break;
    }

    case 10: {
      enterOuterAlt(_localctx, 10);
      setState(128);
      graphComprehension();
      break;
    }

    case 11: {
      enterOuterAlt(_localctx, 11);
      setState(129);
      arrayAssignStatement();
      break;
    }

    case 12: {
      enterOuterAlt(_localctx, 12);
      setState(130);
      assignmentStatement();
      break;
    }

    case 13: {
      enterOuterAlt(_localctx, 13);
      setState(131);
      queryStatement();
      break;
    }

    case 14: {
      enterOuterAlt(_localctx, 14);
      setState(132);
      showgraph();
      break;
    }

    case 15: {
      enterOuterAlt(_localctx, 15);
      setState(133);
      nodeEdgeOperation();
      break;
    }

    case 16: {
      enterOuterAlt(_localctx, 16);
      setState(134);
      setOperation();
      break;
    }

    case 17: {
      enterOuterAlt(_localctx, 17);
      setState(135);
      setMethodCall();
      break;
    }

    case 18: {
      enterOuterAlt(_localctx, 18);
      setState(136);
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
  enterRule(_localctx, 4, BaseParser::RuleGraphDef);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(164);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 7, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<BaseParser::WeightedGraphDefContext>(_localctx);
      enterOuterAlt(_localctx, 1);
      setState(139);
      match(BaseParser::GRAPH);
      setState(140);
      graphID();
      setState(141);
      match(BaseParser::T__1);
      setState(143);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == BaseParser::T__3) {
        setState(142);
        nodes();
      }
      setState(146);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == BaseParser::T__4) {
        setState(145);
        edges();
      }
      setState(148);
      match(BaseParser::TRUE);
      setState(149);
      match(BaseParser::T__2);
      setState(150);
      match(BaseParser::T__0);
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<BaseParser::UnweightedGraphDefContext>(_localctx);
      enterOuterAlt(_localctx, 2);
      setState(152);
      match(BaseParser::GRAPH);
      setState(153);
      graphID();
      setState(154);
      match(BaseParser::T__1);
      setState(156);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == BaseParser::T__3) {
        setState(155);
        nodes();
      }
      setState(159);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == BaseParser::T__4) {
        setState(158);
        edges();
      }
      setState(161);
      match(BaseParser::T__2);
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
  enterRule(_localctx, 6, BaseParser::RuleNodes);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(166);
    match(BaseParser::T__3);
    setState(167);
    nodeList();
    setState(168);
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
  enterRule(_localctx, 8, BaseParser::RuleEdges);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(170);
    match(BaseParser::T__4);
    setState(173);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case BaseParser::INT: {
        setState(171);
        edgeList();
        break;
      }

      case BaseParser::T__6: {
        setState(172);
        fileEdgeList();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    setState(175);
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
  enterRule(_localctx, 10, BaseParser::RuleNodeList);
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
    setState(177);
    nodeID();
    setState(182);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == BaseParser::T__5) {
      setState(178);
      match(BaseParser::T__5);
      setState(179);
      nodeID();
      setState(184);
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
  enterRule(_localctx, 12, BaseParser::RuleEdgeList);
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
    setState(185);
    edge();
    setState(190);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == BaseParser::T__5) {
      setState(186);
      match(BaseParser::T__5);
      setState(187);
      edge();
      setState(192);
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
  enterRule(_localctx, 14, BaseParser::RuleGraphID);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(193);
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
  enterRule(_localctx, 16, BaseParser::RuleNodeID);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(195);
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
  enterRule(_localctx, 18, BaseParser::RuleFileEdgeList);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(197);
    match(BaseParser::T__6);
    setState(198);
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
  enterRule(_localctx, 20, BaseParser::RuleEdge);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(200);
    nodeID();
    setState(201);
    match(BaseParser::T__7);
    setState(202);
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
  enterRule(_localctx, 22, BaseParser::RuleVarDecl);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(220);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 13, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<BaseParser::SimpleDeclarationContext>(_localctx);
      enterOuterAlt(_localctx, 1);
      setState(204);
      type();
      setState(205);
      match(BaseParser::ID);
      setState(208);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == BaseParser::T__8) {
        setState(206);
        match(BaseParser::T__8);
        setState(207);
        expr(0);
      }
      setState(210);
      match(BaseParser::T__0);
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<BaseParser::ArrayDeclarationContext>(_localctx);
      enterOuterAlt(_localctx, 2);
      setState(212);
      type();
      setState(213);
      arrayDeclarator();
      setState(216);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == BaseParser::T__8) {
        setState(214);
        match(BaseParser::T__8);
        setState(215);
        arrayInitializer();
      }
      setState(218);
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
  enterRule(_localctx, 24, BaseParser::RuleSetDecl);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(237);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 14, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(222);
      match(BaseParser::SET);
      setState(223);
      match(BaseParser::ID);
      setState(224);
      match(BaseParser::T__0);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(225);
      match(BaseParser::SET);
      setState(226);
      match(BaseParser::ID);
      setState(227);
      match(BaseParser::T__8);
      setState(228);
      setInitializer();
      setState(229);
      match(BaseParser::T__0);
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(231);
      match(BaseParser::SET);
      setState(232);
      match(BaseParser::ID);
      setState(233);
      match(BaseParser::T__8);
      setState(234);
      setExpr(0);
      setState(235);
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
  enterRule(_localctx, 26, BaseParser::RuleSetInitializer);
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
    setState(239);
    match(BaseParser::T__1);
    setState(248);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 1765974003882657796) != 0) || ((((_la - 74) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 74)) & 7) != 0)) {
      setState(240);
      expr(0);
      setState(245);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == BaseParser::T__5) {
        setState(241);
        match(BaseParser::T__5);
        setState(242);
        expr(0);
        setState(247);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
    }
    setState(250);
    match(BaseParser::T__2);
   
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
  enterRule(_localctx, 28, BaseParser::RuleSetOperation);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(252);
    match(BaseParser::ID);
    setState(253);
    match(BaseParser::T__8);
    setState(254);
    setExpr(0);
    setState(255);
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
  enterRule(_localctx, 30, BaseParser::RuleSetTarget);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(266);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 17, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(257);
      match(BaseParser::ID);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(258);
      graphID();
      setState(259);
      match(BaseParser::T__9);
      setState(260);
      match(BaseParser::NODE);
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(262);
      graphID();
      setState(263);
      match(BaseParser::T__9);
      setState(264);
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
  size_t startState = 32;
  enterRecursionRule(_localctx, 32, BaseParser::RuleSetExpr, precedence);

    

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
    setState(283);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 18, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<GraphNodesSetContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;

      setState(269);
      graphID();
      setState(270);
      match(BaseParser::T__9);
      setState(271);
      match(BaseParser::NODE);
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<GraphEdgesSetContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(273);
      graphID();
      setState(274);
      match(BaseParser::T__9);
      setState(275);
      match(BaseParser::EDGE);
      break;
    }

    case 3: {
      _localctx = _tracker.createInstance<SetIdContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(277);
      match(BaseParser::ID);
      break;
    }

    case 4: {
      _localctx = _tracker.createInstance<SetLiteralContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(278);
      setInitializer();
      break;
    }

    case 5: {
      _localctx = _tracker.createInstance<ParenSetContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(279);
      match(BaseParser::T__10);
      setState(280);
      setExpr(0);
      setState(281);
      match(BaseParser::T__11);
      break;
    }

    default:
      break;
    }
    _ctx->stop = _input->LT(-1);
    setState(293);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 20, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(291);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 19, _ctx)) {
        case 1: {
          auto newContext = _tracker.createInstance<SetUnionContext>(_tracker.createInstance<SetExprContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleSetExpr);
          setState(285);

          if (!(precpred(_ctx, 7))) throw FailedPredicateException(this, "precpred(_ctx, 7)");
          setState(286);
          match(BaseParser::UNION);
          setState(287);
          setExpr(8);
          break;
        }

        case 2: {
          auto newContext = _tracker.createInstance<SetIntersectContext>(_tracker.createInstance<SetExprContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleSetExpr);
          setState(288);

          if (!(precpred(_ctx, 6))) throw FailedPredicateException(this, "precpred(_ctx, 6)");
          setState(289);
          match(BaseParser::INTERSECT);
          setState(290);
          setExpr(7);
          break;
        }

        default:
          break;
        } 
      }
      setState(295);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 20, _ctx);
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
  enterRule(_localctx, 34, BaseParser::RuleSetMethodCall);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(312);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 21, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<BaseParser::SetAddMethodContext>(_localctx);
      enterOuterAlt(_localctx, 1);
      setState(296);
      setTarget();
      setState(297);
      match(BaseParser::T__9);
      setState(298);
      match(BaseParser::T__12);
      setState(299);
      match(BaseParser::T__10);
      setState(300);
      expr(0);
      setState(301);
      match(BaseParser::T__11);
      setState(302);
      match(BaseParser::T__0);
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<BaseParser::SetRemoveMethodContext>(_localctx);
      enterOuterAlt(_localctx, 2);
      setState(304);
      setTarget();
      setState(305);
      match(BaseParser::T__9);
      setState(306);
      match(BaseParser::T__13);
      setState(307);
      match(BaseParser::T__10);
      setState(308);
      expr(0);
      setState(309);
      match(BaseParser::T__11);
      setState(310);
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
  enterRule(_localctx, 36, BaseParser::RuleConditionalStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(314);
    match(BaseParser::T__14);
    setState(315);
    match(BaseParser::T__10);
    setState(316);
    condition(0);
    setState(317);
    match(BaseParser::T__11);
    setState(318);
    block();
    setState(323);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 22, _ctx)) {
    case 1: {
      setState(319);
      match(BaseParser::T__15);
      setState(320);
      conditionalStatement();
      break;
    }

    case 2: {
      setState(321);
      match(BaseParser::T__15);
      setState(322);
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
  size_t startState = 38;
  enterRecursionRule(_localctx, 38, BaseParser::RuleCondition, precedence);

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
    setState(339);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 23, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<RelationalContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;

      setState(326);
      expr(0);
      setState(327);
      _la = _input->LA(1);
      if (!(((((_la - 68) & ~ 0x3fULL) == 0) &&
        ((1ULL << (_la - 68)) & 63) != 0))) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(328);
      expr(0);
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<NodeCheckContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(330);
      nodeID();
      setState(331);
      match(BaseParser::T__16);
      setState(332);
      graphID();
      break;
    }

    case 3: {
      _localctx = _tracker.createInstance<EdgeCheckContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(334);
      edge();
      setState(335);
      match(BaseParser::T__16);
      setState(336);
      graphID();
      break;
    }

    case 4: {
      _localctx = _tracker.createInstance<ExprConditionContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(338);
      expr(0);
      break;
    }

    default:
      break;
    }
    _ctx->stop = _input->LT(-1);
    setState(349);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 25, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(347);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 24, _ctx)) {
        case 1: {
          auto newContext = _tracker.createInstance<LogicalAndContext>(_tracker.createInstance<ConditionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleCondition);
          setState(341);

          if (!(precpred(_ctx, 6))) throw FailedPredicateException(this, "precpred(_ctx, 6)");
          setState(342);
          match(BaseParser::AND);
          setState(343);
          condition(7);
          break;
        }

        case 2: {
          auto newContext = _tracker.createInstance<LogicalOrContext>(_tracker.createInstance<ConditionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleCondition);
          setState(344);

          if (!(precpred(_ctx, 5))) throw FailedPredicateException(this, "precpred(_ctx, 5)");
          setState(345);
          match(BaseParser::OR);
          setState(346);
          condition(6);
          break;
        }

        default:
          break;
        } 
      }
      setState(351);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 25, _ctx);
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
  enterRule(_localctx, 40, BaseParser::RuleGraphComprehension);
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
    setState(352);
    match(BaseParser::ID);
    setState(353);
    match(BaseParser::T__8);
    setState(354);
    match(BaseParser::T__17);
    setState(355);
    graphExpr();
    setState(358);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == BaseParser::T__18) {
      setState(356);
      match(BaseParser::T__18);
      setState(357);
      graphCondition(0);
    }
    setState(360);
    match(BaseParser::T__19);
    setState(361);
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
  enterRule(_localctx, 42, BaseParser::RuleGraphExpr);
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
    setState(363);
    graphID();
    setState(368);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == BaseParser::AND

    || _la == BaseParser::OR) {
      setState(364);
      _la = _input->LA(1);
      if (!(_la == BaseParser::AND

      || _la == BaseParser::OR)) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(365);
      graphID();
      setState(370);
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

BaseParser::GraphConditionContext* BaseParser::graphCondition() {
   return graphCondition(0);
}

BaseParser::GraphConditionContext* BaseParser::graphCondition(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  BaseParser::GraphConditionContext *_localctx = _tracker.createInstance<GraphConditionContext>(_ctx, parentState);
  BaseParser::GraphConditionContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 44;
  enterRecursionRule(_localctx, 44, BaseParser::RuleGraphCondition, precedence);

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
    setState(383);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case BaseParser::T__20: {
        _localctx = _tracker.createInstance<DegreeConditionContext>(_localctx);
        _ctx = _localctx;
        previousContext = _localctx;

        setState(372);
        match(BaseParser::T__20);
        setState(373);
        _la = _input->LA(1);
        if (!(((((_la - 68) & ~ 0x3fULL) == 0) &&
          ((1ULL << (_la - 68)) & 63) != 0))) {
        _errHandler->recoverInline(this);
        }
        else {
          _errHandler->reportMatch(this);
          consume();
        }
        setState(374);
        match(BaseParser::INT);
        break;
      }

      case BaseParser::T__21: {
        _localctx = _tracker.createInstance<ConnectedConditionContext>(_localctx);
        _ctx = _localctx;
        previousContext = _localctx;
        setState(375);
        match(BaseParser::T__21);
        setState(376);
        match(BaseParser::T__22);
        setState(377);
        nodeID();
        break;
      }

      case BaseParser::T__23: {
        _localctx = _tracker.createInstance<CycleConditionContext>(_localctx);
        _ctx = _localctx;
        previousContext = _localctx;
        setState(378);
        match(BaseParser::T__23);
        break;
      }

      case BaseParser::T__10: {
        _localctx = _tracker.createInstance<ParenGraphConditionContext>(_localctx);
        _ctx = _localctx;
        previousContext = _localctx;
        setState(379);
        match(BaseParser::T__10);
        setState(380);
        graphCondition(0);
        setState(381);
        match(BaseParser::T__11);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    _ctx->stop = _input->LT(-1);
    setState(393);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 30, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(391);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 29, _ctx)) {
        case 1: {
          auto newContext = _tracker.createInstance<GraphLogicalAndContext>(_tracker.createInstance<GraphConditionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleGraphCondition);
          setState(385);

          if (!(precpred(_ctx, 6))) throw FailedPredicateException(this, "precpred(_ctx, 6)");
          setState(386);
          match(BaseParser::AND);
          setState(387);
          graphCondition(7);
          break;
        }

        case 2: {
          auto newContext = _tracker.createInstance<GraphLogicalOrContext>(_tracker.createInstance<GraphConditionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleGraphCondition);
          setState(388);

          if (!(precpred(_ctx, 5))) throw FailedPredicateException(this, "precpred(_ctx, 5)");
          setState(389);
          match(BaseParser::OR);
          setState(390);
          graphCondition(6);
          break;
        }

        default:
          break;
        } 
      }
      setState(395);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 30, _ctx);
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
  enterRule(_localctx, 46, BaseParser::RuleLoopStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(398);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case BaseParser::T__24: {
        enterOuterAlt(_localctx, 1);
        setState(396);
        foreachStatement();
        break;
      }

      case BaseParser::T__30: {
        enterOuterAlt(_localctx, 2);
        setState(397);
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
  enterRule(_localctx, 48, BaseParser::RuleForeachStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(400);
    match(BaseParser::T__24);
    setState(401);
    match(BaseParser::T__25);
    setState(402);
    loopTarget();
    setState(403);
    match(BaseParser::T__16);
    setState(404);
    graphID();
    setState(405);
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
  enterRule(_localctx, 50, BaseParser::RuleLoopTarget);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(420);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case BaseParser::T__26: {
        _localctx = _tracker.createInstance<BaseParser::ForEachVertexContext>(_localctx);
        enterOuterAlt(_localctx, 1);
        setState(407);
        match(BaseParser::T__26);
        setState(408);
        match(BaseParser::ID);
        break;
      }

      case BaseParser::T__27: {
        _localctx = _tracker.createInstance<BaseParser::ForEachEdgeContext>(_localctx);
        enterOuterAlt(_localctx, 2);
        setState(409);
        match(BaseParser::T__27);
        setState(410);
        match(BaseParser::ID);
        setState(411);
        match(BaseParser::T__5);
        setState(412);
        match(BaseParser::ID);
        break;
      }

      case BaseParser::T__28: {
        _localctx = _tracker.createInstance<BaseParser::ForEachAdjContext>(_localctx);
        enterOuterAlt(_localctx, 3);
        setState(413);
        match(BaseParser::T__28);
        setState(414);
        match(BaseParser::ID);
        setState(415);
        match(BaseParser::OF);
        setState(416);
        expr(0);
        break;
      }

      case BaseParser::T__29: {
        _localctx = _tracker.createInstance<BaseParser::ForEachElementContext>(_localctx);
        enterOuterAlt(_localctx, 4);
        setState(417);
        match(BaseParser::T__29);
        setState(418);
        match(BaseParser::ID);
        break;
      }

      case BaseParser::ID: {
        _localctx = _tracker.createInstance<BaseParser::ForEachPlainContext>(_localctx);
        enterOuterAlt(_localctx, 5);
        setState(419);
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
  enterRule(_localctx, 52, BaseParser::RuleWhileStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(422);
    match(BaseParser::T__30);
    setState(423);
    match(BaseParser::T__10);
    setState(424);
    condition(0);
    setState(425);
    match(BaseParser::T__11);
    setState(426);
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
  enterRule(_localctx, 54, BaseParser::RuleNodeEdgeOperation);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(430);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case BaseParser::T__12: {
        enterOuterAlt(_localctx, 1);
        setState(428);
        addOperation();
        break;
      }

      case BaseParser::T__13: {
        enterOuterAlt(_localctx, 2);
        setState(429);
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
  enterRule(_localctx, 56, BaseParser::RuleAddOperation);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(432);
    match(BaseParser::T__12);
    setState(433);
    addTargets();
    setState(434);
    match(BaseParser::T__31);
    setState(435);
    graphID();
    setState(436);
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
  enterRule(_localctx, 58, BaseParser::RuleRemoveOperation);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(438);
    match(BaseParser::T__13);
    setState(439);
    removeTargets();
    setState(440);
    match(BaseParser::T__32);
    setState(441);
    graphID();
    setState(442);
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
  enterRule(_localctx, 60, BaseParser::RuleAddTargets);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(448);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 34, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(444);
      nodeID();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(445);
      edge();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(446);
      nodeList();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(447);
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
  enterRule(_localctx, 62, BaseParser::RuleRemoveTargets);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(454);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 35, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(450);
      nodeID();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(451);
      edge();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(452);
      nodeList();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(453);
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
  enterRule(_localctx, 64, BaseParser::RuleQueryStatement);
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
    setState(456);
    match(BaseParser::T__33);
    setState(457);
    match(BaseParser::ID);
    setState(458);
    match(BaseParser::T__34);
    setState(459);
    match(BaseParser::STRING);
    setState(461);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == BaseParser::INT) {
      setState(460);
      match(BaseParser::INT);
    }
    setState(463);
    match(BaseParser::OF);
    setState(464);
    graphID();
    setState(465);
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
  enterRule(_localctx, 66, BaseParser::RuleShowgraph);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(467);
    match(BaseParser::T__35);
    setState(468);
    graphID();
    setState(469);
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
  enterRule(_localctx, 68, BaseParser::RuleFunction);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(471);
    match(BaseParser::T__36);
    setState(472);
    returnType();
    setState(473);
    match(BaseParser::ID);
    setState(474);
    paramList();
    setState(475);
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
  enterRule(_localctx, 70, BaseParser::RuleReturnType);
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
    setState(477);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 1134421524611072) != 0))) {
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
  enterRule(_localctx, 72, BaseParser::RuleParamList);
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
    setState(479);
    match(BaseParser::T__10);
    setState(488);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 1132222501355520) != 0)) {
      setState(480);
      param();
      setState(485);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == BaseParser::T__5) {
        setState(481);
        match(BaseParser::T__5);
        setState(482);
        param();
        setState(487);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
    }
    setState(490);
    match(BaseParser::T__11);
   
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
  enterRule(_localctx, 74, BaseParser::RuleParam);

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
    type();
    setState(493);
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
  enterRule(_localctx, 76, BaseParser::RuleType);
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
    setState(495);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 1132222501355520) != 0))) {
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

tree::TerminalNode* BaseParser::FunctionCallContext::ID() {
  return getToken(BaseParser::ID, 0);
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
  enterRule(_localctx, 78, BaseParser::RuleFunctionCall);
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
    setState(497);
    match(BaseParser::ID);
    setState(498);
    match(BaseParser::T__10);
    setState(500);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 1765974003882657796) != 0) || ((((_la - 74) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 74)) & 7) != 0)) {
      setState(499);
      argumentList();
    }
    setState(502);
    match(BaseParser::T__11);
   
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
  enterRule(_localctx, 80, BaseParser::RuleArgumentList);
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
    setState(504);
    expr(0);
    setState(509);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == BaseParser::T__5) {
      setState(505);
      match(BaseParser::T__5);
      setState(506);
      expr(0);
      setState(511);
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
  enterRule(_localctx, 82, BaseParser::RuleSleepStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(512);
    match(BaseParser::T__42);
    setState(513);
    match(BaseParser::T__10);
    setState(514);
    expr(0);
    setState(515);
    match(BaseParser::T__11);
    setState(516);
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
  enterRule(_localctx, 84, BaseParser::RuleBlock);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(529);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 43, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(518);
      match(BaseParser::T__1);
      setState(523);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while ((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 5697482860322818) != 0) || _la == BaseParser::ID) {
        setState(521);
        _errHandler->sync(this);
        switch (_input->LA(1)) {
          case BaseParser::T__0:
          case BaseParser::T__12:
          case BaseParser::T__13:
          case BaseParser::T__14:
          case BaseParser::T__24:
          case BaseParser::T__26:
          case BaseParser::T__27:
          case BaseParser::T__30:
          case BaseParser::T__33:
          case BaseParser::T__35:
          case BaseParser::T__37:
          case BaseParser::T__38:
          case BaseParser::T__39:
          case BaseParser::T__41:
          case BaseParser::T__42:
          case BaseParser::T__44:
          case BaseParser::GRAPH:
          case BaseParser::SET:
          case BaseParser::ID: {
            setState(519);
            statement();
            break;
          }

          case BaseParser::T__43: {
            setState(520);
            returnStatement();
            break;
          }

        default:
          throw NoViableAltException(this);
        }
        setState(525);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
      setState(526);
      match(BaseParser::T__2);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(527);
      match(BaseParser::T__1);
      setState(528);
      match(BaseParser::T__2);
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
  enterRule(_localctx, 86, BaseParser::RuleReturnStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(531);
    match(BaseParser::T__43);
    setState(532);
    expr(0);
    setState(533);
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
  enterRule(_localctx, 88, BaseParser::RulePrintStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(541);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 44, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(535);
      match(BaseParser::T__44);
      setState(536);
      printExpr(0);
      setState(537);
      match(BaseParser::T__0);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(539);
      printArrayStatement();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(540);
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
  size_t startState = 90;
  enterRecursionRule(_localctx, 90, BaseParser::RulePrintExpr, precedence);

    

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
    setState(546);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case BaseParser::STRING: {
        setState(544);
        match(BaseParser::STRING);
        break;
      }

      case BaseParser::T__1:
      case BaseParser::T__10:
      case BaseParser::T__48:
      case BaseParser::NOT:
      case BaseParser::TRUE:
      case BaseParser::FALSE:
      case BaseParser::ID:
      case BaseParser::INT:
      case BaseParser::REAL: {
        setState(545);
        expr(0);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    _ctx->stop = _input->LT(-1);
    setState(553);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 46, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        _localctx = _tracker.createInstance<PrintExprContext>(parentContext, parentState);
        pushNewRecursionContext(_localctx, startState, RulePrintExpr);
        setState(548);

        if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
        setState(549);
        match(BaseParser::PLUS);
        setState(550);
        printExpr(2); 
      }
      setState(555);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 46, _ctx);
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
  enterRule(_localctx, 92, BaseParser::RulePrintArrayStatement);

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
    match(BaseParser::T__44);
    setState(557);
    match(BaseParser::ID);
    setState(558);
    match(BaseParser::T__17);
    setState(559);
    expr(0);
    setState(560);
    match(BaseParser::T__19);
    setState(561);
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
  enterRule(_localctx, 94, BaseParser::RulePrintgraph);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(581);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 47, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<BaseParser::EdgePrintContext>(_localctx);
      enterOuterAlt(_localctx, 1);
      setState(563);
      match(BaseParser::T__44);
      setState(564);
      match(BaseParser::EDGE);
      setState(565);
      match(BaseParser::OF);
      setState(566);
      graphID();
      setState(567);
      match(BaseParser::T__0);
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<BaseParser::NodePrintContext>(_localctx);
      enterOuterAlt(_localctx, 2);
      setState(569);
      match(BaseParser::T__44);
      setState(570);
      match(BaseParser::NODE);
      setState(571);
      match(BaseParser::OF);
      setState(572);
      graphID();
      setState(573);
      match(BaseParser::T__0);
      break;
    }

    case 3: {
      _localctx = _tracker.createInstance<BaseParser::GraphPrintContext>(_localctx);
      enterOuterAlt(_localctx, 3);
      setState(575);
      match(BaseParser::T__44);
      setState(576);
      match(BaseParser::GRAPH);
      setState(577);
      match(BaseParser::OF);
      setState(578);
      graphID();
      setState(579);
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

BaseParser::ExprContext* BaseParser::expr() {
   return expr(0);
}

BaseParser::ExprContext* BaseParser::expr(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  BaseParser::ExprContext *_localctx = _tracker.createInstance<ExprContext>(_ctx, parentState);
  BaseParser::ExprContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 96;
  enterRecursionRule(_localctx, 96, BaseParser::RuleExpr, precedence);

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
    setState(619);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 48, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<NotExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;

      setState(584);
      match(BaseParser::NOT);
      setState(585);
      expr(14);
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<FuncExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(586);
      functionCall();
      break;
    }

    case 3: {
      _localctx = _tracker.createInstance<IntExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(587);
      match(BaseParser::INT);
      break;
    }

    case 4: {
      _localctx = _tracker.createInstance<IdExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(588);
      match(BaseParser::ID);
      break;
    }

    case 5: {
      _localctx = _tracker.createInstance<ParenExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(589);
      match(BaseParser::T__10);
      setState(590);
      expr(0);
      setState(591);
      match(BaseParser::T__11);
      break;
    }

    case 6: {
      _localctx = _tracker.createInstance<ArrayAccessExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(593);
      match(BaseParser::ID);
      setState(594);
      match(BaseParser::T__17);
      setState(595);
      expr(0);
      setState(596);
      match(BaseParser::T__19);
      break;
    }

    case 7: {
      _localctx = _tracker.createInstance<SetContainsExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(598);
      setTarget();
      setState(599);
      match(BaseParser::T__9);
      setState(600);
      match(BaseParser::T__45);
      setState(601);
      match(BaseParser::T__10);
      setState(602);
      expr(0);
      setState(603);
      match(BaseParser::T__11);
      break;
    }

    case 8: {
      _localctx = _tracker.createInstance<SetSizeExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(605);
      match(BaseParser::ID);
      setState(606);
      match(BaseParser::T__9);
      setState(607);
      match(BaseParser::T__46);
      setState(608);
      match(BaseParser::T__10);
      setState(609);
      match(BaseParser::T__11);
      break;
    }

    case 9: {
      _localctx = _tracker.createInstance<BoolTrueExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(610);
      match(BaseParser::TRUE);
      break;
    }

    case 10: {
      _localctx = _tracker.createInstance<BoolFalseExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(611);
      match(BaseParser::FALSE);
      break;
    }

    case 11: {
      _localctx = _tracker.createInstance<ArrayPrintContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(612);
      match(BaseParser::ID);
      setState(613);
      match(BaseParser::T__47);
      break;
    }

    case 12: {
      _localctx = _tracker.createInstance<RealExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(614);
      match(BaseParser::REAL);
      break;
    }

    case 13: {
      _localctx = _tracker.createInstance<SetLitExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(615);
      setInitializer();
      break;
    }

    case 14: {
      _localctx = _tracker.createInstance<TimerExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(616);
      match(BaseParser::T__48);
      setState(617);
      match(BaseParser::T__10);
      setState(618);
      match(BaseParser::T__11);
      break;
    }

    default:
      break;
    }
    _ctx->stop = _input->LT(-1);
    setState(632);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 50, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(630);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 49, _ctx)) {
        case 1: {
          auto newContext = _tracker.createInstance<LogicalExprContext>(_tracker.createInstance<ExprContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpr);
          setState(621);

          if (!(precpred(_ctx, 17))) throw FailedPredicateException(this, "precpred(_ctx, 17)");
          setState(622);
          _la = _input->LA(1);
          if (!(_la == BaseParser::AND

          || _la == BaseParser::OR)) {
          _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(623);
          expr(18);
          break;
        }

        case 2: {
          auto newContext = _tracker.createInstance<MulDivExprContext>(_tracker.createInstance<ExprContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpr);
          setState(624);

          if (!(precpred(_ctx, 16))) throw FailedPredicateException(this, "precpred(_ctx, 16)");
          setState(625);
          _la = _input->LA(1);
          if (!(((((_la - 56) & ~ 0x3fULL) == 0) &&
            ((1ULL << (_la - 56)) & 769) != 0))) {
          _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(626);
          expr(17);
          break;
        }

        case 3: {
          auto newContext = _tracker.createInstance<AddSubExprContext>(_tracker.createInstance<ExprContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpr);
          setState(627);

          if (!(precpred(_ctx, 15))) throw FailedPredicateException(this, "precpred(_ctx, 15)");
          setState(628);
          _la = _input->LA(1);
          if (!(_la == BaseParser::PLUS

          || _la == BaseParser::MINUS)) {
          _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(629);
          expr(16);
          break;
        }

        default:
          break;
        } 
      }
      setState(634);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 50, _ctx);
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
  enterRule(_localctx, 98, BaseParser::RuleArrayDeclarator);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(643);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 51, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<BaseParser::SizedArrayContext>(_localctx);
      enterOuterAlt(_localctx, 1);
      setState(635);
      match(BaseParser::ID);
      setState(636);
      match(BaseParser::T__17);
      setState(637);
      expr(0);
      setState(638);
      match(BaseParser::T__19);
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<BaseParser::UnsizedArrayContext>(_localctx);
      enterOuterAlt(_localctx, 2);
      setState(640);
      match(BaseParser::ID);
      setState(641);
      match(BaseParser::T__17);
      setState(642);
      match(BaseParser::T__19);
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
  enterRule(_localctx, 100, BaseParser::RuleArrayInitializer);
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
    setState(645);
    match(BaseParser::T__17);
    setState(646);
    expr(0);
    setState(651);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == BaseParser::T__5) {
      setState(647);
      match(BaseParser::T__5);
      setState(648);
      expr(0);
      setState(653);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(654);
    match(BaseParser::T__19);
   
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
  enterRule(_localctx, 102, BaseParser::RuleAssignmentStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(663);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 53, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(656);
      match(BaseParser::ID);
      setState(657);
      match(BaseParser::T__8);
      setState(658);
      expr(0);
      setState(659);
      match(BaseParser::T__0);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(661);
      match(BaseParser::ID);
      setState(662);
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
  enterRule(_localctx, 104, BaseParser::RuleArrayAssignStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    _localctx = _tracker.createInstance<BaseParser::ArrayAssignStmtContext>(_localctx);
    enterOuterAlt(_localctx, 1);
    setState(665);
    match(BaseParser::ID);
    setState(666);
    match(BaseParser::T__17);
    setState(667);
    expr(0);
    setState(668);
    match(BaseParser::T__19);
    setState(669);
    match(BaseParser::T__8);
    setState(670);
    expr(0);
    setState(671);
    match(BaseParser::T__0);
   
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
  enterRule(_localctx, 106, BaseParser::RuleWeights);
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
    case 16: return setExprSempred(antlrcpp::downCast<SetExprContext *>(context), predicateIndex);
    case 19: return conditionSempred(antlrcpp::downCast<ConditionContext *>(context), predicateIndex);
    case 22: return graphConditionSempred(antlrcpp::downCast<GraphConditionContext *>(context), predicateIndex);
    case 45: return printExprSempred(antlrcpp::downCast<PrintExprContext *>(context), predicateIndex);
    case 48: return exprSempred(antlrcpp::downCast<ExprContext *>(context), predicateIndex);

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
    case 4: return precpred(_ctx, 6);
    case 5: return precpred(_ctx, 5);

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
    case 7: return precpred(_ctx, 17);
    case 8: return precpred(_ctx, 16);
    case 9: return precpred(_ctx, 15);

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
