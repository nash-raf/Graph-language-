
// Generated from Base.g4 by ANTLR 4.13.0


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
      "conditionalStatement", "condition", "graphComprehension", "graphCondition", 
      "loopStatement", "foreachStatement", "loopTarget", "whileStatement", 
      "nodeEdgeOperation", "addOperation", "removeOperation", "addTargets", 
      "removeTargets", "queryStatement", "showgraph", "function", "returnType", 
      "paramList", "param", "type", "functionCall", "argumentList", "block", 
      "returnStatement", "printStatement", "printExpr", "printgraph", "expr", 
      "arrayDeclarator", "arrayInitializer", "assignmentStatement", "arrayAssignStatement"
    },
    std::vector<std::string>{
      "", "';'", "'{'", "'}'", "'nodes:'", "'edges:'", "','", "'file'", 
      "'->'", "'='", "'if'", "'('", "')'", "'else'", "'in'", "'['", "'where'", 
      "']'", "'degree'", "'connected'", "'with'", "'for'", "'each'", "'vertex'", 
      "'edge'", "'neighbor'", "'while'", "'add'", "'to'", "'remove'", "'from'", 
      "'query'", "':'", "'show'", "'fn'", "'int'", "'void'", "'string'", 
      "'real'", "'bool'", "'return'", "'print'", "'graph'", "'edges'", "'nodes'", 
      "'TRUE'", "'FALSE'", "'of'", "'+'", "'-'", "'*'", "'/'", "'&&'", "'||'", 
      "'=='", "'!='", "'<'", "'>'", "'<='", "'>='"
    },
    std::vector<std::string>{
      "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", 
      "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", 
      "", "", "", "", "", "", "", "", "GRAPH", "EDGE", "NODE", "TRUE", "FALSE", 
      "OF", "PLUS", "MINUS", "TIMES", "DIVIDE", "AND", "OR", "EQUAL", "NOTEQUAL", 
      "LESSTHAN", "GREATERTHAN", "LESSEQUAL", "GREATEREQUAL", "ID", "INT", 
      "REAL", "STRING", "Comment", "WS"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,1,65,488,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,2,
  	7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,7,
  	14,2,15,7,15,2,16,7,16,2,17,7,17,2,18,7,18,2,19,7,19,2,20,7,20,2,21,7,
  	21,2,22,7,22,2,23,7,23,2,24,7,24,2,25,7,25,2,26,7,26,2,27,7,27,2,28,7,
  	28,2,29,7,29,2,30,7,30,2,31,7,31,2,32,7,32,2,33,7,33,2,34,7,34,2,35,7,
  	35,2,36,7,36,2,37,7,37,2,38,7,38,2,39,7,39,2,40,7,40,2,41,7,41,2,42,7,
  	42,2,43,7,43,1,0,1,0,5,0,91,8,0,10,0,12,0,94,9,0,1,0,1,0,1,1,1,1,1,1,
  	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,3,1,113,8,1,1,2,1,2,1,
  	2,1,2,3,2,119,8,2,1,2,3,2,122,8,2,1,2,1,2,1,2,1,3,1,3,3,3,129,8,3,1,3,
  	1,3,1,4,1,4,1,4,3,4,136,8,4,1,4,1,4,1,5,1,5,1,5,5,5,143,8,5,10,5,12,5,
  	146,9,5,1,6,1,6,1,6,5,6,151,8,6,10,6,12,6,154,9,6,1,7,1,7,1,8,1,8,1,9,
  	1,9,1,9,1,10,1,10,1,10,1,10,1,11,1,11,1,11,1,11,3,11,171,8,11,1,11,1,
  	11,1,11,1,11,1,11,1,11,3,11,179,8,11,1,11,1,11,3,11,183,8,11,1,12,1,12,
  	1,12,1,12,1,12,1,12,1,12,1,12,1,12,3,12,194,8,12,1,13,1,13,1,13,1,13,
  	1,13,1,13,1,13,1,13,1,13,1,13,1,13,1,13,1,13,3,13,209,8,13,1,13,1,13,
  	1,13,1,13,1,13,1,13,5,13,217,8,13,10,13,12,13,220,9,13,1,14,1,14,1,14,
  	1,14,1,14,1,14,1,14,1,14,1,14,1,15,1,15,1,15,1,15,1,15,1,15,1,15,1,15,
  	1,15,1,15,1,15,3,15,242,8,15,1,15,1,15,1,15,1,15,1,15,1,15,5,15,250,8,
  	15,10,15,12,15,253,9,15,1,16,1,16,3,16,257,8,16,1,17,1,17,1,17,1,17,1,
  	17,1,17,1,17,1,18,1,18,1,18,1,18,1,18,1,18,1,18,1,18,1,18,1,18,3,18,276,
  	8,18,1,19,1,19,1,19,1,19,1,19,1,19,1,20,1,20,3,20,286,8,20,1,21,1,21,
  	1,21,1,21,1,21,1,21,1,22,1,22,1,22,1,22,1,22,1,22,1,23,1,23,1,23,1,23,
  	3,23,304,8,23,1,24,1,24,1,24,1,24,3,24,310,8,24,1,25,1,25,1,25,1,25,1,
  	25,1,25,1,25,1,25,1,26,1,26,1,26,1,26,1,27,1,27,1,27,1,27,1,27,1,27,1,
  	28,1,28,1,29,1,29,1,29,1,29,5,29,336,8,29,10,29,12,29,339,9,29,3,29,341,
  	8,29,1,29,1,29,1,30,1,30,1,30,1,31,1,31,1,32,1,32,1,32,3,32,353,8,32,
  	1,32,1,32,1,33,1,33,1,33,5,33,360,8,33,10,33,12,33,363,9,33,1,34,1,34,
  	1,34,5,34,368,8,34,10,34,12,34,371,9,34,1,34,1,34,1,34,3,34,376,8,34,
  	1,35,1,35,1,35,1,35,1,36,1,36,1,36,1,36,1,36,3,36,387,8,36,1,37,1,37,
  	1,37,3,37,392,8,37,1,37,1,37,1,37,5,37,397,8,37,10,37,12,37,400,9,37,
  	1,38,1,38,1,38,1,38,1,38,1,38,1,38,1,38,1,38,1,38,1,38,1,38,1,38,1,38,
  	1,38,1,38,1,38,1,38,3,38,420,8,38,1,39,1,39,1,39,1,39,1,39,1,39,1,39,
  	1,39,1,39,1,39,1,39,1,39,1,39,1,39,1,39,1,39,3,39,438,8,39,1,39,1,39,
  	1,39,1,39,1,39,1,39,5,39,446,8,39,10,39,12,39,449,9,39,1,40,1,40,1,40,
  	1,40,1,40,1,40,1,40,3,40,458,8,40,1,41,1,41,1,41,1,41,5,41,464,8,41,10,
  	41,12,41,467,9,41,1,41,1,41,1,42,1,42,1,42,1,42,1,42,1,42,1,42,3,42,478,
  	8,42,1,43,1,43,1,43,1,43,1,43,1,43,1,43,1,43,1,43,0,4,26,30,74,78,44,
  	0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,48,
  	50,52,54,56,58,60,62,64,66,68,70,72,74,76,78,80,82,84,86,0,5,1,0,54,59,
  	3,0,23,24,35,37,42,42,4,0,23,24,35,35,37,39,42,42,1,0,50,51,1,0,48,49,
  	510,0,92,1,0,0,0,2,112,1,0,0,0,4,114,1,0,0,0,6,126,1,0,0,0,8,132,1,0,
  	0,0,10,139,1,0,0,0,12,147,1,0,0,0,14,155,1,0,0,0,16,157,1,0,0,0,18,159,
  	1,0,0,0,20,162,1,0,0,0,22,182,1,0,0,0,24,184,1,0,0,0,26,208,1,0,0,0,28,
  	221,1,0,0,0,30,241,1,0,0,0,32,256,1,0,0,0,34,258,1,0,0,0,36,275,1,0,0,
  	0,38,277,1,0,0,0,40,285,1,0,0,0,42,287,1,0,0,0,44,293,1,0,0,0,46,303,
  	1,0,0,0,48,309,1,0,0,0,50,311,1,0,0,0,52,319,1,0,0,0,54,323,1,0,0,0,56,
  	329,1,0,0,0,58,331,1,0,0,0,60,344,1,0,0,0,62,347,1,0,0,0,64,349,1,0,0,
  	0,66,356,1,0,0,0,68,375,1,0,0,0,70,377,1,0,0,0,72,386,1,0,0,0,74,391,
  	1,0,0,0,76,419,1,0,0,0,78,437,1,0,0,0,80,457,1,0,0,0,82,459,1,0,0,0,84,
  	477,1,0,0,0,86,479,1,0,0,0,88,91,3,2,1,0,89,91,3,54,27,0,90,88,1,0,0,
  	0,90,89,1,0,0,0,91,94,1,0,0,0,92,90,1,0,0,0,92,93,1,0,0,0,93,95,1,0,0,
  	0,94,92,1,0,0,0,95,96,5,0,0,1,96,1,1,0,0,0,97,113,3,4,2,0,98,113,3,24,
  	12,0,99,113,3,72,36,0,100,113,3,38,19,0,101,113,3,22,11,0,102,103,3,64,
  	32,0,103,104,5,1,0,0,104,113,1,0,0,0,105,113,3,28,14,0,106,113,3,86,43,
  	0,107,113,3,84,42,0,108,113,3,50,25,0,109,113,3,52,26,0,110,113,3,40,
  	20,0,111,113,5,1,0,0,112,97,1,0,0,0,112,98,1,0,0,0,112,99,1,0,0,0,112,
  	100,1,0,0,0,112,101,1,0,0,0,112,102,1,0,0,0,112,105,1,0,0,0,112,106,1,
  	0,0,0,112,107,1,0,0,0,112,108,1,0,0,0,112,109,1,0,0,0,112,110,1,0,0,0,
  	112,111,1,0,0,0,113,3,1,0,0,0,114,115,5,42,0,0,115,116,3,14,7,0,116,118,
  	5,2,0,0,117,119,3,6,3,0,118,117,1,0,0,0,118,119,1,0,0,0,119,121,1,0,0,
  	0,120,122,3,8,4,0,121,120,1,0,0,0,121,122,1,0,0,0,122,123,1,0,0,0,123,
  	124,5,3,0,0,124,125,5,1,0,0,125,5,1,0,0,0,126,128,5,4,0,0,127,129,3,10,
  	5,0,128,127,1,0,0,0,128,129,1,0,0,0,129,130,1,0,0,0,130,131,5,1,0,0,131,
  	7,1,0,0,0,132,135,5,5,0,0,133,136,3,12,6,0,134,136,3,18,9,0,135,133,1,
  	0,0,0,135,134,1,0,0,0,136,137,1,0,0,0,137,138,5,1,0,0,138,9,1,0,0,0,139,
  	144,3,16,8,0,140,141,5,6,0,0,141,143,3,16,8,0,142,140,1,0,0,0,143,146,
  	1,0,0,0,144,142,1,0,0,0,144,145,1,0,0,0,145,11,1,0,0,0,146,144,1,0,0,
  	0,147,152,3,20,10,0,148,149,5,6,0,0,149,151,3,20,10,0,150,148,1,0,0,0,
  	151,154,1,0,0,0,152,150,1,0,0,0,152,153,1,0,0,0,153,13,1,0,0,0,154,152,
  	1,0,0,0,155,156,5,60,0,0,156,15,1,0,0,0,157,158,5,61,0,0,158,17,1,0,0,
  	0,159,160,5,7,0,0,160,161,5,63,0,0,161,19,1,0,0,0,162,163,3,16,8,0,163,
  	164,5,8,0,0,164,165,3,16,8,0,165,21,1,0,0,0,166,167,3,62,31,0,167,170,
  	5,60,0,0,168,169,5,9,0,0,169,171,3,78,39,0,170,168,1,0,0,0,170,171,1,
  	0,0,0,171,172,1,0,0,0,172,173,5,1,0,0,173,183,1,0,0,0,174,175,3,62,31,
  	0,175,178,3,80,40,0,176,177,5,9,0,0,177,179,3,82,41,0,178,176,1,0,0,0,
  	178,179,1,0,0,0,179,180,1,0,0,0,180,181,5,1,0,0,181,183,1,0,0,0,182,166,
  	1,0,0,0,182,174,1,0,0,0,183,23,1,0,0,0,184,185,5,10,0,0,185,186,5,11,
  	0,0,186,187,3,26,13,0,187,188,5,12,0,0,188,193,3,68,34,0,189,190,5,13,
  	0,0,190,194,3,24,12,0,191,192,5,13,0,0,192,194,3,68,34,0,193,189,1,0,
  	0,0,193,191,1,0,0,0,193,194,1,0,0,0,194,25,1,0,0,0,195,196,6,13,-1,0,
  	196,197,3,78,39,0,197,198,7,0,0,0,198,199,3,78,39,0,199,209,1,0,0,0,200,
  	201,3,16,8,0,201,202,5,14,0,0,202,203,3,14,7,0,203,209,1,0,0,0,204,205,
  	3,20,10,0,205,206,5,14,0,0,206,207,3,14,7,0,207,209,1,0,0,0,208,195,1,
  	0,0,0,208,200,1,0,0,0,208,204,1,0,0,0,209,218,1,0,0,0,210,211,10,5,0,
  	0,211,212,5,52,0,0,212,217,3,26,13,6,213,214,10,4,0,0,214,215,5,53,0,
  	0,215,217,3,26,13,5,216,210,1,0,0,0,216,213,1,0,0,0,217,220,1,0,0,0,218,
  	216,1,0,0,0,218,219,1,0,0,0,219,27,1,0,0,0,220,218,1,0,0,0,221,222,5,
  	60,0,0,222,223,5,9,0,0,223,224,5,15,0,0,224,225,3,14,7,0,225,226,5,16,
  	0,0,226,227,3,30,15,0,227,228,5,17,0,0,228,229,5,1,0,0,229,29,1,0,0,0,
  	230,231,6,15,-1,0,231,232,5,18,0,0,232,233,7,0,0,0,233,242,5,61,0,0,234,
  	235,5,19,0,0,235,236,5,20,0,0,236,242,3,16,8,0,237,238,5,11,0,0,238,239,
  	3,30,15,0,239,240,5,12,0,0,240,242,1,0,0,0,241,230,1,0,0,0,241,234,1,
  	0,0,0,241,237,1,0,0,0,242,251,1,0,0,0,243,244,10,5,0,0,244,245,5,52,0,
  	0,245,250,3,30,15,6,246,247,10,4,0,0,247,248,5,53,0,0,248,250,3,30,15,
  	5,249,243,1,0,0,0,249,246,1,0,0,0,250,253,1,0,0,0,251,249,1,0,0,0,251,
  	252,1,0,0,0,252,31,1,0,0,0,253,251,1,0,0,0,254,257,3,34,17,0,255,257,
  	3,38,19,0,256,254,1,0,0,0,256,255,1,0,0,0,257,33,1,0,0,0,258,259,5,21,
  	0,0,259,260,5,22,0,0,260,261,3,36,18,0,261,262,5,14,0,0,262,263,3,14,
  	7,0,263,264,3,68,34,0,264,35,1,0,0,0,265,266,5,23,0,0,266,276,5,60,0,
  	0,267,268,5,24,0,0,268,269,5,60,0,0,269,270,5,6,0,0,270,276,5,60,0,0,
  	271,272,5,25,0,0,272,273,5,60,0,0,273,274,5,47,0,0,274,276,3,16,8,0,275,
  	265,1,0,0,0,275,267,1,0,0,0,275,271,1,0,0,0,276,37,1,0,0,0,277,278,5,
  	26,0,0,278,279,5,11,0,0,279,280,3,26,13,0,280,281,5,12,0,0,281,282,3,
  	68,34,0,282,39,1,0,0,0,283,286,3,42,21,0,284,286,3,44,22,0,285,283,1,
  	0,0,0,285,284,1,0,0,0,286,41,1,0,0,0,287,288,5,27,0,0,288,289,3,46,23,
  	0,289,290,5,28,0,0,290,291,3,14,7,0,291,292,5,1,0,0,292,43,1,0,0,0,293,
  	294,5,29,0,0,294,295,3,48,24,0,295,296,5,30,0,0,296,297,3,14,7,0,297,
  	298,5,1,0,0,298,45,1,0,0,0,299,304,3,16,8,0,300,304,3,20,10,0,301,304,
  	3,10,5,0,302,304,3,12,6,0,303,299,1,0,0,0,303,300,1,0,0,0,303,301,1,0,
  	0,0,303,302,1,0,0,0,304,47,1,0,0,0,305,310,3,16,8,0,306,310,3,20,10,0,
  	307,310,3,10,5,0,308,310,3,12,6,0,309,305,1,0,0,0,309,306,1,0,0,0,309,
  	307,1,0,0,0,309,308,1,0,0,0,310,49,1,0,0,0,311,312,5,31,0,0,312,313,5,
  	60,0,0,313,314,5,32,0,0,314,315,5,63,0,0,315,316,5,47,0,0,316,317,3,14,
  	7,0,317,318,5,1,0,0,318,51,1,0,0,0,319,320,5,33,0,0,320,321,3,14,7,0,
  	321,322,5,1,0,0,322,53,1,0,0,0,323,324,5,34,0,0,324,325,3,56,28,0,325,
  	326,5,60,0,0,326,327,3,58,29,0,327,328,3,68,34,0,328,55,1,0,0,0,329,330,
  	7,1,0,0,330,57,1,0,0,0,331,340,5,11,0,0,332,337,3,60,30,0,333,334,5,6,
  	0,0,334,336,3,60,30,0,335,333,1,0,0,0,336,339,1,0,0,0,337,335,1,0,0,0,
  	337,338,1,0,0,0,338,341,1,0,0,0,339,337,1,0,0,0,340,332,1,0,0,0,340,341,
  	1,0,0,0,341,342,1,0,0,0,342,343,5,12,0,0,343,59,1,0,0,0,344,345,3,62,
  	31,0,345,346,5,60,0,0,346,61,1,0,0,0,347,348,7,2,0,0,348,63,1,0,0,0,349,
  	350,5,60,0,0,350,352,5,11,0,0,351,353,3,66,33,0,352,351,1,0,0,0,352,353,
  	1,0,0,0,353,354,1,0,0,0,354,355,5,12,0,0,355,65,1,0,0,0,356,361,3,78,
  	39,0,357,358,5,6,0,0,358,360,3,78,39,0,359,357,1,0,0,0,360,363,1,0,0,
  	0,361,359,1,0,0,0,361,362,1,0,0,0,362,67,1,0,0,0,363,361,1,0,0,0,364,
  	369,5,2,0,0,365,368,3,2,1,0,366,368,3,70,35,0,367,365,1,0,0,0,367,366,
  	1,0,0,0,368,371,1,0,0,0,369,367,1,0,0,0,369,370,1,0,0,0,370,372,1,0,0,
  	0,371,369,1,0,0,0,372,376,5,3,0,0,373,374,5,2,0,0,374,376,5,3,0,0,375,
  	364,1,0,0,0,375,373,1,0,0,0,376,69,1,0,0,0,377,378,5,40,0,0,378,379,3,
  	78,39,0,379,380,5,1,0,0,380,71,1,0,0,0,381,382,5,41,0,0,382,383,3,74,
  	37,0,383,384,5,1,0,0,384,387,1,0,0,0,385,387,3,76,38,0,386,381,1,0,0,
  	0,386,385,1,0,0,0,387,73,1,0,0,0,388,389,6,37,-1,0,389,392,5,63,0,0,390,
  	392,3,78,39,0,391,388,1,0,0,0,391,390,1,0,0,0,392,398,1,0,0,0,393,394,
  	10,1,0,0,394,395,5,48,0,0,395,397,3,74,37,2,396,393,1,0,0,0,397,400,1,
  	0,0,0,398,396,1,0,0,0,398,399,1,0,0,0,399,75,1,0,0,0,400,398,1,0,0,0,
  	401,402,5,41,0,0,402,403,5,43,0,0,403,404,5,47,0,0,404,405,3,14,7,0,405,
  	406,5,1,0,0,406,420,1,0,0,0,407,408,5,41,0,0,408,409,5,44,0,0,409,410,
  	5,47,0,0,410,411,3,14,7,0,411,412,5,1,0,0,412,420,1,0,0,0,413,414,5,41,
  	0,0,414,415,5,42,0,0,415,416,5,47,0,0,416,417,3,14,7,0,417,418,5,1,0,
  	0,418,420,1,0,0,0,419,401,1,0,0,0,419,407,1,0,0,0,419,413,1,0,0,0,420,
  	77,1,0,0,0,421,422,6,39,-1,0,422,438,3,64,32,0,423,438,5,61,0,0,424,438,
  	5,60,0,0,425,426,5,11,0,0,426,427,3,78,39,0,427,428,5,12,0,0,428,438,
  	1,0,0,0,429,430,5,60,0,0,430,431,5,15,0,0,431,432,3,78,39,0,432,433,5,
  	17,0,0,433,438,1,0,0,0,434,438,5,45,0,0,435,438,5,46,0,0,436,438,5,62,
  	0,0,437,421,1,0,0,0,437,423,1,0,0,0,437,424,1,0,0,0,437,425,1,0,0,0,437,
  	429,1,0,0,0,437,434,1,0,0,0,437,435,1,0,0,0,437,436,1,0,0,0,438,447,1,
  	0,0,0,439,440,10,10,0,0,440,441,7,3,0,0,441,446,3,78,39,11,442,443,10,
  	9,0,0,443,444,7,4,0,0,444,446,3,78,39,10,445,439,1,0,0,0,445,442,1,0,
  	0,0,446,449,1,0,0,0,447,445,1,0,0,0,447,448,1,0,0,0,448,79,1,0,0,0,449,
  	447,1,0,0,0,450,451,5,60,0,0,451,452,5,15,0,0,452,453,5,61,0,0,453,458,
  	5,17,0,0,454,455,5,60,0,0,455,456,5,15,0,0,456,458,5,17,0,0,457,450,1,
  	0,0,0,457,454,1,0,0,0,458,81,1,0,0,0,459,460,5,15,0,0,460,465,3,78,39,
  	0,461,462,5,6,0,0,462,464,3,78,39,0,463,461,1,0,0,0,464,467,1,0,0,0,465,
  	463,1,0,0,0,465,466,1,0,0,0,466,468,1,0,0,0,467,465,1,0,0,0,468,469,5,
  	17,0,0,469,83,1,0,0,0,470,471,5,60,0,0,471,472,5,9,0,0,472,473,3,78,39,
  	0,473,474,5,1,0,0,474,478,1,0,0,0,475,476,5,60,0,0,476,478,5,1,0,0,477,
  	470,1,0,0,0,477,475,1,0,0,0,478,85,1,0,0,0,479,480,5,60,0,0,480,481,5,
  	15,0,0,481,482,3,78,39,0,482,483,5,17,0,0,483,484,5,9,0,0,484,485,3,78,
  	39,0,485,486,5,1,0,0,486,87,1,0,0,0,41,90,92,112,118,121,128,135,144,
  	152,170,178,182,193,208,216,218,241,249,251,256,275,285,303,309,337,340,
  	352,361,367,369,375,386,391,398,419,437,445,447,457,465,477
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
    setState(92);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 1152929126789678082) != 0)) {
      setState(90);
      _errHandler->sync(this);
      switch (_input->LA(1)) {
        case BaseParser::T__0:
        case BaseParser::T__9:
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
        case BaseParser::T__38:
        case BaseParser::T__40:
        case BaseParser::GRAPH:
        case BaseParser::ID: {
          setState(88);
          statement();
          break;
        }

        case BaseParser::T__33: {
          setState(89);
          function();
          break;
        }

      default:
        throw NoViableAltException(this);
      }
      setState(94);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(95);
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

BaseParser::VarDeclContext* BaseParser::StatementContext::varDecl() {
  return getRuleContext<BaseParser::VarDeclContext>(0);
}

BaseParser::FunctionCallContext* BaseParser::StatementContext::functionCall() {
  return getRuleContext<BaseParser::FunctionCallContext>(0);
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
    setState(112);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 2, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(97);
      graphDef();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(98);
      conditionalStatement();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(99);
      printStatement();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(100);
      whileStatement();
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(101);
      varDecl();
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(102);
      functionCall();
      setState(103);
      match(BaseParser::T__0);
      break;
    }

    case 7: {
      enterOuterAlt(_localctx, 7);
      setState(105);
      graphComprehension();
      break;
    }

    case 8: {
      enterOuterAlt(_localctx, 8);
      setState(106);
      arrayAssignStatement();
      break;
    }

    case 9: {
      enterOuterAlt(_localctx, 9);
      setState(107);
      assignmentStatement();
      break;
    }

    case 10: {
      enterOuterAlt(_localctx, 10);
      setState(108);
      queryStatement();
      break;
    }

    case 11: {
      enterOuterAlt(_localctx, 11);
      setState(109);
      showgraph();
      break;
    }

    case 12: {
      enterOuterAlt(_localctx, 12);
      setState(110);
      nodeEdgeOperation();
      break;
    }

    case 13: {
      enterOuterAlt(_localctx, 13);
      setState(111);
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
    setState(114);
    match(BaseParser::GRAPH);
    setState(115);
    graphID();
    setState(116);
    match(BaseParser::T__1);
    setState(118);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == BaseParser::T__3) {
      setState(117);
      nodes();
    }
    setState(121);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == BaseParser::T__4) {
      setState(120);
      edges();
    }
    setState(123);
    match(BaseParser::T__2);
    setState(124);
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
    setState(126);
    match(BaseParser::T__3);
    setState(128);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == BaseParser::INT) {
      setState(127);
      nodeList();
    }
    setState(130);
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
    setState(132);
    match(BaseParser::T__4);
    setState(135);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case BaseParser::INT: {
        setState(133);
        edgeList();
        break;
      }

      case BaseParser::T__6: {
        setState(134);
        fileEdgeList();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    setState(137);
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
    setState(139);
    nodeID();
    setState(144);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == BaseParser::T__5) {
      setState(140);
      match(BaseParser::T__5);
      setState(141);
      nodeID();
      setState(146);
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
    setState(147);
    edge();
    setState(152);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == BaseParser::T__5) {
      setState(148);
      match(BaseParser::T__5);
      setState(149);
      edge();
      setState(154);
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
    setState(155);
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
    setState(157);
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
    setState(159);
    match(BaseParser::T__6);
    setState(160);
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
    setState(162);
    nodeID();
    setState(163);
    match(BaseParser::T__7);
    setState(164);
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
    setState(182);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 11, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<BaseParser::SimpleDeclarationContext>(_localctx);
      enterOuterAlt(_localctx, 1);
      setState(166);
      type();
      setState(167);
      match(BaseParser::ID);
      setState(170);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == BaseParser::T__8) {
        setState(168);
        match(BaseParser::T__8);
        setState(169);
        expr(0);
      }
      setState(172);
      match(BaseParser::T__0);
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<BaseParser::ArrayDeclarationContext>(_localctx);
      enterOuterAlt(_localctx, 2);
      setState(174);
      type();
      setState(175);
      arrayDeclarator();
      setState(178);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == BaseParser::T__8) {
        setState(176);
        match(BaseParser::T__8);
        setState(177);
        arrayInitializer();
      }
      setState(180);
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
    setState(184);
    match(BaseParser::T__9);
    setState(185);
    match(BaseParser::T__10);
    setState(186);
    condition(0);
    setState(187);
    match(BaseParser::T__11);
    setState(188);
    block();
    setState(193);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 12, _ctx)) {
    case 1: {
      setState(189);
      match(BaseParser::T__12);
      setState(190);
      conditionalStatement();
      break;
    }

    case 2: {
      setState(191);
      match(BaseParser::T__12);
      setState(192);
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
    setState(208);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 13, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<RelationalContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;

      setState(196);
      expr(0);
      setState(197);
      _la = _input->LA(1);
      if (!((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 1134907106097364992) != 0))) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(198);
      expr(0);
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<NodeCheckContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(200);
      nodeID();
      setState(201);
      match(BaseParser::T__13);
      setState(202);
      graphID();
      break;
    }

    case 3: {
      _localctx = _tracker.createInstance<EdgeCheckContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(204);
      edge();
      setState(205);
      match(BaseParser::T__13);
      setState(206);
      graphID();
      break;
    }

    default:
      break;
    }
    _ctx->stop = _input->LT(-1);
    setState(218);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 15, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(216);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 14, _ctx)) {
        case 1: {
          auto newContext = _tracker.createInstance<LogicalAndContext>(_tracker.createInstance<ConditionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleCondition);
          setState(210);

          if (!(precpred(_ctx, 5))) throw FailedPredicateException(this, "precpred(_ctx, 5)");
          setState(211);
          match(BaseParser::AND);
          setState(212);
          condition(6);
          break;
        }

        case 2: {
          auto newContext = _tracker.createInstance<LogicalOrContext>(_tracker.createInstance<ConditionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleCondition);
          setState(213);

          if (!(precpred(_ctx, 4))) throw FailedPredicateException(this, "precpred(_ctx, 4)");
          setState(214);
          match(BaseParser::OR);
          setState(215);
          condition(5);
          break;
        }

        default:
          break;
        } 
      }
      setState(220);
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
    setState(221);
    match(BaseParser::ID);
    setState(222);
    match(BaseParser::T__8);
    setState(223);
    match(BaseParser::T__14);
    setState(224);
    graphID();
    setState(225);
    match(BaseParser::T__15);
    setState(226);
    graphCondition(0);
    setState(227);
    match(BaseParser::T__16);
    setState(228);
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
    setState(241);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case BaseParser::T__17: {
        _localctx = _tracker.createInstance<DegreeConditionContext>(_localctx);
        _ctx = _localctx;
        previousContext = _localctx;

        setState(231);
        match(BaseParser::T__17);
        setState(232);
        _la = _input->LA(1);
        if (!((((_la & ~ 0x3fULL) == 0) &&
          ((1ULL << _la) & 1134907106097364992) != 0))) {
        _errHandler->recoverInline(this);
        }
        else {
          _errHandler->reportMatch(this);
          consume();
        }
        setState(233);
        match(BaseParser::INT);
        break;
      }

      case BaseParser::T__18: {
        _localctx = _tracker.createInstance<ConnectedConditionContext>(_localctx);
        _ctx = _localctx;
        previousContext = _localctx;
        setState(234);
        match(BaseParser::T__18);
        setState(235);
        match(BaseParser::T__19);
        setState(236);
        nodeID();
        break;
      }

      case BaseParser::T__10: {
        _localctx = _tracker.createInstance<ParenGraphConditionContext>(_localctx);
        _ctx = _localctx;
        previousContext = _localctx;
        setState(237);
        match(BaseParser::T__10);
        setState(238);
        graphCondition(0);
        setState(239);
        match(BaseParser::T__11);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    _ctx->stop = _input->LT(-1);
    setState(251);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 18, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(249);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 17, _ctx)) {
        case 1: {
          auto newContext = _tracker.createInstance<GraphLogicalAndContext>(_tracker.createInstance<GraphConditionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleGraphCondition);
          setState(243);

          if (!(precpred(_ctx, 5))) throw FailedPredicateException(this, "precpred(_ctx, 5)");
          setState(244);
          match(BaseParser::AND);
          setState(245);
          graphCondition(6);
          break;
        }

        case 2: {
          auto newContext = _tracker.createInstance<GraphLogicalOrContext>(_tracker.createInstance<GraphConditionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleGraphCondition);
          setState(246);

          if (!(precpred(_ctx, 4))) throw FailedPredicateException(this, "precpred(_ctx, 4)");
          setState(247);
          match(BaseParser::OR);
          setState(248);
          graphCondition(5);
          break;
        }

        default:
          break;
        } 
      }
      setState(253);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 18, _ctx);
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
    setState(256);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case BaseParser::T__20: {
        enterOuterAlt(_localctx, 1);
        setState(254);
        foreachStatement();
        break;
      }

      case BaseParser::T__25: {
        enterOuterAlt(_localctx, 2);
        setState(255);
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
    setState(258);
    match(BaseParser::T__20);
    setState(259);
    match(BaseParser::T__21);
    setState(260);
    loopTarget();
    setState(261);
    match(BaseParser::T__13);
    setState(262);
    graphID();
    setState(263);
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
    setState(275);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case BaseParser::T__22: {
        _localctx = _tracker.createInstance<BaseParser::ForEachVertexContext>(_localctx);
        enterOuterAlt(_localctx, 1);
        setState(265);
        match(BaseParser::T__22);
        setState(266);
        match(BaseParser::ID);
        break;
      }

      case BaseParser::T__23: {
        _localctx = _tracker.createInstance<BaseParser::ForEachEdgeContext>(_localctx);
        enterOuterAlt(_localctx, 2);
        setState(267);
        match(BaseParser::T__23);
        setState(268);
        match(BaseParser::ID);
        setState(269);
        match(BaseParser::T__5);
        setState(270);
        match(BaseParser::ID);
        break;
      }

      case BaseParser::T__24: {
        _localctx = _tracker.createInstance<BaseParser::ForEachAdjContext>(_localctx);
        enterOuterAlt(_localctx, 3);
        setState(271);
        match(BaseParser::T__24);
        setState(272);
        match(BaseParser::ID);
        setState(273);
        match(BaseParser::OF);
        setState(274);
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
    setState(277);
    match(BaseParser::T__25);
    setState(278);
    match(BaseParser::T__10);
    setState(279);
    condition(0);
    setState(280);
    match(BaseParser::T__11);
    setState(281);
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
    setState(285);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case BaseParser::T__26: {
        enterOuterAlt(_localctx, 1);
        setState(283);
        addOperation();
        break;
      }

      case BaseParser::T__28: {
        enterOuterAlt(_localctx, 2);
        setState(284);
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
    setState(287);
    match(BaseParser::T__26);
    setState(288);
    addTargets();
    setState(289);
    match(BaseParser::T__27);
    setState(290);
    graphID();
    setState(291);
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
    setState(293);
    match(BaseParser::T__28);
    setState(294);
    removeTargets();
    setState(295);
    match(BaseParser::T__29);
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
    setState(303);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 22, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(299);
      nodeID();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(300);
      edge();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(301);
      nodeList();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(302);
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
    setState(309);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 23, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(305);
      nodeID();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(306);
      edge();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(307);
      nodeList();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(308);
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
    setState(311);
    match(BaseParser::T__30);
    setState(312);
    match(BaseParser::ID);
    setState(313);
    match(BaseParser::T__31);
    setState(314);
    match(BaseParser::STRING);
    setState(315);
    match(BaseParser::OF);
    setState(316);
    graphID();
    setState(317);
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
    setState(319);
    match(BaseParser::T__32);
    setState(320);
    graphID();
    setState(321);
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
    setState(323);
    match(BaseParser::T__33);
    setState(324);
    returnType();
    setState(325);
    match(BaseParser::ID);
    setState(326);
    paramList();
    setState(327);
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
    setState(329);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 4638589845504) != 0))) {
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
    setState(331);
    match(BaseParser::T__10);
    setState(340);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 5394504089600) != 0)) {
      setState(332);
      param();
      setState(337);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == BaseParser::T__5) {
        setState(333);
        match(BaseParser::T__5);
        setState(334);
        param();
        setState(339);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
    }
    setState(342);
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
    setState(344);
    type();
    setState(345);
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
    setState(347);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 5394504089600) != 0))) {
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
    setState(349);
    match(BaseParser::ID);
    setState(350);
    match(BaseParser::T__10);
    setState(352);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 8070556085364197376) != 0)) {
      setState(351);
      argumentList();
    }
    setState(354);
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
    setState(356);
    expr(0);
    setState(361);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == BaseParser::T__5) {
      setState(357);
      match(BaseParser::T__5);
      setState(358);
      expr(0);
      setState(363);
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
    setState(375);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 30, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(364);
      match(BaseParser::T__1);
      setState(369);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while ((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 1152930209121436674) != 0)) {
        setState(367);
        _errHandler->sync(this);
        switch (_input->LA(1)) {
          case BaseParser::T__0:
          case BaseParser::T__9:
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
          case BaseParser::T__38:
          case BaseParser::T__40:
          case BaseParser::GRAPH:
          case BaseParser::ID: {
            setState(365);
            statement();
            break;
          }

          case BaseParser::T__39: {
            setState(366);
            returnStatement();
            break;
          }

        default:
          throw NoViableAltException(this);
        }
        setState(371);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
      setState(372);
      match(BaseParser::T__2);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(373);
      match(BaseParser::T__1);
      setState(374);
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
    setState(377);
    match(BaseParser::T__39);
    setState(378);
    expr(0);
    setState(379);
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
    setState(386);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 31, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(381);
      match(BaseParser::T__40);
      setState(382);
      printExpr(0);
      setState(383);
      match(BaseParser::T__0);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(385);
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
    setState(391);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case BaseParser::STRING: {
        setState(389);
        match(BaseParser::STRING);
        break;
      }

      case BaseParser::T__10:
      case BaseParser::TRUE:
      case BaseParser::FALSE:
      case BaseParser::ID:
      case BaseParser::INT:
      case BaseParser::REAL: {
        setState(390);
        expr(0);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    _ctx->stop = _input->LT(-1);
    setState(398);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 33, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        _localctx = _tracker.createInstance<PrintExprContext>(parentContext, parentState);
        pushNewRecursionContext(_localctx, startState, RulePrintExpr);
        setState(393);

        if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
        setState(394);
        match(BaseParser::PLUS);
        setState(395);
        printExpr(2); 
      }
      setState(400);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 33, _ctx);
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
    setState(419);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 34, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<BaseParser::EdgePrintContext>(_localctx);
      enterOuterAlt(_localctx, 1);
      setState(401);
      match(BaseParser::T__40);
      setState(402);
      match(BaseParser::EDGE);
      setState(403);
      match(BaseParser::OF);
      setState(404);
      graphID();
      setState(405);
      match(BaseParser::T__0);
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<BaseParser::NodePrintContext>(_localctx);
      enterOuterAlt(_localctx, 2);
      setState(407);
      match(BaseParser::T__40);
      setState(408);
      match(BaseParser::NODE);
      setState(409);
      match(BaseParser::OF);
      setState(410);
      graphID();
      setState(411);
      match(BaseParser::T__0);
      break;
    }

    case 3: {
      _localctx = _tracker.createInstance<BaseParser::GraphPrintContext>(_localctx);
      enterOuterAlt(_localctx, 3);
      setState(413);
      match(BaseParser::T__40);
      setState(414);
      match(BaseParser::GRAPH);
      setState(415);
      match(BaseParser::OF);
      setState(416);
      graphID();
      setState(417);
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


std::any BaseParser::BoolTrueExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitBoolTrueExpr(this);
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
//----------------- BoolFalseExprContext ------------------------------------------------------------------

tree::TerminalNode* BaseParser::BoolFalseExprContext::FALSE() {
  return getToken(BaseParser::FALSE, 0);
}

BaseParser::BoolFalseExprContext::BoolFalseExprContext(ExprContext *ctx) { copyFrom(ctx); }


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


std::any BaseParser::ArrayAccessExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitArrayAccessExpr(this);
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
    setState(437);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 35, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<FuncExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;

      setState(422);
      functionCall();
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<IntExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(423);
      match(BaseParser::INT);
      break;
    }

    case 3: {
      _localctx = _tracker.createInstance<IdExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(424);
      match(BaseParser::ID);
      break;
    }

    case 4: {
      _localctx = _tracker.createInstance<ParenExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(425);
      match(BaseParser::T__10);
      setState(426);
      expr(0);
      setState(427);
      match(BaseParser::T__11);
      break;
    }

    case 5: {
      _localctx = _tracker.createInstance<ArrayAccessExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(429);
      match(BaseParser::ID);
      setState(430);
      match(BaseParser::T__14);
      setState(431);
      expr(0);
      setState(432);
      match(BaseParser::T__16);
      break;
    }

    case 6: {
      _localctx = _tracker.createInstance<BoolTrueExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(434);
      match(BaseParser::TRUE);
      break;
    }

    case 7: {
      _localctx = _tracker.createInstance<BoolFalseExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(435);
      match(BaseParser::FALSE);
      break;
    }

    case 8: {
      _localctx = _tracker.createInstance<RealExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(436);
      match(BaseParser::REAL);
      break;
    }

    default:
      break;
    }
    _ctx->stop = _input->LT(-1);
    setState(447);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 37, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(445);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 36, _ctx)) {
        case 1: {
          auto newContext = _tracker.createInstance<MulDivExprContext>(_tracker.createInstance<ExprContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpr);
          setState(439);

          if (!(precpred(_ctx, 10))) throw FailedPredicateException(this, "precpred(_ctx, 10)");
          setState(440);
          _la = _input->LA(1);
          if (!(_la == BaseParser::TIMES

          || _la == BaseParser::DIVIDE)) {
          _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(441);
          expr(11);
          break;
        }

        case 2: {
          auto newContext = _tracker.createInstance<AddSubExprContext>(_tracker.createInstance<ExprContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpr);
          setState(442);

          if (!(precpred(_ctx, 9))) throw FailedPredicateException(this, "precpred(_ctx, 9)");
          setState(443);
          _la = _input->LA(1);
          if (!(_la == BaseParser::PLUS

          || _la == BaseParser::MINUS)) {
          _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(444);
          expr(10);
          break;
        }

        default:
          break;
        } 
      }
      setState(449);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 37, _ctx);
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

tree::TerminalNode* BaseParser::SizedArrayContext::INT() {
  return getToken(BaseParser::INT, 0);
}

BaseParser::SizedArrayContext::SizedArrayContext(ArrayDeclaratorContext *ctx) { copyFrom(ctx); }


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


std::any BaseParser::UnsizedArrayContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitUnsizedArray(this);
  else
    return visitor->visitChildren(this);
}
BaseParser::ArrayDeclaratorContext* BaseParser::arrayDeclarator() {
  ArrayDeclaratorContext *_localctx = _tracker.createInstance<ArrayDeclaratorContext>(_ctx, getState());
  enterRule(_localctx, 80, BaseParser::RuleArrayDeclarator);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(457);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 38, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<BaseParser::SizedArrayContext>(_localctx);
      enterOuterAlt(_localctx, 1);
      setState(450);
      match(BaseParser::ID);
      setState(451);
      match(BaseParser::T__14);
      setState(452);
      match(BaseParser::INT);
      setState(453);
      match(BaseParser::T__16);
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<BaseParser::UnsizedArrayContext>(_localctx);
      enterOuterAlt(_localctx, 2);
      setState(454);
      match(BaseParser::ID);
      setState(455);
      match(BaseParser::T__14);
      setState(456);
      match(BaseParser::T__16);
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


std::any BaseParser::ArrayInitializerContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitArrayInitializer(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::ArrayInitializerContext* BaseParser::arrayInitializer() {
  ArrayInitializerContext *_localctx = _tracker.createInstance<ArrayInitializerContext>(_ctx, getState());
  enterRule(_localctx, 82, BaseParser::RuleArrayInitializer);
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
    setState(459);
    match(BaseParser::T__14);
    setState(460);
    expr(0);
    setState(465);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == BaseParser::T__5) {
      setState(461);
      match(BaseParser::T__5);
      setState(462);
      expr(0);
      setState(467);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(468);
    match(BaseParser::T__16);
   
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


std::any BaseParser::AssignmentStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitAssignmentStatement(this);
  else
    return visitor->visitChildren(this);
}

BaseParser::AssignmentStatementContext* BaseParser::assignmentStatement() {
  AssignmentStatementContext *_localctx = _tracker.createInstance<AssignmentStatementContext>(_ctx, getState());
  enterRule(_localctx, 84, BaseParser::RuleAssignmentStatement);

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
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 40, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(470);
      match(BaseParser::ID);
      setState(471);
      match(BaseParser::T__8);
      setState(472);
      expr(0);
      setState(473);
      match(BaseParser::T__0);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(475);
      match(BaseParser::ID);
      setState(476);
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


std::any BaseParser::ArrayAssignStmtContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<BaseVisitor*>(visitor))
    return parserVisitor->visitArrayAssignStmt(this);
  else
    return visitor->visitChildren(this);
}
BaseParser::ArrayAssignStatementContext* BaseParser::arrayAssignStatement() {
  ArrayAssignStatementContext *_localctx = _tracker.createInstance<ArrayAssignStatementContext>(_ctx, getState());
  enterRule(_localctx, 86, BaseParser::RuleArrayAssignStatement);

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
    setState(479);
    match(BaseParser::ID);
    setState(480);
    match(BaseParser::T__14);
    setState(481);
    expr(0);
    setState(482);
    match(BaseParser::T__16);
    setState(483);
    match(BaseParser::T__8);
    setState(484);
    expr(0);
    setState(485);
    match(BaseParser::T__0);
   
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
    case 5: return precpred(_ctx, 10);
    case 6: return precpred(_ctx, 9);

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
