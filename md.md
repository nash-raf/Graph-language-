k-Clique Listing: Full DSL Implementation Plan

Target DSL Code

The goal is to be able to write something like this:

graph G { nodes: 0,1,2,3,4;
edges: 0->1, 1->2, 0->2, 2->3, 3->4, 2->4; };

int k = 3;

fn void ListClique(graph Gi, set R, int l) {
    if (l == 2) {
        for each edge u, v in Gi {
            set clique = R union {u};
            clique.add(v);
            print "clique found";
        }
    } else {
        for each vertex v in Gi {
            set nbrs = neighbors(Gi, v);
            graph Gv = subgraph(Gi, nbrs);
            set Rnew = R union {v};
            ListClique(Gv, Rnew, l - 1);
        }
    }
}

ListClique(G, {}, k);

Current State Analysis

flowchart LR
    subgraph works [Working Features]
        A[Set union/intersect]
        B[for each vertex v in G]
        C[for each neighbor u of v in G]
        D[Recursive functions]
        E[Set add/remove/contains/size]
        F[Dynamic arrays]
        G_node[numVertices/degree/hasEdge]
    end
    subgraph missing [Missing Features]
        H["set as function param type"]
        I["for each edge u,v in G"]
        J["graph as runtime value"]
        K["neighbors(G,v) built-in"]
        L["subgraph(G,set) built-in"]
        M["Set literal as fn argument"]
        N["Graph decl inside function"]
    end

Phase 1: set as Function Parameter Type

Problem: The grammar type rule lists graph | vertex | edge | int | string | real | bool but not set. The TypeKind::Set and LLVM mapping already exist.

Files to change:





p2GraphEasy/Base.g4 (lines 170-177): Add 'set' to both type and returnType rules

type: 'graph' | 'vertex' | 'edge' | 'int' | 'string' | 'real' | 'bool' | 'set';
returnType: 'graph' | 'vertex' | 'edge' | 'int' | 'real' | 'bool' | 'void' | 'string' | 'set';





p2GraphEasy/SemanticAnalyzer.cpp (line ~50): Add "set" mapping in typeFromString()

if (typeName == "set") return TypeKind::Set;





p2GraphEasy/IRGenVisitor.cpp (line ~190): Add to getLLVMTypeForName()

if (typeName == "set") return llvm::PointerType::get(Context, 0);





getLLVMTypeFromTypeKind already handles TypeKind::Set (returns opaque pointer) -- no change needed.

After this, fn void ListClique(graph G, set R, int l) will compile.

Phase 2: Fix for each edge u, v in G

Problem: Current edge iteration loops i = 0..n (same as vertex), never extracting edge pairs. Needs a nested loop over CSR.

File: p2GraphEasy/IRGenVisitor.cpp visitForEach() (~line 1530)

Implementation: Generate a nested loop:





Outer: for u = 0 to n-1



Inner: for j = row_ptr[u] to row_ptr[u+1]-1, set v = col_idx[j]



For directed graphs (DAG): emit all edges



For undirected: only emit u < v to avoid duplicates



Store u into var1 alloca, v into var2 alloca

This requires loading row_ptr and col_idx from the graph struct (fields 2 and 3), similar to the neighbor iteration that was just fixed.

Phase 3: neighbors(G, v) Built-in

Problem: Need to extract the neighbor set of a vertex as a roaring bitmap at runtime.

Files to change:





p2GraphEasy/SemanticAnalyzer.cpp: Add built-in neighbors(G, v) returning TypeKind::Set



p2GraphEasy/IRGenVisitor.cpp: Add IR generation in the FunctionCall handler

IR Generation logic:





Look up graph in GraphMap, load row_ptr and col_idx



Compute start = row_ptr[v], end = row_ptr[v+1]



Call roaring_bitmap_create() to make a new bitmap



Loop from start to end, call roaring_bitmap_add(bm, col_idx[i]) for each neighbor



Return the bitmap pointer

Alternative (faster): Add a C runtime function graph_neighbors(GraphStruct*, int32_t v) in p2GraphEasy/roaring_bitmap.cpp that does the same thing in C, then just emit a single call from the IR. This is cleaner and faster.

Phase 4: Runtime Graph Construction -- subgraph(G, vertexSet)

This is the biggest missing piece. Currently graphs are only created statically at compile time (CSR is built in the GraphDeclNode constructor). The algorithm needs to create induced subgraphs dynamically.

New runtime C function in p2GraphEasy/roaring_bitmap.cpp (or a new graph_runtime.cpp):

extern "C" void* graph_induced_subgraph(void* graphPtr, void* vertexSetBitmap);

This function:





Takes the original graph's CSR and a roaring bitmap of vertex IDs



Iterates over the vertex set



For each vertex, iterates over its neighbors in the original CSR



If the neighbor is also in the vertex set, adds the edge to the new graph



Builds a new CSR structure (malloc'd) and returns it as a GraphTy*

Files to change:





New file or addition to roaring_bitmap.cpp: graph_induced_subgraph() C function



p2GraphEasy/SemanticAnalyzer.cpp: Add subgraph(G, S) returning TypeKind::Graph



p2GraphEasy/IRGenVisitor.cpp: Add IR generation that calls the runtime function and registers the result in GraphMap or returns it as a pointer value

Key design decision: The returned graph needs the same GraphTy struct layout {i64 n, i64 m, i64* row_ptr, i32* col_idx, i32* weights}. The runtime function mallocs everything and returns a pointer.

Phase 5: Graph Variables (not just declarations)

Problem: Currently graphs only exist via graph G { ... }; declarations and are stored in GraphMap. To support graph Gv = subgraph(Gi, nbrs); inside a function, we need graphs as regular values (pointers to GraphTy).

Changes needed:





p2GraphEasy/ASTBuilder.cpp: When a varDecl has type graph, treat the initializer as an expression that returns a graph pointer



p2GraphEasy/IRGenVisitor.cpp: 





In visitVarDecl: if type is graph, allocate a GraphTy* alloca and store the expression result



In visitExpr for Variable: if the variable is a graph, load the GraphTy*



In visitForEach and built-in functions: look up graphs in NamedValues (as GraphTy* pointers) in addition to GraphMap



p2GraphEasy/SemanticAnalyzer.cpp: Allow graph type variables with expression initializers

This unifies the two lookup paths (static GraphMap vs dynamic NamedValues) -- all graph accesses should check both.

Phase 6: Passing Set Literals as Function Arguments

Problem: ListClique(G, {}, k) passes an empty set literal {} as an argument. The grammar for argumentList uses expr, and setInitializer is already in the expr rule as SetLitExpr. This should already parse. Need to verify the IR generation handles it (creating a temporary bitmap for the literal and passing its pointer).

This likely already works since SetLiteral is handled in visitExpr (creates a runtime bitmap). Just needs testing.

Phase 7 (Optional): Graph Orientation / Ordering

For the full algorithm, you also need:





degreeOrder(G) or degeneracyOrder(G): Returns an int[] array mapping vertex -> rank. Could be a C runtime function.



orient(G, order): Creates a DAG where edges point from lower-rank to higher-rank. Also a C runtime function.

These are optional since you could hard-code a DAG in the test case initially, then add these built-ins later.

Summary of All Files to Modify







File



Changes





Base.g4: Add set to type and returnType rules



SemanticAnalyzer.cpp: Add set to typeFromString(), add neighbors() and subgraph() built-ins



IRGenVisitor.cpp: Add set to getLLVMTypeForName(), fix edge iteration, add neighbors() and subgraph() IR gen, support graph variables in NamedValues, unify graph lookups



roaring_bitmap.cpp (or new graph_runtime.cpp): Add graph_neighbors() and graph_induced_subgraph() C runtime functions



ASTBuilder.cpp: Handle graph typed variable declarations with expression initializers

Implementation Order

Phases 1-2 are independent and straightforward. Phase 3-5 build on each other. Phase 6 may already work. Phase 7 is optional for initial testing.