#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "antlr4-runtime.h"
#include "BaseLexer.h"
#include "BaseParser.h"
#include "ASTBuilder.h"
#include "ASTNode.h"

using namespace antlr4;

namespace {

std::string escapeDot(const std::string &text)
{
    std::string out;
    out.reserve(text.size());
    for (char c : text)
    {
        switch (c)
        {
        case '\\':
            out += "\\\\";
            break;
        case '"':
            out += "\\\"";
            break;
        case '\n':
            out += "\\n";
            break;
        default:
            out += c;
            break;
        }
    }
    return out;
}

std::string joinEdges(const std::vector<std::pair<int, int>> &edges)
{
    std::ostringstream os;
    os << "[";
    for (size_t i = 0; i < edges.size(); ++i)
    {
        if (i != 0)
            os << ", ";
        os << "(" << edges[i].first << ", " << edges[i].second << ")";
    }
    os << "]";
    return os.str();
}

std::string graphUpdateKindToString(GraphUpdateKind kind)
{
    switch (kind)
    {
    case GraphUpdateKind::Add:
        return "Add";
    case GraphUpdateKind::Remove:
        return "Remove";
    }
    return "Unknown";
}

std::string nodeTypeToString(ASTNodeType type)
{
    switch (type)
    {
    case ASTNodeType::Program:
        return "ProgramNode";
    case ASTNodeType::GraphDecl:
        return "GraphDeclNode";
    case ASTNodeType::GraphUpdate:
        return "GraphUpdateNode";
    default:
        return "ASTNode";
    }
}

std::string makeNodeLabel(const ASTNodePtr &node)
{
    if (!node)
        return "null";

    if (auto prog = std::dynamic_pointer_cast<ProgramNode>(node))
    {
        std::ostringstream os;
        os << "ProgramNode";
        os << "\ntopLevelCount=" << prog->topLevel.size();
        return os.str();
    }

    if (auto graphDecl = std::dynamic_pointer_cast<GraphDeclNode>(node))
    {
        std::ostringstream os;
        os << "GraphDeclNode";
        os << "\nname=" << graphDecl->name;
        os << "\nisFileGraph=" << (graphDecl->isFileGraph ? "true" : "false");
        if (graphDecl->isFileGraph)
            os << "\nedgeFileName=" << graphDecl->edgeFileName;
        return os.str();
    }

    if (auto graphUpdate = std::dynamic_pointer_cast<GraphUpdateNode>(node))
    {
        std::ostringstream os;
        os << "GraphUpdateNode";
        os << "\nkind=" << graphUpdateKindToString(graphUpdate->kind);
        os << "\ngraphName=" << graphUpdate->graphName;
        os << "\nnodes=" << graphUpdate->nodes.size();
        os << "\nedges=" << joinEdges(graphUpdate->edges);
        return os.str();
    }

    return nodeTypeToString(node->type);
}

std::vector<ASTNodePtr> childNodes(const ASTNodePtr &node)
{
    if (!node)
        return {};

    if (auto prog = std::dynamic_pointer_cast<ProgramNode>(node))
        return prog->topLevel;

    return {};
}

class DotEmitter
{
public:
    explicit DotEmitter(std::ostream &out) : out_(out) {}

    void emit(const ASTNodePtr &root)
    {
        out_ << "digraph UserRequestAST {\n";
        out_ << "  rankdir=TB;\n";
        out_ << "  graph [bgcolor=\"white\", pad=\"0.3\", nodesep=\"0.35\", ranksep=\"0.45\"];\n";
        out_ << "  node [shape=box, style=\"rounded,filled\", fillcolor=\"#f8fafc\", color=\"#334155\", fontname=\"Helvetica\"];\n";
        out_ << "  edge [color=\"#64748b\", arrowsize=\"0.8\"];\n\n";
        visit(root);
        out_ << "}\n";
    }

private:
    std::ostream &out_;
    int nextId_ = 0;

    std::string visit(const ASTNodePtr &node)
    {
        const std::string nodeId = "n" + std::to_string(nextId_++);
        out_ << "  " << nodeId << " [label=\"" << escapeDot(makeNodeLabel(node)) << "\"];\n";

        for (const auto &child : childNodes(node))
        {
            const std::string childId = visit(child);
            out_ << "  " << nodeId << " -> " << childId << ";\n";
        }

        return nodeId;
    }
};

int runDot(const std::filesystem::path &dotPath, const std::filesystem::path &svgPath)
{
    const std::string command =
        "dot -Tsvg \"" + dotPath.string() + "\" -o \"" + svgPath.string() + "\"";
    return std::system(command.c_str());
}

} // namespace

int main()
{
    const std::filesystem::path rootDir =
        "/home/kazisahib/sgpl_main/Graph-language-/p1GraphEasy-con-AutoTuner";
    const std::filesystem::path inputPath = rootDir / "user_request_example.graph";
    const std::filesystem::path outDir = rootDir / "out";
    const std::filesystem::path dotPath = outDir / "user_request_ast.dot";
    const std::filesystem::path svgPath = outDir / "user_request_ast.svg";

    std::ifstream in(inputPath);
    if (!in.good())
    {
        std::cerr << "Failed to open input file: " << inputPath << "\n";
        return 1;
    }

    std::filesystem::create_directories(outDir);

    ANTLRInputStream input(in);
    BaseLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    BaseParser parser(&tokens);
    auto *tree = parser.program();

    ASTBuilder builder;
    auto programAny = builder.visitProgram(tree);
    auto program = std::any_cast<ProgramNodePtr>(programAny);

    std::ofstream dotOut(dotPath);
    if (!dotOut.good())
    {
        std::cerr << "Failed to open DOT output file: " << dotPath << "\n";
        return 1;
    }

    DotEmitter emitter(dotOut);
    emitter.emit(program);
    dotOut.close();

    if (runDot(dotPath, svgPath) != 0)
    {
        std::cerr << "Graphviz failed while rendering: " << svgPath << "\n";
        return 1;
    }

    std::cout << "AST DOT: " << dotPath << "\n";
    std::cout << "AST SVG: " << svgPath << "\n";
    return 0;
}
