
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
      "conditionalStatement", "condition", "graphComprehension", "graphCondition", 
      "loopStatement", "foreachStatement", "loopTarget", "whileStatement", 
      "nodeEdgeOperation", "addOperation", "removeOperation", "addTargets", 
      "removeTargets", "queryStatement", "showgraph", "function", "returnType", 
      "paramList", "param", "type", "functionCall", "argumentList", "sleepStatement", 
      "block", "returnStatement", "printStatement", "printExpr", "printgraph", 
      "expr", "arrayDeclarator", "arrayInitializer", "assignmentStatement", 
      "arrayAssignStatement"
    },
    std::vector<std::string>{
      "", "';'", "'{'", "'}'", "'nodes:'", "'edges:'", "','", "'file'", 
      "'->'", "'='", "'if'", "'('", "')'", "'else'", "'in'", "'['", "'where'", 
      "']'", "'degree'", "'connected'", "'with'", "'for'", "'each'", "'vertex'", 
      "'edge'", "'neighbor'", "'while'", "'add'", "'to'", "'remove'", "'from'", 
      "'query'", "':'", "'show'", "'fn'", "'int'", "'void'", "'string'", 
      "'real'", "'bool'", "'sleep'", "'return'", "'print'", "'timer'", "'graph'", 
      "'edges'", "'nodes'", "'TRUE'", "'FALSE'", "'of'", "'+'", "'-'", "'*'", 
      "'/'", "'&&'", "'||'", "'=='", "'!='", "'<'", "'>'", "'<='", "'>='"
    },
    std::vector<std::string>{
      "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", 
      "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", 
      "", "", "", "", "", "", "", "", "", "", "GRAPH", "EDGE", "NODE", "TRUE", 
      "FALSE", "OF", "PLUS", "MINUS", "TIMES", "DIVIDE", "AND", "OR", "EQUAL", 
      "NOTEQUAL", "LESSTHAN", "GREATERTHAN", "LESSEQUAL", "GREATEREQUAL", 
      "ID", "INT", "REAL", "STRING", "Comment", "WS"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,1,67,500,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,2,
  	7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,7,
  	14,2,15,7,15,2,16,7,16,2,17,7,17,2,18,7,18,2,19,7,19,2,20,7,20,2,21,7,
  	21,2,22,7,22,2,23,7,23,2,24,7,24,2,25,7,25,2,26,7,26,2,27,7,27,2,28,7,
  	28,2,29,7,29,2,30,7,30,2,31,7,31,2,32,7,32,2,33,7,33,2,34,7,34,2,35,7,
  	35,2,36,7,36,2,37,7,37,2,38,7,38,2,39,7,39,2,40,7,40,2,41,7,41,2,42,7,
  	42,2,43,7,43,2,44,7,44,1,0,1,0,5,0,93,8,0,10,0,12,0,96,9,0,1,0,1,0,1,
  	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,3,1,116,
  	8,1,1,2,1,2,1,2,1,2,3,2,122,8,2,1,2,3,2,125,8,2,1,2,1,2,1,2,1,3,1,3,3,
  	3,132,8,3,1,3,1,3,1,4,1,4,1,4,3,4,139,8,4,1,4,1,4,1,5,1,5,1,5,5,5,146,
  	8,5,10,5,12,5,149,9,5,1,6,1,6,1,6,5,6,154,8,6,10,6,12,6,157,9,6,1,7,1,
  	7,1,8,1,8,1,9,1,9,1,9,1,10,1,10,1,10,1,10,1,11,1,11,1,11,1,11,3,11,174,
  	8,11,1,11,1,11,1,11,1,11,1,11,1,11,3,11,182,8,11,1,11,1,11,3,11,186,8,
  	11,1,12,1,12,1,12,1,12,1,12,1,12,1,12,1,12,1,12,3,12,197,8,12,1,13,1,
  	13,1,13,1,13,1,13,1,13,1,13,1,13,1,13,1,13,1,13,1,13,1,13,3,13,212,8,
  	13,1,13,1,13,1,13,1,13,1,13,1,13,5,13,220,8,13,10,13,12,13,223,9,13,1,
  	14,1,14,1,14,1,14,1,14,1,14,1,14,1,14,1,14,1,15,1,15,1,15,1,15,1,15,1,
  	15,1,15,1,15,1,15,1,15,1,15,3,15,245,8,15,1,15,1,15,1,15,1,15,1,15,1,
  	15,5,15,253,8,15,10,15,12,15,256,9,15,1,16,1,16,3,16,260,8,16,1,17,1,
  	17,1,17,1,17,1,17,1,17,1,17,1,18,1,18,1,18,1,18,1,18,1,18,1,18,1,18,1,
  	18,1,18,3,18,279,8,18,1,19,1,19,1,19,1,19,1,19,1,19,1,20,1,20,3,20,289,
  	8,20,1,21,1,21,1,21,1,21,1,21,1,21,1,22,1,22,1,22,1,22,1,22,1,22,1,23,
  	1,23,1,23,1,23,3,23,307,8,23,1,24,1,24,1,24,1,24,3,24,313,8,24,1,25,1,
  	25,1,25,1,25,1,25,1,25,1,25,1,25,1,26,1,26,1,26,1,26,1,27,1,27,1,27,1,
  	27,1,27,1,27,1,28,1,28,1,29,1,29,1,29,1,29,5,29,339,8,29,10,29,12,29,
  	342,9,29,3,29,344,8,29,1,29,1,29,1,30,1,30,1,30,1,31,1,31,1,32,1,32,1,
  	32,3,32,356,8,32,1,32,1,32,1,33,1,33,1,33,5,33,363,8,33,10,33,12,33,366,
  	9,33,1,34,1,34,1,34,1,34,1,34,1,34,1,35,1,35,1,35,5,35,377,8,35,10,35,
  	12,35,380,9,35,1,35,1,35,1,35,3,35,385,8,35,1,36,1,36,1,36,1,36,1,37,
  	1,37,1,37,1,37,1,37,3,37,396,8,37,1,38,1,38,1,38,3,38,401,8,38,1,38,1,
  	38,1,38,5,38,406,8,38,10,38,12,38,409,9,38,1,39,1,39,1,39,1,39,1,39,1,
  	39,1,39,1,39,1,39,1,39,1,39,1,39,1,39,1,39,1,39,1,39,1,39,1,39,3,39,429,
  	8,39,1,40,1,40,1,40,1,40,1,40,1,40,1,40,1,40,1,40,1,40,1,40,1,40,1,40,
  	1,40,1,40,1,40,1,40,1,40,1,40,3,40,450,8,40,1,40,1,40,1,40,1,40,1,40,
  	1,40,5,40,458,8,40,10,40,12,40,461,9,40,1,41,1,41,1,41,1,41,1,41,1,41,
  	1,41,3,41,470,8,41,1,42,1,42,1,42,1,42,5,42,476,8,42,10,42,12,42,479,
  	9,42,1,42,1,42,1,43,1,43,1,43,1,43,1,43,1,43,1,43,3,43,490,8,43,1,44,
  	1,44,1,44,1,44,1,44,1,44,1,44,1,44,1,44,0,4,26,30,76,80,45,0,2,4,6,8,
  	10,12,14,16,18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,48,50,52,54,
  	56,58,60,62,64,66,68,70,72,74,76,78,80,82,84,86,88,0,5,1,0,56,61,3,0,
  	23,24,35,37,44,44,4,0,23,24,35,35,37,39,44,44,1,0,52,53,1,0,50,51,523,
  	0,94,1,0,0,0,2,115,1,0,0,0,4,117,1,0,0,0,6,129,1,0,0,0,8,135,1,0,0,0,
  	10,142,1,0,0,0,12,150,1,0,0,0,14,158,1,0,0,0,16,160,1,0,0,0,18,162,1,
  	0,0,0,20,165,1,0,0,0,22,185,1,0,0,0,24,187,1,0,0,0,26,211,1,0,0,0,28,
  	224,1,0,0,0,30,244,1,0,0,0,32,259,1,0,0,0,34,261,1,0,0,0,36,278,1,0,0,
  	0,38,280,1,0,0,0,40,288,1,0,0,0,42,290,1,0,0,0,44,296,1,0,0,0,46,306,
  	1,0,0,0,48,312,1,0,0,0,50,314,1,0,0,0,52,322,1,0,0,0,54,326,1,0,0,0,56,
  	332,1,0,0,0,58,334,1,0,0,0,60,347,1,0,0,0,62,350,1,0,0,0,64,352,1,0,0,
  	0,66,359,1,0,0,0,68,367,1,0,0,0,70,384,1,0,0,0,72,386,1,0,0,0,74,395,
  	1,0,0,0,76,400,1,0,0,0,78,428,1,0,0,0,80,449,1,0,0,0,82,469,1,0,0,0,84,
  	471,1,0,0,0,86,489,1,0,0,0,88,491,1,0,0,0,90,93,3,2,1,0,91,93,3,54,27,
  	0,92,90,1,0,0,0,92,91,1,0,0,0,93,96,1,0,0,0,94,92,1,0,0,0,94,95,1,0,0,
  	0,95,97,1,0,0,0,96,94,1,0,0,0,97,98,5,0,0,1,98,1,1,0,0,0,99,116,3,4,2,
  	0,100,116,3,24,12,0,101,116,3,74,37,0,102,116,3,38,19,0,103,116,3,22,
  	11,0,104,105,3,64,32,0,105,106,5,1,0,0,106,116,1,0,0,0,107,116,3,68,34,
  	0,108,116,3,28,14,0,109,116,3,88,44,0,110,116,3,86,43,0,111,116,3,50,
  	25,0,112,116,3,52,26,0,113,116,3,40,20,0,114,116,5,1,0,0,115,99,1,0,0,
  	0,115,100,1,0,0,0,115,101,1,0,0,0,115,102,1,0,0,0,115,103,1,0,0,0,115,
  	104,1,0,0,0,115,107,1,0,0,0,115,108,1,0,0,0,115,109,1,0,0,0,115,110,1,
  	0,0,0,115,111,1,0,0,0,115,112,1,0,0,0,115,113,1,0,0,0,115,114,1,0,0,0,
  	116,3,1,0,0,0,117,118,5,44,0,0,118,119,3,14,7,0,119,121,5,2,0,0,120,122,
  	3,6,3,0,121,120,1,0,0,0,121,122,1,0,0,0,122,124,1,0,0,0,123,125,3,8,4,
  	0,124,123,1,0,0,0,124,125,1,0,0,0,125,126,1,0,0,0,126,127,5,3,0,0,127,
  	128,5,1,0,0,128,5,1,0,0,0,129,131,5,4,0,0,130,132,3,10,5,0,131,130,1,
  	0,0,0,131,132,1,0,0,0,132,133,1,0,0,0,133,134,5,1,0,0,134,7,1,0,0,0,135,
  	138,5,5,0,0,136,139,3,12,6,0,137,139,3,18,9,0,138,136,1,0,0,0,138,137,
  	1,0,0,0,139,140,1,0,0,0,140,141,5,1,0,0,141,9,1,0,0,0,142,147,3,16,8,
  	0,143,144,5,6,0,0,144,146,3,16,8,0,145,143,1,0,0,0,146,149,1,0,0,0,147,
  	145,1,0,0,0,147,148,1,0,0,0,148,11,1,0,0,0,149,147,1,0,0,0,150,155,3,
  	20,10,0,151,152,5,6,0,0,152,154,3,20,10,0,153,151,1,0,0,0,154,157,1,0,
  	0,0,155,153,1,0,0,0,155,156,1,0,0,0,156,13,1,0,0,0,157,155,1,0,0,0,158,
  	159,5,62,0,0,159,15,1,0,0,0,160,161,5,63,0,0,161,17,1,0,0,0,162,163,5,
  	7,0,0,163,164,5,65,0,0,164,19,1,0,0,0,165,166,3,16,8,0,166,167,5,8,0,
  	0,167,168,3,16,8,0,168,21,1,0,0,0,169,170,3,62,31,0,170,173,5,62,0,0,
  	171,172,5,9,0,0,172,174,3,80,40,0,173,171,1,0,0,0,173,174,1,0,0,0,174,
  	175,1,0,0,0,175,176,5,1,0,0,176,186,1,0,0,0,177,178,3,62,31,0,178,181,
  	3,82,41,0,179,180,5,9,0,0,180,182,3,84,42,0,181,179,1,0,0,0,181,182,1,
  	0,0,0,182,183,1,0,0,0,183,184,5,1,0,0,184,186,1,0,0,0,185,169,1,0,0,0,
  	185,177,1,0,0,0,186,23,1,0,0,0,187,188,5,10,0,0,188,189,5,11,0,0,189,
  	190,3,26,13,0,190,191,5,12,0,0,191,196,3,70,35,0,192,193,5,13,0,0,193,
  	197,3,24,12,0,194,195,5,13,0,0,195,197,3,70,35,0,196,192,1,0,0,0,196,
  	194,1,0,0,0,196,197,1,0,0,0,197,25,1,0,0,0,198,199,6,13,-1,0,199,200,
  	3,80,40,0,200,201,7,0,0,0,201,202,3,80,40,0,202,212,1,0,0,0,203,204,3,
  	16,8,0,204,205,5,14,0,0,205,206,3,14,7,0,206,212,1,0,0,0,207,208,3,20,
  	10,0,208,209,5,14,0,0,209,210,3,14,7,0,210,212,1,0,0,0,211,198,1,0,0,
  	0,211,203,1,0,0,0,211,207,1,0,0,0,212,221,1,0,0,0,213,214,10,5,0,0,214,
  	215,5,54,0,0,215,220,3,26,13,6,216,217,10,4,0,0,217,218,5,55,0,0,218,
  	220,3,26,13,5,219,213,1,0,0,0,219,216,1,0,0,0,220,223,1,0,0,0,221,219,
  	1,0,0,0,221,222,1,0,0,0,222,27,1,0,0,0,223,221,1,0,0,0,224,225,5,62,0,
  	0,225,226,5,9,0,0,226,227,5,15,0,0,227,228,3,14,7,0,228,229,5,16,0,0,
  	229,230,3,30,15,0,230,231,5,17,0,0,231,232,5,1,0,0,232,29,1,0,0,0,233,
  	234,6,15,-1,0,234,235,5,18,0,0,235,236,7,0,0,0,236,245,5,63,0,0,237,238,
  	5,19,0,0,238,239,5,20,0,0,239,245,3,16,8,0,240,241,5,11,0,0,241,242,3,
  	30,15,0,242,243,5,12,0,0,243,245,1,0,0,0,244,233,1,0,0,0,244,237,1,0,
  	0,0,244,240,1,0,0,0,245,254,1,0,0,0,246,247,10,5,0,0,247,248,5,54,0,0,
  	248,253,3,30,15,6,249,250,10,4,0,0,250,251,5,55,0,0,251,253,3,30,15,5,
  	252,246,1,0,0,0,252,249,1,0,0,0,253,256,1,0,0,0,254,252,1,0,0,0,254,255,
  	1,0,0,0,255,31,1,0,0,0,256,254,1,0,0,0,257,260,3,34,17,0,258,260,3,38,
  	19,0,259,257,1,0,0,0,259,258,1,0,0,0,260,33,1,0,0,0,261,262,5,21,0,0,
  	262,263,5,22,0,0,263,264,3,36,18,0,264,265,5,14,0,0,265,266,3,14,7,0,
  	266,267,3,70,35,0,267,35,1,0,0,0,268,269,5,23,0,0,269,279,5,62,0,0,270,
  	271,5,24,0,0,271,272,5,62,0,0,272,273,5,6,0,0,273,279,5,62,0,0,274,275,
  	5,25,0,0,275,276,5,62,0,0,276,277,5,49,0,0,277,279,3,16,8,0,278,268,1,
  	0,0,0,278,270,1,0,0,0,278,274,1,0,0,0,279,37,1,0,0,0,280,281,5,26,0,0,
  	281,282,5,11,0,0,282,283,3,26,13,0,283,284,5,12,0,0,284,285,3,70,35,0,
  	285,39,1,0,0,0,286,289,3,42,21,0,287,289,3,44,22,0,288,286,1,0,0,0,288,
  	287,1,0,0,0,289,41,1,0,0,0,290,291,5,27,0,0,291,292,3,46,23,0,292,293,
  	5,28,0,0,293,294,3,14,7,0,294,295,5,1,0,0,295,43,1,0,0,0,296,297,5,29,
  	0,0,297,298,3,48,24,0,298,299,5,30,0,0,299,300,3,14,7,0,300,301,5,1,0,
  	0,301,45,1,0,0,0,302,307,3,16,8,0,303,307,3,20,10,0,304,307,3,10,5,0,
  	305,307,3,12,6,0,306,302,1,0,0,0,306,303,1,0,0,0,306,304,1,0,0,0,306,
  	305,1,0,0,0,307,47,1,0,0,0,308,313,3,16,8,0,309,313,3,20,10,0,310,313,
  	3,10,5,0,311,313,3,12,6,0,312,308,1,0,0,0,312,309,1,0,0,0,312,310,1,0,
  	0,0,312,311,1,0,0,0,313,49,1,0,0,0,314,315,5,31,0,0,315,316,5,62,0,0,
  	316,317,5,32,0,0,317,318,5,65,0,0,318,319,5,49,0,0,319,320,3,14,7,0,320,
  	321,5,1,0,0,321,51,1,0,0,0,322,323,5,33,0,0,323,324,3,14,7,0,324,325,
  	5,1,0,0,325,53,1,0,0,0,326,327,5,34,0,0,327,328,3,56,28,0,328,329,5,62,
  	0,0,329,330,3,58,29,0,330,331,3,70,35,0,331,55,1,0,0,0,332,333,7,1,0,
  	0,333,57,1,0,0,0,334,343,5,11,0,0,335,340,3,60,30,0,336,337,5,6,0,0,337,
  	339,3,60,30,0,338,336,1,0,0,0,339,342,1,0,0,0,340,338,1,0,0,0,340,341,
  	1,0,0,0,341,344,1,0,0,0,342,340,1,0,0,0,343,335,1,0,0,0,343,344,1,0,0,
  	0,344,345,1,0,0,0,345,346,5,12,0,0,346,59,1,0,0,0,347,348,3,62,31,0,348,
  	349,5,62,0,0,349,61,1,0,0,0,350,351,7,2,0,0,351,63,1,0,0,0,352,353,5,
  	62,0,0,353,355,5,11,0,0,354,356,3,66,33,0,355,354,1,0,0,0,355,356,1,0,
  	0,0,356,357,1,0,0,0,357,358,5,12,0,0,358,65,1,0,0,0,359,364,3,80,40,0,
  	360,361,5,6,0,0,361,363,3,80,40,0,362,360,1,0,0,0,363,366,1,0,0,0,364,
  	362,1,0,0,0,364,365,1,0,0,0,365,67,1,0,0,0,366,364,1,0,0,0,367,368,5,
  	40,0,0,368,369,5,11,0,0,369,370,3,80,40,0,370,371,5,12,0,0,371,372,5,
  	1,0,0,372,69,1,0,0,0,373,378,5,2,0,0,374,377,3,2,1,0,375,377,3,72,36,
  	0,376,374,1,0,0,0,376,375,1,0,0,0,377,380,1,0,0,0,378,376,1,0,0,0,378,
  	379,1,0,0,0,379,381,1,0,0,0,380,378,1,0,0,0,381,385,5,3,0,0,382,383,5,
  	2,0,0,383,385,5,3,0,0,384,373,1,0,0,0,384,382,1,0,0,0,385,71,1,0,0,0,
  	386,387,5,41,0,0,387,388,3,80,40,0,388,389,5,1,0,0,389,73,1,0,0,0,390,
  	391,5,42,0,0,391,392,3,76,38,0,392,393,5,1,0,0,393,396,1,0,0,0,394,396,
  	3,78,39,0,395,390,1,0,0,0,395,394,1,0,0,0,396,75,1,0,0,0,397,398,6,38,
  	-1,0,398,401,5,65,0,0,399,401,3,80,40,0,400,397,1,0,0,0,400,399,1,0,0,
  	0,401,407,1,0,0,0,402,403,10,1,0,0,403,404,5,50,0,0,404,406,3,76,38,2,
  	405,402,1,0,0,0,406,409,1,0,0,0,407,405,1,0,0,0,407,408,1,0,0,0,408,77,
  	1,0,0,0,409,407,1,0,0,0,410,411,5,42,0,0,411,412,5,45,0,0,412,413,5,49,
  	0,0,413,414,3,14,7,0,414,415,5,1,0,0,415,429,1,0,0,0,416,417,5,42,0,0,
  	417,418,5,46,0,0,418,419,5,49,0,0,419,420,3,14,7,0,420,421,5,1,0,0,421,
  	429,1,0,0,0,422,423,5,42,0,0,423,424,5,44,0,0,424,425,5,49,0,0,425,426,
  	3,14,7,0,426,427,5,1,0,0,427,429,1,0,0,0,428,410,1,0,0,0,428,416,1,0,
  	0,0,428,422,1,0,0,0,429,79,1,0,0,0,430,431,6,40,-1,0,431,450,3,64,32,
  	0,432,450,5,63,0,0,433,450,5,62,0,0,434,435,5,11,0,0,435,436,3,80,40,
  	0,436,437,5,12,0,0,437,450,1,0,0,0,438,439,5,62,0,0,439,440,5,15,0,0,
  	440,441,3,80,40,0,441,442,5,17,0,0,442,450,1,0,0,0,443,450,5,47,0,0,444,
  	450,5,48,0,0,445,450,5,64,0,0,446,447,5,43,0,0,447,448,5,11,0,0,448,450,
  	5,12,0,0,449,430,1,0,0,0,449,432,1,0,0,0,449,433,1,0,0,0,449,434,1,0,
  	0,0,449,438,1,0,0,0,449,443,1,0,0,0,449,444,1,0,0,0,449,445,1,0,0,0,449,
  	446,1,0,0,0,450,459,1,0,0,0,451,452,10,11,0,0,452,453,7,3,0,0,453,458,
  	3,80,40,12,454,455,10,10,0,0,455,456,7,4,0,0,456,458,3,80,40,11,457,451,
  	1,0,0,0,457,454,1,0,0,0,458,461,1,0,0,0,459,457,1,0,0,0,459,460,1,0,0,
  	0,460,81,1,0,0,0,461,459,1,0,0,0,462,463,5,62,0,0,463,464,5,15,0,0,464,
  	465,5,63,0,0,465,470,5,17,0,0,466,467,5,62,0,0,467,468,5,15,0,0,468,470,
  	5,17,0,0,469,462,1,0,0,0,469,466,1,0,0,0,470,83,1,0,0,0,471,472,5,15,
  	0,0,472,477,3,80,40,0,473,474,5,6,0,0,474,476,3,80,40,0,475,473,1,0,0,
  	0,476,479,1,0,0,0,477,475,1,0,0,0,477,478,1,0,0,0,478,480,1,0,0,0,479,
  	477,1,0,0,0,480,481,5,17,0,0,481,85,1,0,0,0,482,483,5,62,0,0,483,484,
  	5,9,0,0,484,485,3,80,40,0,485,486,5,1,0,0,486,490,1,0,0,0,487,488,5,62,
  	0,0,488,490,5,1,0,0,489,482,1,0,0,0,489,487,1,0,0,0,490,87,1,0,0,0,491,
  	492,5,62,0,0,492,493,5,15,0,0,493,494,3,80,40,0,494,495,5,17,0,0,495,
  	496,5,9,0,0,496,497,3,80,40,0,497,498,5,1,0,0,498,89,1,0,0,0,41,92,94,
  	115,121,124,131,138,147,155,173,181,185,196,211,219,221,244,252,254,259,
  	278,288,306,312,340,343,355,364,376,378,384,395,400,407,428,449,457,459,
  	469,477,489
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
    setState(94);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 4611710133284635650) != 0)) {
      setState(92);
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
        case BaseParser::T__39:
        case BaseParser::T__41:
        case BaseParser::GRAPH:
        case BaseParser::ID: {
          setState(90);
          statement();
          break;
        }

        case BaseParser::T__33: {
          setState(91);
          function();
          break;
        }

      default:
        throw NoViableAltException(this);
      }
      setState(96);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(97);
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
    setState(115);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 2, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(99);
      graphDef();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(100);
      conditionalStatement();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(101);
      printStatement();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(102);
      whileStatement();
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(103);
      varDecl();
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(104);
      functionCall();
      setState(105);
      match(BaseParser::T__0);
      break;
    }

    case 7: {
      enterOuterAlt(_localctx, 7);
      setState(107);
      sleepStatement();
      break;
    }

    case 8: {
      enterOuterAlt(_localctx, 8);
      setState(108);
      graphComprehension();
      break;
    }

    case 9: {
      enterOuterAlt(_localctx, 9);
      setState(109);
      arrayAssignStatement();
      break;
    }

    case 10: {
      enterOuterAlt(_localctx, 10);
      setState(110);
      assignmentStatement();
      break;
    }

    case 11: {
      enterOuterAlt(_localctx, 11);
      setState(111);
      queryStatement();
      break;
    }

    case 12: {
      enterOuterAlt(_localctx, 12);
      setState(112);
      showgraph();
      break;
    }

    case 13: {
      enterOuterAlt(_localctx, 13);
      setState(113);
      nodeEdgeOperation();
      break;
    }

    case 14: {
      enterOuterAlt(_localctx, 14);
      setState(114);
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

void BaseParser::GraphDefContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterGraphDef(this);
}

void BaseParser::GraphDefContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<BaseListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitGraphDef(this);
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
    setState(117);
    match(BaseParser::GRAPH);
    setState(118);
    graphID();
    setState(119);
    match(BaseParser::T__1);
    setState(121);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == BaseParser::T__3) {
      setState(120);
      nodes();
    }
    setState(124);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == BaseParser::T__4) {
      setState(123);
      edges();
    }
    setState(126);
    match(BaseParser::T__2);
    setState(127);
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
    setState(129);
    match(BaseParser::T__3);
    setState(131);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == BaseParser::INT) {
      setState(130);
      nodeList();
    }
    setState(133);
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
    setState(135);
    match(BaseParser::T__4);
    setState(138);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case BaseParser::INT: {
        setState(136);
        edgeList();
        break;
      }

      case BaseParser::T__6: {
        setState(137);
        fileEdgeList();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    setState(140);
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
    setState(142);
    nodeID();
    setState(147);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == BaseParser::T__5) {
      setState(143);
      match(BaseParser::T__5);
      setState(144);
      nodeID();
      setState(149);
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
    setState(150);
    edge();
    setState(155);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == BaseParser::T__5) {
      setState(151);
      match(BaseParser::T__5);
      setState(152);
      edge();
      setState(157);
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
    setState(158);
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
    setState(160);
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
    setState(162);
    match(BaseParser::T__6);
    setState(163);
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
    setState(165);
    nodeID();
    setState(166);
    match(BaseParser::T__7);
    setState(167);
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
    setState(185);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 11, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<BaseParser::SimpleDeclarationContext>(_localctx);
      enterOuterAlt(_localctx, 1);
      setState(169);
      type();
      setState(170);
      match(BaseParser::ID);
      setState(173);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == BaseParser::T__8) {
        setState(171);
        match(BaseParser::T__8);
        setState(172);
        expr(0);
      }
      setState(175);
      match(BaseParser::T__0);
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<BaseParser::ArrayDeclarationContext>(_localctx);
      enterOuterAlt(_localctx, 2);
      setState(177);
      type();
      setState(178);
      arrayDeclarator();
      setState(181);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == BaseParser::T__8) {
        setState(179);
        match(BaseParser::T__8);
        setState(180);
        arrayInitializer();
      }
      setState(183);
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
    setState(187);
    match(BaseParser::T__9);
    setState(188);
    match(BaseParser::T__10);
    setState(189);
    condition(0);
    setState(190);
    match(BaseParser::T__11);
    setState(191);
    block();
    setState(196);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 12, _ctx)) {
    case 1: {
      setState(192);
      match(BaseParser::T__12);
      setState(193);
      conditionalStatement();
      break;
    }

    case 2: {
      setState(194);
      match(BaseParser::T__12);
      setState(195);
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
    setState(211);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 13, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<RelationalContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;

      setState(199);
      expr(0);
      setState(200);
      _la = _input->LA(1);
      if (!((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 4539628424389459968) != 0))) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(201);
      expr(0);
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<NodeCheckContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(203);
      nodeID();
      setState(204);
      match(BaseParser::T__13);
      setState(205);
      graphID();
      break;
    }

    case 3: {
      _localctx = _tracker.createInstance<EdgeCheckContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(207);
      edge();
      setState(208);
      match(BaseParser::T__13);
      setState(209);
      graphID();
      break;
    }

    default:
      break;
    }
    _ctx->stop = _input->LT(-1);
    setState(221);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 15, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(219);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 14, _ctx)) {
        case 1: {
          auto newContext = _tracker.createInstance<LogicalAndContext>(_tracker.createInstance<ConditionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleCondition);
          setState(213);

          if (!(precpred(_ctx, 5))) throw FailedPredicateException(this, "precpred(_ctx, 5)");
          setState(214);
          match(BaseParser::AND);
          setState(215);
          condition(6);
          break;
        }

        case 2: {
          auto newContext = _tracker.createInstance<LogicalOrContext>(_tracker.createInstance<ConditionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleCondition);
          setState(216);

          if (!(precpred(_ctx, 4))) throw FailedPredicateException(this, "precpred(_ctx, 4)");
          setState(217);
          match(BaseParser::OR);
          setState(218);
          condition(5);
          break;
        }

        default:
          break;
        } 
      }
      setState(223);
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
    setState(224);
    match(BaseParser::ID);
    setState(225);
    match(BaseParser::T__8);
    setState(226);
    match(BaseParser::T__14);
    setState(227);
    graphID();
    setState(228);
    match(BaseParser::T__15);
    setState(229);
    graphCondition(0);
    setState(230);
    match(BaseParser::T__16);
    setState(231);
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
    setState(244);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case BaseParser::T__17: {
        _localctx = _tracker.createInstance<DegreeConditionContext>(_localctx);
        _ctx = _localctx;
        previousContext = _localctx;

        setState(234);
        match(BaseParser::T__17);
        setState(235);
        _la = _input->LA(1);
        if (!((((_la & ~ 0x3fULL) == 0) &&
          ((1ULL << _la) & 4539628424389459968) != 0))) {
        _errHandler->recoverInline(this);
        }
        else {
          _errHandler->reportMatch(this);
          consume();
        }
        setState(236);
        match(BaseParser::INT);
        break;
      }

      case BaseParser::T__18: {
        _localctx = _tracker.createInstance<ConnectedConditionContext>(_localctx);
        _ctx = _localctx;
        previousContext = _localctx;
        setState(237);
        match(BaseParser::T__18);
        setState(238);
        match(BaseParser::T__19);
        setState(239);
        nodeID();
        break;
      }

      case BaseParser::T__10: {
        _localctx = _tracker.createInstance<ParenGraphConditionContext>(_localctx);
        _ctx = _localctx;
        previousContext = _localctx;
        setState(240);
        match(BaseParser::T__10);
        setState(241);
        graphCondition(0);
        setState(242);
        match(BaseParser::T__11);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    _ctx->stop = _input->LT(-1);
    setState(254);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 18, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(252);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 17, _ctx)) {
        case 1: {
          auto newContext = _tracker.createInstance<GraphLogicalAndContext>(_tracker.createInstance<GraphConditionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleGraphCondition);
          setState(246);

          if (!(precpred(_ctx, 5))) throw FailedPredicateException(this, "precpred(_ctx, 5)");
          setState(247);
          match(BaseParser::AND);
          setState(248);
          graphCondition(6);
          break;
        }

        case 2: {
          auto newContext = _tracker.createInstance<GraphLogicalOrContext>(_tracker.createInstance<GraphConditionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleGraphCondition);
          setState(249);

          if (!(precpred(_ctx, 4))) throw FailedPredicateException(this, "precpred(_ctx, 4)");
          setState(250);
          match(BaseParser::OR);
          setState(251);
          graphCondition(5);
          break;
        }

        default:
          break;
        } 
      }
      setState(256);
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
  enterRule(_localctx, 32, BaseParser::RuleLoopStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(259);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case BaseParser::T__20: {
        enterOuterAlt(_localctx, 1);
        setState(257);
        foreachStatement();
        break;
      }

      case BaseParser::T__25: {
        enterOuterAlt(_localctx, 2);
        setState(258);
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
    setState(261);
    match(BaseParser::T__20);
    setState(262);
    match(BaseParser::T__21);
    setState(263);
    loopTarget();
    setState(264);
    match(BaseParser::T__13);
    setState(265);
    graphID();
    setState(266);
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

BaseParser::NodeIDContext* BaseParser::ForEachAdjContext::nodeID() {
  return getRuleContext<BaseParser::NodeIDContext>(0);
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
  enterRule(_localctx, 36, BaseParser::RuleLoopTarget);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(278);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case BaseParser::T__22: {
        _localctx = _tracker.createInstance<BaseParser::ForEachVertexContext>(_localctx);
        enterOuterAlt(_localctx, 1);
        setState(268);
        match(BaseParser::T__22);
        setState(269);
        match(BaseParser::ID);
        break;
      }

      case BaseParser::T__23: {
        _localctx = _tracker.createInstance<BaseParser::ForEachEdgeContext>(_localctx);
        enterOuterAlt(_localctx, 2);
        setState(270);
        match(BaseParser::T__23);
        setState(271);
        match(BaseParser::ID);
        setState(272);
        match(BaseParser::T__5);
        setState(273);
        match(BaseParser::ID);
        break;
      }

      case BaseParser::T__24: {
        _localctx = _tracker.createInstance<BaseParser::ForEachAdjContext>(_localctx);
        enterOuterAlt(_localctx, 3);
        setState(274);
        match(BaseParser::T__24);
        setState(275);
        match(BaseParser::ID);
        setState(276);
        match(BaseParser::OF);
        setState(277);
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
    setState(280);
    match(BaseParser::T__25);
    setState(281);
    match(BaseParser::T__10);
    setState(282);
    condition(0);
    setState(283);
    match(BaseParser::T__11);
    setState(284);
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
  enterRule(_localctx, 40, BaseParser::RuleNodeEdgeOperation);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(288);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case BaseParser::T__26: {
        enterOuterAlt(_localctx, 1);
        setState(286);
        addOperation();
        break;
      }

      case BaseParser::T__28: {
        enterOuterAlt(_localctx, 2);
        setState(287);
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
    setState(290);
    match(BaseParser::T__26);
    setState(291);
    addTargets();
    setState(292);
    match(BaseParser::T__27);
    setState(293);
    graphID();
    setState(294);
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
    setState(296);
    match(BaseParser::T__28);
    setState(297);
    removeTargets();
    setState(298);
    match(BaseParser::T__29);
    setState(299);
    graphID();
    setState(300);
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
  enterRule(_localctx, 46, BaseParser::RuleAddTargets);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(306);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 22, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(302);
      nodeID();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(303);
      edge();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(304);
      nodeList();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(305);
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
  enterRule(_localctx, 48, BaseParser::RuleRemoveTargets);

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
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 23, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(308);
      nodeID();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(309);
      edge();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(310);
      nodeList();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(311);
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
    setState(314);
    match(BaseParser::T__30);
    setState(315);
    match(BaseParser::ID);
    setState(316);
    match(BaseParser::T__31);
    setState(317);
    match(BaseParser::STRING);
    setState(318);
    match(BaseParser::OF);
    setState(319);
    graphID();
    setState(320);
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
    setState(322);
    match(BaseParser::T__32);
    setState(323);
    graphID();
    setState(324);
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
    setState(326);
    match(BaseParser::T__33);
    setState(327);
    returnType();
    setState(328);
    match(BaseParser::ID);
    setState(329);
    paramList();
    setState(330);
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
    setState(332);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 17832729378816) != 0))) {
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
    setState(334);
    match(BaseParser::T__10);
    setState(343);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 18588643622912) != 0)) {
      setState(335);
      param();
      setState(340);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == BaseParser::T__5) {
        setState(336);
        match(BaseParser::T__5);
        setState(337);
        param();
        setState(342);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
    }
    setState(345);
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
    setState(347);
    type();
    setState(348);
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
    setState(350);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 18588643622912) != 0))) {
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
    setState(352);
    match(BaseParser::ID);
    setState(353);
    match(BaseParser::T__10);
    setState(355);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (((((_la - 11) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 11)) & 15762809149194241) != 0)) {
      setState(354);
      argumentList();
    }
    setState(357);
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
    setState(359);
    expr(0);
    setState(364);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == BaseParser::T__5) {
      setState(360);
      match(BaseParser::T__5);
      setState(361);
      expr(0);
      setState(366);
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
  enterRule(_localctx, 68, BaseParser::RuleSleepStatement);

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
    match(BaseParser::T__39);
    setState(368);
    match(BaseParser::T__10);
    setState(369);
    expr(0);
    setState(370);
    match(BaseParser::T__11);
    setState(371);
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
  enterRule(_localctx, 70, BaseParser::RuleBlock);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(384);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 30, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(373);
      match(BaseParser::T__1);
      setState(378);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while ((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 4611712315128022018) != 0)) {
        setState(376);
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
          case BaseParser::T__39:
          case BaseParser::T__41:
          case BaseParser::GRAPH:
          case BaseParser::ID: {
            setState(374);
            statement();
            break;
          }

          case BaseParser::T__40: {
            setState(375);
            returnStatement();
            break;
          }

        default:
          throw NoViableAltException(this);
        }
        setState(380);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
      setState(381);
      match(BaseParser::T__2);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(382);
      match(BaseParser::T__1);
      setState(383);
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
  enterRule(_localctx, 72, BaseParser::RuleReturnStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(386);
    match(BaseParser::T__40);
    setState(387);
    expr(0);
    setState(388);
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
  enterRule(_localctx, 74, BaseParser::RulePrintStatement);

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
      enterOuterAlt(_localctx, 1);
      setState(390);
      match(BaseParser::T__41);
      setState(391);
      printExpr(0);
      setState(392);
      match(BaseParser::T__0);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(394);
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
  size_t startState = 76;
  enterRecursionRule(_localctx, 76, BaseParser::RulePrintExpr, precedence);

    

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
    setState(400);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case BaseParser::STRING: {
        setState(398);
        match(BaseParser::STRING);
        break;
      }

      case BaseParser::T__10:
      case BaseParser::T__42:
      case BaseParser::TRUE:
      case BaseParser::FALSE:
      case BaseParser::ID:
      case BaseParser::INT:
      case BaseParser::REAL: {
        setState(399);
        expr(0);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    _ctx->stop = _input->LT(-1);
    setState(407);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 33, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        _localctx = _tracker.createInstance<PrintExprContext>(parentContext, parentState);
        pushNewRecursionContext(_localctx, startState, RulePrintExpr);
        setState(402);

        if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
        setState(403);
        match(BaseParser::PLUS);
        setState(404);
        printExpr(2); 
      }
      setState(409);
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
  enterRule(_localctx, 78, BaseParser::RulePrintgraph);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(428);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 34, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<BaseParser::EdgePrintContext>(_localctx);
      enterOuterAlt(_localctx, 1);
      setState(410);
      match(BaseParser::T__41);
      setState(411);
      match(BaseParser::EDGE);
      setState(412);
      match(BaseParser::OF);
      setState(413);
      graphID();
      setState(414);
      match(BaseParser::T__0);
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<BaseParser::NodePrintContext>(_localctx);
      enterOuterAlt(_localctx, 2);
      setState(416);
      match(BaseParser::T__41);
      setState(417);
      match(BaseParser::NODE);
      setState(418);
      match(BaseParser::OF);
      setState(419);
      graphID();
      setState(420);
      match(BaseParser::T__0);
      break;
    }

    case 3: {
      _localctx = _tracker.createInstance<BaseParser::GraphPrintContext>(_localctx);
      enterOuterAlt(_localctx, 3);
      setState(422);
      match(BaseParser::T__41);
      setState(423);
      match(BaseParser::GRAPH);
      setState(424);
      match(BaseParser::OF);
      setState(425);
      graphID();
      setState(426);
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
  size_t startState = 80;
  enterRecursionRule(_localctx, 80, BaseParser::RuleExpr, precedence);

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
    setState(449);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 35, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<FuncExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;

      setState(431);
      functionCall();
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<IntExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(432);
      match(BaseParser::INT);
      break;
    }

    case 3: {
      _localctx = _tracker.createInstance<IdExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(433);
      match(BaseParser::ID);
      break;
    }

    case 4: {
      _localctx = _tracker.createInstance<ParenExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(434);
      match(BaseParser::T__10);
      setState(435);
      expr(0);
      setState(436);
      match(BaseParser::T__11);
      break;
    }

    case 5: {
      _localctx = _tracker.createInstance<ArrayAccessExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(438);
      match(BaseParser::ID);
      setState(439);
      match(BaseParser::T__14);
      setState(440);
      expr(0);
      setState(441);
      match(BaseParser::T__16);
      break;
    }

    case 6: {
      _localctx = _tracker.createInstance<BoolTrueExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(443);
      match(BaseParser::TRUE);
      break;
    }

    case 7: {
      _localctx = _tracker.createInstance<BoolFalseExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(444);
      match(BaseParser::FALSE);
      break;
    }

    case 8: {
      _localctx = _tracker.createInstance<RealExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(445);
      match(BaseParser::REAL);
      break;
    }

    case 9: {
      _localctx = _tracker.createInstance<TimerExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(446);
      match(BaseParser::T__42);
      setState(447);
      match(BaseParser::T__10);
      setState(448);
      match(BaseParser::T__11);
      break;
    }

    default:
      break;
    }
    _ctx->stop = _input->LT(-1);
    setState(459);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 37, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(457);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 36, _ctx)) {
        case 1: {
          auto newContext = _tracker.createInstance<MulDivExprContext>(_tracker.createInstance<ExprContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpr);
          setState(451);

          if (!(precpred(_ctx, 11))) throw FailedPredicateException(this, "precpred(_ctx, 11)");
          setState(452);
          _la = _input->LA(1);
          if (!(_la == BaseParser::TIMES

          || _la == BaseParser::DIVIDE)) {
          _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(453);
          expr(12);
          break;
        }

        case 2: {
          auto newContext = _tracker.createInstance<AddSubExprContext>(_tracker.createInstance<ExprContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpr);
          setState(454);

          if (!(precpred(_ctx, 10))) throw FailedPredicateException(this, "precpred(_ctx, 10)");
          setState(455);
          _la = _input->LA(1);
          if (!(_la == BaseParser::PLUS

          || _la == BaseParser::MINUS)) {
          _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(456);
          expr(11);
          break;
        }

        default:
          break;
        } 
      }
      setState(461);
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
  enterRule(_localctx, 82, BaseParser::RuleArrayDeclarator);

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
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 38, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<BaseParser::SizedArrayContext>(_localctx);
      enterOuterAlt(_localctx, 1);
      setState(462);
      match(BaseParser::ID);
      setState(463);
      match(BaseParser::T__14);
      setState(464);
      match(BaseParser::INT);
      setState(465);
      match(BaseParser::T__16);
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<BaseParser::UnsizedArrayContext>(_localctx);
      enterOuterAlt(_localctx, 2);
      setState(466);
      match(BaseParser::ID);
      setState(467);
      match(BaseParser::T__14);
      setState(468);
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
  enterRule(_localctx, 84, BaseParser::RuleArrayInitializer);
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
    match(BaseParser::T__14);
    setState(472);
    expr(0);
    setState(477);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == BaseParser::T__5) {
      setState(473);
      match(BaseParser::T__5);
      setState(474);
      expr(0);
      setState(479);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(480);
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
  enterRule(_localctx, 86, BaseParser::RuleAssignmentStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(489);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 40, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(482);
      match(BaseParser::ID);
      setState(483);
      match(BaseParser::T__8);
      setState(484);
      expr(0);
      setState(485);
      match(BaseParser::T__0);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(487);
      match(BaseParser::ID);
      setState(488);
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
  enterRule(_localctx, 88, BaseParser::RuleArrayAssignStatement);

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
    setState(491);
    match(BaseParser::ID);
    setState(492);
    match(BaseParser::T__14);
    setState(493);
    expr(0);
    setState(494);
    match(BaseParser::T__16);
    setState(495);
    match(BaseParser::T__8);
    setState(496);
    expr(0);
    setState(497);
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
    case 38: return printExprSempred(antlrcpp::downCast<PrintExprContext *>(context), predicateIndex);
    case 40: return exprSempred(antlrcpp::downCast<ExprContext *>(context), predicateIndex);

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
    case 5: return precpred(_ctx, 11);
    case 6: return precpred(_ctx, 10);

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
