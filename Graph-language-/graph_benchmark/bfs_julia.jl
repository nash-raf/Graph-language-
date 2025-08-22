#!/usr/bin/env julia
using Graphs

function load_graph(path::String)
    lines = readlines(path)
    # Parse each line "u v" into a Tuple{Int,Int}
    edges = [tuple(parse(Int, a), parse(Int, b)) for line in lines for (a,b) in [split(line)]]
    n = maximum(vcat(first.(edges), last.(edges))) + 1
    g = SimpleGraph(n)
    for (u,v) in edges
        add_edge!(g, u+1, v+1)  # convert 0-based → 1-based
    end
    return g
end

function run_bfs_parents(path::String, src0::Int=0)
    g = load_graph(path)
    src = src0 + 1
    parents = bfs_parents(g, src; dir=:out)
    return parents .- 1  # convert back to 0-based indices
end

# === main ===
parents = run_bfs_parents("g.txt", 0)
# println(parents)
