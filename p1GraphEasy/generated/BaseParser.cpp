
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
      "program", "statement", "swapStatement", "graphDef", "nodes", "edges", 
      "nodeList", "edgeList", "graphID", "nodeID", "fileEdgeList", "edge", 
      "varDecl", "setDecl", "setInitializer", "setOperation", "setTarget", 
      "setExpr", "setMethodCall", "conditionalStatement", "condition", "graphComprehension", 
      "graphExpr", "graphCondition", "loopStatement", "foreachStatement", 
      "loopTarget", "whileStatement", "nodeEdgeOperation", "addOperation", 
      "removeOperation", "addTargets", "removeTargets", "queryStatement", 
      "showgraph", "function", "returnType", "paramList", "param", "type", 
      "functionCall", "argumentList", "sleepStatement", "block", "returnStatement", 
      "breakStatement", "continueStatement", "printStatement", "printExpr", 
      "printArrayStatement", "printgraph", "expr", "arrayDeclarator", "arrayInitializer", 
      "assignmentStatement", "arrayAssignStatement", "weights"
    },
    std::vector<std::string>{
      "", "';'", "'swap'", "'('", "','", "')'", "'{'", "'}'", "'nodes:'", 
      "'edges:'", "'file'", "'->'", "'='", "'.'", "'add'", "'remove'", "'if'", 
      "'else'", "'in'", "'['", "'where'", "']'", "'degree'", "'connected'", 
      "'with'", "'cycle'", "'for'", "'each'", "'vertex'", "'edge'", "'neighbor'", 
      "'element'", "'while'", "'to'", "'from'", "'query'", "':'", "'show'", 
      "'fn'", "'int'", "'real'", "'bool'", "'void'", "'string'", "'sleep'", 
      "'return'", "'break'", "'continue'", "'print'", "'contains'", "'size'", 
      "'pop'", "'[]'", "'INF'", "'timer'", "'graph'", "'weights'", "'set'", 
      "'union'", "'intersect'", "'!'", "'%'", "'edges'", "'nodes'", "'TRUE'", 
      "'FALSE'", "'of'", "'+'", "'-'", "'*'", "'/'", "'&&'", "'||'", "'=='", 
      "'!='", "'<'", "'>'", "'<='", "'>='"
    },
    std::vector<std::string>{
      "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", 
      "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", 
      "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", 
      "", "", "", "", "GRAPH", "WEIGHTS", "SET", "UNION", "INTERSECT", "NOT", 
      "MODULO", "EDGE", "NODE", "TRUE", "FALSE", "OF", "PLUS", "MINUS", 
      "TIMES", "DIVIDE", "AND", "OR", "EQUAL", "NOTEQUAL", "LESSTHAN", "GREATERTHAN", 
      "LESSEQUAL", "GREATEREQUAL", "ID", "INT", "REAL", "STRING", "Comment", 
      "WS"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,1,84,736,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,2,
  	7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,7,
  	14,2,15,7,15,2,16,7,16,2,17,7,17,2,18,7,18,2,19,7,19,2,20,7,20,2,21,7,
  	21,2,22,7,22,2,23,7,23,2,24,7,24,2,25,7,25,2,26,7,26,2,27,7,27,2,28,7,
  	28,2,29,7,29,2,30,7,30,2,31,7,31,2,32,7,32,2,33,7,33,2,34,7,34,2,35,7,
  	35,2,36,7,36,2,37,7,37,2,38,7,38,2,39,7,39,2,40,7,40,2,41,7,41,2,42,7,
  	42,2,43,7,43,2,44,7,44,2,45,7,45,2,46,7,46,2,47,7,47,2,48,7,48,2,49,7,
  	49,2,50,7,50,2,51,7,51,2,52,7,52,2,53,7,53,2,54,7,54,2,55,7,55,2,56,7,
  	56,1,0,1,0,5,0,117,8,0,10,0,12,0,120,9,0,1,0,1,0,1,1,1,1,1,1,1,1,1,1,
  	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,3,1,145,
  	8,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,3,1,3,1,3,1,3,3,3,159,8,3,1,3,3,
  	3,162,8,3,1,3,1,3,1,3,1,3,1,3,1,3,1,3,1,3,3,3,172,8,3,1,3,3,3,175,8,3,
  	1,3,1,3,1,3,3,3,180,8,3,1,4,1,4,1,4,1,4,1,5,1,5,1,5,3,5,189,8,5,1,5,1,
  	5,1,6,1,6,1,6,5,6,196,8,6,10,6,12,6,199,9,6,1,7,1,7,1,7,5,7,204,8,7,10,
  	7,12,7,207,9,7,1,8,1,8,1,9,1,9,1,10,1,10,1,10,1,11,1,11,1,11,1,11,1,12,
  	1,12,1,12,1,12,3,12,224,8,12,1,12,1,12,1,12,1,12,1,12,1,12,3,12,232,8,
  	12,1,12,1,12,3,12,236,8,12,1,13,1,13,1,13,1,13,1,13,1,13,1,13,1,13,1,
  	13,1,13,1,13,1,13,1,13,1,13,1,13,3,13,253,8,13,1,14,1,14,1,14,1,14,5,
  	14,259,8,14,10,14,12,14,262,9,14,3,14,264,8,14,1,14,1,14,1,15,1,15,1,
  	15,1,15,1,15,1,16,1,16,1,16,1,16,1,16,1,16,1,16,1,16,1,16,3,16,282,8,
  	16,1,17,1,17,1,17,1,17,1,17,1,17,1,17,1,17,1,17,1,17,1,17,1,17,1,17,1,
  	17,1,17,3,17,299,8,17,1,17,1,17,1,17,1,17,1,17,1,17,5,17,307,8,17,10,
  	17,12,17,310,9,17,1,18,1,18,1,18,1,18,1,18,1,18,1,18,1,18,1,18,1,18,1,
  	18,1,18,1,18,1,18,1,18,1,18,3,18,328,8,18,1,19,1,19,1,19,1,19,1,19,1,
  	19,1,19,1,19,1,19,3,19,339,8,19,1,20,1,20,1,20,1,20,1,20,1,20,1,20,1,
  	20,1,20,1,20,1,20,1,20,1,20,1,20,3,20,355,8,20,1,20,1,20,1,20,1,20,1,
  	20,1,20,5,20,363,8,20,10,20,12,20,366,9,20,1,21,1,21,1,21,1,21,1,21,1,
  	21,3,21,374,8,21,1,21,1,21,1,21,1,22,1,22,1,22,5,22,382,8,22,10,22,12,
  	22,385,9,22,1,23,1,23,1,23,1,23,1,23,1,23,1,23,1,23,1,23,1,23,1,23,1,
  	23,3,23,399,8,23,1,23,1,23,1,23,1,23,1,23,1,23,5,23,407,8,23,10,23,12,
  	23,410,9,23,1,24,1,24,3,24,414,8,24,1,25,1,25,1,25,1,25,1,25,1,25,1,25,
  	1,26,1,26,1,26,1,26,1,26,1,26,1,26,1,26,1,26,1,26,1,26,1,26,1,26,3,26,
  	436,8,26,1,27,1,27,1,27,1,27,1,27,1,27,1,28,1,28,3,28,446,8,28,1,29,1,
  	29,1,29,1,29,1,29,1,29,1,30,1,30,1,30,1,30,1,30,1,30,1,31,1,31,1,31,1,
  	31,3,31,464,8,31,1,32,1,32,1,32,1,32,3,32,470,8,32,1,33,1,33,1,33,1,33,
  	1,33,3,33,477,8,33,1,33,1,33,1,33,1,33,1,34,1,34,1,34,1,34,1,35,1,35,
  	1,35,1,35,1,35,1,35,1,36,1,36,1,37,1,37,1,37,1,37,5,37,499,8,37,10,37,
  	12,37,502,9,37,3,37,504,8,37,1,37,1,37,1,38,1,38,1,38,1,39,1,39,1,40,
  	1,40,1,40,3,40,516,8,40,1,40,1,40,1,41,1,41,1,41,5,41,523,8,41,10,41,
  	12,41,526,9,41,1,42,1,42,1,42,1,42,1,42,1,42,1,43,1,43,1,43,1,43,1,43,
  	5,43,539,8,43,10,43,12,43,542,9,43,1,43,1,43,1,43,3,43,547,8,43,1,44,
  	1,44,1,44,1,44,1,45,1,45,1,45,1,46,1,46,1,46,1,47,1,47,1,47,1,47,1,47,
  	1,47,3,47,565,8,47,1,48,1,48,1,48,3,48,570,8,48,1,48,1,48,1,48,5,48,575,
  	8,48,10,48,12,48,578,9,48,1,49,1,49,1,49,1,49,1,49,1,49,1,49,1,50,1,50,
  	1,50,1,50,1,50,1,50,1,50,1,50,1,50,1,50,1,50,1,50,1,50,1,50,1,50,1,50,
  	1,50,1,50,3,50,605,8,50,1,51,1,51,1,51,1,51,1,51,1,51,1,51,1,51,1,51,
  	1,51,1,51,1,51,1,51,1,51,1,51,1,51,1,51,1,51,1,51,1,51,1,51,1,51,1,51,
  	1,51,1,51,1,51,1,51,1,51,1,51,1,51,1,51,1,51,1,51,1,51,1,51,1,51,1,51,
  	1,51,1,51,1,51,1,51,1,51,1,51,1,51,1,51,1,51,1,51,1,51,1,51,1,51,1,51,
  	1,51,3,51,659,8,51,1,51,1,51,1,51,1,51,1,51,1,51,1,51,1,51,1,51,5,51,
  	670,8,51,10,51,12,51,673,9,51,1,52,1,52,1,52,1,52,1,52,1,52,1,52,1,52,
  	1,52,1,52,1,52,1,52,1,52,1,52,1,52,1,52,3,52,691,8,52,1,53,1,53,1,53,
  	1,53,5,53,697,8,53,10,53,12,53,700,9,53,1,53,1,53,1,54,1,54,1,54,1,54,
  	1,54,1,54,1,54,3,54,711,8,54,1,55,1,55,1,55,1,55,1,55,1,55,1,55,1,55,
  	1,55,1,55,1,55,1,55,1,55,1,55,1,55,1,55,1,55,1,55,1,55,3,55,732,8,55,
  	1,56,1,56,1,56,0,5,34,40,46,96,102,57,0,2,4,6,8,10,12,14,16,18,20,22,
  	24,26,28,30,32,34,36,38,40,42,44,46,48,50,52,54,56,58,60,62,64,66,68,
  	70,72,74,76,78,80,82,84,86,88,90,92,94,96,98,100,102,104,106,108,110,
  	112,0,7,1,0,73,78,1,0,71,72,4,0,28,29,39,43,55,55,57,57,5,0,28,29,39,
  	41,43,43,55,55,57,57,2,0,61,61,69,70,1,0,67,68,1,0,64,65,789,0,118,1,
  	0,0,0,2,144,1,0,0,0,4,146,1,0,0,0,6,179,1,0,0,0,8,181,1,0,0,0,10,185,
  	1,0,0,0,12,192,1,0,0,0,14,200,1,0,0,0,16,208,1,0,0,0,18,210,1,0,0,0,20,
  	212,1,0,0,0,22,215,1,0,0,0,24,235,1,0,0,0,26,252,1,0,0,0,28,254,1,0,0,
  	0,30,267,1,0,0,0,32,281,1,0,0,0,34,298,1,0,0,0,36,327,1,0,0,0,38,329,
  	1,0,0,0,40,354,1,0,0,0,42,367,1,0,0,0,44,378,1,0,0,0,46,398,1,0,0,0,48,
  	413,1,0,0,0,50,415,1,0,0,0,52,435,1,0,0,0,54,437,1,0,0,0,56,445,1,0,0,
  	0,58,447,1,0,0,0,60,453,1,0,0,0,62,463,1,0,0,0,64,469,1,0,0,0,66,471,
  	1,0,0,0,68,482,1,0,0,0,70,486,1,0,0,0,72,492,1,0,0,0,74,494,1,0,0,0,76,
  	507,1,0,0,0,78,510,1,0,0,0,80,512,1,0,0,0,82,519,1,0,0,0,84,527,1,0,0,
  	0,86,546,1,0,0,0,88,548,1,0,0,0,90,552,1,0,0,0,92,555,1,0,0,0,94,564,
  	1,0,0,0,96,569,1,0,0,0,98,579,1,0,0,0,100,604,1,0,0,0,102,658,1,0,0,0,
  	104,690,1,0,0,0,106,692,1,0,0,0,108,710,1,0,0,0,110,731,1,0,0,0,112,733,
  	1,0,0,0,114,117,3,2,1,0,115,117,3,70,35,0,116,114,1,0,0,0,116,115,1,0,
  	0,0,117,120,1,0,0,0,118,116,1,0,0,0,118,119,1,0,0,0,119,121,1,0,0,0,120,
  	118,1,0,0,0,121,122,5,0,0,1,122,1,1,0,0,0,123,145,3,6,3,0,124,145,3,38,
  	19,0,125,145,3,94,47,0,126,145,3,54,27,0,127,145,3,50,25,0,128,145,3,
  	24,12,0,129,145,3,26,13,0,130,131,3,80,40,0,131,132,5,1,0,0,132,145,1,
  	0,0,0,133,145,3,84,42,0,134,145,3,42,21,0,135,145,3,110,55,0,136,145,
  	3,108,54,0,137,145,3,66,33,0,138,145,3,68,34,0,139,145,3,56,28,0,140,
  	145,3,30,15,0,141,145,3,36,18,0,142,145,3,4,2,0,143,145,5,1,0,0,144,123,
  	1,0,0,0,144,124,1,0,0,0,144,125,1,0,0,0,144,126,1,0,0,0,144,127,1,0,0,
  	0,144,128,1,0,0,0,144,129,1,0,0,0,144,130,1,0,0,0,144,133,1,0,0,0,144,
  	134,1,0,0,0,144,135,1,0,0,0,144,136,1,0,0,0,144,137,1,0,0,0,144,138,1,
  	0,0,0,144,139,1,0,0,0,144,140,1,0,0,0,144,141,1,0,0,0,144,142,1,0,0,0,
  	144,143,1,0,0,0,145,3,1,0,0,0,146,147,5,2,0,0,147,148,5,3,0,0,148,149,
  	5,79,0,0,149,150,5,4,0,0,150,151,5,79,0,0,151,152,5,5,0,0,152,153,5,1,
  	0,0,153,5,1,0,0,0,154,155,5,55,0,0,155,156,3,16,8,0,156,158,5,6,0,0,157,
  	159,3,8,4,0,158,157,1,0,0,0,158,159,1,0,0,0,159,161,1,0,0,0,160,162,3,
  	10,5,0,161,160,1,0,0,0,161,162,1,0,0,0,162,163,1,0,0,0,163,164,5,64,0,
  	0,164,165,5,7,0,0,165,166,5,1,0,0,166,180,1,0,0,0,167,168,5,55,0,0,168,
  	169,3,16,8,0,169,171,5,6,0,0,170,172,3,8,4,0,171,170,1,0,0,0,171,172,
  	1,0,0,0,172,174,1,0,0,0,173,175,3,10,5,0,174,173,1,0,0,0,174,175,1,0,
  	0,0,175,176,1,0,0,0,176,177,5,7,0,0,177,178,5,1,0,0,178,180,1,0,0,0,179,
  	154,1,0,0,0,179,167,1,0,0,0,180,7,1,0,0,0,181,182,5,8,0,0,182,183,3,12,
  	6,0,183,184,5,1,0,0,184,9,1,0,0,0,185,188,5,9,0,0,186,189,3,14,7,0,187,
  	189,3,20,10,0,188,186,1,0,0,0,188,187,1,0,0,0,189,190,1,0,0,0,190,191,
  	5,1,0,0,191,11,1,0,0,0,192,197,3,18,9,0,193,194,5,4,0,0,194,196,3,18,
  	9,0,195,193,1,0,0,0,196,199,1,0,0,0,197,195,1,0,0,0,197,198,1,0,0,0,198,
  	13,1,0,0,0,199,197,1,0,0,0,200,205,3,22,11,0,201,202,5,4,0,0,202,204,
  	3,22,11,0,203,201,1,0,0,0,204,207,1,0,0,0,205,203,1,0,0,0,205,206,1,0,
  	0,0,206,15,1,0,0,0,207,205,1,0,0,0,208,209,5,79,0,0,209,17,1,0,0,0,210,
  	211,5,80,0,0,211,19,1,0,0,0,212,213,5,10,0,0,213,214,5,82,0,0,214,21,
  	1,0,0,0,215,216,3,18,9,0,216,217,5,11,0,0,217,218,3,18,9,0,218,23,1,0,
  	0,0,219,220,3,78,39,0,220,223,5,79,0,0,221,222,5,12,0,0,222,224,3,102,
  	51,0,223,221,1,0,0,0,223,224,1,0,0,0,224,225,1,0,0,0,225,226,5,1,0,0,
  	226,236,1,0,0,0,227,228,3,78,39,0,228,231,3,104,52,0,229,230,5,12,0,0,
  	230,232,3,106,53,0,231,229,1,0,0,0,231,232,1,0,0,0,232,233,1,0,0,0,233,
  	234,5,1,0,0,234,236,1,0,0,0,235,219,1,0,0,0,235,227,1,0,0,0,236,25,1,
  	0,0,0,237,238,5,57,0,0,238,239,5,79,0,0,239,253,5,1,0,0,240,241,5,57,
  	0,0,241,242,5,79,0,0,242,243,5,12,0,0,243,244,3,28,14,0,244,245,5,1,0,
  	0,245,253,1,0,0,0,246,247,5,57,0,0,247,248,5,79,0,0,248,249,5,12,0,0,
  	249,250,3,34,17,0,250,251,5,1,0,0,251,253,1,0,0,0,252,237,1,0,0,0,252,
  	240,1,0,0,0,252,246,1,0,0,0,253,27,1,0,0,0,254,263,5,6,0,0,255,260,3,
  	102,51,0,256,257,5,4,0,0,257,259,3,102,51,0,258,256,1,0,0,0,259,262,1,
  	0,0,0,260,258,1,0,0,0,260,261,1,0,0,0,261,264,1,0,0,0,262,260,1,0,0,0,
  	263,255,1,0,0,0,263,264,1,0,0,0,264,265,1,0,0,0,265,266,5,7,0,0,266,29,
  	1,0,0,0,267,268,5,79,0,0,268,269,5,12,0,0,269,270,3,34,17,0,270,271,5,
  	1,0,0,271,31,1,0,0,0,272,282,5,79,0,0,273,274,3,16,8,0,274,275,5,13,0,
  	0,275,276,5,63,0,0,276,282,1,0,0,0,277,278,3,16,8,0,278,279,5,13,0,0,
  	279,280,5,62,0,0,280,282,1,0,0,0,281,272,1,0,0,0,281,273,1,0,0,0,281,
  	277,1,0,0,0,282,33,1,0,0,0,283,284,6,17,-1,0,284,285,3,16,8,0,285,286,
  	5,13,0,0,286,287,5,63,0,0,287,299,1,0,0,0,288,289,3,16,8,0,289,290,5,
  	13,0,0,290,291,5,62,0,0,291,299,1,0,0,0,292,299,5,79,0,0,293,299,3,28,
  	14,0,294,295,5,3,0,0,295,296,3,34,17,0,296,297,5,5,0,0,297,299,1,0,0,
  	0,298,283,1,0,0,0,298,288,1,0,0,0,298,292,1,0,0,0,298,293,1,0,0,0,298,
  	294,1,0,0,0,299,308,1,0,0,0,300,301,10,7,0,0,301,302,5,58,0,0,302,307,
  	3,34,17,8,303,304,10,6,0,0,304,305,5,59,0,0,305,307,3,34,17,7,306,300,
  	1,0,0,0,306,303,1,0,0,0,307,310,1,0,0,0,308,306,1,0,0,0,308,309,1,0,0,
  	0,309,35,1,0,0,0,310,308,1,0,0,0,311,312,3,32,16,0,312,313,5,13,0,0,313,
  	314,5,14,0,0,314,315,5,3,0,0,315,316,3,102,51,0,316,317,5,5,0,0,317,318,
  	5,1,0,0,318,328,1,0,0,0,319,320,3,32,16,0,320,321,5,13,0,0,321,322,5,
  	15,0,0,322,323,5,3,0,0,323,324,3,102,51,0,324,325,5,5,0,0,325,326,5,1,
  	0,0,326,328,1,0,0,0,327,311,1,0,0,0,327,319,1,0,0,0,328,37,1,0,0,0,329,
  	330,5,16,0,0,330,331,5,3,0,0,331,332,3,40,20,0,332,333,5,5,0,0,333,338,
  	3,86,43,0,334,335,5,17,0,0,335,339,3,38,19,0,336,337,5,17,0,0,337,339,
  	3,86,43,0,338,334,1,0,0,0,338,336,1,0,0,0,338,339,1,0,0,0,339,39,1,0,
  	0,0,340,341,6,20,-1,0,341,342,3,102,51,0,342,343,7,0,0,0,343,344,3,102,
  	51,0,344,355,1,0,0,0,345,346,3,18,9,0,346,347,5,18,0,0,347,348,3,16,8,
  	0,348,355,1,0,0,0,349,350,3,22,11,0,350,351,5,18,0,0,351,352,3,16,8,0,
  	352,355,1,0,0,0,353,355,3,102,51,0,354,340,1,0,0,0,354,345,1,0,0,0,354,
  	349,1,0,0,0,354,353,1,0,0,0,355,364,1,0,0,0,356,357,10,6,0,0,357,358,
  	5,71,0,0,358,363,3,40,20,7,359,360,10,5,0,0,360,361,5,72,0,0,361,363,
  	3,40,20,6,362,356,1,0,0,0,362,359,1,0,0,0,363,366,1,0,0,0,364,362,1,0,
  	0,0,364,365,1,0,0,0,365,41,1,0,0,0,366,364,1,0,0,0,367,368,5,79,0,0,368,
  	369,5,12,0,0,369,370,5,19,0,0,370,373,3,44,22,0,371,372,5,20,0,0,372,
  	374,3,46,23,0,373,371,1,0,0,0,373,374,1,0,0,0,374,375,1,0,0,0,375,376,
  	5,21,0,0,376,377,5,1,0,0,377,43,1,0,0,0,378,383,3,16,8,0,379,380,7,1,
  	0,0,380,382,3,16,8,0,381,379,1,0,0,0,382,385,1,0,0,0,383,381,1,0,0,0,
  	383,384,1,0,0,0,384,45,1,0,0,0,385,383,1,0,0,0,386,387,6,23,-1,0,387,
  	388,5,22,0,0,388,389,7,0,0,0,389,399,5,80,0,0,390,391,5,23,0,0,391,392,
  	5,24,0,0,392,399,3,18,9,0,393,399,5,25,0,0,394,395,5,3,0,0,395,396,3,
  	46,23,0,396,397,5,5,0,0,397,399,1,0,0,0,398,386,1,0,0,0,398,390,1,0,0,
  	0,398,393,1,0,0,0,398,394,1,0,0,0,399,408,1,0,0,0,400,401,10,6,0,0,401,
  	402,5,71,0,0,402,407,3,46,23,7,403,404,10,5,0,0,404,405,5,72,0,0,405,
  	407,3,46,23,6,406,400,1,0,0,0,406,403,1,0,0,0,407,410,1,0,0,0,408,406,
  	1,0,0,0,408,409,1,0,0,0,409,47,1,0,0,0,410,408,1,0,0,0,411,414,3,50,25,
  	0,412,414,3,54,27,0,413,411,1,0,0,0,413,412,1,0,0,0,414,49,1,0,0,0,415,
  	416,5,26,0,0,416,417,5,27,0,0,417,418,3,52,26,0,418,419,5,18,0,0,419,
  	420,3,16,8,0,420,421,3,86,43,0,421,51,1,0,0,0,422,423,5,28,0,0,423,436,
  	5,79,0,0,424,425,5,29,0,0,425,426,5,79,0,0,426,427,5,4,0,0,427,436,5,
  	79,0,0,428,429,5,30,0,0,429,430,5,79,0,0,430,431,5,66,0,0,431,436,3,102,
  	51,0,432,433,5,31,0,0,433,436,5,79,0,0,434,436,5,79,0,0,435,422,1,0,0,
  	0,435,424,1,0,0,0,435,428,1,0,0,0,435,432,1,0,0,0,435,434,1,0,0,0,436,
  	53,1,0,0,0,437,438,5,32,0,0,438,439,5,3,0,0,439,440,3,40,20,0,440,441,
  	5,5,0,0,441,442,3,86,43,0,442,55,1,0,0,0,443,446,3,58,29,0,444,446,3,
  	60,30,0,445,443,1,0,0,0,445,444,1,0,0,0,446,57,1,0,0,0,447,448,5,14,0,
  	0,448,449,3,62,31,0,449,450,5,33,0,0,450,451,3,16,8,0,451,452,5,1,0,0,
  	452,59,1,0,0,0,453,454,5,15,0,0,454,455,3,64,32,0,455,456,5,34,0,0,456,
  	457,3,16,8,0,457,458,5,1,0,0,458,61,1,0,0,0,459,464,3,18,9,0,460,464,
  	3,22,11,0,461,464,3,12,6,0,462,464,3,14,7,0,463,459,1,0,0,0,463,460,1,
  	0,0,0,463,461,1,0,0,0,463,462,1,0,0,0,464,63,1,0,0,0,465,470,3,18,9,0,
  	466,470,3,22,11,0,467,470,3,12,6,0,468,470,3,14,7,0,469,465,1,0,0,0,469,
  	466,1,0,0,0,469,467,1,0,0,0,469,468,1,0,0,0,470,65,1,0,0,0,471,472,5,
  	35,0,0,472,473,5,79,0,0,473,474,5,36,0,0,474,476,5,82,0,0,475,477,5,80,
  	0,0,476,475,1,0,0,0,476,477,1,0,0,0,477,478,1,0,0,0,478,479,5,66,0,0,
  	479,480,3,16,8,0,480,481,5,1,0,0,481,67,1,0,0,0,482,483,5,37,0,0,483,
  	484,3,16,8,0,484,485,5,1,0,0,485,69,1,0,0,0,486,487,5,38,0,0,487,488,
  	3,72,36,0,488,489,5,79,0,0,489,490,3,74,37,0,490,491,3,86,43,0,491,71,
  	1,0,0,0,492,493,7,2,0,0,493,73,1,0,0,0,494,503,5,3,0,0,495,500,3,76,38,
  	0,496,497,5,4,0,0,497,499,3,76,38,0,498,496,1,0,0,0,499,502,1,0,0,0,500,
  	498,1,0,0,0,500,501,1,0,0,0,501,504,1,0,0,0,502,500,1,0,0,0,503,495,1,
  	0,0,0,503,504,1,0,0,0,504,505,1,0,0,0,505,506,5,5,0,0,506,75,1,0,0,0,
  	507,508,3,78,39,0,508,509,5,79,0,0,509,77,1,0,0,0,510,511,7,3,0,0,511,
  	79,1,0,0,0,512,513,5,79,0,0,513,515,5,3,0,0,514,516,3,82,41,0,515,514,
  	1,0,0,0,515,516,1,0,0,0,516,517,1,0,0,0,517,518,5,5,0,0,518,81,1,0,0,
  	0,519,524,3,102,51,0,520,521,5,4,0,0,521,523,3,102,51,0,522,520,1,0,0,
  	0,523,526,1,0,0,0,524,522,1,0,0,0,524,525,1,0,0,0,525,83,1,0,0,0,526,
  	524,1,0,0,0,527,528,5,44,0,0,528,529,5,3,0,0,529,530,3,102,51,0,530,531,
  	5,5,0,0,531,532,5,1,0,0,532,85,1,0,0,0,533,540,5,6,0,0,534,539,3,2,1,
  	0,535,539,3,88,44,0,536,539,3,90,45,0,537,539,3,92,46,0,538,534,1,0,0,
  	0,538,535,1,0,0,0,538,536,1,0,0,0,538,537,1,0,0,0,539,542,1,0,0,0,540,
  	538,1,0,0,0,540,541,1,0,0,0,541,543,1,0,0,0,542,540,1,0,0,0,543,547,5,
  	7,0,0,544,545,5,6,0,0,545,547,5,7,0,0,546,533,1,0,0,0,546,544,1,0,0,0,
  	547,87,1,0,0,0,548,549,5,45,0,0,549,550,3,102,51,0,550,551,5,1,0,0,551,
  	89,1,0,0,0,552,553,5,46,0,0,553,554,5,1,0,0,554,91,1,0,0,0,555,556,5,
  	47,0,0,556,557,5,1,0,0,557,93,1,0,0,0,558,559,5,48,0,0,559,560,3,96,48,
  	0,560,561,5,1,0,0,561,565,1,0,0,0,562,565,3,98,49,0,563,565,3,100,50,
  	0,564,558,1,0,0,0,564,562,1,0,0,0,564,563,1,0,0,0,565,95,1,0,0,0,566,
  	567,6,48,-1,0,567,570,5,82,0,0,568,570,3,102,51,0,569,566,1,0,0,0,569,
  	568,1,0,0,0,570,576,1,0,0,0,571,572,10,1,0,0,572,573,5,67,0,0,573,575,
  	3,96,48,2,574,571,1,0,0,0,575,578,1,0,0,0,576,574,1,0,0,0,576,577,1,0,
  	0,0,577,97,1,0,0,0,578,576,1,0,0,0,579,580,5,48,0,0,580,581,5,79,0,0,
  	581,582,5,19,0,0,582,583,3,102,51,0,583,584,5,21,0,0,584,585,5,1,0,0,
  	585,99,1,0,0,0,586,587,5,48,0,0,587,588,5,62,0,0,588,589,5,66,0,0,589,
  	590,3,16,8,0,590,591,5,1,0,0,591,605,1,0,0,0,592,593,5,48,0,0,593,594,
  	5,63,0,0,594,595,5,66,0,0,595,596,3,16,8,0,596,597,5,1,0,0,597,605,1,
  	0,0,0,598,599,5,48,0,0,599,600,5,55,0,0,600,601,5,66,0,0,601,602,3,16,
  	8,0,602,603,5,1,0,0,603,605,1,0,0,0,604,586,1,0,0,0,604,592,1,0,0,0,604,
  	598,1,0,0,0,605,101,1,0,0,0,606,607,6,51,-1,0,607,608,5,68,0,0,608,659,
  	3,102,51,18,609,610,5,60,0,0,610,659,3,102,51,17,611,659,3,80,40,0,612,
  	659,5,80,0,0,613,659,5,79,0,0,614,615,5,3,0,0,615,616,3,102,51,0,616,
  	617,5,5,0,0,617,659,1,0,0,0,618,619,5,79,0,0,619,620,5,19,0,0,620,621,
  	3,102,51,0,621,622,5,21,0,0,622,623,5,19,0,0,623,624,3,102,51,0,624,625,
  	5,21,0,0,625,659,1,0,0,0,626,627,5,79,0,0,627,628,5,19,0,0,628,629,3,
  	102,51,0,629,630,5,21,0,0,630,659,1,0,0,0,631,632,3,32,16,0,632,633,5,
  	13,0,0,633,634,5,49,0,0,634,635,5,3,0,0,635,636,3,102,51,0,636,637,5,
  	5,0,0,637,659,1,0,0,0,638,639,5,79,0,0,639,640,5,13,0,0,640,641,5,50,
  	0,0,641,642,5,3,0,0,642,659,5,5,0,0,643,644,5,79,0,0,644,645,5,13,0,0,
  	645,646,5,51,0,0,646,647,5,3,0,0,647,659,5,5,0,0,648,659,5,64,0,0,649,
  	659,5,65,0,0,650,651,5,79,0,0,651,659,5,52,0,0,652,659,5,81,0,0,653,659,
  	3,28,14,0,654,659,5,53,0,0,655,656,5,54,0,0,656,657,5,3,0,0,657,659,5,
  	5,0,0,658,606,1,0,0,0,658,609,1,0,0,0,658,611,1,0,0,0,658,612,1,0,0,0,
  	658,613,1,0,0,0,658,614,1,0,0,0,658,618,1,0,0,0,658,626,1,0,0,0,658,631,
  	1,0,0,0,658,638,1,0,0,0,658,643,1,0,0,0,658,648,1,0,0,0,658,649,1,0,0,
  	0,658,650,1,0,0,0,658,652,1,0,0,0,658,653,1,0,0,0,658,654,1,0,0,0,658,
  	655,1,0,0,0,659,671,1,0,0,0,660,661,10,21,0,0,661,662,7,1,0,0,662,670,
  	3,102,51,22,663,664,10,20,0,0,664,665,7,4,0,0,665,670,3,102,51,21,666,
  	667,10,19,0,0,667,668,7,5,0,0,668,670,3,102,51,20,669,660,1,0,0,0,669,
  	663,1,0,0,0,669,666,1,0,0,0,670,673,1,0,0,0,671,669,1,0,0,0,671,672,1,
  	0,0,0,672,103,1,0,0,0,673,671,1,0,0,0,674,675,5,79,0,0,675,676,5,19,0,
  	0,676,677,3,102,51,0,677,678,5,21,0,0,678,679,5,19,0,0,679,680,3,102,
  	51,0,680,681,5,21,0,0,681,691,1,0,0,0,682,683,5,79,0,0,683,684,5,19,0,
  	0,684,685,3,102,51,0,685,686,5,21,0,0,686,691,1,0,0,0,687,688,5,79,0,
  	0,688,689,5,19,0,0,689,691,5,21,0,0,690,674,1,0,0,0,690,682,1,0,0,0,690,
  	687,1,0,0,0,691,105,1,0,0,0,692,693,5,19,0,0,693,698,3,102,51,0,694,695,
  	5,4,0,0,695,697,3,102,51,0,696,694,1,0,0,0,697,700,1,0,0,0,698,696,1,
  	0,0,0,698,699,1,0,0,0,699,701,1,0,0,0,700,698,1,0,0,0,701,702,5,21,0,
  	0,702,107,1,0,0,0,703,704,5,79,0,0,704,705,5,12,0,0,705,706,3,102,51,
  	0,706,707,5,1,0,0,707,711,1,0,0,0,708,709,5,79,0,0,709,711,5,1,0,0,710,
  	703,1,0,0,0,710,708,1,0,0,0,711,109,1,0,0,0,712,713,5,79,0,0,713,714,
  	5,19,0,0,714,715,3,102,51,0,715,716,5,21,0,0,716,717,5,19,0,0,717,718,
  	3,102,51,0,718,719,5,21,0,0,719,720,5,12,0,0,720,721,3,102,51,0,721,722,
  	5,1,0,0,722,732,1,0,0,0,723,724,5,79,0,0,724,725,5,19,0,0,725,726,3,102,
  	51,0,726,727,5,21,0,0,727,728,5,12,0,0,728,729,3,102,51,0,729,730,5,1,
  	0,0,730,732,1,0,0,0,731,712,1,0,0,0,731,723,1,0,0,0,732,111,1,0,0,0,733,
  	734,7,6,0,0,734,113,1,0,0,0,55,116,118,144,158,161,171,174,179,188,197,
  	205,223,231,235,252,260,263,281,298,306,308,327,338,354,362,364,373,383,
  	398,406,408,413,435,445,463,469,476,500,503,515,524,538,540,546,564,569,
  	576,604,658,669,671,690,698,710,731
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
    setState(118);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 180456148485390342) != 0) || _la == BaseParser::ID) {
      setState(116);
      _errHandler->sync(this);
      switch (_input->LA(1)) {
        case BaseParser::T__0:
        case BaseParser::T__1:
        case BaseParser::T__13:
        case BaseParser::T__14:
        case BaseParser::T__15:
        case BaseParser::T__25:
        case BaseParser::T__27:
        case BaseParser::T__28:
        case BaseParser::T__31:
        case BaseParser::T__34:
        case BaseParser::T__36:
        case BaseParser::T__38:
        case BaseParser::T__39:
        case BaseParser::T__40:
        case BaseParser::T__42:
        case BaseParser::T__43:
        case BaseParser::T__47:
        case BaseParser::GRAPH:
        case BaseParser::SET:
        case BaseParser::ID: {
          setState(114);
          statement();
          break;
        }

        case BaseParser::T__37: {
          setState(115);
          function();
          break;
        }

      default:
        throw NoViableAltException(this);
      }
      setState(120);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(121);
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
    setState(144);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 2, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(123);
      graphDef();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(124);
      conditionalStatement();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(125);
      printStatement();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(126);
      whileStatement();
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(127);
      foreachStatement();
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(128);
      varDecl();
      break;
    }

    case 7: {
      enterOuterAlt(_localctx, 7);
      setState(129);
      setDecl();
      break;
    }

    case 8: {
      enterOuterAlt(_localctx, 8);
      setState(130);
      functionCall();
      setState(131);
      match(BaseParser::T__0);
      break;
    }

    case 9: {
      enterOuterAlt(_localctx, 9);
      setState(133);
      sleepStatement();
      break;
    }

    case 10: {
      enterOuterAlt(_localctx, 10);
      setState(134);
      graphComprehension();
      break;
    }

    case 11: {
      enterOuterAlt(_localctx, 11);
      setState(135);
      arrayAssignStatement();
      break;
    }

    case 12: {
      enterOuterAlt(_localctx, 12);
      setState(136);
      assignmentStatement();
      break;
    }

    case 13: {
      enterOuterAlt(_localctx, 13);
      setState(137);
      queryStatement();
      break;
    }

    case 14: {
      enterOuterAlt(_localctx, 14);
      setState(138);
      showgraph();
      break;
    }

    case 15: {
      enterOuterAlt(_localctx, 15);
      setState(139);
      nodeEdgeOperation();
      break;
    }

    case 16: {
      enterOuterAlt(_localctx, 16);
      setState(140);
      setOperation();
      break;
    }

    case 17: {
      enterOuterAlt(_localctx, 17);
      setState(141);
      setMethodCall();
      break;
    }

    case 18: {
      enterOuterAlt(_localctx, 18);
      setState(142);
      swapStatement();
      break;
    }

    case 19: {
      enterOuterAlt(_localctx, 19);
      setState(143);
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
    setState(146);
    match(BaseParser::T__1);
    setState(147);
    match(BaseParser::T__2);
    setState(148);
    match(BaseParser::ID);
    setState(149);
    match(BaseParser::T__3);
    setState(150);
    match(BaseParser::ID);
    setState(151);
    match(BaseParser::T__4);
    setState(152);
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
    setState(179);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 7, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<BaseParser::WeightedGraphDefContext>(_localctx);
      enterOuterAlt(_localctx, 1);
      setState(154);
      match(BaseParser::GRAPH);
      setState(155);
      graphID();
      setState(156);
      match(BaseParser::T__5);
      setState(158);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == BaseParser::T__7) {
        setState(157);
        nodes();
      }
      setState(161);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == BaseParser::T__8) {
        setState(160);
        edges();
      }
      setState(163);
      match(BaseParser::TRUE);
      setState(164);
      match(BaseParser::T__6);
      setState(165);
      match(BaseParser::T__0);
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<BaseParser::UnweightedGraphDefContext>(_localctx);
      enterOuterAlt(_localctx, 2);
      setState(167);
      match(BaseParser::GRAPH);
      setState(168);
      graphID();
      setState(169);
      match(BaseParser::T__5);
      setState(171);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == BaseParser::T__7) {
        setState(170);
        nodes();
      }
      setState(174);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == BaseParser::T__8) {
        setState(173);
        edges();
      }
      setState(176);
      match(BaseParser::T__6);
      setState(177);
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
  enterRule(_localctx, 8, BaseParser::RuleNodes);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(181);
    match(BaseParser::T__7);
    setState(182);
    nodeList();
    setState(183);
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
  enterRule(_localctx, 10, BaseParser::RuleEdges);

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
    match(BaseParser::T__8);
    setState(188);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case BaseParser::INT: {
        setState(186);
        edgeList();
        break;
      }

      case BaseParser::T__9: {
        setState(187);
        fileEdgeList();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    setState(190);
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
  enterRule(_localctx, 12, BaseParser::RuleNodeList);
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
    setState(192);
    nodeID();
    setState(197);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == BaseParser::T__3) {
      setState(193);
      match(BaseParser::T__3);
      setState(194);
      nodeID();
      setState(199);
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
  enterRule(_localctx, 14, BaseParser::RuleEdgeList);
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
    setState(200);
    edge();
    setState(205);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == BaseParser::T__3) {
      setState(201);
      match(BaseParser::T__3);
      setState(202);
      edge();
      setState(207);
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
  enterRule(_localctx, 16, BaseParser::RuleGraphID);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(208);
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
  enterRule(_localctx, 18, BaseParser::RuleNodeID);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(210);
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
  enterRule(_localctx, 20, BaseParser::RuleFileEdgeList);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(212);
    match(BaseParser::T__9);
    setState(213);
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
  enterRule(_localctx, 22, BaseParser::RuleEdge);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(215);
    nodeID();
    setState(216);
    match(BaseParser::T__10);
    setState(217);
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
  enterRule(_localctx, 24, BaseParser::RuleVarDecl);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(235);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 13, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<BaseParser::SimpleDeclarationContext>(_localctx);
      enterOuterAlt(_localctx, 1);
      setState(219);
      type();
      setState(220);
      match(BaseParser::ID);
      setState(223);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == BaseParser::T__11) {
        setState(221);
        match(BaseParser::T__11);
        setState(222);
        expr(0);
      }
      setState(225);
      match(BaseParser::T__0);
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<BaseParser::ArrayDeclarationContext>(_localctx);
      enterOuterAlt(_localctx, 2);
      setState(227);
      type();
      setState(228);
      arrayDeclarator();
      setState(231);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == BaseParser::T__11) {
        setState(229);
        match(BaseParser::T__11);
        setState(230);
        arrayInitializer();
      }
      setState(233);
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
  enterRule(_localctx, 26, BaseParser::RuleSetDecl);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(252);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 14, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(237);
      match(BaseParser::SET);
      setState(238);
      match(BaseParser::ID);
      setState(239);
      match(BaseParser::T__0);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(240);
      match(BaseParser::SET);
      setState(241);
      match(BaseParser::ID);
      setState(242);
      match(BaseParser::T__11);
      setState(243);
      setInitializer();
      setState(244);
      match(BaseParser::T__0);
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(246);
      match(BaseParser::SET);
      setState(247);
      match(BaseParser::ID);
      setState(248);
      match(BaseParser::T__11);
      setState(249);
      setExpr(0);
      setState(250);
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
  enterRule(_localctx, 28, BaseParser::RuleSetInitializer);
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
    setState(254);
    match(BaseParser::T__5);
    setState(263);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 1179943102371070024) != 0) || ((((_la - 64) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 64)) & 229395) != 0)) {
      setState(255);
      expr(0);
      setState(260);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == BaseParser::T__3) {
        setState(256);
        match(BaseParser::T__3);
        setState(257);
        expr(0);
        setState(262);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
    }
    setState(265);
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
  enterRule(_localctx, 30, BaseParser::RuleSetOperation);

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
    match(BaseParser::ID);
    setState(268);
    match(BaseParser::T__11);
    setState(269);
    setExpr(0);
    setState(270);
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
  enterRule(_localctx, 32, BaseParser::RuleSetTarget);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(281);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 17, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(272);
      match(BaseParser::ID);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(273);
      graphID();
      setState(274);
      match(BaseParser::T__12);
      setState(275);
      match(BaseParser::NODE);
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(277);
      graphID();
      setState(278);
      match(BaseParser::T__12);
      setState(279);
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
  size_t startState = 34;
  enterRecursionRule(_localctx, 34, BaseParser::RuleSetExpr, precedence);

    

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
    setState(298);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 18, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<GraphNodesSetContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;

      setState(284);
      graphID();
      setState(285);
      match(BaseParser::T__12);
      setState(286);
      match(BaseParser::NODE);
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<GraphEdgesSetContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(288);
      graphID();
      setState(289);
      match(BaseParser::T__12);
      setState(290);
      match(BaseParser::EDGE);
      break;
    }

    case 3: {
      _localctx = _tracker.createInstance<SetIdContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(292);
      match(BaseParser::ID);
      break;
    }

    case 4: {
      _localctx = _tracker.createInstance<SetLiteralContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(293);
      setInitializer();
      break;
    }

    case 5: {
      _localctx = _tracker.createInstance<ParenSetContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(294);
      match(BaseParser::T__2);
      setState(295);
      setExpr(0);
      setState(296);
      match(BaseParser::T__4);
      break;
    }

    default:
      break;
    }
    _ctx->stop = _input->LT(-1);
    setState(308);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 20, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(306);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 19, _ctx)) {
        case 1: {
          auto newContext = _tracker.createInstance<SetUnionContext>(_tracker.createInstance<SetExprContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleSetExpr);
          setState(300);

          if (!(precpred(_ctx, 7))) throw FailedPredicateException(this, "precpred(_ctx, 7)");
          setState(301);
          match(BaseParser::UNION);
          setState(302);
          setExpr(8);
          break;
        }

        case 2: {
          auto newContext = _tracker.createInstance<SetIntersectContext>(_tracker.createInstance<SetExprContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleSetExpr);
          setState(303);

          if (!(precpred(_ctx, 6))) throw FailedPredicateException(this, "precpred(_ctx, 6)");
          setState(304);
          match(BaseParser::INTERSECT);
          setState(305);
          setExpr(7);
          break;
        }

        default:
          break;
        } 
      }
      setState(310);
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
  enterRule(_localctx, 36, BaseParser::RuleSetMethodCall);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(327);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 21, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<BaseParser::SetAddMethodContext>(_localctx);
      enterOuterAlt(_localctx, 1);
      setState(311);
      setTarget();
      setState(312);
      match(BaseParser::T__12);
      setState(313);
      match(BaseParser::T__13);
      setState(314);
      match(BaseParser::T__2);
      setState(315);
      expr(0);
      setState(316);
      match(BaseParser::T__4);
      setState(317);
      match(BaseParser::T__0);
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<BaseParser::SetRemoveMethodContext>(_localctx);
      enterOuterAlt(_localctx, 2);
      setState(319);
      setTarget();
      setState(320);
      match(BaseParser::T__12);
      setState(321);
      match(BaseParser::T__14);
      setState(322);
      match(BaseParser::T__2);
      setState(323);
      expr(0);
      setState(324);
      match(BaseParser::T__4);
      setState(325);
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
  enterRule(_localctx, 38, BaseParser::RuleConditionalStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(329);
    match(BaseParser::T__15);
    setState(330);
    match(BaseParser::T__2);
    setState(331);
    condition(0);
    setState(332);
    match(BaseParser::T__4);
    setState(333);
    block();
    setState(338);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 22, _ctx)) {
    case 1: {
      setState(334);
      match(BaseParser::T__16);
      setState(335);
      conditionalStatement();
      break;
    }

    case 2: {
      setState(336);
      match(BaseParser::T__16);
      setState(337);
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
  size_t startState = 40;
  enterRecursionRule(_localctx, 40, BaseParser::RuleCondition, precedence);

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
    setState(354);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 23, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<RelationalContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;

      setState(341);
      expr(0);
      setState(342);
      _la = _input->LA(1);
      if (!(((((_la - 73) & ~ 0x3fULL) == 0) &&
        ((1ULL << (_la - 73)) & 63) != 0))) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(343);
      expr(0);
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<NodeCheckContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(345);
      nodeID();
      setState(346);
      match(BaseParser::T__17);
      setState(347);
      graphID();
      break;
    }

    case 3: {
      _localctx = _tracker.createInstance<EdgeCheckContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(349);
      edge();
      setState(350);
      match(BaseParser::T__17);
      setState(351);
      graphID();
      break;
    }

    case 4: {
      _localctx = _tracker.createInstance<ExprConditionContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(353);
      expr(0);
      break;
    }

    default:
      break;
    }
    _ctx->stop = _input->LT(-1);
    setState(364);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 25, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(362);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 24, _ctx)) {
        case 1: {
          auto newContext = _tracker.createInstance<LogicalAndContext>(_tracker.createInstance<ConditionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleCondition);
          setState(356);

          if (!(precpred(_ctx, 6))) throw FailedPredicateException(this, "precpred(_ctx, 6)");
          setState(357);
          match(BaseParser::AND);
          setState(358);
          condition(7);
          break;
        }

        case 2: {
          auto newContext = _tracker.createInstance<LogicalOrContext>(_tracker.createInstance<ConditionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleCondition);
          setState(359);

          if (!(precpred(_ctx, 5))) throw FailedPredicateException(this, "precpred(_ctx, 5)");
          setState(360);
          match(BaseParser::OR);
          setState(361);
          condition(6);
          break;
        }

        default:
          break;
        } 
      }
      setState(366);
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
  enterRule(_localctx, 42, BaseParser::RuleGraphComprehension);
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
    setState(367);
    match(BaseParser::ID);
    setState(368);
    match(BaseParser::T__11);
    setState(369);
    match(BaseParser::T__18);
    setState(370);
    graphExpr();
    setState(373);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == BaseParser::T__19) {
      setState(371);
      match(BaseParser::T__19);
      setState(372);
      graphCondition(0);
    }
    setState(375);
    match(BaseParser::T__20);
    setState(376);
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
  enterRule(_localctx, 44, BaseParser::RuleGraphExpr);
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
    setState(378);
    graphID();
    setState(383);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == BaseParser::AND

    || _la == BaseParser::OR) {
      setState(379);
      _la = _input->LA(1);
      if (!(_la == BaseParser::AND

      || _la == BaseParser::OR)) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(380);
      graphID();
      setState(385);
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
  size_t startState = 46;
  enterRecursionRule(_localctx, 46, BaseParser::RuleGraphCondition, precedence);

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
    setState(398);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case BaseParser::T__21: {
        _localctx = _tracker.createInstance<DegreeConditionContext>(_localctx);
        _ctx = _localctx;
        previousContext = _localctx;

        setState(387);
        match(BaseParser::T__21);
        setState(388);
        _la = _input->LA(1);
        if (!(((((_la - 73) & ~ 0x3fULL) == 0) &&
          ((1ULL << (_la - 73)) & 63) != 0))) {
        _errHandler->recoverInline(this);
        }
        else {
          _errHandler->reportMatch(this);
          consume();
        }
        setState(389);
        match(BaseParser::INT);
        break;
      }

      case BaseParser::T__22: {
        _localctx = _tracker.createInstance<ConnectedConditionContext>(_localctx);
        _ctx = _localctx;
        previousContext = _localctx;
        setState(390);
        match(BaseParser::T__22);
        setState(391);
        match(BaseParser::T__23);
        setState(392);
        nodeID();
        break;
      }

      case BaseParser::T__24: {
        _localctx = _tracker.createInstance<CycleConditionContext>(_localctx);
        _ctx = _localctx;
        previousContext = _localctx;
        setState(393);
        match(BaseParser::T__24);
        break;
      }

      case BaseParser::T__2: {
        _localctx = _tracker.createInstance<ParenGraphConditionContext>(_localctx);
        _ctx = _localctx;
        previousContext = _localctx;
        setState(394);
        match(BaseParser::T__2);
        setState(395);
        graphCondition(0);
        setState(396);
        match(BaseParser::T__4);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    _ctx->stop = _input->LT(-1);
    setState(408);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 30, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(406);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 29, _ctx)) {
        case 1: {
          auto newContext = _tracker.createInstance<GraphLogicalAndContext>(_tracker.createInstance<GraphConditionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleGraphCondition);
          setState(400);

          if (!(precpred(_ctx, 6))) throw FailedPredicateException(this, "precpred(_ctx, 6)");
          setState(401);
          match(BaseParser::AND);
          setState(402);
          graphCondition(7);
          break;
        }

        case 2: {
          auto newContext = _tracker.createInstance<GraphLogicalOrContext>(_tracker.createInstance<GraphConditionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleGraphCondition);
          setState(403);

          if (!(precpred(_ctx, 5))) throw FailedPredicateException(this, "precpred(_ctx, 5)");
          setState(404);
          match(BaseParser::OR);
          setState(405);
          graphCondition(6);
          break;
        }

        default:
          break;
        } 
      }
      setState(410);
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
  enterRule(_localctx, 48, BaseParser::RuleLoopStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(413);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case BaseParser::T__25: {
        enterOuterAlt(_localctx, 1);
        setState(411);
        foreachStatement();
        break;
      }

      case BaseParser::T__31: {
        enterOuterAlt(_localctx, 2);
        setState(412);
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
  enterRule(_localctx, 50, BaseParser::RuleForeachStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(415);
    match(BaseParser::T__25);
    setState(416);
    match(BaseParser::T__26);
    setState(417);
    loopTarget();
    setState(418);
    match(BaseParser::T__17);
    setState(419);
    graphID();
    setState(420);
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
  enterRule(_localctx, 52, BaseParser::RuleLoopTarget);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(435);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case BaseParser::T__27: {
        _localctx = _tracker.createInstance<BaseParser::ForEachVertexContext>(_localctx);
        enterOuterAlt(_localctx, 1);
        setState(422);
        match(BaseParser::T__27);
        setState(423);
        match(BaseParser::ID);
        break;
      }

      case BaseParser::T__28: {
        _localctx = _tracker.createInstance<BaseParser::ForEachEdgeContext>(_localctx);
        enterOuterAlt(_localctx, 2);
        setState(424);
        match(BaseParser::T__28);
        setState(425);
        match(BaseParser::ID);
        setState(426);
        match(BaseParser::T__3);
        setState(427);
        match(BaseParser::ID);
        break;
      }

      case BaseParser::T__29: {
        _localctx = _tracker.createInstance<BaseParser::ForEachAdjContext>(_localctx);
        enterOuterAlt(_localctx, 3);
        setState(428);
        match(BaseParser::T__29);
        setState(429);
        match(BaseParser::ID);
        setState(430);
        match(BaseParser::OF);
        setState(431);
        expr(0);
        break;
      }

      case BaseParser::T__30: {
        _localctx = _tracker.createInstance<BaseParser::ForEachElementContext>(_localctx);
        enterOuterAlt(_localctx, 4);
        setState(432);
        match(BaseParser::T__30);
        setState(433);
        match(BaseParser::ID);
        break;
      }

      case BaseParser::ID: {
        _localctx = _tracker.createInstance<BaseParser::ForEachPlainContext>(_localctx);
        enterOuterAlt(_localctx, 5);
        setState(434);
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
  enterRule(_localctx, 54, BaseParser::RuleWhileStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(437);
    match(BaseParser::T__31);
    setState(438);
    match(BaseParser::T__2);
    setState(439);
    condition(0);
    setState(440);
    match(BaseParser::T__4);
    setState(441);
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
  enterRule(_localctx, 56, BaseParser::RuleNodeEdgeOperation);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(445);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case BaseParser::T__13: {
        enterOuterAlt(_localctx, 1);
        setState(443);
        addOperation();
        break;
      }

      case BaseParser::T__14: {
        enterOuterAlt(_localctx, 2);
        setState(444);
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
  enterRule(_localctx, 58, BaseParser::RuleAddOperation);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(447);
    match(BaseParser::T__13);
    setState(448);
    addTargets();
    setState(449);
    match(BaseParser::T__32);
    setState(450);
    graphID();
    setState(451);
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
  enterRule(_localctx, 60, BaseParser::RuleRemoveOperation);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(453);
    match(BaseParser::T__14);
    setState(454);
    removeTargets();
    setState(455);
    match(BaseParser::T__33);
    setState(456);
    graphID();
    setState(457);
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
  enterRule(_localctx, 62, BaseParser::RuleAddTargets);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(463);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 34, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(459);
      nodeID();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(460);
      edge();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(461);
      nodeList();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(462);
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
  enterRule(_localctx, 64, BaseParser::RuleRemoveTargets);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(469);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 35, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(465);
      nodeID();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(466);
      edge();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(467);
      nodeList();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(468);
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
  enterRule(_localctx, 66, BaseParser::RuleQueryStatement);
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
    setState(471);
    match(BaseParser::T__34);
    setState(472);
    match(BaseParser::ID);
    setState(473);
    match(BaseParser::T__35);
    setState(474);
    match(BaseParser::STRING);
    setState(476);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == BaseParser::INT) {
      setState(475);
      match(BaseParser::INT);
    }
    setState(478);
    match(BaseParser::OF);
    setState(479);
    graphID();
    setState(480);
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
  enterRule(_localctx, 68, BaseParser::RuleShowgraph);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(482);
    match(BaseParser::T__36);
    setState(483);
    graphID();
    setState(484);
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
  enterRule(_localctx, 70, BaseParser::RuleFunction);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(486);
    match(BaseParser::T__37);
    setState(487);
    returnType();
    setState(488);
    match(BaseParser::ID);
    setState(489);
    paramList();
    setState(490);
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
  enterRule(_localctx, 72, BaseParser::RuleReturnType);
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
    setState(492);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 180161028330356736) != 0))) {
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
  enterRule(_localctx, 74, BaseParser::RuleParamList);
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
    setState(494);
    match(BaseParser::T__2);
    setState(503);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 180156630283845632) != 0)) {
      setState(495);
      param();
      setState(500);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == BaseParser::T__3) {
        setState(496);
        match(BaseParser::T__3);
        setState(497);
        param();
        setState(502);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
    }
    setState(505);
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
  enterRule(_localctx, 76, BaseParser::RuleParam);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(507);
    type();
    setState(508);
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
  enterRule(_localctx, 78, BaseParser::RuleType);
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
    setState(510);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 180156630283845632) != 0))) {
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
  enterRule(_localctx, 80, BaseParser::RuleFunctionCall);
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
    setState(512);
    match(BaseParser::ID);
    setState(513);
    match(BaseParser::T__2);
    setState(515);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 1179943102371070024) != 0) || ((((_la - 64) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 64)) & 229395) != 0)) {
      setState(514);
      argumentList();
    }
    setState(517);
    match(BaseParser::T__4);
   
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
  enterRule(_localctx, 82, BaseParser::RuleArgumentList);
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
    setState(519);
    expr(0);
    setState(524);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == BaseParser::T__3) {
      setState(520);
      match(BaseParser::T__3);
      setState(521);
      expr(0);
      setState(526);
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
  enterRule(_localctx, 84, BaseParser::RuleSleepStatement);

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
    match(BaseParser::T__43);
    setState(528);
    match(BaseParser::T__2);
    setState(529);
    expr(0);
    setState(530);
    match(BaseParser::T__4);
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
  enterRule(_localctx, 86, BaseParser::RuleBlock);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(546);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 43, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(533);
      match(BaseParser::T__5);
      setState(540);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while ((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 180702164212105222) != 0) || _la == BaseParser::ID) {
        setState(538);
        _errHandler->sync(this);
        switch (_input->LA(1)) {
          case BaseParser::T__0:
          case BaseParser::T__1:
          case BaseParser::T__13:
          case BaseParser::T__14:
          case BaseParser::T__15:
          case BaseParser::T__25:
          case BaseParser::T__27:
          case BaseParser::T__28:
          case BaseParser::T__31:
          case BaseParser::T__34:
          case BaseParser::T__36:
          case BaseParser::T__38:
          case BaseParser::T__39:
          case BaseParser::T__40:
          case BaseParser::T__42:
          case BaseParser::T__43:
          case BaseParser::T__47:
          case BaseParser::GRAPH:
          case BaseParser::SET:
          case BaseParser::ID: {
            setState(534);
            statement();
            break;
          }

          case BaseParser::T__44: {
            setState(535);
            returnStatement();
            break;
          }

          case BaseParser::T__45: {
            setState(536);
            breakStatement();
            break;
          }

          case BaseParser::T__46: {
            setState(537);
            continueStatement();
            break;
          }

        default:
          throw NoViableAltException(this);
        }
        setState(542);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
      setState(543);
      match(BaseParser::T__6);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(544);
      match(BaseParser::T__5);
      setState(545);
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
  enterRule(_localctx, 88, BaseParser::RuleReturnStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(548);
    match(BaseParser::T__44);
    setState(549);
    expr(0);
    setState(550);
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
  enterRule(_localctx, 90, BaseParser::RuleBreakStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(552);
    match(BaseParser::T__45);
    setState(553);
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
  enterRule(_localctx, 92, BaseParser::RuleContinueStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(555);
    match(BaseParser::T__46);
    setState(556);
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
  enterRule(_localctx, 94, BaseParser::RulePrintStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(564);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 44, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(558);
      match(BaseParser::T__47);
      setState(559);
      printExpr(0);
      setState(560);
      match(BaseParser::T__0);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(562);
      printArrayStatement();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(563);
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
  size_t startState = 96;
  enterRecursionRule(_localctx, 96, BaseParser::RulePrintExpr, precedence);

    

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
    setState(569);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case BaseParser::STRING: {
        setState(567);
        match(BaseParser::STRING);
        break;
      }

      case BaseParser::T__2:
      case BaseParser::T__5:
      case BaseParser::T__52:
      case BaseParser::T__53:
      case BaseParser::NOT:
      case BaseParser::TRUE:
      case BaseParser::FALSE:
      case BaseParser::MINUS:
      case BaseParser::ID:
      case BaseParser::INT:
      case BaseParser::REAL: {
        setState(568);
        expr(0);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    _ctx->stop = _input->LT(-1);
    setState(576);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 46, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        _localctx = _tracker.createInstance<PrintExprContext>(parentContext, parentState);
        pushNewRecursionContext(_localctx, startState, RulePrintExpr);
        setState(571);

        if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
        setState(572);
        match(BaseParser::PLUS);
        setState(573);
        printExpr(2); 
      }
      setState(578);
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
  enterRule(_localctx, 98, BaseParser::RulePrintArrayStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(579);
    match(BaseParser::T__47);
    setState(580);
    match(BaseParser::ID);
    setState(581);
    match(BaseParser::T__18);
    setState(582);
    expr(0);
    setState(583);
    match(BaseParser::T__20);
    setState(584);
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
  enterRule(_localctx, 100, BaseParser::RulePrintgraph);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(604);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 47, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<BaseParser::EdgePrintContext>(_localctx);
      enterOuterAlt(_localctx, 1);
      setState(586);
      match(BaseParser::T__47);
      setState(587);
      match(BaseParser::EDGE);
      setState(588);
      match(BaseParser::OF);
      setState(589);
      graphID();
      setState(590);
      match(BaseParser::T__0);
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<BaseParser::NodePrintContext>(_localctx);
      enterOuterAlt(_localctx, 2);
      setState(592);
      match(BaseParser::T__47);
      setState(593);
      match(BaseParser::NODE);
      setState(594);
      match(BaseParser::OF);
      setState(595);
      graphID();
      setState(596);
      match(BaseParser::T__0);
      break;
    }

    case 3: {
      _localctx = _tracker.createInstance<BaseParser::GraphPrintContext>(_localctx);
      enterOuterAlt(_localctx, 3);
      setState(598);
      match(BaseParser::T__47);
      setState(599);
      match(BaseParser::GRAPH);
      setState(600);
      match(BaseParser::OF);
      setState(601);
      graphID();
      setState(602);
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
  size_t startState = 102;
  enterRecursionRule(_localctx, 102, BaseParser::RuleExpr, precedence);

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
    setState(658);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 48, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<UnaryMinusExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;

      setState(607);
      match(BaseParser::MINUS);
      setState(608);
      expr(18);
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<NotExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(609);
      match(BaseParser::NOT);
      setState(610);
      expr(17);
      break;
    }

    case 3: {
      _localctx = _tracker.createInstance<FuncExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(611);
      functionCall();
      break;
    }

    case 4: {
      _localctx = _tracker.createInstance<IntExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(612);
      match(BaseParser::INT);
      break;
    }

    case 5: {
      _localctx = _tracker.createInstance<IdExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(613);
      match(BaseParser::ID);
      break;
    }

    case 6: {
      _localctx = _tracker.createInstance<ParenExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(614);
      match(BaseParser::T__2);
      setState(615);
      expr(0);
      setState(616);
      match(BaseParser::T__4);
      break;
    }

    case 7: {
      _localctx = _tracker.createInstance<Array2DAccessExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(618);
      match(BaseParser::ID);
      setState(619);
      match(BaseParser::T__18);
      setState(620);
      expr(0);
      setState(621);
      match(BaseParser::T__20);
      setState(622);
      match(BaseParser::T__18);
      setState(623);
      expr(0);
      setState(624);
      match(BaseParser::T__20);
      break;
    }

    case 8: {
      _localctx = _tracker.createInstance<ArrayAccessExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(626);
      match(BaseParser::ID);
      setState(627);
      match(BaseParser::T__18);
      setState(628);
      expr(0);
      setState(629);
      match(BaseParser::T__20);
      break;
    }

    case 9: {
      _localctx = _tracker.createInstance<SetContainsExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(631);
      setTarget();
      setState(632);
      match(BaseParser::T__12);
      setState(633);
      match(BaseParser::T__48);
      setState(634);
      match(BaseParser::T__2);
      setState(635);
      expr(0);
      setState(636);
      match(BaseParser::T__4);
      break;
    }

    case 10: {
      _localctx = _tracker.createInstance<SetSizeExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(638);
      match(BaseParser::ID);
      setState(639);
      match(BaseParser::T__12);
      setState(640);
      match(BaseParser::T__49);
      setState(641);
      match(BaseParser::T__2);
      setState(642);
      match(BaseParser::T__4);
      break;
    }

    case 11: {
      _localctx = _tracker.createInstance<SetPopExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(643);
      match(BaseParser::ID);
      setState(644);
      match(BaseParser::T__12);
      setState(645);
      match(BaseParser::T__50);
      setState(646);
      match(BaseParser::T__2);
      setState(647);
      match(BaseParser::T__4);
      break;
    }

    case 12: {
      _localctx = _tracker.createInstance<BoolTrueExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(648);
      match(BaseParser::TRUE);
      break;
    }

    case 13: {
      _localctx = _tracker.createInstance<BoolFalseExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(649);
      match(BaseParser::FALSE);
      break;
    }

    case 14: {
      _localctx = _tracker.createInstance<ArrayPrintContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(650);
      match(BaseParser::ID);
      setState(651);
      match(BaseParser::T__51);
      break;
    }

    case 15: {
      _localctx = _tracker.createInstance<RealExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(652);
      match(BaseParser::REAL);
      break;
    }

    case 16: {
      _localctx = _tracker.createInstance<SetLitExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(653);
      setInitializer();
      break;
    }

    case 17: {
      _localctx = _tracker.createInstance<InfExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(654);
      match(BaseParser::T__52);
      break;
    }

    case 18: {
      _localctx = _tracker.createInstance<TimerExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(655);
      match(BaseParser::T__53);
      setState(656);
      match(BaseParser::T__2);
      setState(657);
      match(BaseParser::T__4);
      break;
    }

    default:
      break;
    }
    _ctx->stop = _input->LT(-1);
    setState(671);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 50, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(669);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 49, _ctx)) {
        case 1: {
          auto newContext = _tracker.createInstance<LogicalExprContext>(_tracker.createInstance<ExprContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpr);
          setState(660);

          if (!(precpred(_ctx, 21))) throw FailedPredicateException(this, "precpred(_ctx, 21)");
          setState(661);
          _la = _input->LA(1);
          if (!(_la == BaseParser::AND

          || _la == BaseParser::OR)) {
          _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(662);
          expr(22);
          break;
        }

        case 2: {
          auto newContext = _tracker.createInstance<MulDivExprContext>(_tracker.createInstance<ExprContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpr);
          setState(663);

          if (!(precpred(_ctx, 20))) throw FailedPredicateException(this, "precpred(_ctx, 20)");
          setState(664);
          _la = _input->LA(1);
          if (!(((((_la - 61) & ~ 0x3fULL) == 0) &&
            ((1ULL << (_la - 61)) & 769) != 0))) {
          _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(665);
          expr(21);
          break;
        }

        case 3: {
          auto newContext = _tracker.createInstance<AddSubExprContext>(_tracker.createInstance<ExprContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpr);
          setState(666);

          if (!(precpred(_ctx, 19))) throw FailedPredicateException(this, "precpred(_ctx, 19)");
          setState(667);
          _la = _input->LA(1);
          if (!(_la == BaseParser::PLUS

          || _la == BaseParser::MINUS)) {
          _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(668);
          expr(20);
          break;
        }

        default:
          break;
        } 
      }
      setState(673);
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
  enterRule(_localctx, 104, BaseParser::RuleArrayDeclarator);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(690);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 51, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<BaseParser::Sized2DArrayContext>(_localctx);
      enterOuterAlt(_localctx, 1);
      setState(674);
      match(BaseParser::ID);
      setState(675);
      match(BaseParser::T__18);
      setState(676);
      expr(0);
      setState(677);
      match(BaseParser::T__20);
      setState(678);
      match(BaseParser::T__18);
      setState(679);
      expr(0);
      setState(680);
      match(BaseParser::T__20);
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<BaseParser::SizedArrayContext>(_localctx);
      enterOuterAlt(_localctx, 2);
      setState(682);
      match(BaseParser::ID);
      setState(683);
      match(BaseParser::T__18);
      setState(684);
      expr(0);
      setState(685);
      match(BaseParser::T__20);
      break;
    }

    case 3: {
      _localctx = _tracker.createInstance<BaseParser::UnsizedArrayContext>(_localctx);
      enterOuterAlt(_localctx, 3);
      setState(687);
      match(BaseParser::ID);
      setState(688);
      match(BaseParser::T__18);
      setState(689);
      match(BaseParser::T__20);
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
  enterRule(_localctx, 106, BaseParser::RuleArrayInitializer);
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
    setState(692);
    match(BaseParser::T__18);
    setState(693);
    expr(0);
    setState(698);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == BaseParser::T__3) {
      setState(694);
      match(BaseParser::T__3);
      setState(695);
      expr(0);
      setState(700);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(701);
    match(BaseParser::T__20);
   
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
  enterRule(_localctx, 108, BaseParser::RuleAssignmentStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(710);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 53, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(703);
      match(BaseParser::ID);
      setState(704);
      match(BaseParser::T__11);
      setState(705);
      expr(0);
      setState(706);
      match(BaseParser::T__0);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(708);
      match(BaseParser::ID);
      setState(709);
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
  enterRule(_localctx, 110, BaseParser::RuleArrayAssignStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(731);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 54, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<BaseParser::Array2DAssignStmtContext>(_localctx);
      enterOuterAlt(_localctx, 1);
      setState(712);
      match(BaseParser::ID);
      setState(713);
      match(BaseParser::T__18);
      setState(714);
      expr(0);
      setState(715);
      match(BaseParser::T__20);
      setState(716);
      match(BaseParser::T__18);
      setState(717);
      expr(0);
      setState(718);
      match(BaseParser::T__20);
      setState(719);
      match(BaseParser::T__11);
      setState(720);
      expr(0);
      setState(721);
      match(BaseParser::T__0);
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<BaseParser::ArrayAssignStmtContext>(_localctx);
      enterOuterAlt(_localctx, 2);
      setState(723);
      match(BaseParser::ID);
      setState(724);
      match(BaseParser::T__18);
      setState(725);
      expr(0);
      setState(726);
      match(BaseParser::T__20);
      setState(727);
      match(BaseParser::T__11);
      setState(728);
      expr(0);
      setState(729);
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
  enterRule(_localctx, 112, BaseParser::RuleWeights);
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
    setState(733);
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
    case 17: return setExprSempred(antlrcpp::downCast<SetExprContext *>(context), predicateIndex);
    case 20: return conditionSempred(antlrcpp::downCast<ConditionContext *>(context), predicateIndex);
    case 23: return graphConditionSempred(antlrcpp::downCast<GraphConditionContext *>(context), predicateIndex);
    case 48: return printExprSempred(antlrcpp::downCast<PrintExprContext *>(context), predicateIndex);
    case 51: return exprSempred(antlrcpp::downCast<ExprContext *>(context), predicateIndex);

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
