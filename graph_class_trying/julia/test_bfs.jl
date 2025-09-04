using Graphs

function bfs_disconnected(file_path::String)
    # Read edges
    edges = Tuple{Int,Int}[]
    max_vertex = -1
    for line in eachline(file_path)
        line = strip(line)
        if isempty(line) || startswith(line, "#")
            continue
        end
        u, v = parse.(Int, split(line))
        push!(edges, (u+1, v+1))  # Graphs.jl uses 1-based indexing
        max_vertex = max(max_vertex, u, v)
    end

    # Create undirected graph
    g = SimpleGraph(max_vertex + 1)
    for (u, v) in edges
        add_edge!(g, u, v)
    end

    visited = falses(nv(g))

    function bfs_from(start)
        queue = [start]
        visited[start] = true
        order = Int[]
        while !isempty(queue)
            u = popfirst!(queue)
            push!(order, u)
            for v in neighbors(g, u)
                if !visited[v]
                    visited[v] = true
                    push!(queue, v)
                end
            end
        end
        return order
    end

    total_order = Int[]
    for v in 1:nv(g)
        if !visited[v]
            append!(total_order, bfs_from(v))
        end
    end

    # Convert back to 0-based indexing
    println("BFS vertex order (covers disconnected components): ", total_order .- 1)
end

# Run from command line
if length(ARGS) < 1
    println("Usage: julia test_bfs.jl <edge_list_file>")
else
    bfs_disconnected(ARGS[1])
end
