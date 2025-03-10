
// Generated from Base.g4 by ANTLR 4.13.2


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
std::unique_ptr<BaseParserStaticData> baseParserStaticData = nullptr;

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
      "conditionalStatement", "condition", "graphComprehension", "graphCondition", 
      "loopStatement", "foreachStatement", "loopTarget", "whileStatement", 
      "nodeEdgeOperation", "addOperation", "removeOperation", "addTargets", 
      "removeTargets", "queryStatement", "showgraph", "function", "returnType", 
      "paramList", "param", "type", "functionCall", "argumentList", "block", 
      "returnStatement", "printStatement", "printExpr", "printgraph", "expr"
    },
    std::vector<std::string>{
      "", "';'", "'{'", "'}'", "'nodes:'", "'edges:'", "','", "'file'", 
      "'->'", "'='", "'if'", "'('", "')'", "'else'", "'in'", "'['", "'where'", 
      "']'", "'degree'", "'connected'", "'with'", "'for'", "'each'", "'vertex'", 
      "'edge'", "'neighbor'", "'while'", "'add'", "'to'", "'remove'", "'from'", 
      "'query'", "':'", "'show'", "'fn'", "'int'", "'void'", "'string'", 
      "'real'", "'return'", "'print'", "'graph'", "'edges'", "'nodes'", 
      "'of'", "'+'", "'-'", "'*'", "'/'", "'&&'", "'||'", "'=='", "'!='", 
      "'<'", "'>'", "'<='", "'>='"
    },
    std::vector<std::string>{
      "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", 
      "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", 
      "", "", "", "", "", "", "", "GRAPH", "EDGE", "NODE", "OF", "PLUS", 
      "MINUS", "TIMES", "DIVIDE", "AND", "OR", "EQUAL", "NOTEQUAL", "LESSTHAN", 
      "GREATERTHAN", "LESSEQUAL", "GREATEREQUAL", "ID", "INT", "REAL", "STRING", 
      "Comment", "WS"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,1,62,420,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,2,
  	7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,7,
  	14,2,15,7,15,2,16,7,16,2,17,7,17,2,18,7,18,2,19,7,19,2,20,7,20,2,21,7,
  	21,2,22,7,22,2,23,7,23,2,24,7,24,2,25,7,25,2,26,7,26,2,27,7,27,2,28,7,
  	28,2,29,7,29,2,30,7,30,2,31,7,31,2,32,7,32,2,33,7,33,2,34,7,34,2,35,7,
  	35,2,36,7,36,2,37,7,37,2,38,7,38,2,39,7,39,1,0,1,0,5,0,83,8,0,10,0,12,
  	0,86,9,0,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,3,1,101,
  	8,1,1,2,1,2,1,2,1,2,3,2,107,8,2,1,2,3,2,110,8,2,1,2,1,2,1,2,1,3,1,3,1,
  	3,1,3,1,4,1,4,1,4,3,4,122,8,4,1,4,1,4,1,5,1,5,1,5,5,5,129,8,5,10,5,12,
  	5,132,9,5,1,6,1,6,1,6,5,6,137,8,6,10,6,12,6,140,9,6,1,7,1,7,1,8,1,8,1,
  	9,1,9,1,9,1,10,1,10,1,10,1,10,1,11,1,11,1,11,1,11,3,11,157,8,11,1,11,
  	1,11,1,12,1,12,1,12,1,12,1,12,1,12,1,12,1,12,1,12,3,12,170,8,12,1,13,
  	1,13,1,13,1,13,1,13,1,13,1,13,1,13,1,13,1,13,1,13,1,13,1,13,3,13,185,
  	8,13,1,13,1,13,1,13,1,13,1,13,1,13,5,13,193,8,13,10,13,12,13,196,9,13,
  	1,14,1,14,1,14,1,14,1,14,1,14,1,14,1,14,1,14,1,15,1,15,1,15,1,15,1,15,
  	1,15,1,15,1,15,1,15,1,15,1,15,3,15,218,8,15,1,15,1,15,1,15,1,15,1,15,
  	1,15,5,15,226,8,15,10,15,12,15,229,9,15,1,16,1,16,3,16,233,8,16,1,17,
  	1,17,1,17,1,17,1,17,1,17,1,17,1,18,1,18,1,18,1,18,1,18,1,18,1,18,1,18,
  	1,18,1,18,3,18,252,8,18,1,19,1,19,1,19,1,19,1,19,1,19,1,20,1,20,3,20,
  	262,8,20,1,21,1,21,1,21,1,21,1,21,1,21,1,22,1,22,1,22,1,22,1,22,1,22,
  	1,23,1,23,1,23,1,23,3,23,280,8,23,1,24,1,24,1,24,1,24,3,24,286,8,24,1,
  	25,1,25,1,25,1,25,1,25,1,25,1,25,1,25,1,26,1,26,1,26,1,26,1,27,1,27,1,
  	27,1,27,1,27,1,27,1,28,1,28,1,29,1,29,1,29,1,29,5,29,312,8,29,10,29,12,
  	29,315,9,29,3,29,317,8,29,1,29,1,29,1,30,1,30,1,30,1,31,1,31,1,32,1,32,
  	1,32,3,32,329,8,32,1,32,1,32,1,33,1,33,1,33,5,33,336,8,33,10,33,12,33,
  	339,9,33,1,34,1,34,1,34,5,34,344,8,34,10,34,12,34,347,9,34,1,34,1,34,
  	1,34,3,34,352,8,34,1,35,1,35,1,35,1,35,1,36,1,36,1,36,1,36,1,36,3,36,
  	363,8,36,1,37,1,37,1,37,3,37,368,8,37,1,37,1,37,1,37,5,37,373,8,37,10,
  	37,12,37,376,9,37,1,38,1,38,1,38,1,38,1,38,1,38,1,38,1,38,1,38,1,38,1,
  	38,1,38,1,38,1,38,1,38,1,38,1,38,1,38,3,38,396,8,38,1,39,1,39,1,39,1,
  	39,1,39,1,39,1,39,1,39,1,39,3,39,407,8,39,1,39,1,39,1,39,1,39,1,39,1,
  	39,5,39,415,8,39,10,39,12,39,418,9,39,1,39,0,4,26,30,74,78,40,0,2,4,6,
  	8,10,12,14,16,18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,48,50,52,54,
  	56,58,60,62,64,66,68,70,72,74,76,78,0,5,1,0,51,56,3,0,23,24,35,37,41,
  	41,4,0,23,24,35,35,37,38,41,41,1,0,47,48,1,0,45,46,435,0,84,1,0,0,0,2,
  	100,1,0,0,0,4,102,1,0,0,0,6,114,1,0,0,0,8,118,1,0,0,0,10,125,1,0,0,0,
  	12,133,1,0,0,0,14,141,1,0,0,0,16,143,1,0,0,0,18,145,1,0,0,0,20,148,1,
  	0,0,0,22,152,1,0,0,0,24,160,1,0,0,0,26,184,1,0,0,0,28,197,1,0,0,0,30,
  	217,1,0,0,0,32,232,1,0,0,0,34,234,1,0,0,0,36,251,1,0,0,0,38,253,1,0,0,
  	0,40,261,1,0,0,0,42,263,1,0,0,0,44,269,1,0,0,0,46,279,1,0,0,0,48,285,
  	1,0,0,0,50,287,1,0,0,0,52,295,1,0,0,0,54,299,1,0,0,0,56,305,1,0,0,0,58,
  	307,1,0,0,0,60,320,1,0,0,0,62,323,1,0,0,0,64,325,1,0,0,0,66,332,1,0,0,
  	0,68,351,1,0,0,0,70,353,1,0,0,0,72,362,1,0,0,0,74,367,1,0,0,0,76,395,
  	1,0,0,0,78,406,1,0,0,0,80,83,3,2,1,0,81,83,3,54,27,0,82,80,1,0,0,0,82,
  	81,1,0,0,0,83,86,1,0,0,0,84,82,1,0,0,0,84,85,1,0,0,0,85,87,1,0,0,0,86,
  	84,1,0,0,0,87,88,5,0,0,1,88,1,1,0,0,0,89,101,3,4,2,0,90,101,3,24,12,0,
  	91,101,3,72,36,0,92,101,3,32,16,0,93,101,3,22,11,0,94,101,3,64,32,0,95,
  	101,3,28,14,0,96,101,3,50,25,0,97,101,3,52,26,0,98,101,3,40,20,0,99,101,
  	5,1,0,0,100,89,1,0,0,0,100,90,1,0,0,0,100,91,1,0,0,0,100,92,1,0,0,0,100,
  	93,1,0,0,0,100,94,1,0,0,0,100,95,1,0,0,0,100,96,1,0,0,0,100,97,1,0,0,
  	0,100,98,1,0,0,0,100,99,1,0,0,0,101,3,1,0,0,0,102,103,5,41,0,0,103,104,
  	3,14,7,0,104,106,5,2,0,0,105,107,3,6,3,0,106,105,1,0,0,0,106,107,1,0,
  	0,0,107,109,1,0,0,0,108,110,3,8,4,0,109,108,1,0,0,0,109,110,1,0,0,0,110,
  	111,1,0,0,0,111,112,5,3,0,0,112,113,5,1,0,0,113,5,1,0,0,0,114,115,5,4,
  	0,0,115,116,3,10,5,0,116,117,5,1,0,0,117,7,1,0,0,0,118,121,5,5,0,0,119,
  	122,3,12,6,0,120,122,3,18,9,0,121,119,1,0,0,0,121,120,1,0,0,0,122,123,
  	1,0,0,0,123,124,5,1,0,0,124,9,1,0,0,0,125,130,3,16,8,0,126,127,5,6,0,
  	0,127,129,3,16,8,0,128,126,1,0,0,0,129,132,1,0,0,0,130,128,1,0,0,0,130,
  	131,1,0,0,0,131,11,1,0,0,0,132,130,1,0,0,0,133,138,3,20,10,0,134,135,
  	5,6,0,0,135,137,3,20,10,0,136,134,1,0,0,0,137,140,1,0,0,0,138,136,1,0,
  	0,0,138,139,1,0,0,0,139,13,1,0,0,0,140,138,1,0,0,0,141,142,5,57,0,0,142,
  	15,1,0,0,0,143,144,5,58,0,0,144,17,1,0,0,0,145,146,5,7,0,0,146,147,5,
  	60,0,0,147,19,1,0,0,0,148,149,3,16,8,0,149,150,5,8,0,0,150,151,3,16,8,
  	0,151,21,1,0,0,0,152,153,3,62,31,0,153,156,5,57,0,0,154,155,5,9,0,0,155,
  	157,3,78,39,0,156,154,1,0,0,0,156,157,1,0,0,0,157,158,1,0,0,0,158,159,
  	5,1,0,0,159,23,1,0,0,0,160,161,5,10,0,0,161,162,5,11,0,0,162,163,3,26,
  	13,0,163,164,5,12,0,0,164,169,3,68,34,0,165,166,5,13,0,0,166,170,3,24,
  	12,0,167,168,5,13,0,0,168,170,3,68,34,0,169,165,1,0,0,0,169,167,1,0,0,
  	0,169,170,1,0,0,0,170,25,1,0,0,0,171,172,6,13,-1,0,172,173,3,78,39,0,
  	173,174,7,0,0,0,174,175,3,78,39,0,175,185,1,0,0,0,176,177,3,16,8,0,177,
  	178,5,14,0,0,178,179,3,14,7,0,179,185,1,0,0,0,180,181,3,20,10,0,181,182,
  	5,14,0,0,182,183,3,14,7,0,183,185,1,0,0,0,184,171,1,0,0,0,184,176,1,0,
  	0,0,184,180,1,0,0,0,185,194,1,0,0,0,186,187,10,5,0,0,187,188,5,49,0,0,
  	188,193,3,26,13,6,189,190,10,4,0,0,190,191,5,50,0,0,191,193,3,26,13,5,
  	192,186,1,0,0,0,192,189,1,0,0,0,193,196,1,0,0,0,194,192,1,0,0,0,194,195,
  	1,0,0,0,195,27,1,0,0,0,196,194,1,0,0,0,197,198,5,57,0,0,198,199,5,9,0,
  	0,199,200,5,15,0,0,200,201,3,14,7,0,201,202,5,16,0,0,202,203,3,30,15,
  	0,203,204,5,17,0,0,204,205,5,1,0,0,205,29,1,0,0,0,206,207,6,15,-1,0,207,
  	208,5,18,0,0,208,209,7,0,0,0,209,218,5,58,0,0,210,211,5,19,0,0,211,212,
  	5,20,0,0,212,218,3,16,8,0,213,214,5,11,0,0,214,215,3,30,15,0,215,216,
  	5,12,0,0,216,218,1,0,0,0,217,206,1,0,0,0,217,210,1,0,0,0,217,213,1,0,
  	0,0,218,227,1,0,0,0,219,220,10,5,0,0,220,221,5,49,0,0,221,226,3,30,15,
  	6,222,223,10,4,0,0,223,224,5,50,0,0,224,226,3,30,15,5,225,219,1,0,0,0,
  	225,222,1,0,0,0,226,229,1,0,0,0,227,225,1,0,0,0,227,228,1,0,0,0,228,31,
  	1,0,0,0,229,227,1,0,0,0,230,233,3,34,17,0,231,233,3,38,19,0,232,230,1,
  	0,0,0,232,231,1,0,0,0,233,33,1,0,0,0,234,235,5,21,0,0,235,236,5,22,0,
  	0,236,237,3,36,18,0,237,238,5,14,0,0,238,239,3,14,7,0,239,240,3,68,34,
  	0,240,35,1,0,0,0,241,242,5,23,0,0,242,252,5,57,0,0,243,244,5,24,0,0,244,
  	245,5,57,0,0,245,246,5,6,0,0,246,252,5,57,0,0,247,248,5,25,0,0,248,249,
  	5,57,0,0,249,250,5,44,0,0,250,252,3,16,8,0,251,241,1,0,0,0,251,243,1,
  	0,0,0,251,247,1,0,0,0,252,37,1,0,0,0,253,254,5,26,0,0,254,255,5,11,0,
  	0,255,256,3,26,13,0,256,257,5,12,0,0,257,258,3,68,34,0,258,39,1,0,0,0,
  	259,262,3,42,21,0,260,262,3,44,22,0,261,259,1,0,0,0,261,260,1,0,0,0,262,
  	41,1,0,0,0,263,264,5,27,0,0,264,265,3,46,23,0,265,266,5,28,0,0,266,267,
  	3,14,7,0,267,268,5,1,0,0,268,43,1,0,0,0,269,270,5,29,0,0,270,271,3,48,
  	24,0,271,272,5,30,0,0,272,273,3,14,7,0,273,274,5,1,0,0,274,45,1,0,0,0,
  	275,280,3,16,8,0,276,280,3,20,10,0,277,280,3,10,5,0,278,280,3,12,6,0,
  	279,275,1,0,0,0,279,276,1,0,0,0,279,277,1,0,0,0,279,278,1,0,0,0,280,47,
  	1,0,0,0,281,286,3,16,8,0,282,286,3,20,10,0,283,286,3,10,5,0,284,286,3,
  	12,6,0,285,281,1,0,0,0,285,282,1,0,0,0,285,283,1,0,0,0,285,284,1,0,0,
  	0,286,49,1,0,0,0,287,288,5,31,0,0,288,289,5,57,0,0,289,290,5,32,0,0,290,
  	291,5,60,0,0,291,292,5,44,0,0,292,293,3,14,7,0,293,294,5,1,0,0,294,51,
  	1,0,0,0,295,296,5,33,0,0,296,297,3,14,7,0,297,298,5,1,0,0,298,53,1,0,
  	0,0,299,300,5,34,0,0,300,301,3,56,28,0,301,302,5,57,0,0,302,303,3,58,
  	29,0,303,304,3,68,34,0,304,55,1,0,0,0,305,306,7,1,0,0,306,57,1,0,0,0,
  	307,316,5,11,0,0,308,313,3,60,30,0,309,310,5,6,0,0,310,312,3,60,30,0,
  	311,309,1,0,0,0,312,315,1,0,0,0,313,311,1,0,0,0,313,314,1,0,0,0,314,317,
  	1,0,0,0,315,313,1,0,0,0,316,308,1,0,0,0,316,317,1,0,0,0,317,318,1,0,0,
  	0,318,319,5,12,0,0,319,59,1,0,0,0,320,321,3,62,31,0,321,322,5,57,0,0,
  	322,61,1,0,0,0,323,324,7,2,0,0,324,63,1,0,0,0,325,326,5,57,0,0,326,328,
  	5,11,0,0,327,329,3,66,33,0,328,327,1,0,0,0,328,329,1,0,0,0,329,330,1,
  	0,0,0,330,331,5,12,0,0,331,65,1,0,0,0,332,337,3,78,39,0,333,334,5,6,0,
  	0,334,336,3,78,39,0,335,333,1,0,0,0,336,339,1,0,0,0,337,335,1,0,0,0,337,
  	338,1,0,0,0,338,67,1,0,0,0,339,337,1,0,0,0,340,345,5,2,0,0,341,344,3,
  	2,1,0,342,344,3,70,35,0,343,341,1,0,0,0,343,342,1,0,0,0,344,347,1,0,0,
  	0,345,343,1,0,0,0,345,346,1,0,0,0,346,348,1,0,0,0,347,345,1,0,0,0,348,
  	352,5,3,0,0,349,350,5,2,0,0,350,352,5,3,0,0,351,340,1,0,0,0,351,349,1,
  	0,0,0,352,69,1,0,0,0,353,354,5,39,0,0,354,355,3,78,39,0,355,356,5,1,0,
  	0,356,71,1,0,0,0,357,358,5,40,0,0,358,359,3,74,37,0,359,360,5,1,0,0,360,
  	363,1,0,0,0,361,363,3,76,38,0,362,357,1,0,0,0,362,361,1,0,0,0,363,73,
  	1,0,0,0,364,365,6,37,-1,0,365,368,5,60,0,0,366,368,3,78,39,0,367,364,
  	1,0,0,0,367,366,1,0,0,0,368,374,1,0,0,0,369,370,10,1,0,0,370,371,5,45,
  	0,0,371,373,3,74,37,2,372,369,1,0,0,0,373,376,1,0,0,0,374,372,1,0,0,0,
  	374,375,1,0,0,0,375,75,1,0,0,0,376,374,1,0,0,0,377,378,5,40,0,0,378,379,
  	5,42,0,0,379,380,5,44,0,0,380,381,3,14,7,0,381,382,5,1,0,0,382,396,1,
  	0,0,0,383,384,5,40,0,0,384,385,5,43,0,0,385,386,5,44,0,0,386,387,3,14,
  	7,0,387,388,5,1,0,0,388,396,1,0,0,0,389,390,5,40,0,0,390,391,5,41,0,0,
  	391,392,5,44,0,0,392,393,3,14,7,0,393,394,5,1,0,0,394,396,1,0,0,0,395,
  	377,1,0,0,0,395,383,1,0,0,0,395,389,1,0,0,0,396,77,1,0,0,0,397,398,6,
  	39,-1,0,398,407,3,64,32,0,399,407,5,58,0,0,400,407,5,57,0,0,401,402,5,
  	11,0,0,402,403,3,78,39,0,403,404,5,12,0,0,404,407,1,0,0,0,405,407,5,59,
  	0,0,406,397,1,0,0,0,406,399,1,0,0,0,406,400,1,0,0,0,406,401,1,0,0,0,406,
  	405,1,0,0,0,407,416,1,0,0,0,408,409,10,7,0,0,409,410,7,3,0,0,410,415,
  	3,78,39,8,411,412,10,6,0,0,412,413,7,4,0,0,413,415,3,78,39,7,414,408,
  	1,0,0,0,414,411,1,0,0,0,415,418,1,0,0,0,416,414,1,0,0,0,416,417,1,0,0,
  	0,417,79,1,0,0,0,418,416,1,0,0,0,35,82,84,100,106,109,121,130,138,156,
  	169,184,192,194,217,225,227,232,251,261,279,285,313,316,328,337,343,345,
  	351,362,367,374,395,406,414,416
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  baseParserStaticData = std::move(staticData);
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
    setState(84);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 144118961970086914) != 0)) {
      setState(82);
      _errHandler->sync(this);
      switch (_input->LA(1)) {
        case BaseParser::T__0:
        case BaseParser::T__9:
        case BaseParser::T__20:
        case BaseParser::T__22:
        case BaseParser::T__23:
        case BaseParser::T__25:
        case BaseParser::T__26:
        case BaseParser::T__28:
        case BaseParser::T__30:
        case BaseParser::T__32:
        case BaseParser::T__34:
        case BaseParser::T__36:
        case BaseParser::T__37:
        case BaseParser::T__39:
        case BaseParser::GRAPH:
        case BaseParser::ID: {
          setState(80);
          statement();
          break;
        }

        case BaseParser::T__33: {
          setState(81);
          function();
          break;
        }

      default:
        throw NoViableAltException(this);
      }
      setState(86);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(87);
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

BaseParser::LoopStatementContext* BaseParser::StatementContext::loopStatement() {
  return getRuleContext<BaseParser::LoopStatementContext>(0);
}

BaseParser::VarDeclContext* BaseParser::StatementContext::varDecl() {
  return getRuleContext<BaseParser::VarDeclContext>(0);
}

BaseParser::FunctionCallContext* BaseParser::StatementContext::functionCall() {
  return getRuleContext<BaseParser::FunctionCallContext>(0);
}

BaseParser::GraphComprehensionContext* BaseParser::StatementContext::graphComprehension() {
  return getRuleContext<BaseParser::GraphComprehensionContext>(0);
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


size_t BaseParser::StatementContext::getRuleIndex() const {
  return BaseParser::RuleStatement;
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
    setState(100);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 2, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(89);
      graphDef();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(90);
      conditionalStatement();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(91);
      printStatement();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(92);
      loopStatement();
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(93);
      varDecl();
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(94);
      functionCall();
      break;
    }

    case 7: {
      enterOuterAlt(_localctx, 7);
      setState(95);
      graphComprehension();
      break;
    }

    case 8: {
      enterOuterAlt(_localctx, 8);
      setState(96);
      queryStatement();
      break;
    }

    case 9: {
      enterOuterAlt(_localctx, 9);
      setState(97);
      showgraph();
      break;
    }

    case 10: {
      enterOuterAlt(_localctx, 10);
      setState(98);
      nodeEdgeOperation();
      break;
    }

    case 11: {
      enterOuterAlt(_localctx, 11);
      setState(99);
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

tree::TerminalNode* BaseParser::GraphDefContext::GRAPH() {
  return getToken(BaseParser::GRAPH, 0);
}

BaseParser::GraphIDContext* BaseParser::GraphDefContext::graphID() {
  return getRuleContext<BaseParser::GraphIDContext>(0);
}

BaseParser::NodesContext* BaseParser::GraphDefContext::nodes() {
  return getRuleContext<BaseParser::NodesContext>(0);
}

BaseParser::EdgesContext* BaseParser::GraphDefContext::edges() {
  return getRuleContext<BaseParser::EdgesContext>(0);
}


size_t BaseParser::GraphDefContext::getRuleIndex() const {
  return BaseParser::RuleGraphDef;
}


std::any BaseParser::GraphDefContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitGraphDef(this);
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
    enterOuterAlt(_localctx, 1);
    setState(102);
    match(BaseParser::GRAPH);
    setState(103);
    graphID();
    setState(104);
    match(BaseParser::T__1);
    setState(106);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == BaseParser::T__3) {
      setState(105);
      nodes();
    }
    setState(109);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == BaseParser::T__4) {
      setState(108);
      edges();
    }
    setState(111);
    match(BaseParser::T__2);
    setState(112);
    match(BaseParser::T__0);
   
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
    setState(114);
    match(BaseParser::T__3);
    setState(115);
    nodeList();
    setState(116);
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
    setState(118);
    match(BaseParser::T__4);
    setState(121);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case BaseParser::INT: {
        setState(119);
        edgeList();
        break;
      }

      case BaseParser::T__6: {
        setState(120);
        fileEdgeList();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    setState(123);
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
    setState(125);
    nodeID();
    setState(130);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == BaseParser::T__5) {
      setState(126);
      match(BaseParser::T__5);
      setState(127);
      nodeID();
      setState(132);
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
    setState(133);
    edge();
    setState(138);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == BaseParser::T__5) {
      setState(134);
      match(BaseParser::T__5);
      setState(135);
      edge();
      setState(140);
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
    setState(141);
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
    setState(143);
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
    setState(145);
    match(BaseParser::T__6);
    setState(146);
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
    setState(148);
    nodeID();
    setState(149);
    match(BaseParser::T__7);
    setState(150);
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

BaseParser::TypeContext* BaseParser::VarDeclContext::type() {
  return getRuleContext<BaseParser::TypeContext>(0);
}

tree::TerminalNode* BaseParser::VarDeclContext::ID() {
  return getToken(BaseParser::ID, 0);
}

BaseParser::ExprContext* BaseParser::VarDeclContext::expr() {
  return getRuleContext<BaseParser::ExprContext>(0);
}


size_t BaseParser::VarDeclContext::getRuleIndex() const {
  return BaseParser::RuleVarDecl;
}


std::any BaseParser::VarDeclContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitVarDecl(this);
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
    enterOuterAlt(_localctx, 1);
    setState(152);
    type();
    setState(153);
    match(BaseParser::ID);
    setState(156);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == BaseParser::T__8) {
      setState(154);
      match(BaseParser::T__8);
      setState(155);
      expr(0);
    }
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


std::any BaseParser::ConditionalStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitConditionalStatement(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::ConditionalStatementContext* BaseParser::conditionalStatement() {
  ConditionalStatementContext *_localctx = _tracker.createInstance<ConditionalStatementContext>(_ctx, getState());
  enterRule(_localctx, 24, BaseParser::RuleConditionalStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(160);
    match(BaseParser::T__9);
    setState(161);
    match(BaseParser::T__10);
    setState(162);
    condition(0);
    setState(163);
    match(BaseParser::T__11);
    setState(164);
    block();
    setState(169);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 9, _ctx)) {
    case 1: {
      setState(165);
      match(BaseParser::T__12);
      setState(166);
      conditionalStatement();
      break;
    }

    case 2: {
      setState(167);
      match(BaseParser::T__12);
      setState(168);
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

//----------------- NodeCheckContext ------------------------------------------------------------------

BaseParser::NodeIDContext* BaseParser::NodeCheckContext::nodeID() {
  return getRuleContext<BaseParser::NodeIDContext>(0);
}

BaseParser::GraphIDContext* BaseParser::NodeCheckContext::graphID() {
  return getRuleContext<BaseParser::GraphIDContext>(0);
}

BaseParser::NodeCheckContext::NodeCheckContext(ConditionContext *ctx) { copyFrom(ctx); }


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
  size_t startState = 26;
  enterRecursionRule(_localctx, 26, BaseParser::RuleCondition, precedence);

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
    setState(184);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 10, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<RelationalContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;

      setState(172);
      expr(0);
      setState(173);
      _la = _input->LA(1);
      if (!((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 141863388262170624) != 0))) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(174);
      expr(0);
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<NodeCheckContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(176);
      nodeID();
      setState(177);
      match(BaseParser::T__13);
      setState(178);
      graphID();
      break;
    }

    case 3: {
      _localctx = _tracker.createInstance<EdgeCheckContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(180);
      edge();
      setState(181);
      match(BaseParser::T__13);
      setState(182);
      graphID();
      break;
    }

    default:
      break;
    }
    _ctx->stop = _input->LT(-1);
    setState(194);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 12, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(192);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 11, _ctx)) {
        case 1: {
          auto newContext = _tracker.createInstance<LogicalAndContext>(_tracker.createInstance<ConditionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleCondition);
          setState(186);

          if (!(precpred(_ctx, 5))) throw FailedPredicateException(this, "precpred(_ctx, 5)");
          setState(187);
          match(BaseParser::AND);
          setState(188);
          condition(6);
          break;
        }

        case 2: {
          auto newContext = _tracker.createInstance<LogicalOrContext>(_tracker.createInstance<ConditionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleCondition);
          setState(189);

          if (!(precpred(_ctx, 4))) throw FailedPredicateException(this, "precpred(_ctx, 4)");
          setState(190);
          match(BaseParser::OR);
          setState(191);
          condition(5);
          break;
        }

        default:
          break;
        } 
      }
      setState(196);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 12, _ctx);
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

BaseParser::GraphIDContext* BaseParser::GraphComprehensionContext::graphID() {
  return getRuleContext<BaseParser::GraphIDContext>(0);
}

BaseParser::GraphConditionContext* BaseParser::GraphComprehensionContext::graphCondition() {
  return getRuleContext<BaseParser::GraphConditionContext>(0);
}


size_t BaseParser::GraphComprehensionContext::getRuleIndex() const {
  return BaseParser::RuleGraphComprehension;
}


std::any BaseParser::GraphComprehensionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitGraphComprehension(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::GraphComprehensionContext* BaseParser::graphComprehension() {
  GraphComprehensionContext *_localctx = _tracker.createInstance<GraphComprehensionContext>(_ctx, getState());
  enterRule(_localctx, 28, BaseParser::RuleGraphComprehension);

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
    match(BaseParser::ID);
    setState(198);
    match(BaseParser::T__8);
    setState(199);
    match(BaseParser::T__14);
    setState(200);
    graphID();
    setState(201);
    match(BaseParser::T__15);
    setState(202);
    graphCondition(0);
    setState(203);
    match(BaseParser::T__16);
    setState(204);
    match(BaseParser::T__0);
   
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


std::any BaseParser::DegreeConditionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitDegreeCondition(this);
  else
    return visitor->visitChildren(this);
}
//----------------- ConnectedConditionContext ------------------------------------------------------------------

BaseParser::NodeIDContext* BaseParser::ConnectedConditionContext::nodeID() {
  return getRuleContext<BaseParser::NodeIDContext>(0);
}

BaseParser::ConnectedConditionContext::ConnectedConditionContext(GraphConditionContext *ctx) { copyFrom(ctx); }


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
  size_t startState = 30;
  enterRecursionRule(_localctx, 30, BaseParser::RuleGraphCondition, precedence);

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
    setState(217);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case BaseParser::T__17: {
        _localctx = _tracker.createInstance<DegreeConditionContext>(_localctx);
        _ctx = _localctx;
        previousContext = _localctx;

        setState(207);
        match(BaseParser::T__17);
        setState(208);
        _la = _input->LA(1);
        if (!((((_la & ~ 0x3fULL) == 0) &&
          ((1ULL << _la) & 141863388262170624) != 0))) {
        _errHandler->recoverInline(this);
        }
        else {
          _errHandler->reportMatch(this);
          consume();
        }
        setState(209);
        match(BaseParser::INT);
        break;
      }

      case BaseParser::T__18: {
        _localctx = _tracker.createInstance<ConnectedConditionContext>(_localctx);
        _ctx = _localctx;
        previousContext = _localctx;
        setState(210);
        match(BaseParser::T__18);
        setState(211);
        match(BaseParser::T__19);
        setState(212);
        nodeID();
        break;
      }

      case BaseParser::T__10: {
        _localctx = _tracker.createInstance<ParenGraphConditionContext>(_localctx);
        _ctx = _localctx;
        previousContext = _localctx;
        setState(213);
        match(BaseParser::T__10);
        setState(214);
        graphCondition(0);
        setState(215);
        match(BaseParser::T__11);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    _ctx->stop = _input->LT(-1);
    setState(227);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 15, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(225);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 14, _ctx)) {
        case 1: {
          auto newContext = _tracker.createInstance<GraphLogicalAndContext>(_tracker.createInstance<GraphConditionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleGraphCondition);
          setState(219);

          if (!(precpred(_ctx, 5))) throw FailedPredicateException(this, "precpred(_ctx, 5)");
          setState(220);
          match(BaseParser::AND);
          setState(221);
          graphCondition(6);
          break;
        }

        case 2: {
          auto newContext = _tracker.createInstance<GraphLogicalOrContext>(_tracker.createInstance<GraphConditionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleGraphCondition);
          setState(222);

          if (!(precpred(_ctx, 4))) throw FailedPredicateException(this, "precpred(_ctx, 4)");
          setState(223);
          match(BaseParser::OR);
          setState(224);
          graphCondition(5);
          break;
        }

        default:
          break;
        } 
      }
      setState(229);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 15, _ctx);
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


std::any BaseParser::LoopStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitLoopStatement(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::LoopStatementContext* BaseParser::loopStatement() {
  LoopStatementContext *_localctx = _tracker.createInstance<LoopStatementContext>(_ctx, getState());
  enterRule(_localctx, 32, BaseParser::RuleLoopStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(232);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case BaseParser::T__20: {
        enterOuterAlt(_localctx, 1);
        setState(230);
        foreachStatement();
        break;
      }

      case BaseParser::T__25: {
        enterOuterAlt(_localctx, 2);
        setState(231);
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


std::any BaseParser::ForeachStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitForeachStatement(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::ForeachStatementContext* BaseParser::foreachStatement() {
  ForeachStatementContext *_localctx = _tracker.createInstance<ForeachStatementContext>(_ctx, getState());
  enterRule(_localctx, 34, BaseParser::RuleForeachStatement);

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
    match(BaseParser::T__20);
    setState(235);
    match(BaseParser::T__21);
    setState(236);
    loopTarget();
    setState(237);
    match(BaseParser::T__13);
    setState(238);
    graphID();
    setState(239);
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

BaseParser::NodeIDContext* BaseParser::ForEachAdjContext::nodeID() {
  return getRuleContext<BaseParser::NodeIDContext>(0);
}

BaseParser::ForEachAdjContext::ForEachAdjContext(LoopTargetContext *ctx) { copyFrom(ctx); }


std::any BaseParser::ForEachAdjContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitForEachAdj(this);
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


std::any BaseParser::ForEachEdgeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitForEachEdge(this);
  else
    return visitor->visitChildren(this);
}
BaseParser::LoopTargetContext* BaseParser::loopTarget() {
  LoopTargetContext *_localctx = _tracker.createInstance<LoopTargetContext>(_ctx, getState());
  enterRule(_localctx, 36, BaseParser::RuleLoopTarget);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(251);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case BaseParser::T__22: {
        _localctx = _tracker.createInstance<BaseParser::ForEachVertexContext>(_localctx);
        enterOuterAlt(_localctx, 1);
        setState(241);
        match(BaseParser::T__22);
        setState(242);
        match(BaseParser::ID);
        break;
      }

      case BaseParser::T__23: {
        _localctx = _tracker.createInstance<BaseParser::ForEachEdgeContext>(_localctx);
        enterOuterAlt(_localctx, 2);
        setState(243);
        match(BaseParser::T__23);
        setState(244);
        match(BaseParser::ID);
        setState(245);
        match(BaseParser::T__5);
        setState(246);
        match(BaseParser::ID);
        break;
      }

      case BaseParser::T__24: {
        _localctx = _tracker.createInstance<BaseParser::ForEachAdjContext>(_localctx);
        enterOuterAlt(_localctx, 3);
        setState(247);
        match(BaseParser::T__24);
        setState(248);
        match(BaseParser::ID);
        setState(249);
        match(BaseParser::OF);
        setState(250);
        nodeID();
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


std::any BaseParser::WhileStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitWhileStatement(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::WhileStatementContext* BaseParser::whileStatement() {
  WhileStatementContext *_localctx = _tracker.createInstance<WhileStatementContext>(_ctx, getState());
  enterRule(_localctx, 38, BaseParser::RuleWhileStatement);

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
    match(BaseParser::T__25);
    setState(254);
    match(BaseParser::T__10);
    setState(255);
    condition(0);
    setState(256);
    match(BaseParser::T__11);
    setState(257);
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


std::any BaseParser::NodeEdgeOperationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitNodeEdgeOperation(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::NodeEdgeOperationContext* BaseParser::nodeEdgeOperation() {
  NodeEdgeOperationContext *_localctx = _tracker.createInstance<NodeEdgeOperationContext>(_ctx, getState());
  enterRule(_localctx, 40, BaseParser::RuleNodeEdgeOperation);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(261);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case BaseParser::T__26: {
        enterOuterAlt(_localctx, 1);
        setState(259);
        addOperation();
        break;
      }

      case BaseParser::T__28: {
        enterOuterAlt(_localctx, 2);
        setState(260);
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


std::any BaseParser::AddOperationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitAddOperation(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::AddOperationContext* BaseParser::addOperation() {
  AddOperationContext *_localctx = _tracker.createInstance<AddOperationContext>(_ctx, getState());
  enterRule(_localctx, 42, BaseParser::RuleAddOperation);

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
    match(BaseParser::T__26);
    setState(264);
    addTargets();
    setState(265);
    match(BaseParser::T__27);
    setState(266);
    graphID();
    setState(267);
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


std::any BaseParser::RemoveOperationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitRemoveOperation(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::RemoveOperationContext* BaseParser::removeOperation() {
  RemoveOperationContext *_localctx = _tracker.createInstance<RemoveOperationContext>(_ctx, getState());
  enterRule(_localctx, 44, BaseParser::RuleRemoveOperation);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(269);
    match(BaseParser::T__28);
    setState(270);
    removeTargets();
    setState(271);
    match(BaseParser::T__29);
    setState(272);
    graphID();
    setState(273);
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


std::any BaseParser::AddTargetsContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitAddTargets(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::AddTargetsContext* BaseParser::addTargets() {
  AddTargetsContext *_localctx = _tracker.createInstance<AddTargetsContext>(_ctx, getState());
  enterRule(_localctx, 46, BaseParser::RuleAddTargets);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(279);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 19, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(275);
      nodeID();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(276);
      edge();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(277);
      nodeList();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(278);
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


std::any BaseParser::RemoveTargetsContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitRemoveTargets(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::RemoveTargetsContext* BaseParser::removeTargets() {
  RemoveTargetsContext *_localctx = _tracker.createInstance<RemoveTargetsContext>(_ctx, getState());
  enterRule(_localctx, 48, BaseParser::RuleRemoveTargets);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(285);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 20, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(281);
      nodeID();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(282);
      edge();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(283);
      nodeList();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(284);
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


size_t BaseParser::QueryStatementContext::getRuleIndex() const {
  return BaseParser::RuleQueryStatement;
}


std::any BaseParser::QueryStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitQueryStatement(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::QueryStatementContext* BaseParser::queryStatement() {
  QueryStatementContext *_localctx = _tracker.createInstance<QueryStatementContext>(_ctx, getState());
  enterRule(_localctx, 50, BaseParser::RuleQueryStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(287);
    match(BaseParser::T__30);
    setState(288);
    match(BaseParser::ID);
    setState(289);
    match(BaseParser::T__31);
    setState(290);
    match(BaseParser::STRING);
    setState(291);
    match(BaseParser::OF);
    setState(292);
    graphID();
    setState(293);
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


std::any BaseParser::ShowgraphContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitShowgraph(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::ShowgraphContext* BaseParser::showgraph() {
  ShowgraphContext *_localctx = _tracker.createInstance<ShowgraphContext>(_ctx, getState());
  enterRule(_localctx, 52, BaseParser::RuleShowgraph);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(295);
    match(BaseParser::T__32);
    setState(296);
    graphID();
    setState(297);
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


std::any BaseParser::FunctionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitFunction(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::FunctionContext* BaseParser::function() {
  FunctionContext *_localctx = _tracker.createInstance<FunctionContext>(_ctx, getState());
  enterRule(_localctx, 54, BaseParser::RuleFunction);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(299);
    match(BaseParser::T__33);
    setState(300);
    returnType();
    setState(301);
    match(BaseParser::ID);
    setState(302);
    paramList();
    setState(303);
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


std::any BaseParser::ReturnTypeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitReturnType(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::ReturnTypeContext* BaseParser::returnType() {
  ReturnTypeContext *_localctx = _tracker.createInstance<ReturnTypeContext>(_ctx, getState());
  enterRule(_localctx, 56, BaseParser::RuleReturnType);
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
    setState(305);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 2439566589952) != 0))) {
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


std::any BaseParser::ParamListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitParamList(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::ParamListContext* BaseParser::paramList() {
  ParamListContext *_localctx = _tracker.createInstance<ParamListContext>(_ctx, getState());
  enterRule(_localctx, 58, BaseParser::RuleParamList);
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
    setState(307);
    match(BaseParser::T__10);
    setState(316);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 2645725020160) != 0)) {
      setState(308);
      param();
      setState(313);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == BaseParser::T__5) {
        setState(309);
        match(BaseParser::T__5);
        setState(310);
        param();
        setState(315);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
    }
    setState(318);
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


std::any BaseParser::ParamContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitParam(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::ParamContext* BaseParser::param() {
  ParamContext *_localctx = _tracker.createInstance<ParamContext>(_ctx, getState());
  enterRule(_localctx, 60, BaseParser::RuleParam);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(320);
    type();
    setState(321);
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


std::any BaseParser::TypeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitType(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::TypeContext* BaseParser::type() {
  TypeContext *_localctx = _tracker.createInstance<TypeContext>(_ctx, getState());
  enterRule(_localctx, 62, BaseParser::RuleType);
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
    setState(323);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 2645725020160) != 0))) {
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


std::any BaseParser::FunctionCallContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitFunctionCall(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::FunctionCallContext* BaseParser::functionCall() {
  FunctionCallContext *_localctx = _tracker.createInstance<FunctionCallContext>(_ctx, getState());
  enterRule(_localctx, 64, BaseParser::RuleFunctionCall);
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
    setState(325);
    match(BaseParser::ID);
    setState(326);
    match(BaseParser::T__10);
    setState(328);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 1008806316530993152) != 0)) {
      setState(327);
      argumentList();
    }
    setState(330);
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


std::any BaseParser::ArgumentListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitArgumentList(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::ArgumentListContext* BaseParser::argumentList() {
  ArgumentListContext *_localctx = _tracker.createInstance<ArgumentListContext>(_ctx, getState());
  enterRule(_localctx, 66, BaseParser::RuleArgumentList);
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
    setState(332);
    expr(0);
    setState(337);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == BaseParser::T__5) {
      setState(333);
      match(BaseParser::T__5);
      setState(334);
      expr(0);
      setState(339);
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


std::any BaseParser::BlockContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitBlock(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::BlockContext* BaseParser::block() {
  BlockContext *_localctx = _tracker.createInstance<BlockContext>(_ctx, getState());
  enterRule(_localctx, 68, BaseParser::RuleBlock);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(351);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 27, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(340);
      match(BaseParser::T__1);
      setState(345);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while ((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 144119494546031618) != 0)) {
        setState(343);
        _errHandler->sync(this);
        switch (_input->LA(1)) {
          case BaseParser::T__0:
          case BaseParser::T__9:
          case BaseParser::T__20:
          case BaseParser::T__22:
          case BaseParser::T__23:
          case BaseParser::T__25:
          case BaseParser::T__26:
          case BaseParser::T__28:
          case BaseParser::T__30:
          case BaseParser::T__32:
          case BaseParser::T__34:
          case BaseParser::T__36:
          case BaseParser::T__37:
          case BaseParser::T__39:
          case BaseParser::GRAPH:
          case BaseParser::ID: {
            setState(341);
            statement();
            break;
          }

          case BaseParser::T__38: {
            setState(342);
            returnStatement();
            break;
          }

        default:
          throw NoViableAltException(this);
        }
        setState(347);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
      setState(348);
      match(BaseParser::T__2);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(349);
      match(BaseParser::T__1);
      setState(350);
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


std::any BaseParser::ReturnStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitReturnStatement(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::ReturnStatementContext* BaseParser::returnStatement() {
  ReturnStatementContext *_localctx = _tracker.createInstance<ReturnStatementContext>(_ctx, getState());
  enterRule(_localctx, 70, BaseParser::RuleReturnStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(353);
    match(BaseParser::T__38);
    setState(354);
    expr(0);
    setState(355);
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

BaseParser::PrintgraphContext* BaseParser::PrintStatementContext::printgraph() {
  return getRuleContext<BaseParser::PrintgraphContext>(0);
}


size_t BaseParser::PrintStatementContext::getRuleIndex() const {
  return BaseParser::RulePrintStatement;
}


std::any BaseParser::PrintStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitPrintStatement(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::PrintStatementContext* BaseParser::printStatement() {
  PrintStatementContext *_localctx = _tracker.createInstance<PrintStatementContext>(_ctx, getState());
  enterRule(_localctx, 72, BaseParser::RulePrintStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(362);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 28, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(357);
      match(BaseParser::T__39);
      setState(358);
      printExpr(0);
      setState(359);
      match(BaseParser::T__0);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(361);
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
  size_t startState = 74;
  enterRecursionRule(_localctx, 74, BaseParser::RulePrintExpr, precedence);

    

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
    setState(367);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case BaseParser::STRING: {
        setState(365);
        match(BaseParser::STRING);
        break;
      }

      case BaseParser::T__10:
      case BaseParser::ID:
      case BaseParser::INT:
      case BaseParser::REAL: {
        setState(366);
        expr(0);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    _ctx->stop = _input->LT(-1);
    setState(374);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 30, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        _localctx = _tracker.createInstance<PrintExprContext>(parentContext, parentState);
        pushNewRecursionContext(_localctx, startState, RulePrintExpr);
        setState(369);

        if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
        setState(370);
        match(BaseParser::PLUS);
        setState(371);
        printExpr(2); 
      }
      setState(376);
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


std::any BaseParser::EdgePrintContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitEdgePrint(this);
  else
    return visitor->visitChildren(this);
}
BaseParser::PrintgraphContext* BaseParser::printgraph() {
  PrintgraphContext *_localctx = _tracker.createInstance<PrintgraphContext>(_ctx, getState());
  enterRule(_localctx, 76, BaseParser::RulePrintgraph);

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
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 31, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<BaseParser::EdgePrintContext>(_localctx);
      enterOuterAlt(_localctx, 1);
      setState(377);
      match(BaseParser::T__39);
      setState(378);
      match(BaseParser::EDGE);
      setState(379);
      match(BaseParser::OF);
      setState(380);
      graphID();
      setState(381);
      match(BaseParser::T__0);
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<BaseParser::NodePrintContext>(_localctx);
      enterOuterAlt(_localctx, 2);
      setState(383);
      match(BaseParser::T__39);
      setState(384);
      match(BaseParser::NODE);
      setState(385);
      match(BaseParser::OF);
      setState(386);
      graphID();
      setState(387);
      match(BaseParser::T__0);
      break;
    }

    case 3: {
      _localctx = _tracker.createInstance<BaseParser::GraphPrintContext>(_localctx);
      enterOuterAlt(_localctx, 3);
      setState(389);
      match(BaseParser::T__39);
      setState(390);
      match(BaseParser::GRAPH);
      setState(391);
      match(BaseParser::OF);
      setState(392);
      graphID();
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

BaseParser::MulDivExprContext::MulDivExprContext(ExprContext *ctx) { copyFrom(ctx); }


std::any BaseParser::MulDivExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitMulDivExpr(this);
  else
    return visitor->visitChildren(this);
}
//----------------- IdExprContext ------------------------------------------------------------------

tree::TerminalNode* BaseParser::IdExprContext::ID() {
  return getToken(BaseParser::ID, 0);
}

BaseParser::IdExprContext::IdExprContext(ExprContext *ctx) { copyFrom(ctx); }


std::any BaseParser::IdExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitIdExpr(this);
  else
    return visitor->visitChildren(this);
}
//----------------- IntExprContext ------------------------------------------------------------------

tree::TerminalNode* BaseParser::IntExprContext::INT() {
  return getToken(BaseParser::INT, 0);
}

BaseParser::IntExprContext::IntExprContext(ExprContext *ctx) { copyFrom(ctx); }


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


std::any BaseParser::ParenExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitParenExpr(this);
  else
    return visitor->visitChildren(this);
}
//----------------- FuncExprContext ------------------------------------------------------------------

BaseParser::FunctionCallContext* BaseParser::FuncExprContext::functionCall() {
  return getRuleContext<BaseParser::FunctionCallContext>(0);
}

BaseParser::FuncExprContext::FuncExprContext(ExprContext *ctx) { copyFrom(ctx); }


std::any BaseParser::FuncExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitFuncExpr(this);
  else
    return visitor->visitChildren(this);
}
//----------------- RealExprContext ------------------------------------------------------------------

tree::TerminalNode* BaseParser::RealExprContext::REAL() {
  return getToken(BaseParser::REAL, 0);
}

BaseParser::RealExprContext::RealExprContext(ExprContext *ctx) { copyFrom(ctx); }


std::any BaseParser::RealExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitRealExpr(this);
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
  size_t startState = 78;
  enterRecursionRule(_localctx, 78, BaseParser::RuleExpr, precedence);

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
    setState(406);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 32, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<FuncExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;

      setState(398);
      functionCall();
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<IntExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(399);
      match(BaseParser::INT);
      break;
    }

    case 3: {
      _localctx = _tracker.createInstance<IdExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(400);
      match(BaseParser::ID);
      break;
    }

    case 4: {
      _localctx = _tracker.createInstance<ParenExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(401);
      match(BaseParser::T__10);
      setState(402);
      expr(0);
      setState(403);
      match(BaseParser::T__11);
      break;
    }

    case 5: {
      _localctx = _tracker.createInstance<RealExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(405);
      match(BaseParser::REAL);
      break;
    }

    default:
      break;
    }
    _ctx->stop = _input->LT(-1);
    setState(416);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 34, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(414);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 33, _ctx)) {
        case 1: {
          auto newContext = _tracker.createInstance<MulDivExprContext>(_tracker.createInstance<ExprContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpr);
          setState(408);

          if (!(precpred(_ctx, 7))) throw FailedPredicateException(this, "precpred(_ctx, 7)");
          setState(409);
          _la = _input->LA(1);
          if (!(_la == BaseParser::TIMES

          || _la == BaseParser::DIVIDE)) {
          _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(410);
          expr(8);
          break;
        }

        case 2: {
          auto newContext = _tracker.createInstance<AddSubExprContext>(_tracker.createInstance<ExprContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpr);
          setState(411);

          if (!(precpred(_ctx, 6))) throw FailedPredicateException(this, "precpred(_ctx, 6)");
          setState(412);
          _la = _input->LA(1);
          if (!(_la == BaseParser::PLUS

          || _la == BaseParser::MINUS)) {
          _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(413);
          expr(7);
          break;
        }

        default:
          break;
        } 
      }
      setState(418);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 34, _ctx);
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
    case 13: return conditionSempred(antlrcpp::downCast<ConditionContext *>(context), predicateIndex);
    case 15: return graphConditionSempred(antlrcpp::downCast<GraphConditionContext *>(context), predicateIndex);
    case 37: return printExprSempred(antlrcpp::downCast<PrintExprContext *>(context), predicateIndex);
    case 39: return exprSempred(antlrcpp::downCast<ExprContext *>(context), predicateIndex);

  default:
    break;
  }
  return true;
}

bool BaseParser::conditionSempred(ConditionContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 0: return precpred(_ctx, 5);
    case 1: return precpred(_ctx, 4);

  default:
    break;
  }
  return true;
}

bool BaseParser::graphConditionSempred(GraphConditionContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 2: return precpred(_ctx, 5);
    case 3: return precpred(_ctx, 4);

  default:
    break;
  }
  return true;
}

bool BaseParser::printExprSempred(PrintExprContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 4: return precpred(_ctx, 1);

  default:
    break;
  }
  return true;
}

bool BaseParser::exprSempred(ExprContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 5: return precpred(_ctx, 7);
    case 6: return precpred(_ctx, 6);

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
